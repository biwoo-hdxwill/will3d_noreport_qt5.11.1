//#include "MatlabEngine.h"
//#include <engine.h>
//#include <matrix.h>
//#include <mat.h>
//#include <engine.h>
//#include <exception>
//#include "../Core/dump.hpp"
//using namespace std;
//
//#define DEBUG_EVAL 1
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//bool MatlabEngineWrapper::MERR(const mxArray* ptr) const {
//	return ptr == nullptr;
//}
//
//bool MatlabEngineWrapper::MERR(bool val) const {
//	return val;
//}
//MatlabEngineWrapper::MatlabEngineWrapper(){
//	try{
//		if ( (e = engOpen("")) == nullptr ){
//			throw exception("engine open error");
//		}
//	}
//	catch (exception& e){
//		dout << "MatlabEngineWrapper::MatlabEngineWrapper: " << e.what() << endl;
//	}
//}
//bool MatlabEngineWrapper::hasBeenError() const {
//	eval("__myLastErrChk__=lasterr");
//	string errStr;
//	getVariable(errStr, "__myLastErrChk__");
//	return !errStr.empty();
//}
//bool MatlabEngineWrapper::eval(const string& expression) const{
//	engEvalString(e, expression.c_str());
//#if DEBUG_EVAL == 1
//	return !hasBeenError();
//#else
//	return true;
//#endif
//}
//
//bool MatlabEngineWrapper::putVariable(const mxArrayWrapper& var, const std::string& name) const{
//	return !engPutVariable(e, name.c_str(), var.getRawPtr());
//	// engPutVariable return	1: err
//	//							0: successful
//	// this function return		1: successful
//	//							0: err
//}
//
//bool MatlabEngineWrapper::getVariable(mxArrayWrapper& var, const std::string& name) const{
//	return var.assign(engGetVariable(e, name.c_str()));
//	// this function return		1: successful
//	//							0: err
//	
//}
//bool MatlabEngineWrapper::putVariable(const int& var, const std::string& name) const{
//	return 0;
//}
//bool MatlabEngineWrapper::getVariable(int& var, const std::string& name) const{
//	return 0;
//}
//bool MatlabEngineWrapper::putVariable(const float& var, const std::string& name) const{
//	return 0;
//}
//bool MatlabEngineWrapper::getVariable(float& var, const std::string& name) const{
//	return 0;
//}
//bool MatlabEngineWrapper::putVariable(const double& var, const std::string& name) const{
//	return 0;
//}
//bool MatlabEngineWrapper::getVariable(double& var, const std::string& name) const{
//	return 0;
//}
//bool MatlabEngineWrapper::putVariable(const std::string& var, const std::string& name) const{
//	return 0;
//}
//bool MatlabEngineWrapper::getVariable(std::string& var, const std::string& name) const{
//	return 0;
//}
