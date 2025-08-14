#include "CylinderMeshGenerator.h"
#include <exception>
//#include "../../NormalEstimationWrapper/KnnWrapper.h"
#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include <boost/algorithm/clamp.hpp>
#include <Util/Core/coutOverloading.h>
#include "../PointProcessing/XFormMaker.h"
#include "../KnnWrapper/KnnWrapper.h"
#include "MeshGenerator.h"
using namespace std;
bool CylinderMeshGenerator::_executeKnn(
	vector<glm::vec3>& pointsOut,
	vector<vector<int>>& trianglesOut,
	const vector<glm::vec3>& points,
	const glm::vec3& center,
	const glm::vec3& zAxis,
	const int nx,
	const int ny) {
	try {
		if (points.empty()) {
			throw runtime_error("point is empty");
		}
		geometry::CylinderSystem cs(points, center, zAxis);
		cout << "[debug] cs.angle=(" << cs.getMinAngle() << ", " << cs.getMaxAngle() << ")" << endl;

		// make depthMap with knn (with distance based normalization)
			// make knnPolars
		vector<glm::vec3> knnPolars;
		knnPolars.reserve(points.size());
		float radiusMean = 0;
		for (const auto& p : points) {
			radiusMean += cs.pointToCylinder(p)[0];
		}
		radiusMean /= points.size();
		auto lambda_normalize = [radiusMean](float& th, float& z) {
			th = th*radiusMean;
			z = z;
		};
		for (const auto& p : points) {
			glm::vec3 temp = cs.pointToCylinder(p);
			temp[0] = 0;
			lambda_normalize(temp[1], temp[2]);
			knnPolars.push_back(temp);
		}

		//make depthMap with knn
		auto l_less = [](const glm::vec3& v1, const glm::vec3& v2) {
			return v1[1] < v2[1] && v1[2] < v2[2];
		};
		auto l_lessEq = [](const glm::vec3& v1, const glm::vec3& v2) {
			return v1[1] <= v2[1] && v1[2] <= v2[2];
		};

		auto l_getVal = [](const vector<float>& rVals)->float {
			if (rVals.empty()) return 0;
			float cnt = 0;
			float res = 0;
			float maxVal = *std::max_element(rVals.begin(), rVals.end());
			for (float val : rVals) {
				if (val > maxVal*0.9) {
					cnt += 1;
					res += val;
				}
			}
			return res / cnt;
		};
		vector<glm::vec3> polars;
		polars.reserve(points.size());
		for (const auto& p : points) {
			polars.push_back(cs.pointToCylinder(p));
		}

		float length;
		glm::vec3 center;
		XFormMaker::getMinMaxLengthAndCenter(length, center, points);
		KnnWrapper knn(knnPolars);
		//int knear = ceil( (float)points.size()/(nx*ny/2) );
		int knear = 28;
		cout << "knear per cylinder pixel=" << knear << endl;
		float tol = 0.03*length / radiusMean; tol = tol * tol;
		cv::Mat depthMap = cv::Mat::zeros(nx, ny, CV_32F);
		for (int ith = 0; ith < nx; ith++) {
			if (ith % (int)ceil(nx / 10.0) == 0) cout << ".";
			for (int iz = 0; iz < ny; iz++) {
				vector<int> knearIdx;
				vector<float> dist;
				glm::vec3 polar = glm::vec3(0,
					(cs.getMaxAngle() - cs.getMinAngle()) * ith / (nx)+cs.getMinAngle(),
					(cs.getMaxZ() - cs.getMinZ()) * iz / (ny)+cs.getMinZ()
				);
				lambda_normalize(polar[1], polar[2]);

				// knn
				knn.knnSearch(knearIdx, dist, polar, knear);
				float mindist = *std::min_element(dist.begin(), dist.end());
				if (mindist >= tol) {
					depthMap.at<float>(ith, iz) = 0;
				} else {
					vector<float> resVec;
					glm::vec3 polarLow = glm::vec3(0,
						(cs.getMaxAngle() - cs.getMinAngle())*ith / (nx)+cs.getMinAngle(),
						(cs.getMaxZ() - cs.getMinZ())*iz / (ny)+cs.getMinZ()
					);
					glm::vec3 polarHigh = glm::vec3(0,
						(cs.getMaxAngle() - cs.getMinAngle())*(ith + 1) / (nx)+cs.getMinAngle(),
						(cs.getMaxZ() - cs.getMinZ())*(iz + 1) / (ny)+cs.getMinZ()
					);
					for (int i = 0; i < knearIdx.size(); i++) {
						if (dist[i] < tol) {
							const auto& p = polars[knearIdx[i]];
							if (l_lessEq(polarLow, p) && l_less(p, polarHigh))
								resVec.push_back(p[0]);
						}
					}
					depthMap.at<float>(ith, iz) = l_getVal(resVec);
				}
			}
		}
		cout << endl;

		// generate mesh
		auto f = [&depthMap, &cs, nx, ny](int ith, int iz)->glm::vec3 {
			float r = depthMap.at<float>(ith, iz);
			float th = (cs.getMaxAngle() - cs.getMinAngle()) * ith / (nx)+cs.getMinAngle();
			return glm::vec3(
				r * cos(th),
				r * sin(th),
				(cs.getMaxZ() - cs.getMinZ()) * iz / (ny)+cs.getMinZ()
			);
		};
		MeshGenerator::execute(pointsOut, trianglesOut, nx, ny, f);
		return true;
	} catch (runtime_error& e) {
		cout << "CylinderMeshGenerator::execute: " << e.what() << endl;
		return false;
	}
}

