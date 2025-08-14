#include "MatlabEngine.h"
#include <sstream>
#include <regex>
#include <fstream>
#include "../Core/IOParser_v3.h"
#include "../Core/RegexHelper.h"
using namespace std;

const int MatlabEngine::_printBufSize = 2048;
///////////////////////////////////////////////////////////// create & delete /////////////////////////////////////////////
void MatlabEngine::_init(){
	_printBuf.resize(_printBufSize);
	_eng = nullptr;
	_persistent = false;
	_deleter = [this](Engine* eng){
		try{
			if (this->isPersistent()){
				cout << "MATLAB Engine set to be persistent. this engine will not be closed" << endl;
			}
			else{
				cout << "MATLAB Engine close ." << endl;
				if (engClose(eng) != 0){
					throw exception("MATLAB engine close err");
				}
			}
		}
		catch (exception& e){
			cout << "lambda _deleter (of MatlabEngine): " << e.what() << endl;
		}
	};
	
}
MatlabEngine::MatlabEngine(){
	_init();
	try{
		cout << "MATLAB Engine start" << endl;
		if ((_eng = shared_ptr<Engine>(engOpen(""), _deleter)) == nullptr){
			throw exception("MATLAB engine open err");
		}
		engEvalString(_eng.get(), "lasterror('reset');"); // clear lasterr;
	}
	catch (exception& e){
		cout << "MatlabEngine::MatlabEngine: " << e.what() << endl;
	}
}
MatlabEngine::~MatlabEngine(){

}
Engine* MatlabEngine::getPtr(){
	return _eng.get();
}
void MatlabEngine::setPersistent(bool persistent){
	_persistent = persistent;
	if (_persistent) {
		cout << "MATLAB Engine is set to be persistent. this Engine will not be closed even if application is terminated." << endl;
	}
	else{
		cout << "MATLAB Engine is set to be non-persistent. this Engine will be closed if MatlabEngine object is out of scope in cpp code." << endl;
	}

}
bool MatlabEngine::isPersistent() const {
	return _persistent;
}
///////////////////////////////////////////////////////////// access /////////////////////////////////////////////
bool MatlabEngine::putVariable(const mxArrayWrapper& var){
	try{
		if (engPutVariable(_eng.get(), var.name.c_str(), var.getPtr()) != 0){
			stringstream ss;
			ss << "put varaible [cppVarName=" << var.name << "]" << " fails";
			throw exception(ss.str().c_str());
		}
		return true;
	}
	catch (exception& e){
		cout << "MatlabEngine::putVariable: " << e.what() << endl;
		return false;
	}
}
bool MatlabEngine::getVariable(mxArrayWrapper& var, const std::string& varName) const {
	try{
		mxArray* ptr;
		if ( ( ptr = engGetVariable(_eng.get(), varName.c_str()) ) == nullptr){
			stringstream ss;
			ss << "get variable [cppVarName=" << var.name << ", matlabVarName=" << varName << "]" << " fails";
			throw exception(ss.str().c_str());
		}
		var = mxArrayWrapper(ptr, varName);
		return true;
	}
	catch (exception& e){
		cout << "MatlabEngine::getVariable: " << e.what() << endl;
		return false;
	}
}

bool MatlabEngine::eval(std::string& outStr, const std::string& evalStr){
	outStr.clear();
	string matlabComplainStr;
#if 0
	matlabPrintBegin();
	if (engEvalString(_eng.get(), evalStr.c_str()) != 0){
		throw exception("MATLAB engine is invalid now");
	}
	if (hasBeenErr(matlabComplainStr)){
		stringstream ss;
		ss << "## MATLAB eval err" << endl;
		ss << "## eval str: " << evalStr << endl;
		ss << "## matlab complain: " << matlabComplainStr;
		outStr = ss.str();
		return false;
	}
	outStr = matlabPrintEnd();
	return true;
#else
	try{
		matlabPrintBegin();
		if (engEvalString(_eng.get(), evalStr.c_str()) != 0){
			throw exception("MATLAB engine is invalid now");
		}
		if (hasBeenErr(matlabComplainStr)){
			stringstream ss;
			ss << "## MATLAB eval err" << endl;
			ss << "## eval str: " << evalStr << endl;
			ss << "## matlab complain: " << matlabComplainStr;
			outStr = ss.str();
			return false;
		}
		outStr = matlabPrintEnd();
		return true;
	}
	catch (exception& e){
		cout << "MatlabEngine::eval: eval error occur" << endl;
		outStr = e.what();
		return false;
	}
#endif
}

std::string MatlabEngine::eval(const std::string& evalStr){
	string outStr;
	eval(outStr, evalStr);
	return outStr;
}

