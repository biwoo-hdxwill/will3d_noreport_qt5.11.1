#include "WCLplatform.h"
#include <windows.h>

#include <sstream>

#include "../../Common/Common/W3Logger.h"

#if defined (__APPLE__) || defined(MACOSX)
#define GL_SHARING_EXTENSION "cl_APPLE_gl_sharing"
#else
#include <windows.h>
#define GL_SHARING_EXTENSION "cl_khr_gl_sharing"
#endif

using common::LogType;

namespace {
	const int kMaxPlatformNum = 5;
	const int kCharNameMaxSize = 10240;
}

CLplatform::CLplatform(bool IsGLsharing) {
	context_ = (cl_context)0;
	device_id_ = (cl_device_id)0;
	command_queue_ = (cl_command_queue)0;
	common::Logger* logger = common::Logger::instance();

	logger->Print(LogType::INF, "--------------<OpenCL Infos>--------------");

	if (IsGLsharing) {
		InitializeGLsharing();
	} else {
		InitializeGLbestDevice();
	}
	
	PrintOpenCLVariables();

	logger->Print(LogType::INF, "------------------------------------------");
}

CLplatform::~CLplatform() {
	if (context_ != (cl_context)0) {
		clReleaseContext(context_);
		context_ = (cl_context)0;
	}

	if (command_queue_ != (cl_command_queue)0) {
		clReleaseCommandQueue(command_queue_);
		command_queue_ = (cl_command_queue)0;
	}
}

bool CLplatform::checkError(const cl_int& status, const int lineNum, const char* errMsg) const {
	if (status != CL_SUCCESS) {
		std::string full_err_msg =
			std::string("[line: ")
			+ std::to_string(lineNum)
			+ std::string(" ] ")
			+ std::string(errMsg);
		common::Logger::instance()->Print(common::LogType::INF, full_err_msg);
		printError(status);
		//exit(1);

		return false;
	}

	return true;
}