bool CylinderMeshGenerator::_executeGrid(
	vector<glm::vec3>& pointsOut,
	vector<vector<int>>& trianglesOut,
	const vector<glm::vec3>& points,
	const glm::vec3& center,
	const glm::vec3& zAxis,
	const int nx,
	const int ny) {
	try {
		if (points.empty()) {
			throw runtime_error("point is empty");
		}
		geometry::CylinderSystem cs(points, center, zAxis);

		// make polars
		vector<glm::vec3> polars;
		polars.reserve(points.size());
		for (const auto& p : points) {
			polars.push_back(cs.pointToCylinder(p));
		}

		// make depthMap with only grid
		cv::Mat depthMap = cv::Mat::zeros(nx, ny, CV_32F);
		auto l_less = [](const glm::vec3& v1, const glm::vec3& v2) {
			return v1[1] < v2[1] && v1[2] < v2[2];
		};
		auto l_lessEq = [](const glm::vec3& v1, const glm::vec3& v2) {
			return v1[1] <= v2[1] && v1[2] <= v2[2];
		};
		auto l_getVal = [](const vector<float>& rVals)->float {
			if (rVals.empty()) return 0;
			float cnt = 0;
			float res = 0;
			float maxVal = *std::max_element(rVals.begin(), rVals.end());
			for (float val : rVals) {
				if (val > maxVal*0.9) {
					cnt += 1;
					res += val;
				}
			}
			return res / cnt;
		};
		for (int ith = 0; ith < nx; ith++) {
			if (ith % (int)ceil(nx / 10.0) == 0) cout << ".";
			for (int iz = 0; iz < ny; iz++) {
				glm::vec3 polarLow = glm::vec3(0,
					(cs.getMaxAngle() - cs.getMinAngle())*ith / (nx)+cs.getMinAngle(),
					(cs.getMaxZ() - cs.getMinZ())*iz / (ny)+cs.getMinZ()
				);
				glm::vec3 polarHigh = glm::vec3(0,
					(cs.getMaxAngle() - cs.getMinAngle())*(ith + 1) / (nx)+cs.getMinAngle(),
					(cs.getMaxZ() - cs.getMinZ())*(iz + 1) / (ny)+cs.getMinZ()
				);
				vector<float> rVals;
				for (const auto& p : polars) {
					if (l_lessEq(polarLow, p) && l_less(p, polarHigh)) {
						rVals.push_back(p[0]);
					}
				}
				depthMap.at<float>(ith, iz) = l_getVal(rVals);
			}
		}
		cout << endl;

		auto f = [&depthMap, &cs, nx, ny](int ith, int iz)->glm::vec3 {
			float r = depthMap.at<float>(ith, iz);
			float th = (cs.getMaxAngle() - cs.getMinAngle()) * ith / (nx)+cs.getMinAngle();
			return glm::vec3(
				r * cos(th),
				r * sin(th),
				(cs.getMaxZ() - cs.getMinZ()) * iz / (ny)+cs.getMinZ()
			);
		};
		MeshGenerator::execute(pointsOut, trianglesOut, nx, ny, f);
		return true;
	} catch (runtime_error& e) {
		cout << "CylinderMeshGenerator::executeGrid: " << e.what() << endl;
		return false;
	}
}

