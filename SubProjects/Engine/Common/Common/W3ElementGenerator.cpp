#include "W3ElementGenerator.h"

#include <qmath.h>
#include <iostream>

using std::runtime_error;
using std::cout;
using std::endl;
using glm::vec3;

/////////////////////////////////////////////////////////////////////////////////////
// public functions
/////////////////////////////////////////////////////////////////////////////////////

class Vec3Less {
public:
	bool operator()(const glm::vec3& v0, const glm::vec3& v1) const {
		return v0.x < v1.x ||
			(v0.x == v1.x && v0.y < v1.y) ||
			(v0.x == v1.x && v0.y == v1.y && v0.z < v1.z);
	}
}; 

void CW3ElementGenerator::GenerateTMJCube(const std::vector<glm::vec3>& inVert, std::vector<glm::vec3>& outNorm,
									   std::vector<unsigned int>& outIndices, bool isClockwise) {

	std::vector<std::vector<unsigned int>> tblVertToFace;
	tblVertToFace.resize(inVert.size());

	pushTriIndex(0, 1, 3, tblVertToFace, outIndices, isClockwise);
	pushTriIndex(2, 3, 1, tblVertToFace, outIndices, isClockwise);
	pushTriIndex(0, 3, 4, tblVertToFace, outIndices, isClockwise);
	pushTriIndex(3, 7, 4, tblVertToFace, outIndices, isClockwise);
	pushTriIndex(6, 5, 4, tblVertToFace, outIndices, isClockwise);
	pushTriIndex(4, 7, 6, tblVertToFace, outIndices, isClockwise);
	pushTriIndex(2, 1, 5, tblVertToFace, outIndices, isClockwise);
	pushTriIndex(5, 6, 2, tblVertToFace, outIndices, isClockwise);
	pushTriIndex(1, 0, 5, tblVertToFace, outIndices, isClockwise);
	pushTriIndex(0, 4, 5, tblVertToFace, outIndices, isClockwise);
	pushTriIndex(3, 2, 6, tblVertToFace, outIndices, isClockwise);
	pushTriIndex(6, 7, 3, tblVertToFace, outIndices, isClockwise);

	CalculateSmoothNormal(tblVertToFace, inVert, outNorm);
}
void CW3ElementGenerator::GenerateCube(const glm::vec3 & center,
									   const glm::vec3 & x_vector, const glm::vec3 & y_vector, const glm::vec3 & z_vector,
									   float width, float height, float depth,
									   std::vector<glm::vec3>& outVert, std::vector<glm::vec3>& outNorm,
									   std::vector<unsigned int>& outIndices, bool isClockwise) {
	glm::vec3 x_offset = x_vector * 0.5f * width;
	glm::vec3 y_offset = y_vector * 0.5f * height;
	glm::vec3 z_offset = z_vector * 0.5f * depth; 

	outVert.push_back(center + x_offset + y_offset + z_offset);
	outVert.push_back(center + x_offset + y_offset - z_offset);
	outVert.push_back(center - x_offset + y_offset - z_offset);
	outVert.push_back(center - x_offset + y_offset + z_offset);
	outVert.push_back(center + x_offset - y_offset + z_offset);
	outVert.push_back(center + x_offset - y_offset - z_offset);
	outVert.push_back(center - x_offset - y_offset - z_offset);
	outVert.push_back(center - x_offset - y_offset + z_offset);

	std::vector<std::vector<unsigned int>> tblVertToFace;
	tblVertToFace.resize(outVert.size());

	pushTriIndex(0, 1, 3, tblVertToFace, outIndices, isClockwise);
	pushTriIndex(2, 3, 1, tblVertToFace, outIndices, isClockwise);
	pushTriIndex(0, 3, 4, tblVertToFace, outIndices, isClockwise);
	pushTriIndex(3, 7, 4, tblVertToFace, outIndices, isClockwise);
	pushTriIndex(6, 5, 4, tblVertToFace, outIndices, isClockwise);
	pushTriIndex(4, 7, 6, tblVertToFace, outIndices, isClockwise);
	pushTriIndex(2, 1, 5, tblVertToFace, outIndices, isClockwise);
	pushTriIndex(5, 6, 2, tblVertToFace, outIndices, isClockwise);
	pushTriIndex(1, 0, 5, tblVertToFace, outIndices, isClockwise);
	pushTriIndex(0, 4, 5, tblVertToFace, outIndices, isClockwise);
	pushTriIndex(3, 2, 6, tblVertToFace, outIndices, isClockwise);
	pushTriIndex(6, 7, 3, tblVertToFace, outIndices, isClockwise);

	CalculateSmoothNormal(tblVertToFace, outVert, outNorm);
}

