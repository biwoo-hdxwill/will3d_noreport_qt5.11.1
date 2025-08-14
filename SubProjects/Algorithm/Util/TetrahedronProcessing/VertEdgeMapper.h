#pragma once
#include "../Core/util_global.h"
#include <vector>
#include "../PointProcessing/Edge.h"
class UTIL_EXPORT VertEdgeMapper {
protected:
	std::vector<tora::Edge> _edges;
	std::vector<std::vector<int>> _vert2EdgeMap;
	void _constr(const std::vector<tora::Edge>& edges);
public:
	VertEdgeMapper();
	VertEdgeMapper(const std::vector<std::vector<int>>& tetras);
	std::vector<tora::Edge> vert2Edge(int vertIdx) const;
	std::vector<int> neighbor(int vertIdx) const;
	std::vector<tora::Edge> getEdges() const;
};