bool CylinderMeshGenerator::execute(
	vector<glm::vec3>& pointsOut,
	vector<vector<int>>& trianglesOut,
	const vector<glm::vec3>& points,
	const glm::vec3& center,
	const glm::vec3& zAxis,
	const int nx,
	const int ny) {
	return CylinderMeshGenerator::_executePointIteration(pointsOut, trianglesOut, points, center, zAxis, nx, ny);
}

bool CylinderMeshGenerator::_executePointIteration(
	vector<glm::vec3>& pointsOut,
	vector<vector<int>>& trianglesOut,
	const vector<glm::vec3>& points,
	const glm::vec3& center,
	const glm::vec3& zAxis,
	const int nx,
	const int ny) {
	try {
		if (points.empty()) {
			throw runtime_error("point is empty");
		}
		geometry::CylinderSystem cs(points, center, zAxis);

		// make polars
		vector<glm::vec3> polars;
		polars.reserve(points.size());
		for (const auto& p : points) {
			polars.push_back(cs.pointToCylinder(p));
		}
		// make depthMap with only grid
		cv::Mat depthMap = cv::Mat::zeros(nx, ny, CV_32F);
		auto l_getVal = [](const vector<float>& rVals)->float {
			if (rVals.empty()) return 0;
			float cnt = 0;
			float res = 0;
			float maxVal = *std::max_element(rVals.begin(), rVals.end());
			for (float val : rVals) {
				if (val > maxVal*0.9) {
					cnt += 1;
					res += val;
				}
			}
			return res / cnt;
		};
		auto l_polar2idx = [nx, ny, &cs](int& ith, int& iz, const glm::vec3& polar) {
			ith = boost::algorithm::clamp((polar[1] - cs.getMinAngle()) / (cs.getMaxAngle() - cs.getMinAngle())*nx, 0, nx - 1);
			iz = boost::algorithm::clamp((polar[2] - cs.getMinZ()) / (cs.getMaxZ() - cs.getMinZ())*ny, 0, ny - 1);
		};

		typedef Eigen::Matrix<vector<float>, Eigen::Dynamic, Eigen::Dynamic> Matv;
		Eigen::MatrixXi validMap = Eigen::MatrixXi::Zero(nx, ny);
		Matv depthMapVec(nx, ny);
		for (const auto& polar : polars) {
			int ith;
			int iz;
			l_polar2idx(ith, iz, polar);
			if (ith < nx && iz < ny) {
				depthMapVec(ith, iz).push_back(polar[0]);
			} else {
				cout << ith << " " << iz << endl;
			}
		}

		for (int ith = 0; ith < nx; ith++) {
			for (int iz = 0; iz < ny; iz++) {
				vector<float>& pixels = depthMapVec(ith, iz);
				if (pixels.empty()) {
					depthMap.at<float>(ith, iz) = 0;
					validMap(ith, iz) = 0;
				} else {
					depthMap.at<float>(ith, iz) = l_getVal(pixels);
					validMap(ith, iz) = 1;
				}
			}
		}

		auto f2 = [&depthMap, &validMap, &cs, nx, ny](glm::vec3& p, bool& valid, int ith, int iz) {
			float r = depthMap.at<float>(ith, iz);
			float th = (cs.getMaxAngle() - cs.getMinAngle()) * ith / (nx)+cs.getMinAngle();
			p = cs.cylinderToPoint(glm::vec3(r, th, (cs.getMaxZ() - cs.getMinZ()) * iz / (ny)+cs.getMinZ()));
			valid = validMap(ith, iz);
		};
		MeshGenerator::execute2(pointsOut, trianglesOut, nx, ny, f2);

		return true;
	} catch (runtime_error& e) {
		cout << "CylinderMeshGenerator::executeGridPointsIteration: " << e.what() << endl;
		return false;
	}
}