void CLplatform::printError(const cl_int err) {
	using common::Logger;
	using common::LogType;
	Logger* logger = Logger::instance();
	switch (err) {
		case CL_BUILD_PROGRAM_FAILURE:
			logger->Print(LogType::ERR, std::string("Program build failed."));						break;
		case CL_COMPILER_NOT_AVAILABLE:
			logger->Print(LogType::ERR, std::string("OpenCL compiler is not available."));			break;
		case CL_DEVICE_NOT_AVAILABLE:
			logger->Print(LogType::ERR, std::string("Device is not avilable."));					break;
		case CL_DEVICE_NOT_FOUND:
			logger->Print(LogType::ERR, std::string("Device not found."));							break;
		case CL_IMAGE_FORMAT_NOT_SUPPORTED:
			logger->Print(LogType::ERR, std::string("Image format is not supported."));			break;
		case CL_IMAGE_FORMAT_MISMATCH:
			logger->Print(LogType::ERR, std::string("Image format mismatch."));					break;
		case CL_INVALID_ARG_INDEX:
			logger->Print(LogType::ERR, std::string("Invalid arg index."));						break;
		case CL_INVALID_ARG_SIZE:
			logger->Print(LogType::ERR, std::string("Invalid arg size."));							break;
		case CL_INVALID_ARG_VALUE:
			logger->Print(LogType::ERR, std::string("Invalid arg value."));						break;
		case CL_INVALID_BINARY:
			logger->Print(LogType::ERR, std::string("Invalid binary."));							break;
		case CL_INVALID_BUFFER_SIZE:
			logger->Print(LogType::ERR, std::string("Invalid buffer size."));						break;
		case CL_INVALID_BUILD_OPTIONS:
			logger->Print(LogType::ERR, std::string("Invalid build options."));					break;
		case CL_INVALID_COMMAND_QUEUE:
			logger->Print(LogType::ERR, std::string("Invalid command queue."));					break;
		case CL_INVALID_CONTEXT:
			logger->Print(LogType::ERR, std::string("Invalid context."));							break;
		case CL_INVALID_DEVICE:
			logger->Print(LogType::ERR, std::string("Invalid device."));							break;
		case CL_INVALID_DEVICE_TYPE:
			logger->Print(LogType::ERR, std::string("Invalid device type."));						break;
		case CL_INVALID_EVENT:
			logger->Print(LogType::ERR, std::string("Invalid event."));							break;
		case CL_INVALID_EVENT_WAIT_LIST:
			logger->Print(LogType::ERR, std::string("Invalid event wait list."));					break;
		case CL_INVALID_GL_OBJECT:
			logger->Print(LogType::ERR, std::string("Invalid OpenGL object."));					break;
		case CL_INVALID_GLOBAL_OFFSET:
			logger->Print(LogType::ERR, std::string("Invalid global offset."));					break;
		case CL_INVALID_HOST_PTR:
			logger->Print(LogType::ERR, std::string("Invalid host pointer."));						break;
		case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
			logger->Print(LogType::ERR, std::string("Invalid image format descriptor."));			break;
		case CL_INVALID_IMAGE_SIZE:
			logger->Print(LogType::ERR, std::string("Invalid image size."));						break;
		case CL_INVALID_KERNEL:
			logger->Print(LogType::ERR, std::string("Invalid kernel."));							break;
		case CL_INVALID_KERNEL_ARGS:
			logger->Print(LogType::ERR, std::string("Invalid kernel args."));						break;
		case CL_INVALID_KERNEL_DEFINITION:
			logger->Print(LogType::ERR, std::string("Invalid kernel definition."));				break;
		case CL_INVALID_KERNEL_NAME:
			logger->Print(LogType::ERR, std::string("Invalid kernel name."));						break;
		case CL_INVALID_MEM_OBJECT:
			logger->Print(LogType::ERR, std::string("Invalid memory object."));					break;
		case CL_INVALID_MIP_LEVEL:
			logger->Print(LogType::ERR, std::string("Invalid MIP level."));						break;
		case CL_INVALID_OPERATION:
			logger->Print(LogType::ERR, std::string("Invalid operation."));						break;
		case CL_INVALID_PLATFORM:
			logger->Print(LogType::ERR, std::string("Invalid platform."));							break;
		case CL_INVALID_PROGRAM:
			logger->Print(LogType::ERR, std::string("Invalid program."));							break;
		case CL_INVALID_PROGRAM_EXECUTABLE:
			logger->Print(LogType::ERR, std::string("Invalid program executable."));				break;
		case CL_INVALID_QUEUE_PROPERTIES:
			logger->Print(LogType::ERR, std::string("Invalid queue properties."));					break;
		case CL_INVALID_SAMPLER:
			logger->Print(LogType::ERR, std::string("Invalid sampler."));							break;
		case CL_INVALID_VALUE:
			logger->Print(LogType::ERR, std::string("Invalid value."));							break;
		case CL_INVALID_WORK_DIMENSION:
			logger->Print(LogType::ERR, std::string("Invalid work dimension."));					break;
		case CL_INVALID_WORK_GROUP_SIZE:
			logger->Print(LogType::ERR, std::string("Invalid work group size."));					break;
		case CL_INVALID_WORK_ITEM_SIZE:
			logger->Print(LogType::ERR, std::string("Invalid work item size."));					break;
		case CL_MAP_FAILURE:
			logger->Print(LogType::ERR, std::string("Memory mapping failed."));					break;
		case CL_MEM_COPY_OVERLAP:
			logger->Print(LogType::ERR, std::string("Copying overlapped memory address."));		break;
		case CL_MEM_OBJECT_ALLOCATION_FAILURE:
			logger->Print(LogType::ERR, std::string("Memory object allocation failure."));			break;
		case CL_OUT_OF_HOST_MEMORY:
			logger->Print(LogType::ERR, std::string("Out of host memory."));						break;
		case CL_OUT_OF_RESOURCES:
			logger->Print(LogType::ERR, std::string("Out of resources."));							break;
		case CL_PROFILING_INFO_NOT_AVAILABLE:
			logger->Print(LogType::ERR, std::string("Profiling info is not available."));			break;
		case CL_SUCCESS:
			logger->Print(LogType::INF, std::string("Succeeded."));								break;
		default:
			logger->Print(LogType::ERR, std::string("UNKNOWN ERROR CODE.") + std::to_string(err));	break;
	}
}

