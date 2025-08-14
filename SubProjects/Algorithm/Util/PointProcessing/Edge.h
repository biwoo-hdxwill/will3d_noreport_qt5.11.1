#pragma once
#include <string>
#include "../Core/util_global.h"
namespace tora {
	class UTIL_EXPORT Edge {
	public:
		int v0;
		int v1;
		Edge();
		Edge(int v0, int v1);
		bool operator < (const Edge& o) const;
		int operator[] (int i) const;
		std::string toString() const;
	};
}

UTIL_EXPORT std::ostream& operator << (std::ostream& o, const tora::Edge& e);