void CW3ElementGenerator::GenerateSmoothNormals(const std::vector<glm::vec3>& vertices,
												const std::vector<unsigned int>& indices,
												std::vector<glm::vec3>& out_normals, bool isClockwise) {
	std::vector<std::vector<unsigned int>> tblVertToFace;
	tblVertToFace.resize(vertices.size());

	for (int i = 0; i < indices.size(); i += 3) {
		CW3ElementGenerator::pushTriIndex(indices[i], indices[i + 1], indices[i + 2],
										  tblVertToFace, isClockwise);
	}

	CW3ElementGenerator::CalculateSmoothNormal(tblVertToFace, vertices, out_normals);
}

void CW3ElementGenerator::generateSectorFace(glm::vec3 origin, const std::vector<glm::vec3>& arch,
											 std::vector<glm::vec3>& outVert, std::vector<glm::vec3>& outNorm, std::vector<unsigned int>& outIndices, bool isClockwise) {
	std::vector<vec3> vert, norm;
	std::vector<unsigned int> indices;

	vert.push_back(origin);
	vert.insert(vert.end(), arch.begin(), arch.end());

	std::vector<std::vector<unsigned int>> tblVertToFace;
	tblVertToFace.resize(vert.size());

	for (int i = 1; i < (int)vert.size() - 1; i++) {
		pushTriIndex(0, i, i + 1, tblVertToFace, indices, isClockwise);
	}

	CalculateSmoothNormal(tblVertToFace, vert, norm);

	int outVertSize = outVert.size();
	if (outVertSize != 0) {
		for (auto &i : indices)
			i = i + outVertSize;
	}

	outVert.insert(outVert.end(), vert.begin(), vert.end());
	outNorm.insert(outNorm.end(), norm.begin(), norm.end());
	outIndices.insert(outIndices.end(), indices.begin(), indices.end());
}

void CW3ElementGenerator::generateRectFace(const std::vector<glm::vec3>& line1, const std::vector<glm::vec3>& line2,
										   std::vector<glm::vec3>& outVert, std::vector<glm::vec3>& outNorm, std::vector<unsigned int>& outIndices,
										   bool isClockwise) {
	try {
		//equalize count of point.
		auto equlPointCount = [](int pointCnt, std::vector<vec3>& points) {
			float length = 0;
			for (int i = 0; i < (int)points.size() - 1; i++)
				length += glm::length(points[i + 1] - points[i]);

			float step = length / pointCnt;

			std::vector<glm::vec3> eline;
			eline.push_back(points.front());

			for (int i = 1; i < points.size(); i++) {
				vec3 v = points[i] - eline.back();

				float sampleCnt = (glm::length(v) / step);

				for (int m = 0; m < (int)sampleCnt - 1; m++)
					eline.push_back(v / sampleCnt + eline.back());
			}
			eline.pop_back();
			eline.push_back(points.back());
			points.assign(eline.begin(), eline.end());
			points.resize(pointCnt, eline.back());
		};

		std::vector<glm::vec3> uline, lline;
		uline.assign(line1.begin(), line1.end());
		lline.assign(line2.begin(), line2.end());

		if (uline.size() > lline.size())
			equlPointCount(uline.size(), lline);
		else if (uline.size() < lline.size())
			equlPointCount(lline.size(), uline);

		if (lline.size() != uline.size()) {
			assert(false);
			throw runtime_error("upper point is not equal to lower point");
		}
		//equlize end

		std::vector<vec3> vert, norm;
		std::vector<unsigned int> indices;

		vert.assign(uline.begin(), uline.end());
		vert.insert(vert.end(), lline.begin(), lline.end());

		std::vector<std::vector<unsigned int>> tblVertToFace;
		tblVertToFace.resize(vert.size());

		int lineSize = uline.size();

		for (int i = 0; i < lineSize - 1; i++) {
			int index1, index2, index3;

			index1 = i;
			index2 = lineSize + i;
			index3 = i + 1;

			pushTriIndex(index1, index2, index3, tblVertToFace, indices, isClockwise);

			index1 = lineSize + i;
			index2 = lineSize + i + 1;
			index3 = i + 1;

			pushTriIndex(index1, index2, index3, tblVertToFace, indices, isClockwise);
		}

		CalculateSmoothNormal(tblVertToFace, vert, norm);

		int outVertSize = outVert.size();
		if (outVertSize != 0) {
			for (auto &i : indices)
				i = i + outVertSize;
		}

		outVert.insert(outVert.end(), vert.begin(), vert.end());
		outNorm.insert(outNorm.end(), norm.begin(), norm.end());
		outIndices.insert(outIndices.end(), indices.begin(), indices.end());
	} catch (runtime_error& e) {
		cout << "CW3ElementGenerator::generateRectFace: " << e.what() << endl;
	}
}

