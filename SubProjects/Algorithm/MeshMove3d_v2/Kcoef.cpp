#include "Kcoef.h"
using namespace std;

Kcoef::Kcoef(){

}
bool Kcoef::get(Val& val, int k0, int k1) const{
	Key key(k0, k1);
	if (m_data.find(key) != m_data.end()){
		val = m_data.at(key);
		return true;
	}
	else{
		val = Val(0.f);
		return false;
	}
}
bool Kcoef::get(Eigen::Matrix3f& val, int k0, int k1) const{
	Key key(k0, k1);
	if (m_data.find(key) != m_data.end()){
		auto temp = m_data.at(key);
		for (int i = 0; i < 3; i++){
			for (int j = 0; j < 3; j++){
				val(i, j) = temp[j][i];
			}
		}
		return true;
	}
	else{
		val = Eigen::Matrix3f::Zero(3, 3);
		return false;
	}
}
const Kcoef::Val& Kcoef::set(int k0, int k1, const Val& val) {
	Key key(k0, k1);
	m_data[key] = val;
	return val;
}

void Kcoef::clear(){
	m_data.clear();
}