void CLplatform::InitializeGLsharing() {
	common::Logger* logger = common::Logger::instance();

	cl_uint num_platforms;
	cl_platform_id platform_ids[kMaxPlatformNum];
	this->GetPlatformInfo(platform_ids, &num_platforms);

	if (!is_valid_)
	{
		logger->Print(LogType::INF, std::string("OpenCL is not valid."));
		return;
	}

	for (int i = 0; i < num_platforms; i++) {
		support_gl_sharing_ = IsSharingSupport(platform_ids[i]);
		if (support_gl_sharing_) {
			logger->Print(LogType::INF, std::string("GL-CL sharing is supported"));
			CreateCLcontextGLsharing(platform_ids[i]);
			CreateCommandQueue();
			return;
		}
	}

	logger->Print(LogType::ERR, std::string("GL-CL sharing is NOT supported!"));

	logger->Print(LogType::INF, std::string("OpenCL initialized to the best device.."));
	this->InitializeGLbestDevice();
	//todo create default cl context.
}

void CLplatform::InitializeGLbestDevice() {
	common::Logger* logger = common::Logger::instance();

	cl_int status;

	cl_uint num_platforms;
	cl_platform_id platform_ids[kMaxPlatformNum];
	cl_platform_id best_platform;
	this->GetPlatformInfo(platform_ids, &num_platforms);

	if (!is_valid_)
	{
		logger->Print(LogType::INF, std::string("OpenCL is not valid."));
		return;
	}

	this->GetBestPlatform(platform_ids, num_platforms, best_platform);

	status = clGetDeviceIDs(best_platform, CL_DEVICE_TYPE_ALL, 1, &device_id_, nullptr);
	checkError(status, __LINE__, "failed to get device id.");

	CreateCLcontext(best_platform);
	CreateCommandQueue();
}

void CLplatform::PrintOpenCLVariables() {
	if (!is_valid_)
	{
		return;
	}

	this->PrintCLmemAllocSize();
	this->PrintCLcomputeUnits();
	cl_uint item_dimensions = this->PrintCLworkItemDimensions();
	this->PrintCLworkItemSize(item_dimensions);
}

void CLplatform::GetPlatformInfo(cl_platform_id* platform_ids, cl_uint* num_platforms) {
	common::Logger* logger = common::Logger::instance();

	cl_int status;
	status = clGetPlatformIDs(kMaxPlatformNum, platform_ids, num_platforms);
	logger->Print(LogType::INF, std::string("num_platforms : ") + std::to_string(*num_platforms));
	is_valid_ = checkError(status, __LINE__, "failed to get platform IDs.");
}

