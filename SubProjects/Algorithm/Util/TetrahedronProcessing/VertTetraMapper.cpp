#include "VertTetraMapper.h"
#include <algorithm>
#include <exception>
#include <iostream>
using namespace std;

VertTetraMapper::VertTetraMapper() {
}
VertTetraMapper::VertTetraMapper(const std::vector<std::vector<int>> &tetras) {
	try {
		if (tetras.empty()) {
			throw runtime_error("tetrahedrons is empty");
		}
		int maxiv = -1;
		for (int i = 0; i < tetras.size(); i++) {
			const auto& tetra = tetras[i];
			if (tetra.size() != 4) {
				throw runtime_error("there is tetrahedron whose size is not 4");
			}
			for (int j = 0; j < tetra.size(); j++) {
				if (maxiv < tetra[j]) {
					maxiv = tetra[j];
				}
			}
		}
		maxiv++;
		_tetras = tetras;
		_vert2TetraMap.resize(maxiv);
		//auto maxTetra =
		//	*std::max_element(_tetras.begin(), _tetras.end(), [](const vector<int>& tetra0, const vector<int>& tetra1)->bool{
		//	return *std::max_element(tetra0.begin(), tetra0.end()) < *std::max_element(tetra1.begin(), tetra1.end());
		//});
		//int maxiv = *std::max_element(maxTetra.begin(), maxTetra.end());

		for (int it = 0; it < _tetras.size(); it++) {
			const auto& tetra = _tetras[it];
			for (const auto& iv : tetra) {
				_vert2TetraMap[iv].push_back(it);
			}
		}
	} catch (runtime_error& e) {
		cout << "VertTetraMapper::VertTetraMapper: " << e.what() << endl;
	}
}
std::vector<int> VertTetraMapper::vert2TetraIdx(int vertIdx) const {
	try {
		if (vertIdx >= _vert2TetraMap.size()) {
			throw runtime_error("vertIdx is out of bound");
		}
		if (_vert2TetraMap[vertIdx].empty()) {
			throw runtime_error("vertIdx is not contained to any tetrahedron");
		}
		return _vert2TetraMap[vertIdx];
	} catch (runtime_error& e) {
		cout << "VertTetraMapper::vert2TetraIdx: " << e.what() << endl;
		return vector<int>();
	}
}
std::vector<std::vector<int>> VertTetraMapper::vert2Tetra(int vertIdx) const {
	vector<vector<int>> subTetras;
	for (int it : vert2TetraIdx(vertIdx)) {
		subTetras.push_back(_tetras[it]);
	}
	return subTetras;
}
std::vector<int> VertTetraMapper::getTetra(int tetraIdx) const {
	return _tetras[tetraIdx];
}
