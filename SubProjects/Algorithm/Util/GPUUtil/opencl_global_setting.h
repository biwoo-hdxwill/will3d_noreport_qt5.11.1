#pragma once
#include <exception>
#include <sstream>
#include <string>

#include <Util/Core/util_global.h>
#if defined(__APPLE__)
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif
UTIL_EXPORT const char* translateOpenCLErr(cl_int errorCode);

#define cle(err)\
if((err) != CL_SUCCESS){\
	stringstream ss;\
	ss << "## opencl err at " << __FILE__ << "(" << __LINE__ << "):\n\t@code: "\
	<< #err << "\n\t@err: " << translateOpenCLErr(err) << std::endl; \
	throw std::runtime_error(ss.str().c_str());\
}

#define cle2(eval, err)\
eval;\
if((err) != CL_SUCCESS){\
	stringstream ss;\
	ss << "## opencl err at " << __FILE__ << "(" << __LINE__ << "):\n\t@code: "\
	<< #err << "\n\t@err: " << translateOpenCLErr(err) << std::endl; \
	throw std::runtime_error(ss.str().c_str());\
}

#define cl_kernel_loop(i, n)\
for (int i = get_global_id(0); i < (n); i += get_global_size(0))
