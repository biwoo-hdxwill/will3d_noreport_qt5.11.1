#pragma once

#ifdef _WIN64
#include <Util/Core/util_global.h>
#include "opencl_global_setting.h"
#if defined(__APPLE__)
#include <clSPARSE/clSPARSE.h>
#include <clSPARSE/clSPARSE-error.h>
#else
#include <clSPARSE.h>
#include <clSPARSE-error.h>
#endif

UTIL_EXPORT const char* clsparseError(clsparseStatus err);
#define cls(err)\
if((err) != clsparseSuccess){\
	stringstream ss;\
	ss << "## clsparse err at " << __FILE__ << "(" << __LINE__ << "):\n\t@code: "\
	<< #err << "\n\t@err: " << translateOpenCLErr(err) << std::endl; \
    throw std::runtime_error(ss.str().c_str());\
}

//#define cls2(eval, err)\
//eval;\
//if((err) != clsparseSuccess){\
//	stringstream ss;\
//	ss << "## clsparse err at " << __FILE__ << "(" << __LINE__ << "):\n\t@code: "\
//	<< #err << "\n\t@err: " << translateOpenCLErr(err) << std::endl; \
//	throw std::exception(ss.str().c_str());\
//}
#else
#endif
