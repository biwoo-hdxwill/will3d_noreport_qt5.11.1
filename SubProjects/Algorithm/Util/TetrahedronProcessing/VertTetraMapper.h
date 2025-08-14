#pragma once
#include "../Core/util_global.h"
#include <vector>
class UTIL_EXPORT VertTetraMapper {
protected:
	std::vector<std::vector<int>> _tetras;
	std::vector<std::vector<int>> _vert2TetraMap;
public:
	VertTetraMapper();
	VertTetraMapper(const std::vector<std::vector<int>> &tetras);
	std::vector<int> vert2TetraIdx(int vertIdx) const;
	std::vector<std::vector<int>> vert2Tetra(int vertIdx) const;
	std::vector<int> getTetra(int tetraIdx) const;
};
