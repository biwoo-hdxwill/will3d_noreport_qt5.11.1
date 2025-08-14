#pragma once
#include <iostream>
#include <vector>
#include <bitset>
#include <array>
#include <string>
#if defined(_WIN32)
#include <intrin.h>
#endif

#include "common_global.h"

namespace sw_info
{
	class COMMON_EXPORT SWInfo
	{
	public:
		static void printSWVersion();
		static std::string GetSWVersion();
		static void printCPUInfo();
		static void printCPUMemoryStatus();
		static bool IsNewerVersion(const std::string& cloud);

	protected:
		SWInfo() {};
		~SWInfo() {};
	};

	/**********************************************************************************************//**
	* @class	InstructionSet
	* @date	2017-06-20
	**************************************************************************************************/
	class InstructionSet
	{
		// forward declarations
		class InstructionSet_Internal;

	public:
		// getters
		static std::string Vendor(void) { return CPU_Rep.vendor_; }
		static std::string Brand(void) { return CPU_Rep.brand_; }

	private:
		static const InstructionSet_Internal CPU_Rep;

		class InstructionSet_Internal
		{
		public:
			InstructionSet_Internal()
				: nIds_{ 0 }, nExIds_{ 0 },
				data_{}, extdata_{}
			{
				//int cpuInfo[4] = {-1};
				std::array<int, 4> cpui;

				// Calling __cpuid with 0x0 as the function_id argument
				// gets the number of the highest valid function ID.
				__cpuid(cpui.data(), 0);
				nIds_ = cpui[0];

				for (int i = 0; i <= nIds_; ++i)
				{
					__cpuidex(cpui.data(), i, 0);
					data_.push_back(cpui);
				}

				// Capture vendor string
				char vendor[0x20];
				memset(vendor, 0, sizeof(vendor));
				*reinterpret_cast<int*>(vendor) = data_[0][1];
				*reinterpret_cast<int*>(vendor + 4) = data_[0][3];
				*reinterpret_cast<int*>(vendor + 8) = data_[0][2];
				vendor_ = vendor;

				// Calling __cpuid with 0x80000000 as the function_id argument
				// gets the number of the highest valid extended ID.
				__cpuid(cpui.data(), 0x80000000);
				nExIds_ = cpui[0];

				char brand[0x40];
				memset(brand, 0, sizeof(brand));

				for (int i = 0x80000000; i <= nExIds_; ++i)
				{
					__cpuidex(cpui.data(), i, 0);
					extdata_.push_back(cpui);
				}

				// Interpret CPU brand string if reported
				if (nExIds_ >= 0x80000004)
				{
					memcpy(brand, extdata_[2].data(), sizeof(cpui));
					memcpy(brand + 16, extdata_[3].data(), sizeof(cpui));
					memcpy(brand + 32, extdata_[4].data(), sizeof(cpui));
					brand_ = brand;
				}
			};

			int nIds_, nExIds_;
			std::string vendor_, brand_;
			std::vector<std::array<int, 4>> data_;
			std::vector<std::array<int, 4>> extdata_;
		};
	};
}; // end of namespace SWInfo
