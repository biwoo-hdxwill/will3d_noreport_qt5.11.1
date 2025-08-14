#pragma once
#include "../Core/util_global.h"
#include <vector>
//
// 나중에 필요하다면 Triangle말고도 임의의 convex Polygon에 대해서 되도록 확장
// VertexToTriIdxMapper class
// this executes mapping vertex index --> triangle index
class UTIL_EXPORT VertexToTriIdxMapper {
protected:
	std::vector<std::vector<int>> _tris;
	std::vector<std::vector<int>> _vertexIdxToTriIdx;
public:
	VertexToTriIdxMapper();

	// constructor (tris)
	// tris = 3d mesh idxs
	VertexToTriIdxMapper(const std::vector<std::vector<int>>& tris);

	// for given vertex index, find trinagle index containing the vertex index.
	std::vector<int> findTriIdxByVertexIdx(int vertexIdx) const;

	// for given triangle index, find vertex indexs in the triangle
	std::vector<int> findVertexIdxByTriIdx(int triIdx) const;

	// for given one triangle, vertex index, find next vertex index
	int findNextVertexIdx(const std::vector<int>& triangle, int vertexIdx) const;
};
