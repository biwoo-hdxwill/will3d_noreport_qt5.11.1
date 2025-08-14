#pragma once

/**=================================================================================================

Project:		CommonJobMgr
File:			cl_platform.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2017-12-01
Last modify: 	2017-12-01

Copyright (c) 2017 HDXWILL. All rights reserved.

*===============================================================================================**/

#include "clplatform_global.h"

#if defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#include <OpenCL/opencl.h>
typedef cl_uint cl_gl_context_info;
#else
#include <CL/cl.h>
#include <CL/cl_gl.h>
#endif

#include <exception>
#include <iostream>
#include <vector>
#include <memory>

class CLException : public std::exception {
public:
	// define exception ID.
	enum class EID {
		GPU_QUERY_FAILED,
		CREATE_PROGRAM_FAILED,
		BUILD_PROGRAM_FAILED,
		CREATE_KERNEL_FAILED,
		FILE_ERROR,
		UNKNOWN
	};

	CLException(EID eid = EID::UNKNOWN, std::string str = nullptr) {
		eid_ = eid;
		str_err_ = str;
	}

	inline const char* what() const throw() override { return str_err_.c_str(); }
	inline const EID ErrorType(void) const { return eid_; }

private:
	EID eid_;
	std::string	str_err_;
};

class CLPLATFORM_EXPORT CLplatform {
public:
	CLplatform(bool IsglSharing);
	~CLplatform();

	bool checkError(const cl_int& status, const int lineNum, const char* errMsg) const;
	static void printError(const cl_int err);
	cl_context			getContext() const { return context_; };
	cl_device_id		getDevice() const { return device_id_; }
	cl_command_queue	getCommandQueue() const { return command_queue_; };

	inline const bool support_gl_sharing() { return support_gl_sharing_; }
	inline const bool is_valid() { return is_valid_; }

private:
	void InitializeGLsharing();
	void InitializeGLbestDevice();
	void PrintOpenCLVariables();

	void GetPlatformInfo(cl_platform_id* platform_ids, cl_uint* num_platforms);
	void GetBestPlatform(const cl_platform_id* platform_ids, const cl_uint& num_platforms, cl_platform_id& best_platform_id);
	bool IsSharingSupport(const cl_platform_id& platform_id);
	void CreateCLcontextGLsharing(const cl_platform_id& platform_id);
	void CreateCLcontext(const cl_platform_id& platform_id);
	void CreateCommandQueue();
	float PrintCLmemAllocSize();
	cl_uint PrintCLcomputeUnits();
	cl_uint PrintCLworkItemDimensions();
	std::vector<size_t> PrintCLworkItemSize(const cl_uint& num_work_dimensions);

	bool IsGPUplatform(char * platform_name) const;
	bool IsDeviceAvailableSharing() const;

private:
	cl_context			context_;		// context.
	cl_device_id		device_id_;		// device ID.
	cl_command_queue	command_queue_;	// command queue.

	bool support_gl_sharing_ = false;
	bool is_valid_ = false;
};
