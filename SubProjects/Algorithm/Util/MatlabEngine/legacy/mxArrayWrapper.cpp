//#include "mxArrayWrapper.h"
//#include <matrix.h>
//#include <mat.h>
//#include <memory>
//#include <iostream>
//using namespace std;
//function<void(mxArray*)> mxArrayWrapper::deleter =
//[](mxArray* data){
//	// do nothing
//	cout << "shared_ptr delete call" << endl;
//};
//
//void mxArrayWrapper::_init(){
//	_data = nullptr;
//}
//
//mxArrayWrapper::mxArrayWrapper(){
//	_init();
//}
//mxArrayWrapper::mxArrayWrapper(mxArray* data){
//	_data = shared_ptr<mxArray>(data,
//		mxArrayWrapper::deleter
//		);
//}
//
//bool mxArrayWrapper::isEmpty() const {
//	return _data == nullptr;
//}
//
//const shared_ptr<mxArray>& mxArrayWrapper::assign(mxArray* data){
//	_data = shared_ptr<mxArray>(data, mxArrayWrapper::deleter);
//	return _data;
//}
//const shared_ptr<mxArray>& mxArrayWrapper::assign(const shared_ptr<mxArray>& data){
//	_data = data;
//	return _data;
//}
//const mxArrayWrapper& mxArrayWrapper::assign(const mxArrayWrapper& var){
//	*this = var; // there is std container only in mxArrayWrapper members. so, default assign operator is sufficient.
//	return *this;
//}
//
//mxArrayWrapper& mxArrayWrapper::operator = (mxArrayWrapper& var){
//	assign(var);
//	return *this;
//}
//const mxArrayWrapper& mxArrayWrapper::operator = (const mxArrayWrapper& var){
//	assign(var);
//	return *this;
//}
//
//shared_ptr<mxArray>& mxArrayWrapper::getSharedPtr(){
//	return _data;
//}
//const shared_ptr<mxArray>& mxArrayWrapper::getSharedPtr() const {
//	return _data;
//}
//mxArray* mxArrayWrapper::getRawPtr(){
//	return _data.get();
//}
//const mxArray* mxArrayWrapper::getRawPtr() const{
//	return _data.get();
//}
//
//// getPr, getPi, 그리고 타입도 double, float, 여러가지가 되도록 해야지
//const double& mxArrayWrapper::operator[](int i) const{
//	return mxGetPr(getRawPtr())[i]; // 멤버에 pr, pi 만들고 mxGetPr은 매번 부르지 않도록 바꾸끼
//}
//double& mxArrayWrapper::operator[](int i) {
//	return mxGetPr(getRawPtr())[i];
//}
