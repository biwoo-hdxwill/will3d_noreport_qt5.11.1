#include "clsparse_global_setting.h"


#ifdef _WIN64
const char* clsparseError(clsparseStatus err) {
	switch (err) {
	case clsparseSuccess: return "clsparseSuccess";
	case clsparseInvalidValue: return "clsparseInvalidValue";
	case clsparseInvalidCommandQueue: return "clsparseInvalidCommandQueue";
	case clsparseInvalidContext: return "clsparseInvalidContext";
	case clsparseInvalidMemObject: return "clsparseInvalidMemObject";
	case clsparseInvalidDevice: return "clsparseInvalidDevice";
	case clsparseInvalidEventWaitList: return "clsparseInvalidEventWaitList";
	case clsparseInvalidEvent: return "clsparseInvalidEvent";
	case clsparseOutOfResources: return "clsparseOutOfResources";
	case clsparseOutOfHostMemory: return "clsparseOutOfHostMemory";
	case clsparseInvalidOperation: return "clsparseInvalidOperation";
	case clsparseCompilerNotAvailable: return "clsparseCompilerNotAvailable";
	case clsparseBuildProgramFailure: return "clsparseBuildProgramFailure";
	case clsparseInvalidKernelArgs: return "clsparseInvalidKernelArgs";
	case clsparseNotImplemented: return "clsparseNotImplemented";
	case clsparseNotInitialized: return "clsparseNotInitialized";
	case clsparseStructInvalid: return "clsparseStructInvalid";
	case clsparseInvalidSize: return "clsparseInvalidSize";
	case clsparseInvalidMemObj: return "clsparseInvalidMemObj";
	case clsparseInsufficientMemory: return "clsparseInsufficientMemory";
	case clsparseInvalidControlObject: return "clsparseInvalidControlObject";
	case clsparseInvalidFile: return "clsparseInvalidFile";
	case clsparseInvalidFileFormat: return "clsparseInvalidFileFormat";
	case clsparseInvalidKernelExecution: return "clsparseInvalidKernelExecution";
	case clsparseInvalidType: return "clsparseInvalidType";
	case clsparseInvalidSolverControlObject: return "clsparseInvalidSolverControlObject";
	case clsparseInvalidSystemSize: return "clsparseInvalidSystemSize";
	case clsparseIterationsExceeded: return "clsparseIterationsExceeded";
	case clsparseToleranceNotReached: return "clsparseToleranceNotReached";
	case clsparseSolverError: return "clsparseSolverError";
	default: return "clsparseUnknown err";
	}
}
#else
#endif
