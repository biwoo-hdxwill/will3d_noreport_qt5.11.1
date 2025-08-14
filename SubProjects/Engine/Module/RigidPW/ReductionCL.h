#pragma once
/*=========================================================================

File:			class ReductionCL
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-12-18
Last modify:	2015-12-18

=========================================================================*/
#if defined(__APPLE__)
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

#include "../../Core/CLPlatform/WCLprogram.h"
#include "rigidpw_global.h"
class CLplatform;

class RIGIDPW_EXPORT ReductionCL : public WCLprogram
{
public:
	ReductionCL(CLplatform *OCL);
	~ReductionCL();

	float runReduction(float *data, int N);
	float runReduction(cl_mem *data, int N);

private:
	unsigned int nextPow2(unsigned int x); // x 이상이면서 2 의 제곱수
	bool isPow2(unsigned int x);
	void getNumBlocksAndThreads(int &blocks, int &threads, int N, bool init);
	void checkError(const cl_int& status, const int lineNum, const char* errMsg);

private:
	const int m_kMaxThread = 128;
	int m_maxBlock = 0;
	
	const unsigned int m_MAX_BLOCK_DIM_SIZE = 0;

	cl_kernel m_kernel_sumReduction1CL = 0;
	cl_kernel m_kernel_sumReduction2CL = 0;
};
