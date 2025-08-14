#pragma once

#include "util_global.h"
#include <iostream>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif
#include <vector>
#include <set>
#include <map>

UTIL_EXPORT std::ostream& operator<< (std::ostream& o, const glm::vec4& v);
UTIL_EXPORT std::ostream& operator<< (std::ostream& o, const glm::vec3& v);
UTIL_EXPORT std::ostream& operator<< (std::ostream& o, const glm::vec2& v);
UTIL_EXPORT std::ostream& operator<< (std::ostream& o, const glm::mat4& m);


template<typename T>
std::ostream& operator<< (std::ostream& o, const std::vector<T>& tvec){
	o << "[";
	for (int i = 0; i < tvec.size(); i++){
		const auto& t = tvec[i];
		if (i < (int)tvec.size() - 1){
			o << t << ", ";
		}
		else{
			o << t;
		}
	}
	o << "]";
	return o;
}

template<typename T>
std::ostream& operator << (std::ostream& o, const std::set<T>& tset){
	o << "{";
	int cnt = 0;
	for (const auto& e : tset){
		if (cnt < (int)tset.size() - 1){
			o << e << ", ";
		}
		else{
			o << e;
		}
		++cnt;
	}
	o << "}";
	return o;
}

template<typename Key, typename Val>
std::ostream& operator << (std::ostream& o, const std::map<Key, Val>& m){
	o << "{";
	int cnt = 0;
	for (const auto& elem : m){
		if (cnt < (int)m.size() - 1){
            o << elem.first << "=" << elem.second << ", " << std::endl;
		}
		else{
			o << elem.first << "=" << elem.second;
		}
		++cnt;
	}
	o << "}";
	return o;
}

template<typename T1, typename T2>
std::ostream& operator << (std::ostream& o, const std::pair<T1, T2>& p){
	o << "(" << p.first << "," << p.second << ")";
	return o;
}
