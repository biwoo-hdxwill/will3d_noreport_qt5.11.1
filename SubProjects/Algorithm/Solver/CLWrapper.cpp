#include "CLWrapper.h"
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

bool CLWrapper::_constr(
	const std::string targetPlatform,
	const cl_device_type& targetDeviceType
){
	try {
		cl_int err;
		bool resPlatform = false;
		bool resDevice = false;
		vector<cl::Platform> platforms;
		vector<cl::Device> devices;
		cle(cl::Platform::get(&platforms)); //

		if (platforms.size() < 1)
		{
			throw runtime_error("platform size < 1");
		}

		cout << "platform:" << endl;
		for (const auto& p : platforms) {
			string str;
			cle(p.getInfo(CL_PLATFORM_NAME, &str)); //
			if (str.find(targetPlatform) != string::npos) {
				cout << "\t" << "CL_PLATFORM_NAME=" << str << " [selected]" << endl;
				resPlatform = true;
				platform = p;
			}
			else {
				cout << "\t" << "CL_PLATFORM_NAME=" << str << endl;
			}
		}

		if (!resPlatform) {
#if 0
			throw runtime_error("platform selection fail");
#else
			resPlatform = true;
			platform = platforms.at(0);
#endif
		}

		cout << "device:" << endl;
		cle(platform.getDevices(targetDeviceType, &devices)); //
		for (const auto& d : devices) {
			string str;
			cle(d.getInfo(CL_DEVICE_NAME, &str)); //
			if (!resDevice) {
				resDevice = true;
				device = d;
				cout << "CL_DEVICE_NAME=" << str << " [selected]" << endl;
			}
			else {
				cout << "CL_DEVICE_NAME=" << str << endl;
			}
		}

		if (!resDevice) {
			throw runtime_error("device selection fail");
		}

		cl_context_properties ctxProp[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform(), 0 };
		cle2(context = cl::Context(targetDeviceType, ctxProp, 0, 0, &err), err);

		cl_command_queue_properties queueProps = CL_QUEUE_PROFILING_ENABLE;
		cle2(queue = cl::CommandQueue(context, device, queueProps, &err), err);

		return true;
	}
	catch (runtime_error& e) {
		cout << "CLWrapper::_constr: " << e.what() << endl;
		return false;
	}

}
CLWrapper::CLWrapper(){
#if defined(__APPLE__)
    _constr("Apple", CL_DEVICE_TYPE_GPU);
#else
	_constr("NVIDIA", CL_DEVICE_TYPE_GPU);
	//_constr("Intel", CL_DEVICE_TYPE_CPU);
#endif
}
CLWrapper::CLWrapper(
	const std::string targetPlatform,
	const cl_device_type& targetDeviceType
){
	_constr(targetPlatform, targetDeviceType);
}
bool CLWrapper::programWithSrc(cl::Program& program, const std::string& src) {
	try {
		cl_int err;
		program = cl::Program(context, src, true, &err);
		if (err == CL_BUILD_PROGRAM_FAILURE) {
			string buildLog;
			cle(program.getBuildInfo<string>(device, CL_PROGRAM_BUILD_LOG, &buildLog));
			stringstream ss;
			ss << "build err at " << __FILE__ << " (" << __LINE__ << ")" << endl
				<< "buildLog: " << buildLog << endl;
			throw runtime_error(ss.str().c_str());
		}
		cle(err);
		return true;
	}
	catch (runtime_error& e) {
		cout << "CLWrapper::programWithSrc: " << e.what() << endl;
		return false;
	}
}
bool CLWrapper::programWithFile(cl::Program& program, const std::string& fpath) {
	try {
		string src;
		ifstream fsrc(fpath);
		if (!fsrc) {
			stringstream ss;
			ss << fpath << " open err" << endl;
			throw runtime_error(ss.str().c_str());
		}
		stringstream ss;
		ss << fsrc.rdbuf();
		src = ss.str();
		return programWithSrc(program, src);
	}
	catch (runtime_error& e) {
		cout << "CLWrapper::programWithFile: " << e.what() << endl;
		return false;
	}
}
