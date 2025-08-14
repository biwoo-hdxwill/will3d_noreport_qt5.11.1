#include "ReductionCL.h"
/*=========================================================================

File:			class ReductionCL
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-12-18
Last modify:	2015-12-18

=========================================================================*/
#include <QString>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Logger.h"

#include "../../Core/CLPlatform/WCLplatform.h"

ReductionCL::ReductionCL(CLplatform *OCL)
	: WCLprogram(OCL), m_MAX_BLOCK_DIM_SIZE(65535)
{
	if (!m_pOCL->is_valid())
	{
		return;
	}

	Q_INIT_RESOURCE(cl);

	//buildProgramFromFile("../../SubProjects/Managers/JobMgr/cl/Reduction.cl");
	buildProgramFromFile(QString(":/cl/Reduction.cl"));

	cl_int status;
	m_kernel_sumReduction1CL = clCreateKernel(m_program, "sumReduction1CL", &status);
	m_kernel_sumReduction2CL = clCreateKernel(m_program, "sumReduction2CL", &status);
	checkError(status, __LINE__, "clCreateKernel failed.");
}

ReductionCL::~ReductionCL()
{
	if(m_kernel_sumReduction1CL)
	{
		clReleaseKernel(m_kernel_sumReduction1CL);
		m_kernel_sumReduction1CL = 0;
	}

	if(m_kernel_sumReduction2CL)
	{
		clReleaseKernel(m_kernel_sumReduction2CL);
		m_kernel_sumReduction2CL = 0;
	}
}

bool ReductionCL::isPow2(unsigned int x)
{
    return ((x&(x-1))==0);
}

unsigned int ReductionCL::nextPow2(unsigned int x)
{
	--x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;

	return ++x;
}

void ReductionCL::getNumBlocksAndThreads(int &blocks, int &threads, int N, bool init)
{
	threads = (N < m_kMaxThread * 2) ? nextPow2((N + 1) / 2) : m_kMaxThread;
    blocks = (N + (threads * 2 - 1)) / (threads * 2);
    
	if(init)
		blocks = std::min(m_maxBlock, blocks);
}

float ReductionCL::runReduction(float *data, int N)
{
	float result = 0.0f;
	if (!m_pOCL->is_valid())
	{
		return result;
	}

	cl_int status;
	cl_mem mdata;
	
	mdata = clCreateBuffer(
		m_pOCL->getContext(),
		CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		sizeof(cl_float)*N,
		data,
		&status);

	checkError(status, __LINE__, "clCreateBuffer failed.");

	m_maxBlock = std::min(unsigned int(N / m_kMaxThread), m_MAX_BLOCK_DIM_SIZE);
	m_maxBlock = std::max(m_maxBlock, 1);
	
	int numBlocks, numThreads;
	getNumBlocksAndThreads(numBlocks, numThreads, N, true);

	unsigned int bisPow2 = isPow2(N);

	status = clSetKernelArg(m_kernel_sumReduction1CL, 0, sizeof(cl_mem), &mdata);
	status |= clSetKernelArg(m_kernel_sumReduction1CL, 1, sizeof(cl_mem), &mdata);
	status |= clSetKernelArg(m_kernel_sumReduction1CL, 2, sizeof(cl_uint), &N);
	status |= clSetKernelArg(m_kernel_sumReduction1CL, 3, sizeof(cl_float)*numThreads, nullptr);
	status |= clSetKernelArg(m_kernel_sumReduction1CL, 4, sizeof(cl_uint), &numThreads);
	status |= clSetKernelArg(m_kernel_sumReduction1CL, 5, sizeof(cl_uint), &bisPow2);
	checkError(status, __LINE__, "clSetKernelArg failed.");

	size_t globalWorkSize[1];
	globalWorkSize[0] = numBlocks*numThreads;
	size_t localWorkSize[1];
	localWorkSize[0] = numThreads;

	status = clEnqueueNDRangeKernel(m_pOCL->getCommandQueue(), m_kernel_sumReduction1CL, 1, nullptr, globalWorkSize, localWorkSize, 0, nullptr, nullptr);
	checkError(status, __LINE__, "clEnqueueNDRangeKernel failed.");

	unsigned int s = numBlocks;
	int numBlocks2, numThreads2;
	while(s > 1)
	{
		getNumBlocksAndThreads(numBlocks2, numThreads2, s, false);

		bisPow2 = isPow2(s);

		status = clSetKernelArg(m_kernel_sumReduction2CL, 0, sizeof(cl_mem), &mdata);
		status |= clSetKernelArg(m_kernel_sumReduction2CL, 1, sizeof(cl_mem), &mdata);
		status |= clSetKernelArg(m_kernel_sumReduction2CL, 2, sizeof(cl_uint), &s);
		status |= clSetKernelArg(m_kernel_sumReduction2CL, 3, sizeof(cl_float)*numThreads2, nullptr);
		status |= clSetKernelArg(m_kernel_sumReduction2CL, 4, sizeof(cl_uint), &numThreads2);
		status |= clSetKernelArg(m_kernel_sumReduction2CL, 5, sizeof(cl_uint), &bisPow2);

		globalWorkSize[0] = numBlocks2*numThreads2;
		localWorkSize[0] = numThreads2;

		status |= clEnqueueNDRangeKernel(m_pOCL->getCommandQueue(), m_kernel_sumReduction2CL, 1, 0, globalWorkSize, localWorkSize, 0, nullptr, nullptr);

		s = (s + (numThreads2*2 - 1))/(numThreads2*2);
	}
	checkError(status, __LINE__, "clEnqueueNDRangeKernel failed.");

	clFinish(m_pOCL->getCommandQueue());

	status = clEnqueueReadBuffer(m_pOCL->getCommandQueue(), mdata, CL_TRUE, 0, sizeof(float), &result, 0, NULL, NULL);
	checkError(status, __LINE__, "clEnqueueReadBuffer failed.");

	clReleaseMemObject(mdata);

	return result;
}

