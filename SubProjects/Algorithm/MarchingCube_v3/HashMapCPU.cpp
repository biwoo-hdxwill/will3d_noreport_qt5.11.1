#pragma message("# HashMapCPU.cpp visited")
#include "HashMapCPU.h"
#define min(a,b) ((a)<(b)? (a): (b))
using namespace std;
///////////////////////////////////////// Node ////////////////////////////////////////
HashMapCPU::Node::Node(): occupied(false) {}
HashMapCPU::Node::Node(uint64 key, int val) : key(key), val(val), occupied(true) {}

///////////////////////////////////////// Hasher ////////////////////////////////////////
size_t HashMapCPU::Hasher::operator()(const uint64& key){
	return _hash_seq((const unsigned char*)&key, sizeof(uint64));
}
size_t HashMapCPU::Hasher::_hash_seq(const unsigned char* _First, size_t _Count){
	static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");
	const size_t _FNV_offset_basis = 14695981039346656037ULL;
	const size_t _FNV_prime = 1099511628211ULL;
	size_t _Val = _FNV_offset_basis;
	for (size_t _Next = 0; _Next < _Count; ++_Next)
	{	// fold in another byte
		_Val ^= (size_t)_First[_Next];
		_Val *= _FNV_prime;
	}
	_Val ^= _Val >> 32;
	return _Val;
}
///////////////////////////////////////// HashMapCPU ////////////////////////////////////////
HashMapCPU::HashMapCPU(){
	m_cap = 1e4;
	m_maxIter = 1e4;
	m_data.resize(m_cap, Node());
	hasher = Hasher();
}
HashMapCPU::HashMapCPU(int cap){
	m_cap = cap;
	m_maxIter = min(m_cap, 1e4);
	m_data.resize(m_cap, Node());
	hasher = Hasher();
}
bool HashMapCPU::insert(uint64 key, int val){
	int base = hasher(key);
	int bid;
	for (int i = 1; i <= m_maxIter; i++){
		bid = (uint32(base) + i) % m_cap;
		Node& bucket = m_data[bid];
		if (!bucket.occupied){
			bucket.occupied = true;
			bucket.key = key;
			bucket.val = val;
			return true;
		}
	}
	return false;
}
bool HashMapCPU::insert(int& attempt, uint64 key, int val){
	int base = hasher(key);
	int bid;
	for (int i = 1; i <= m_maxIter; i++){
		bid = (uint32(base) + i) % m_cap;
		Node& bucket = m_data[bid];
		if (!bucket.occupied){
			bucket.occupied = true;
			bucket.key = key;
			bucket.val = val;
			attempt += i;
			return true;
		}
	}
	attempt += m_maxIter;
	return false;
}
bool HashMapCPU::get(int& val, uint64 key){
	int base = hasher(key);
	int bid;
	for (int i = 1; i <= m_maxIter; i++){
		int bid = (uint32(base) + i) % m_cap;
		Node& bucket = m_data[bid];
		if (bucket.occupied && bucket.key == key){
			val = bucket.val;
			return true;
		}
	}
	return false;
}