void CW3ElementGenerator::generateCircleFace(glm::vec3 origin, float radius, int numOfsides,
											 std::vector<glm::vec3>& outVert, std::vector<glm::vec3>& outNorm, std::vector<unsigned int>& outIndices, bool isClockwise) {
	std::vector<glm::vec3> side;
	side.resize(numOfsides + 1);
	float doublePI = 2.0f * M_PI;

	for (int i = 0; i < side.size(); i++) {
		glm::vec3 vert;
		vert.x = origin.x + (radius * cos(static_cast<float>(i)* doublePI / numOfsides));
		vert.y = origin.y + (radius * sin(static_cast<float>(i)* doublePI / numOfsides));
		vert.z = origin.z;

		side[i] = vert;
	}
	generateSectorFace(origin, side, outVert, outNorm, outIndices, isClockwise);
}

/////////////////////////////////////////////////////////////////////////////////////
// private functions
/////////////////////////////////////////////////////////////////////////////////////

void CW3ElementGenerator::pushTriIndex(int i1, int i2, int i3, std::vector<std::vector<unsigned int>>& table, bool isClockwise) {
	std::vector<unsigned int> tri;

	if (!isClockwise) {
		tri.push_back(i1);
		tri.push_back(i2);
		tri.push_back(i3);
	} else {
		tri.push_back(i1);
		tri.push_back(i3);
		tri.push_back(i2);
	}

	table[i1].insert(table[i1].end(), tri.begin(), tri.end());
	table[i2].insert(table[i2].end(), tri.begin(), tri.end());
	table[i3].insert(table[i3].end(), tri.begin(), tri.end());
};

void CW3ElementGenerator::pushTriIndex(int i1, int i2, int i3, std::vector<std::vector<unsigned int>>& table,
									   std::vector<unsigned int>& vec, bool isClockwise) {
	std::vector<unsigned int> tri;

	if (!isClockwise) {
		tri.push_back(i1);
		tri.push_back(i2);
		tri.push_back(i3);
	} else {
		tri.push_back(i1);
		tri.push_back(i3);
		tri.push_back(i2);
	}

	table[i1].insert(table[i1].end(), tri.begin(), tri.end());
	table[i2].insert(table[i2].end(), tri.begin(), tri.end());
	table[i3].insert(table[i3].end(), tri.begin(), tri.end());

	vec.insert(vec.end(), tri.begin(), tri.end());
};

void CW3ElementGenerator::CalculateSmoothNormal(const std::vector<std::vector<unsigned int>>& table,
												const std::vector<glm::vec3> vertexs, std::vector<glm::vec3>& normals) {
	normals.clear();
	normals.reserve(table.size());
	//normal
	for (int j = 0; j < table.size(); j++) {
		int cntNormal = 0;
		vec3 normal = vec3(0.0f);
		for (int i = 0; i < table[j].size(); i += 3) {
			vec3 v1, v2, v3;

			v1 = vertexs[table[j].at(i)];
			v2 = vertexs[table[j].at(i + 1)];
			v3 = vertexs[table[j].at(i + 2)];

			++cntNormal;

			if ((v1 != v2) ||
				(v1 != v3) ||
				(v2 != v3))
				normal += glm::normalize(glm::cross(v2 - v1, v3 - v1));
		}
		if (cntNormal != 0)
			normal = glm::normalize(normal);
		normals.push_back(normal);
	}
}
