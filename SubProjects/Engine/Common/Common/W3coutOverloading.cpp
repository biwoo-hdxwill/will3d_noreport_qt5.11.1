#include "W3coutOverloading.h"

std::ostream& operator<< (std::ostream& o, const glm::vec4& v){
	o << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3];
	return o;
}
std::ostream& operator<< (std::ostream& o, const glm::vec3& v){
	o << v[0] << ", " << v[1] << ", " << v[2];
	return o;
}
std::ostream& operator<< (std::ostream& o, const glm::vec2& v){
	o << v[0] << ", " << v[1];
	return o;
}
std::ostream& operator<< (std::ostream& o, const glm::mat4& m){
	for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++){
			o <<  m[j][i] << "  ";
		}
		o << std::endl;
	}
	return o;
}
