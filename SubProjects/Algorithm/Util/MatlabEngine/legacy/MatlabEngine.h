//#pragma once
//#include <Util/Core/util_global.h>
//#include <mex.h>
//#include <string>
//#include "mxArrayWrapper.h"
//#include <engine.h>
////class Engine;
//
//class UTIL_EXPORT MatlabEngineWrapper{
//protected:
//	Engine* e;
//	bool MERR(const mxArray* ptr) const;
//	bool MERR(bool val) const;
//public:
//	MatlabEngineWrapper();
//	// this function return		1: there is err
//	//							0: there is no err
//	bool hasBeenError() const;
//	// this function return		1: successful
//	//							0: err
//	bool eval(const std::string& expression) const;
//	// this function return		1: successful
//	//							0: err
//	bool putVariable(const mxArrayWrapper& var, const std::string& name) const;
//	// this function return		1: successful
//	//							0: err
//	bool getVariable(mxArrayWrapper& var, const std::string& name) const;
//	bool putVariable(const int& var, const std::string& name)const;
//	bool getVariable(int& var, const std::string& name) const;
//	bool putVariable(const float& var, const std::string& name) const;
//	bool getVariable(float& var, const std::string& name) const;
//	bool putVariable(const double& var, const std::string& name) const;
//	bool getVariable(double& var, const std::string& name) const;
//	bool putVariable(const std::string& var, const std::string& name) const;
//	bool getVariable(std::string& var, const std::string& name) const;
//
//	// other custom type will be implemented with "Interface Manager"
//	// ex
//	// bool putVariable(const T& var, const std::string& name){
//	//		return InterfaceManagerT::putVaraible(this, var, name);
//	// }
//	// bool getVariable(T& var, const std::string& name){
//	//		return InterfaceManagerT::getVariable(this, var, name);
//	// }
//};
