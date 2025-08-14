#pragma once
#include "../Core/util_global.h"
#include <vector>
#include "../PointProcessing/Edge.h"
#include <map>
class UTIL_EXPORT EdgeTetraMapper {
protected:
	std::vector<std::vector<int>> _tetras;
	std::map<tora::Edge, std::vector<int>> _edgeTetraMap;
public:
	EdgeTetraMapper();
	EdgeTetraMapper(const std::vector<std::vector<int>>& tetras);
	std::vector<int> edge2TetraIdx(const tora::Edge& edge) const;
	std::vector<std::vector<int>> edge2Tetra(const tora::Edge& edge) const;
	std::vector<tora::Edge> tetra2Edge(int tetraIdx) const;
};
