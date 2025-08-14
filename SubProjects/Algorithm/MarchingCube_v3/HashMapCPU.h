#pragma once
#pragma message("# HashMapCPU.h visited")
#include "marchingcube_v3_global.h"
#include <Util/Core/typeDef.h>
#include <vector>
class MARCHINGCUBE_V3_EXPORT HashMapCPU{
public: /* sub class */
	struct Node{
		uint64 key = 0;
		int val = 0;
		uint32 occupied = 0;
		Node();
		Node(uint64 key, int val);
	};
	struct Hasher{
		inline size_t operator()(const uint64& key);
		inline size_t _hash_seq(const uint8* first, size_t const);
	};
public: /* members */
	int m_cap;
	int m_maxIter;
	std::vector<Node> m_data;
	Hasher hasher;
public: /* methods */
	HashMapCPU();
	HashMapCPU(int cap);
	bool insert(uint64 key, int val);
	bool insert(int& attempt, uint64 key, int val);
	bool get(int& val, uint64 key);
};