float ReductionCL::runReduction(cl_mem *mdata, int N)
{
	float result = 0.0f;
	if (!m_pOCL->is_valid())
	{
		return result;
	}

	m_maxBlock = std::min(unsigned int(N / m_kMaxThread), m_MAX_BLOCK_DIM_SIZE);
	m_maxBlock = std::max(m_maxBlock, 1);

	cl_int status;
	cl_mem mdata2;
	mdata2 = clCreateBuffer(
			m_pOCL->getContext(),
			CL_MEM_READ_WRITE,
			sizeof(cl_float)*m_maxBlock,
			nullptr,
			&status);

	checkError(status, __LINE__, "clCreateBuffer failed.");

	int numBlocks, numThreads;
	getNumBlocksAndThreads(numBlocks, numThreads, N, true);

	unsigned int bisPow2 = isPow2(N);

	status = clSetKernelArg(m_kernel_sumReduction1CL, 0, sizeof(cl_mem), &mdata2);
	status |= clSetKernelArg(m_kernel_sumReduction1CL, 1, sizeof(cl_mem), mdata);
	status |= clSetKernelArg(m_kernel_sumReduction1CL, 2, sizeof(cl_uint), &N);
	status |= clSetKernelArg(m_kernel_sumReduction1CL, 3, sizeof(cl_float)*numThreads, nullptr);
	status |= clSetKernelArg(m_kernel_sumReduction1CL, 4, sizeof(cl_uint), &numThreads);
	status |= clSetKernelArg(m_kernel_sumReduction1CL, 5, sizeof(cl_uint), &bisPow2);
	checkError(status, __LINE__, "clSetKernelArg failed.");

	size_t globalWorkSize[1];
	size_t localWorkSize[1];

	globalWorkSize[0] = numBlocks*numThreads;
	localWorkSize[0] = numThreads;

	status = clEnqueueNDRangeKernel(m_pOCL->getCommandQueue(), m_kernel_sumReduction1CL, 1, nullptr, globalWorkSize, localWorkSize, 0, nullptr, nullptr);
	checkError(status, __LINE__, "clEnqueueNDRangeKernel failed.");

	unsigned int s = numBlocks;
	int numBlocks2, numThreads2;
	while(s > 1)
	{
		getNumBlocksAndThreads(numBlocks2, numThreads2, s, false);

		bisPow2 = isPow2(s);

		status = clSetKernelArg(m_kernel_sumReduction2CL, 0, sizeof(cl_mem), &mdata2);
		status |= clSetKernelArg(m_kernel_sumReduction2CL, 1, sizeof(cl_mem), &mdata2);
		status |= clSetKernelArg(m_kernel_sumReduction2CL, 2, sizeof(cl_uint), &s);
		status |= clSetKernelArg(m_kernel_sumReduction2CL, 3, sizeof(cl_float)*numThreads2, nullptr);
		status |= clSetKernelArg(m_kernel_sumReduction2CL, 4, sizeof(cl_uint), &numThreads2);
		status |= clSetKernelArg(m_kernel_sumReduction2CL, 5, sizeof(cl_uint), &bisPow2);

		globalWorkSize[0] = numBlocks2*numThreads2;
		localWorkSize[0] = numThreads2;

		status |= clEnqueueNDRangeKernel(m_pOCL->getCommandQueue(), m_kernel_sumReduction2CL, 1, 0, globalWorkSize, localWorkSize, 0, nullptr, nullptr);

		s = (s + (numThreads2*2 - 1))/(numThreads2*2);
	}
	checkError(status, __LINE__, "clEnqueueNDRangeKernel failed.");

	clFinish(m_pOCL->getCommandQueue());

	status = clEnqueueReadBuffer(m_pOCL->getCommandQueue(), mdata2, CL_TRUE, 0, sizeof(float), &result, 0, NULL, NULL);
	checkError(status, __LINE__, "clEnqueueReadBuffer failed.");

	clReleaseMemObject(mdata2);

	return result;
}

void ReductionCL::checkError(const cl_int& status, const int lineNum, const char* errMsg)
{
	if (status != CL_SUCCESS)
	{
		std::string errMsg =
			std::string("[line: ")
			+ std::to_string(lineNum)
			+ std::string(" ] ")
			+ errMsg;
		common::Logger::instance()->Print(common::LogType::ERR, errMsg);
		CLplatform::printError(status);
		exit(1);
	}
}
