#include "Edge.h"
#include <iostream>
#include <sstream>
using namespace std;
namespace tora {
	Edge::Edge() : v0(-1), v1(-1) {
	}
	Edge::Edge(int v0, int v1) : v0(v0), v1(v1) {
		try {
			if (this->v0 == this->v1) {
				throw runtime_error("v0 == v1 is not edge");
			}
		} catch (runtime_error& e) {
			cout << "Edge: " << e.what() << endl;
		}
	}
	int Edge::operator[] (int i) const {
		return (i == 0) ? v0 : v1;
	}
	bool Edge::operator < (const Edge& o) const {
		int min0 = min(v0, v1);
		int max0 = max(v0, v1);
		int min1 = min(o.v0, o.v1);
		int max1 = max(o.v0, o.v1);
		return (max0 < max1) || (max0 == max1 && min0 < min1);
	}
	string Edge::toString() const {
		stringstream ss;
		ss << "(" << v0 << "," << v1 << ")";
		return ss.str();
	}
}

std::ostream& operator << (std::ostream& o, const tora::Edge& e) {
	o << e.toString();
	return o;
}
