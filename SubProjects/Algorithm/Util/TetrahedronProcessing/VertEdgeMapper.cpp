#include "VertEdgeMapper.h"
#include <exception>
#include <set>
#include <iostream>
using namespace std;

void VertEdgeMapper::_constr(const std::vector<tora::Edge>& edges) {
	int maxiv = -1;
	for (int i = 0; i < edges.size(); i++) {
		const auto& edge = edges[i];
		for (int j = 0; j < 2; j++) {
			int vertIdx = edge[j];
			if (maxiv < vertIdx) {
				maxiv = vertIdx;
			}
		}
	}
	maxiv++;
	_vert2EdgeMap.resize(maxiv);
	for (int i = 0; i < edges.size(); i++) {
		const auto& edge = edges[i];
		for (int j = 0; j < 2; j++) {
			int vertIdx = edge[j];
			_vert2EdgeMap[vertIdx].push_back(i);
		}
	}
}
VertEdgeMapper::VertEdgeMapper() {
}
VertEdgeMapper::VertEdgeMapper(const std::vector<std::vector<int>>& tetras) {
	try {
		set<tora::Edge> edgesGen;
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
					edgesGen.insert(tora::Edge(tetra[j], tetra[k]));
				}
			}
		}
		_edges.reserve(edgesGen.size());
		for (const auto& e : edgesGen) {
			_edges.push_back(e);
		}
		_constr(_edges);
	} catch (runtime_error& e) {
		cout << "VertEdgeMapper::VertEdgeMapper: " << e.what() << endl;
	}
}
std::vector<tora::Edge> VertEdgeMapper::vert2Edge(int vertIdx) const {
	try {
		if (vertIdx >= _vert2EdgeMap.size()) {
			throw runtime_error("vertIdx is out of bound");
		}
		if (_vert2EdgeMap[vertIdx].empty()) {
			throw runtime_error("vertIdx is not contained to any edges");
		}
		vector<tora::Edge> subEdges;
		subEdges.reserve(_vert2EdgeMap[vertIdx].size());
		for (int ie : _vert2EdgeMap[vertIdx]) {
			subEdges.push_back(_edges[ie]);
		}
		return subEdges;
	} catch (runtime_error& e) {
		cout << "VertEdgeMapper::vert2Edge: " << e.what() << endl;
		return vector<tora::Edge>();
	}
}
std::vector<int> VertEdgeMapper::neighbor(int vertIdx) const {
	auto subEdges = vert2Edge(vertIdx);
	set<int> neis;
	for (const auto& edge : subEdges) {
		for (int j = 0; j < 2; j++) {
			if (edge[j] != vertIdx) {
				neis.insert(edge[j]);
			}
		}
	}
	vector<int> res;
	res.reserve(neis.size());
	for (const auto& iv : neis) {
		res.push_back(iv);
	}
	return res;
}
std::vector<tora::Edge> VertEdgeMapper::getEdges() const {
	return _edges;
}
