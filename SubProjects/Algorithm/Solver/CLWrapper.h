#pragma once
#include "solver_global.h"
#include <Util/GPUUtil/opencl_global_setting.h>
#include <string>
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class	CLWrapper
///
/// @brief	A cl wrapper.
/// @detail for example
/// 		# init:
/// 			CLWrapper clw("NVIDIA", CL_DEVICE_TYPE_GPU);
/// 			auto& context = clw.context;
/// 			auto& queue = clw.queue;
/// 			cl::Program program;
/// 			clw.programWithFile(program, "add.cl");
/// 		# Buffer:
/// 			cle2(cl::Buffer d_out = cl::Buffer(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
/// 			 sizeof(float)*size, c_out.data(), &err), err);
/// 			cle2(cl::Buffer d_in1 = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
/// 			 sizeof(float)*size, c_in1.data(), &err), err);
/// 		# Kernel:
/// 			cle2(cl::Kernel kernel(program, "add2", &err), err);
/// 			cle(kernel.setArg(0, d_out));
///				cle(kernel.setArg(1, d_in1));
///				cle(kernel.setArg(2, d_in2));
///				cle(kernel.setArg(3, size));
///			# queue:
///				int global_size = 10;
///				cle(queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(global_size)));
///			# read back:
///				cle(queue.enqueueReadBuffer(d_out /*device*/, CL_TRUE /*sync*/, 0 /*offset*/, sizeof(float)*size /*memsize*/, c_out.data() /*host*/));
///			# there is no need release object.
/// @author	Hosan
/// @date	2016-06-20
///////////////////////////////////////////////////////////////////////////////////////////////////
class SOLVER_EXPORT CLWrapper {
public: /* members */
	cl::Platform platform;
	cl::Device device;
	cl::Context context;
	cl::CommandQueue queue;
protected: /* methods */
	bool _constr(
		const std::string targetPlatform,
		const cl_device_type& targetDeviceType
	);
public: /* methods */
	CLWrapper();
	CLWrapper(
		const std::string targetPlatform,
		const cl_device_type& targetDeviceType
	);
	bool programWithSrc(cl::Program& program, const std::string& src);
	bool programWithFile(cl::Program& program, const std::string& fpath);
};
///////////////////////////////////////////////////////////////////////////////////////////////////
// full ex:
// /* add.cl */
//#include "opencl_kernel_setting.h"
//
//__kernel void add2(
//	__global float* d_out,
//	__global float* d_in1,
//	__global float* d_in2,
//	int size
//) {
//	cl_kernel_loop(i, size) {
//		d_out[i] = d_in1[i] + d_in2[i];
//	}
//}
// 
// /* main.cpp */
//int main() {
//	try {
//		/* data */
//		int size = 10;
//		vector<float> c_out(size);
//		vector<float> c_in1(size);
//		vector<float> c_in2(size);
//		for (int i = 0; i < size; i++) {
//			c_in1[i] = i;
//			c_in2[i] = i;
//		}
//		/* init */
//		cl_int err;
//		int global_size = 10;
//		CLWrapper clw("NVIDIA", CL_DEVICE_TYPE_GPU);
//		auto& context = clw.context;
//		auto& queue = clw.queue;
//		cl::Program program;
//		clw.programWithFile(program, "add.cl");
//		/* buffer */
//		cle2(cl::Buffer d_out = cl::Buffer(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
//			sizeof(float)*size, c_out.data(), &err), err);
//		cle2(cl::Buffer d_in1 = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
//			sizeof(float)*size, c_in1.data(), &err), err);
//		cle2(cl::Buffer d_in2 = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
//			sizeof(float)*size, c_in2.data(), &err), err);
//		/* kernel */
//		cle2(cl::Kernel kernel(program, "add2", &err), err);
//		cle(kernel.setArg(0, d_out));
//		cle(kernel.setArg(1, d_in1));
//		cle(kernel.setArg(2, d_in2));
//		cle(kernel.setArg(3, size));
//		/* queue */
//		cle(queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(global_size)));
//		/* read back */
//		cle(queue.enqueueReadBuffer(d_out, CL_TRUE, 0, sizeof(float)*size, c_out.data()));
//
//		cout << c_out << endl;
//	}
//	catch (runtime_error& e) {
//		cout << "main: " << e.what() << endl;
//	}
//
//	return 0;
//}
///////////////////////////////////////////////////////////////////////////////////////////////////
