#pragma once
#include "../Core/util_global.h"
#include <engine.h>
#include "mxArrayWrapper.h"
#include <memory>
#include <functional>
#include <set>

class UTIL_EXPORT MatlabEngine{
private:
	std::vector<char> _printBuf;
	static const int _printBufSize;
protected:
	std::function<void(Engine*)> _deleter;
	std::shared_ptr<Engine> _eng;
	bool _persistent;
	void _init();
public:

	std::string currentPath;
	std::set<std::string> includePath;

// create & delete
	MatlabEngine();
	~MatlabEngine();
	void setPersistent(bool persistent);
	bool isPersistent() const;
// access
	Engine* getPtr();
	bool putVariable(const mxArrayWrapper& var);
	bool getVariable(mxArrayWrapper& var, const std::string& varName) const;
	bool eval(std::string& outStr, const std::string& evalStr);
	std::string eval(const std::string& evalStr);
	std::string evalVerbose(const std::string& evalStr);
// error chk
	bool hasBeenErr(std::string& matlabComplainStr);
	void matlabPrintBegin();
	std::string matlabPrintEnd();
// set path
	bool setCurrentPath(const std::string& currentPath);
	bool setIncludePath(const std::set<std::string>& includePath);
	bool setAllPath(const std::string& fpath);
};