void CLplatform::GetBestPlatform(const cl_platform_id* platform_ids, const cl_uint& num_platforms,
								 cl_platform_id& best_platform_id) {

	if (!is_valid_)
	{
		return;
	}

	common::Logger* logger = common::Logger::instance();

	float max_mem_alloc_size = 0.0f;
	int best_platform_index = -1;
	char platform_name[kCharNameMaxSize];

	for (int i = 0; i < num_platforms; i++) {
		cl_int status = clGetPlatformInfo(platform_ids[i], CL_PLATFORM_NAME, sizeof(platform_name) - 1, platform_name, nullptr);
		checkError(status, __LINE__, "failed to get platform info.");

		logger->Print(LogType::INF, std::string("opencl platform[") +
						 std::to_string(i) +
						 std::string("] = "));
		logger->Print(LogType::INF, platform_name);

		status = clGetDeviceIDs(platform_ids[i], CL_DEVICE_TYPE_ALL, 1, &device_id_, nullptr);
		checkError(status, __LINE__, "failed to get device id.");

		float mem_alloc_size = this->PrintCLmemAllocSize();
		if (max_mem_alloc_size < mem_alloc_size) {
			max_mem_alloc_size = mem_alloc_size;
			best_platform_index = i;
		}
		logger->Print(LogType::INF, "");
	}

	cl_int status = clGetPlatformInfo(platform_ids[best_platform_index], CL_PLATFORM_NAME, sizeof(platform_name) - 1, platform_name, nullptr);
	checkError(status, __LINE__, "failed to get platform info.");

	logger->Print(LogType::INF, std::string("opencl best platform = ") + platform_name);

	best_platform_id = platform_ids[best_platform_index];
}

bool CLplatform::IsSharingSupport(const cl_platform_id& platform_id) {
	if (!is_valid_)
	{
		return false;
	}

	common::Logger* logger = common::Logger::instance();

	char platform_name[kCharNameMaxSize];

	cl_int status = clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, sizeof(platform_name) - 1, platform_name, nullptr);
	checkError(status, __LINE__, "failed to get platform info.");

	if (!IsGPUplatform(platform_name))
		return false;

	status = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id_, nullptr);
	checkError(status, __LINE__, "failed to get device id.");

	if (IsDeviceAvailableSharing()) {
		logger->Print(LogType::INF, std::string("opencl platform : ") + platform_name);
		return true;
	}
	else
	{
		return false;
	}
}

void CLplatform::CreateCLcontextGLsharing(const cl_platform_id& platform_id) {
	if (!is_valid_)
	{
		return;
	}

#if defined (__APPLE__) || defined(MACOSX)
	CGLContextObj kCGLContext = CGLGetCurrentContext();
	CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
	cl_context_properties contextProperties[] =
	{
		CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup,
		0
	};
	context_ = clCreateContext(contextProperties, 0, 0, NULL, NULL, &status);
	#elseif defined(UNIX)
		cl_context_properties contextProperties[] =
	{
		CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
		CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)m_firstPlatformId,
		0
	};
	context_ = clCreateContext(contextProperties, 1, &device_id_, NULL, NULL, &status);
#else
	cl_int status;

	cl_context_properties contextProperties[] =
	{
		CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
		CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id,
		0
	};

	context_ = clCreateContext(contextProperties, 1, &device_id_, NULL, NULL, &status);
	checkError(status, __LINE__, "failed to create GPU context.");

#endif
}

void CLplatform::CreateCLcontext(const cl_platform_id& platform_id) {
	if (!is_valid_)
	{
		return;
	}

	cl_int status;
	cl_context_properties contextProperties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id, 0 };

	context_ = clCreateContext(contextProperties, 1, &device_id_, NULL, NULL, &status);
	checkError(status, __LINE__, "failed to create GPU context.");
}

void CLplatform::CreateCommandQueue() {
	if (!is_valid_)
	{
		return;
	}

	cl_int status;
	command_queue_ = clCreateCommandQueue(context_, device_id_, NULL, &status);
	checkError(status, __LINE__, "failed to create command queue.");
}

float CLplatform::PrintCLmemAllocSize() {
	cl_ulong max_mem_size = 0;
	if (!is_valid_)
	{
		return static_cast<float>(max_mem_size);
	}

	common::Logger* logger = common::Logger::instance();

	cl_int status = clGetDeviceInfo(device_id_, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &max_mem_size, nullptr);
	checkError(status, __LINE__, "clGetDeviceInfo failed.");

	float max_memory = float(max_mem_size / (1024.0f));
	logger->Print(LogType::INF, std::string("CL_DEVICE_MAX_MEM_ALLOC_SIZE[KB] : ") + std::to_string(max_memory));
	return max_mem_size;
}

