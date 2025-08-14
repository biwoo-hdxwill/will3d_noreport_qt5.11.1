#include "WCLprogram.h"

#include <fstream>
#include <sstream>

#include <QString>
#include <QFile>
#include <QTextStream>

#include "../../Common/Common/W3Logger.h"
#include "WCLplatform.h"

WCLprogram::WCLprogram(CLplatform *OCL)
	: m_pOCL(OCL)
{
	m_program = 0;
}

WCLprogram::~WCLprogram()
{
	if(!m_program)
		clReleaseProgram(m_program);
}

void WCLprogram::buildProgramFromFile(const char *fileName)
{
	if (!m_pOCL->is_valid())
	{
		return;
	}

	// read from file
	std::ifstream inFile(fileName, std::ios::in);
	if( !inFile.is_open() ) {
		common::Logger::instance()->Print(common::LogType::ERR,
			std::string("fopen. file : ") + std::string(fileName));

		common::Logger::instance()->Print(common::LogType::ERR,
			std::string("kernel program file open failed."));
		exit(1);
	}

	std::ostringstream oss;
    oss << inFile.rdbuf();

    std::string srcStdStr = oss.str();
    char *source = (char *)srcStdStr.c_str();

	// release if allocated.
	if(m_program != (cl_program)0 ) 
	{
		clReleaseProgram(m_program);
		m_program = (cl_program)0;
	}

	// create program object
	cl_int status;
	m_program = clCreateProgramWithSource(m_pOCL->getContext(), 1, (const char**)&source, nullptr, &status);
	if(m_program == (cl_program)0) 
	{
		CLplatform::printError(status);
		common::Logger::instance()->Print(common::LogType::ERR,
			std::string("failed to create program object."));
		exit(1);
	}

	// build option.
	//std::stringstream ss;
	//ss << " -DdistCamPlane="<<distCamPlane<<" -DpixelSpacing="<<pixelSpacing<<" -DsliceSpacing="<<sliceSpacing;

	cl_device_id deviceID = m_pOCL->getDevice();
	status = clBuildProgram(m_program, 1, &deviceID, nullptr, nullptr, nullptr);
	if(status != CL_SUCCESS) 
	{
		CLplatform::printError(status);
		showBuildLog(m_program, deviceID);
		common::Logger::instance()->Print(common::LogType::ERR,
			std::string("failed to build program object."));
		exit(1);
	}
}

void WCLprogram::buildProgramFromFile(QString fileName)
{
	if (!m_pOCL->is_valid())
	{
		return;
	}

	// read from file
	QFile cl(fileName);
	if (!cl.open(QIODevice::ReadOnly))
	{
		common::Logger::instance()->Print(common::LogType::ERR,
			std::string("fopen. file : ") + fileName.toStdString());
		common::Logger::instance()->Print(common::LogType::ERR,
			std::string("kernel program file open failed."));
		exit(1);
	}

	QTextStream stream(&cl);

	QString code = stream.readAll();
	cl.close();

	std::string srcStdStr = code.toStdString();
	char *source = (char *)srcStdStr.c_str();

	// release if allocated.
	if (m_program != (cl_program)0)
	{
		clReleaseProgram(m_program);
		m_program = (cl_program)0;
	}

	// create program object
	cl_int status;
	m_program = clCreateProgramWithSource(m_pOCL->getContext(), 1, (const char**)&source, nullptr, &status);
	if (m_program == (cl_program)0)
	{
		CLplatform::printError(status);
		common::Logger::instance()->Print(common::LogType::ERR,
			std::string("failed to create program object."));
		exit(1);
	}

	// build option.
	//std::stringstream ss;
	//ss << " -DdistCamPlane="<<distCamPlane<<" -DpixelSpacing="<<pixelSpacing<<" -DsliceSpacing="<<sliceSpacing;

	cl_device_id deviceID = m_pOCL->getDevice();
	status = clBuildProgram(m_program, 1, &deviceID, nullptr, nullptr, nullptr);
	if (status != CL_SUCCESS)
	{
		CLplatform::printError(status);
		showBuildLog(m_program, deviceID);
		common::Logger::instance()->Print(common::LogType::ERR,
			std::string("failed to create program object."));
		exit(1);
	}
}

void WCLprogram::showBuildLog(const cl_program program, const cl_device_id device) const
{
	if (!m_pOCL->is_valid())
	{
		return;
	}

	cl_int status;
	size_t size_ret;
	char buffer[4096];
	status = clGetProgramBuildInfo(
		program,
		device,
		CL_PROGRAM_BUILD_LOG,
		sizeof(buffer)-1,
		buffer,
		&size_ret);
	if(status != CL_SUCCESS) 
	{
		common::Logger::instance()->Print(common::LogType::ERR, "clGetProgramInfo failed.");
		//std::cerr << "clGetProgramInfo failed." << std::endl;
		CLplatform::printError(status);
	}
	else 
	{
		buffer[size_ret] = '\0';
		std::stringstream ss;
		ss << "build log :" << buffer << ">>>end of build log <<<" <<std::endl;
		common::Logger::instance()->Print(common::LogType::INF, ss.str());
	}
}

const bool WCLprogram::IsValid()
{
	if (!m_pOCL)
	{
		return false;
	}

	return m_pOCL->is_valid();
}
