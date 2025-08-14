#pragma once
#include "../Core/util_global.h"
#include <vector>
#include <functional>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif
class UTIL_EXPORT MeshCompacter {
public:
	//static std::vector<int> _remap;
public:

	// _executeRemapVertexIdxOfTriangle(points, triangles, l_remap)
	// element(vertex idx) of triangle will be remapped.
	// s.t for all triangles element, vidx = remap(vidx)
	// points will be intact
	// [WARN] return not matched mesh (idx of triangles is not appropriate to points)
	//static bool executeRemapVertexIdx(
	//	std::vector<glm::vec3>& points,						:
	//	std::vector<std::vector<int>>& triangles,			:
	//	const std::function<int (int)>& l_remap				:
	//)
	static bool _executeRemapVertexIdxOfTriangle(
		std::vector<glm::vec3>& points,
		std::vector<std::vector<int>>& triangles,
		const std::function<int(int)>& l_remap
	);

	// _executeRemapTriangles(triangles, l_remap)
	static bool _executeRemapTriangles(
		std::vector<std::vector<int>>& triangles,
		const std::function<int(int)>& l_remap
	);

	// _executeRemoveTriangleByTriangleCriteria(points, triangles, removeTriangle)
	// remove triangles if removeTriangle(i) == true
	// points will be intact
	// [WARN] return incompact mesh (there may be unused danggling vertex)
	//static bool executeRemoveTriangleByCriteria(
	//	std::vector<glm::vec3>& points,						:
	//	std::vector<std::vector<int>>& triangles,			:
	//	const std::function<bool (int)>& removeTriangle		:
	//)
	static bool _executeRemoveTriangleByTriangleCriteria(
		std::vector<glm::vec3>& points,
		std::vector<std::vector<int>>& triangles,
		const std::function<bool(int)>& removeTriangle
	);

	// executeremoveMeshByTriangleCriteria(points, triangles, l_remap, removeTriangle)
	// remove triangles if removeTriangle(i) == true
	// unused danggling point will be removed
	// return compact mesh
	//static bool executeRemoveTriangleByCriteria(
	//	std::vector<glm::vec3>& points,						:
	//	std::vector<std::vector<int>>& triangles,			:
	//	std::function<int (int)>& l_remap,					:
	//	const std::function<bool (int)>& removeTriangle		:
	//)
	static bool executeRemoveMeshByTriangleCriteria(
		std::vector<glm::vec3>& points,
		std::vector<std::vector<int>>& triangles,
		std::function<int(int)>& l_remap,  // remap -> (idx prop)
		const std::function<bool(int)>& removeTriangle
	);

	// executeRemoveMeshByVertexCriteria(points, triangles, l_remap, removeVertex)
	// remove vertex if removeVertex(i) == true
	// triangle that contains removed vertex idx will be deleted too
	// return compact mesh
	//static bool executeRemoveVertexByCriteria(
	//	std::vector<glm::vec3>& points,						:
	//	std::vector<std::vector<int>>& triangles,			:
	//	std::function<int (int)>& l_remap,					:
	//	const std::function<bool (int)>& removeVertex		:
	//)
	static bool executeRemoveMeshByVertexCriteria(
		std::vector<glm::vec3>& points,
		std::vector<std::vector<int>>& triangles,
		std::function<int(int)>& l_remap,
		const std::function<bool(int)>& removeVertex
	);

public:
	// executeRemoveUnusedVertex(points, triangles, l_remap)
	// remove dangling vertex, which is not contained to any triangle
	// triangle will be remapped.
	// return compact mesh
	//static bool executeRemoveUnusedVertex(
	//	std::vector<glm::vec3>& points,						:
	//	std::vector<std::vector<int>>& triangles			:
	//	std::function<int (int)>& l_remap					:
	//)
	static bool executeRemoveUnusedVertex(
		std::vector<glm::vec3>& points,
		std::vector<std::vector<int>>& triangles,
		std::function<int(int)>& l_remap
	);

	// executeRemoveMeshByTriangleAreaZero(points, triangles, l_remap)
	// remove triangles whose area is 0.
	// after deleting triangles, unused danggling vertex will be removed
	// return compact mesh
	//static bool executeRemoveTriangleByAreaZero(
	//	std::vector<glm::vec3>& points,						:
	//	std::vector<std::vector<int>>& triangles			:
	//	std::function<int (int)>& l_remap					:
	//)
	static bool executeRemoveMeshByTriangleAreaZero(
		std::vector<glm::vec3>& points,
		std::vector<std::vector<int>>& triangles,
		std::function<int(int)>& l_remap
	);

public: // tools
	// executeRemapIdx(idxs, l_remap)
	static bool executeRemapIdx(
		std::vector<int>& idxs,
		const std::function<int(int)>& l_remap
	);
};