cl_uint CLplatform::PrintCLcomputeUnits() {
	cl_uint num_compute_units = 0;
	if (!is_valid_)
	{
		return num_compute_units;
	}

	common::Logger* logger = common::Logger::instance();
	cl_int status = clGetDeviceInfo(device_id_, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &num_compute_units, nullptr);
	checkError(status, __LINE__, "clGetDeviceInfo failed.");
	logger->Print(LogType::INF, std::string("CL_DEVICE_MAX_COMPUTE_UNITS : ") + std::to_string(num_compute_units));
	return num_compute_units;
}

cl_uint CLplatform::PrintCLworkItemDimensions() {
	cl_uint num_work_item_dimensions = 0;
	if (!is_valid_)
	{
		return num_work_item_dimensions;
	}

	common::Logger* logger = common::Logger::instance();
	cl_int status = clGetDeviceInfo(device_id_, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_uint), &num_work_item_dimensions, nullptr);
	checkError(status, __LINE__, "clGetDeviceInfo failed.");
	logger->Print(LogType::INF, std::string("CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS : ") + std::to_string(num_work_item_dimensions));
	return num_work_item_dimensions;
}

std::vector<size_t> CLplatform::PrintCLworkItemSize(const cl_uint & num_work_dimensions) {
	std::vector<size_t> num_work_item_sizes(num_work_dimensions);
	if (!is_valid_)
	{
		return num_work_item_sizes;
	}

	common::Logger* logger = common::Logger::instance();

	cl_int status = clGetDeviceInfo(device_id_, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t)*(num_work_dimensions), &num_work_item_sizes[0], nullptr);
	checkError(status, __LINE__, "clGetDeviceInfo failed.");

	std::stringstream ss;
	for (int j = 0; j < num_work_dimensions; j++) {
		if (j == 0)
			ss << num_work_item_sizes[j];
		else
			ss << "x" << num_work_item_sizes[0];
	}

	logger->Print(LogType::INF, std::string("CL_DEVICE_MAX_WORK_ITEM_SIZES : ") + ss.str());
	return num_work_item_sizes;
}

bool CLplatform::IsGPUplatform(char* platform_name) const {
	if (((int)std::string(platform_name).find(std::string("NVIDIA CUDA"))) >= 0 ||
		((int)std::string(platform_name).find(std::string("AMD")) >= 0))
		return true;
	else
		return false;
}

bool CLplatform::IsDeviceAvailableSharing() const {
	if (!is_valid_)
	{
		return false;
	}

	size_t extensionSize;
	cl_int status = clGetDeviceInfo(device_id_, CL_DEVICE_EXTENSIONS, 0, NULL, &extensionSize);
	checkError(status, __LINE__, "clGetDeviceInfo failed.");

	if (extensionSize > 0) {
		char* extensions = (char*)malloc(extensionSize);
		status = clGetDeviceInfo(device_id_, CL_DEVICE_EXTENSIONS, extensionSize, extensions, &extensionSize);
		checkError(status, __LINE__, "clGetDeviceInfo failed.");

		std::string stdDevString(extensions);
		free(extensions);

		size_t szOldPos = 0;
		size_t szSpacePos = stdDevString.find(' ', szOldPos); // extensions string is space delimited

		while (szSpacePos != stdDevString.npos) {
			if (strcmp(GL_SHARING_EXTENSION, stdDevString.substr(szOldPos, szSpacePos - szOldPos).c_str()) == 0) {
				// Device supports context sharing with OpenGL
				return true;
			}
			do {
				szOldPos = szSpacePos + 1;
				szSpacePos = stdDevString.find(' ', szOldPos);
			} while (szSpacePos == szOldPos);
		}
	}

	return false;
}
