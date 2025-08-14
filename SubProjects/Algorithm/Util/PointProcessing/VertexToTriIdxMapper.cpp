#include "VertexToTriIdxMapper.h"
#include <iostream>
using namespace std;

VertexToTriIdxMapper::VertexToTriIdxMapper() {
}

VertexToTriIdxMapper::VertexToTriIdxMapper(const vector<vector<int>>& tris) {
	_tris = tris;

	// make _vertexIdxToTriIdx
		// find max vertexIdx
	int maxIdx = -1;
	for (const auto& t : _tris) {
		for (int idx : t) {
			if (maxIdx < idx) {
				maxIdx = idx;
			}
		}
	}

	maxIdx++;
	//if(maxIdx == 0) return;

	_vertexIdxToTriIdx = vector<vector<int>>(maxIdx);

	// make reverse mapping
	for (int i = 0; i < _tris.size(); i++) {
		for (int idx : _tris[i]) {
			_vertexIdxToTriIdx[idx].push_back(i);
		}
	}
}

vector<int> VertexToTriIdxMapper::findTriIdxByVertexIdx(int vertexIdx) const {
	return _vertexIdxToTriIdx[vertexIdx];
}

vector<int> VertexToTriIdxMapper::findVertexIdxByTriIdx(int triIdx) const {
	return _tris[triIdx];
}

int VertexToTriIdxMapper::findNextVertexIdx(const vector<int>& triangle, int vertexIdx) const {
	try {
		int idx = -1;
		for (int i = 0; i < triangle.size(); i++) {
			if (vertexIdx == triangle[i]) {
				idx = triangle[(i + 1) % triangle.size()];
				break;
			}
		}
		if (idx == -1) {
			throw runtime_error("the triangle doesnt contain the vertexIdx");
		}
		return idx;
	} catch (runtime_error& e) {
		cout << "[FATAL] VertexToTriIdxMapper::findNextVertexIdx: " << e.what() << endl;
		abort();
	}
}