std::string MatlabEngine::evalVerbose(const std::string& evalStr){
	stringstream ss;
	ss << evalStr << endl <<  ">>" << endl << eval(evalStr);
	return ss.str();
}
///////////////////////////////////////////////////////////// error chk /////////////////////////////////////////////
bool MatlabEngine::hasBeenErr(std::string& matlabComplainStr){
	try{
		engEvalString(_eng.get(), "lasterr__ = lasterr;");
		mxArrayWrapper var;
		getVariable(var, "lasterr__");
		if (var.getNElems() != 0){
			// unreachable code
			if (!mxIsChar(var.getPtr())){ 
				throw exception("lasterr__ is not char!");
			}
			// reachable code
			else{
				matlabComplainStr = mxArrayToString(var.getPtr());
			}
			engEvalString(_eng.get(), "lasterror('reset');"); // clear lasterr;
			return true;
		}
		else{
			return false;
		}
	}
	catch (exception& e){
		cout << "MatlabEngine::hasBeenErr: " << e.what() << endl;
		engEvalString(_eng.get(), "lasterror('reset');");
		return true;
	}
}

void MatlabEngine::matlabPrintBegin(){
	try{
		_printBuf.clear();
		_printBuf.resize(_printBufSize, '\0');
		if (engOutputBuffer(_eng.get(), _printBuf.data(), _printBuf.size()) != 0){
			throw exception("MATLAB Engine is invalid now");
		}
	}
	catch (exception& e){
		cout << "MatlabEngine::printBegin: " << e.what();
	}
}

std::string MatlabEngine::matlabPrintEnd(){
	// null terminate하게 잘 만들기.
	// 처음에 \0 으로 다 채우고
	// 마지막에 \0도 하나 추가 pushback.
	try{
		string outputStr(_printBuf.begin(), _printBuf.end());
		outputStr.push_back('\0');
		int pos = outputStr.find_last_not_of('\0');
		if (pos >= 0){
			outputStr.erase(pos);
		}
		else{
			outputStr.clear();
		}
		return outputStr;
	}
	catch (exception& e){
		cout << "MatlabEngine::printEnd: " << e.what() << endl;
	}

	// MATLAB 에서는 띄어쓰기를 '\0'으로 넘겨주네... 미친
	//char buf2[_printBufSize+1];
	//memset(buf2, 0, sizeof(char)*(_printBufSize + 1));
	//for (int i = 0; i < _printBufSize; i ++){
	//	char c = _printBuf[i];
	//	if (c != '\0'){
	//		cout << "@" << c;
	//		buf2[i] = c;
	//	}
	//	else{
	//		buf2[i] = '\0';
	//		//break;
	//	}
	//}
	//return string(buf2);
}
///////////////////////////////////////////////////////////// set path /////////////////////////////////////////////
bool MatlabEngine::setCurrentPath(const std::string& currentPath){
	string outStr;
	try{
		stringstream ss;
		ss << "cd('" << currentPath << "');";
		if (!eval(outStr, ss.str())){
			throw exception(outStr.c_str());
		}
		this->currentPath = currentPath;
	}
	catch (exception& e){
		cout << "MatlabEngine::setCurrentPath: " << outStr << endl;
		return false;
	}
}
bool MatlabEngine::setIncludePath(const std::set<std::string>& includePath){
	string outStr;
	vector<string> outStrs;
	bool chk = true;
	try{
		
		for (const auto& seg : includePath){
			stringstream ss;
			ss << "addpath('" << seg << "');";
			chk &= eval(outStr, ss.str());
			if (!chk){
				outStrs.push_back(outStr);
			}
		}
		if (!chk){
			throw exception();
		}
		this->includePath = includePath;
		return chk;
	}
	catch (exception& e){
		cout << "MatlabEngine::setIncludePath: " << endl;
		for (const auto& str : outStrs){
			cout << ">>" << str << endl;
		}
	}
	return true;
}
bool MatlabEngine::setAllPath(const std::string& fpath){
	ifstream in = IOParser_v3::openInputStream(fpath);
	string currentPath_;
	set<string> includePath_;
	enum {UNKNOWN, CURRENT_PATH, INCLUDE_PATH};
	int kind = UNKNOWN;
	for (string line; std::getline(in, line, '\n'); ){
		if (line.empty() || RegexHelper::match(line, "\\s") ){
			continue;
		}
		else if (RegexHelper::match(line, "\\s*currentPath\\s*")){
			kind = CURRENT_PATH;
		}
		else if (RegexHelper::match(line, "\\s*includePath\\s*")){
			kind = INCLUDE_PATH;
		}
		else{
			switch (kind){
			case CURRENT_PATH:
				currentPath_ = RegexHelper::trim(line, "\\s|;");
				break;
			case INCLUDE_PATH:
				for (const auto& seg : RegexHelper::split(RegexHelper::trim(line), ";|\\n")){
					includePath_.insert(RegexHelper::trim(seg));
				}
				
				break;
			default:
				;
			}
		}
	}

	// set current path
	setCurrentPath(currentPath_);
	
	// set include path
	setIncludePath(includePath_);


	return true;
} 
