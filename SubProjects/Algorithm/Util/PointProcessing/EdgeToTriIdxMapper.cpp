#include "EdgeToTriIdxMapper.h"
#include <iostream>
#include <set>
#include <sstream>
#include "../Core/coutOverloading.h"
using namespace std;
using namespace tora;

EdgeToTriIdxMapper::EdgeToTriIdxMapper() {
}

EdgeToTriIdxMapper::EdgeToTriIdxMapper(const vector<vector<int>>& tris) {
	_tris = tris;

	// make _edgeToTriIdx
	for (int i = 0; i < _tris.size(); i++) {
		const auto& tri = _tris[i];
		for (int j = 0; j < tri.size(); j++) {
			Edge e(tri[j], tri[(j + 1) % tri.size()]);
			_edgeToTriIdx[e].push_back(i);
		}
	}
}

vector<int> EdgeToTriIdxMapper::findTriIdxByEdge(const Edge& e) const {
	if (_edgeToTriIdx.find(e) != _edgeToTriIdx.end())
		return _edgeToTriIdx.at(e);
	else return vector<int>();
}

vector<Edge> EdgeToTriIdxMapper::findEdgeByTriIdx(int triIdx) const {
	vector<Edge> result;
	const auto& tri = _tris[triIdx];
	for (int j = 0; j < tri.size(); j++) {
		Edge e(tri[j], tri[(j + 1) % tri.size()]);
		result.push_back(e);
	}
	return result;
}

vector<Edge> EdgeToTriIdxMapper::getEdges() const {
	vector<Edge> result;
	result.reserve(_edgeToTriIdx.size());
	for (const auto& elem : _edgeToTriIdx) {
		result.push_back(elem.first);
	}
	return result;
}

vector<Edge> EdgeToTriIdxMapper::getBoundaryEdges() const {
	vector<Edge> result;
	try {
		for (const auto& elem : _edgeToTriIdx) {
			if (elem.second.empty()) {
				throw runtime_error(
					"there is edge which not contains triangle idx."
					" this edge is not contained any triangle !"
				);
			} else if (elem.second.size() <= 1) {
				result.push_back(elem.first);
			}
		}
		return result;
	} catch (runtime_error& e) {
		cout << "EdgeToTriIdxMapper::getBoundaryEdges(): " << e.what() << endl;
		return result;
	}
}

vector<int> EdgeToTriIdxMapper::boundarySort(const std::vector<Edge>& boundaryEdges) {
	vector<int> result;
	try {
		if (boundaryEdges.empty()) {
			throw runtime_error("boundaryEdges is empty");
		}

		result.push_back(boundaryEdges[0].v0);
		result.push_back(boundaryEdges[0].v1);

		map<int, vector<int>> vertexToEdgeIdx;
		for (int i = 0; i < boundaryEdges.size(); i++) {
			const auto& e = boundaryEdges[i];
			vertexToEdgeIdx[e.v0].push_back(i);
			vertexToEdgeIdx[e.v1].push_back(i);
		}

		set<Edge> edgeNotVisited;
		edgeNotVisited.insert(boundaryEdges.begin(), boundaryEdges.end());
		edgeNotVisited.erase(boundaryEdges[0]);

		int vertex = result.back();
		for (int i = 1; i < boundaryEdges.size(); i++) {
			for (const auto& eidx : vertexToEdgeIdx[vertex]) {
				const auto& e = boundaryEdges[eidx];
				auto it = edgeNotVisited.find(e);
				if (it != edgeNotVisited.end()) {
					vertex = (vertex == e.v0) ? e.v1 : e.v0;
					result.push_back(vertex);
					edgeNotVisited.erase(it);
				}
			}
		}

		if (!edgeNotVisited.empty()) {
			stringstream ss;
			ss << "given boundaryEdges is not bounary edges." << endl;
			ss << "last vertex = " << vertex << endl;
			ss << "edge not visited size = " << edgeNotVisited.size() << "/" << boundaryEdges.size() << endl;
			ss << "edge not visited = " << edgeNotVisited << endl;
			ss << "vertexToEdgeIdx=" << endl;
			for (const auto& elem : vertexToEdgeIdx) {
				ss << "<" << elem.first << ">=" << elem.second << endl;
			}

			throw runtime_error(ss.str().c_str());
		}

		return result;
	} catch (runtime_error& e) {
		cout << "EdgeToTriIdxMapper::boudnarySort(): " << e.what() << endl;
		return vector<int>();
	}
}
