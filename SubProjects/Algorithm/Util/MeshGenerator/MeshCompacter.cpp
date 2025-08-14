#include "MeshCompacter.h"
#include <algorithm>
#include <iterator>
using namespace std;

//std::vector<int> MeshCompacter::_remap;

bool MeshCompacter::_executeRemapVertexIdxOfTriangle(
	std::vector<glm::vec3>& points,
	std::vector<std::vector<int>>& triangles,
	const std::function<int(int)>& l_remap
) {
	for (auto& triangle : triangles) {
		for (auto& vidx : triangle) {
			vidx = l_remap(vidx);
		}
	}
	return true;
}

bool MeshCompacter::_executeRemapTriangles(
	std::vector<std::vector<int>>& triangles,
	const std::function<int(int)>& l_remap
) {
	for (auto& triangle : triangles) {
		for (auto& vidx : triangle) {
			vidx = l_remap(vidx);
		}
	}
	return true;
}

bool MeshCompacter::_executeRemoveTriangleByTriangleCriteria(
	std::vector<glm::vec3>& points,
	std::vector<std::vector<int>>& triangles,
	const std::function<bool(int)>& removeTriangle
) {
	vector<vector<int>> triangles_out;
	triangles_out.clear();
	triangles_out.reserve(triangles.size());
	//std::remove_copy_if(triangles.begin(), triangles.end(), back_inserter(triangles_out), removeTriangle);
	for (int i = 0; i < triangles.size(); i++) {
		if (!removeTriangle(i)) {
			triangles_out.push_back(triangles[i]);
		}
	}
	triangles = triangles_out;
	return true;
}

bool MeshCompacter::executeRemoveMeshByTriangleCriteria(
	std::vector<glm::vec3>& points,
	std::vector<std::vector<int>>& triangles,
	//std::vector<int>& remap,
	std::function<int(int)>& l_remap,
	const std::function<bool(int)>& removeTriangle
) {
	MeshCompacter::_executeRemoveTriangleByTriangleCriteria(points, triangles, removeTriangle);
	MeshCompacter::executeRemoveUnusedVertex(points, triangles, l_remap);
	return true;
}

bool MeshCompacter::executeRemoveMeshByVertexCriteria(
	std::vector<glm::vec3>& points,
	std::vector<std::vector<int>>& triangles,
	//std::vector<int>& remap,
	std::function<int(int)>& l_remap,
	const std::function<bool(int)>& removeVertex
) {
	vector<int> remap_(points.size());

	vector<glm::vec3> points_out;
	points_out.reserve(points.size());
	int cnt = 0;
	for (int i = 0; i < remap_.size(); i++) {
		if (!removeVertex(i)) {
			remap_[i] = cnt++;
			points_out.push_back(points[i]);
		} else {
			remap_[i] = -1;
		}
	}
	//auto l_remap_ = [&remap_](int i)->int{
	//	return remap_[i];
	//};
	l_remap = [remap_](int i)->int {
		return remap_[i];
	};
	auto l_removeTriangle = [&triangles](int i)->bool {
		for (int vidx : triangles[i]) {
			if (vidx < 0) {
				return true;
			}
		}
		return false;
	};
	MeshCompacter::_executeRemapVertexIdxOfTriangle(points, triangles, l_remap);
	MeshCompacter::_executeRemoveTriangleByTriangleCriteria(points, triangles, l_removeTriangle);
	points = points_out;

	return true;
}

bool MeshCompacter::executeRemoveUnusedVertex(
	std::vector<glm::vec3>& points,
	std::vector<std::vector<int>>& triangles,
	//std::vector<int>& remap
	std::function<int(int)>& l_remap
) {
	vector<int> used(points.size(), 0);
	for (const auto& triangle : triangles) {
		for (int vidx : triangle) {
			used[vidx] = 1;
		}
	}
	auto l_removeVertex = [&used](int i)->bool {
		return !used[i];
	};
	MeshCompacter::executeRemoveMeshByVertexCriteria(points, triangles, l_remap, l_removeVertex);

	return true;
}

bool MeshCompacter::executeRemoveMeshByTriangleAreaZero(
	std::vector<glm::vec3>& points,
	std::vector<std::vector<int>>& triangles,
	//std::vector<int>& remap
	std::function<int(int)>& l_remap
) {
	vector<int> isTriangleAreaZero(triangles.size(), 0);
	for (int i = 0; i < triangles.size(); i++) {
		const auto& triangle = triangles[i];
		glm::vec3& p0 = points[triangle[0]];
		glm::vec3& p1 = points[triangle[1]];
		glm::vec3& p2 = points[triangle[2]];

		// if triangle has same vertex
		if (triangle[0] == triangle[1] || triangle[1] == triangle[2] || triangle[2] == triangle[0]) {
			isTriangleAreaZero[i] = 1;
		}

		// if triangle's area is 0
		if (glm::cross(p2 - p0, p1 - p0) == glm::vec3(0, 0, 0)) {
			isTriangleAreaZero[i] = 1;
		}
	}

	// remove triangle whose area is zero
	// and remove unused danggling vertex
	MeshCompacter::executeRemoveMeshByTriangleCriteria(points, triangles, l_remap,
		[&isTriangleAreaZero](int i)->bool {
		return isTriangleAreaZero[i];
	}
	);

	return true;
}

bool MeshCompacter::executeRemapIdx(
	std::vector<int>& idxs,
	const std::function<int(int)>& l_remap
) {
	vector<int> res;
	res.reserve(idxs.size());
	for (int i = 0; i < idxs.size(); i++) {
		if (l_remap(idxs[i]) != -1) {
			res.push_back(l_remap(idxs[i]));
		}
	}
	idxs = res;
	return true;
}
