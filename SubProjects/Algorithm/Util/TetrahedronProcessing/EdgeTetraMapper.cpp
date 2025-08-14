#include "EdgeTetraMapper.h"
#include <exception>
#include <iostream>
using namespace std;

EdgeTetraMapper::EdgeTetraMapper() {
}
EdgeTetraMapper::EdgeTetraMapper(const std::vector<std::vector<int>>& tetras) {
	try {
		if (tetras.empty()) {
			throw runtime_error("tetrahedrons is empty");
		}
		for (int i = 0; i < tetras.size(); i++) {
			const auto& tetra = tetras[i];
			if (tetra.size() != 4) {
				throw runtime_error("there is tetrahedron whose size is not 4");
			}
			for (int j = 0; j < tetra.size(); j++) {
				for (int k = j + 1; k < tetra.size(); k++) {
					_edgeTetraMap[tora::Edge(tetra[j], tetra[k])].push_back(i);
				}
			}
		}
		_tetras = tetras;
	} catch (runtime_error& e) {
		cout << "EdgeTetraMapper::EdgeTetraMapper: " << e.what();
	}
}
std::vector<int> EdgeTetraMapper::edge2TetraIdx(const tora::Edge& edge) const {
	try {
		if (_edgeTetraMap.find(edge) == _edgeTetraMap.end()) {
			throw runtime_error("edge is not contained to any tetrahedron");
		}
		return _edgeTetraMap.at(edge);
	} catch (runtime_error& e) {
		cout << "EdgeTetraMapper::edge2TetraIdx: " << e.what() << endl;
		return vector<int>();
	}
}
std::vector<std::vector<int>> EdgeTetraMapper::edge2Tetra(const tora::Edge& edge) const {
	vector<vector<int>> subTetras;
	for (const auto& it : edge2TetraIdx(edge)) {
		subTetras.push_back(_tetras[it]);
	}
	return subTetras;
}
std::vector<tora::Edge> EdgeTetraMapper::tetra2Edge(int tetraIdx) const {
	const auto& tetra = _tetras[tetraIdx];
	vector<tora::Edge> edgesGen;
	edgesGen.reserve(6);
	for (int j = 0; j < tetra.size(); j++) {
		for (int k = j + 1; k < tetra.size(); k++) {
			edgesGen.push_back(tora::Edge(j, k));
		}
	}
	return edgesGen;
}
