#pragma once
#if defined(__APPLE__)
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include "clplatform_global.h"

class CLplatform;

class CLPLATFORM_EXPORT WCLprogram
{
public:
	WCLprogram(CLplatform *OCL);
	~WCLprogram();

	void buildProgramFromFile(const char *fileName);
	void buildProgramFromFile(QString fileName);

	const bool IsValid();

private:
	void showBuildLog(const cl_program program, const cl_device_id device) const;

protected:
	CLplatform *m_pOCL;
	cl_program m_program;
};
