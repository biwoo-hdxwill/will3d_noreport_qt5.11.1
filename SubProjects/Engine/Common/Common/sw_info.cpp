#include "sw_info.h"

#include <sstream>
#if defined(_WIN32)
#include <Windows.h>
#include <winver.h>
#include <psapi.h>
#endif

#include "W3Logger.h"

const sw_info::InstructionSet::InstructionSet_Internal sw_info::InstructionSet::CPU_Rep;

namespace sw_info
{
	void SWInfo::printSWVersion()
	{
		using common::Logger;
		auto logger = Logger::instance();

		logger->Print(common::LogType::INF, std::string("Version : ") + GetSWVersion());
	}

	std::string SWInfo::GetSWVersion()
	{
		std::stringstream ss;

#if defined(_WIN32)
		using common::Logger;
		auto logger = Logger::instance();

		TCHAR file_name[MAX_PATH + 1] = { 0 };
		if (GetModuleFileName(NULL, file_name, MAX_PATH) == 0)
		{
			logger->Print(common::LogType::ERR, "GetModuleFileName failed");
		}

		// allocate a block of memory for the version info
		DWORD dummy;
		DWORD size = GetFileVersionInfoSize(file_name, &dummy);
		if (size == 0)
		{
			logger->Print(common::LogType::ERR, "GetFileVersionInfoSize failed");
		}

		std::vector<BYTE> data(size);
		if (!GetFileVersionInfo(file_name, NULL, size, &data[0]))
		{
			logger->Print(common::LogType::ERR, "GetFileVersionInfo failed");
		}

		// get the name and version strings
		LPVOID product_version = NULL;
		unsigned int product_version_length = 0;
		// "041204b0" : language ID, 한국어(대한민국)
		char buffer1[] = "\\StringFileInfo\\041204b0\\ProductVersion";
		wchar_t path_version[MAX_PATH];
		mbstowcs(path_version, buffer1, strlen(buffer1) + 1);
		if (!VerQueryValue(&data[0], path_version, &product_version, &product_version_length))
		{
			logger->Print(common::LogType::ERR, "Can't obtain ProductVersion from resources");
		}

		auto version = (LPCSTR)product_version;
		for (int idx = 0; idx < 2 * (product_version_length - 1); ++idx)
		{
			ss << version++;
		}
#endif

		return ss.str();
	}

  bool SWInfo::IsNewerVersion(const std::string& cloud) {
    std::istringstream ssCurrent(GetSWVersion());
    std::istringstream ssCloud(cloud);

    std::string tokenCurrent, tokenCloud;
    while (std::getline(ssCurrent, tokenCurrent, '.') &&
      std::getline(ssCloud, tokenCloud, '.')) {
      int verCurrent = std::stoi(tokenCurrent);
      int verCloud = std::stoi(tokenCloud);

      if (verCloud > verCurrent)
        return true;
      else if (verCloud < verCurrent)
        return false;
      // 같으면 다음으로 넘어감
    }

    while (std::getline(ssCloud, tokenCloud, '.')) {
      if (std::stoi(tokenCloud) > 0)
        return true;
    }

    while (std::getline(ssCurrent, tokenCurrent, '.')) {
      if (std::stoi(tokenCurrent) > 0)
        return false;
    }

    return false; // 완전히 같음
  }

	void SWInfo::printCPUInfo()
	{
#if defined(_WIN32)
		using common::Logger;
		auto logger = Logger::instance();
		logger->Print(common::LogType::INF, std::string("CPU vendor - ") + sw_info::InstructionSet::Vendor());
		logger->Print(common::LogType::INF, std::string("CPU product name - ") + sw_info::InstructionSet::Brand());
#endif
	}

	void SWInfo::printCPUMemoryStatus()
	{
#if defined(_WIN32)
		using common::Logger;
		auto logger = Logger::instance();
		// Use to convert bytes to MB
		const int kDivMB = 1024 * 1024;

		MEMORYSTATUSEX statex;
		statex.dwLength = sizeof(statex);
		GlobalMemoryStatusEx(&statex);
		logger->Print(common::LogType::INF, std::string("--------------<CPU memory status>--------------"));
		logger->Print(common::LogType::INF,
			std::string("There is ") + std::to_string(statex.dwMemoryLoad) + std::string(" percent of memory in use."));
		logger->Print(common::LogType::INF,
			std::string("There are ") + std::to_string(statex.ullTotalPhys / kDivMB) + std::string(" total MB of physical memory."));
		logger->Print(common::LogType::INF,
			std::string("There are ") + std::to_string(statex.ullAvailPhys / kDivMB) + std::string(" free MB of physical memory."));
		logger->Print(common::LogType::INF,
			std::string("There are ") + std::to_string(statex.ullTotalPageFile / kDivMB) + std::string(" total MB of paging file."));
		logger->Print(common::LogType::INF,
			std::string("There are ") + std::to_string(statex.ullAvailPageFile / kDivMB) + std::string(" free  MB of paging file."));
		logger->Print(common::LogType::INF,
			std::string("There are ") + std::to_string(statex.ullTotalVirtual / kDivMB) + std::string(" total MB of virtual memory."));
		logger->Print(common::LogType::INF,
			std::string("There are ") + std::to_string(statex.ullAvailVirtual / kDivMB) + std::string(" free MB of virtual memory."));
		logger->Print(common::LogType::INF, std::string("-----------------------------------------------"));
#endif
	}
}; // end of namespace SWInfo
