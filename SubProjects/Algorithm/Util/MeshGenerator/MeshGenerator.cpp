#include "MeshGenerator.h"
#include <iostream>
#include <boost/format.hpp>
#include <Util/MeshGenerator/MeshCompacter.h>
using namespace std;

bool MeshGenerator::execute(
	vector<glm::vec3>& points,
	vector<std::vector<int>>& triangles,
	int nx,
	int ny,
	const function<glm::vec3(int, int)>& f
) {
	try {
		if (nx < 2 || ny < 2) {
			throw runtime_error((boost::format("not enought nx(=%d), ny(=%d). assert(nx, ny >= 2)") % nx %ny).str().c_str());
		}

		auto c = [nx, ny](int ix, int iy)->int {
			return ix + nx*iy;
		};

		points.clear();
		points.reserve(nx*ny);
		triangles.clear();
		triangles.reserve(2 * (nx - 1)*(ny - 1));
		for (int iy = 0; iy < ny; iy++) {
			for (int ix = 0; ix < nx; ix++) {
				points.push_back(f(ix, iy));
			}
		}
		for (int ix = 0; ix < nx - 1; ix++) {
			for (int iy = 0; iy < ny - 1; iy++) {
				vector<int> temp(3);
				temp[0] = c(ix, iy);
				temp[1] = c(ix + 1, iy);
				temp[2] = c(ix + 1, iy + 1);
				triangles.push_back(temp);
				temp[0] = c(ix, iy);
				temp[1] = c(ix + 1, iy + 1);
				temp[2] = c(ix, iy + 1);
				triangles.push_back(temp);
			}
		}

		return true;
	} catch (runtime_error& e) {
		cout << "MeshGenerator::execute: " << e.what() << endl;
		points.clear();
		triangles.clear();
		return false;
	}
}

bool MeshGenerator::execute2(
	vector<glm::vec3>& points,
	vector<std::vector<int>>& triangles,
	int nx,
	int ny,
	const std::function<void(glm::vec3&/*return*/, bool&/*valid*/, int/*nx*/, int/*ny*/)>& f
) {
	try {
		if (nx < 2 || ny < 2) {
			throw runtime_error((boost::format("not enought nx(=%d), ny(=%d). assert(nx, ny >= 2)") % nx %ny).str().c_str());
		}

		auto c = [nx, ny](int ix, int iy)->int {
			return ix + nx*iy;
		};
		vector<int> valids;
		points.clear();
		points.reserve(nx*ny);
		triangles.clear();
		triangles.reserve(2 * (nx - 1)*(ny - 1));
		for (int iy = 0; iy < ny; iy++) {
			for (int ix = 0; ix < nx; ix++) {
				glm::vec3 p;
				bool valid;
				f(p, valid, ix, iy);
				points.push_back(p);
				valids.push_back(valid);
			}
		}
		for (int ix = 0; ix < nx - 1; ix++) {
			for (int iy = 0; iy < ny - 1; iy++) {
				if (valids[c(ix, iy)] &&
					valids[c(ix + 1, iy)] &&
					valids[c(ix, iy + 1)] &&
					valids[c(ix + 1, iy + 1)]
					) {
					vector<int> temp(3);
					temp[0] = c(ix, iy);
					temp[1] = c(ix + 1, iy);
					temp[2] = c(ix + 1, iy + 1);
					triangles.push_back(temp);
					temp[0] = c(ix, iy);
					temp[1] = c(ix + 1, iy + 1);
					temp[2] = c(ix, iy + 1);
					triangles.push_back(temp);
				}
			}
		}
		MeshCompacter::executeRemoveUnusedVertex(points, triangles, function<int(int)>());

		return true;
	} catch (runtime_error& e) {
		cout << "MeshGenerator::execute: " << e.what() << endl;
		points.clear();
		triangles.clear();
		return false;
	}
}

//bool MeshGenerator::execute(
//	vector<glm::vec3>& points,
//	vector<std::vector<int>>& triangles,
//	int nx,
//	int ny,
//	const function<glm::vec3 (int, int)>& f,
//	const function<bool (int, int)>& exist
//)
//{
//	try{
//		if(nx < 2 || ny < 2){
//			throw exception((boost::format("not enought nx(=%d), ny(=%d). assert(nx, ny >= 2)")%nx %ny).str().c_str());
//		}
//
//	} catch (exception& e) {
//		cout << "MeshGenerator::execute: " << e.what() << endl;
//		points.clear();
//		triangles.clear();
//		return false;
//	}
//}
