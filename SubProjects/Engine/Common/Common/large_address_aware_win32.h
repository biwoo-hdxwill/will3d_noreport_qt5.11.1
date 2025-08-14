#pragma once
/*=========================================================================
File:			large_address_aware_win32.h
Language:		C++11
Library:		Qt 5.8, Standard C++ Library
Author:			LIM TAE KYUN
First Date:		2020-07-21
Last Modify:	2020-08-18

Copyright (c) 2020 All rights reserved by HDXWILL.
=========================================================================*/

#define NOMINMAX
#include <windows.h>
#include <string>

#include "common_global.h"

enum class OSType
{
	OS_NT_PREV, //3.1, 95, 98
	OS_NT,
	OS_END
};

class COMMON_EXPORT LargeAddressAwareWin32
{
public:
	inline bool large_address_aware() { return large_address_aware_; }
	inline OSType os_type() { return os_type_; }
	inline const std::string& os_version() { return os_version_; }

	static LargeAddressAwareWin32* GetInstance();

private:
	LargeAddressAwareWin32();
	~LargeAddressAwareWin32();
	LargeAddressAwareWin32(const LargeAddressAwareWin32&) = delete;
	const LargeAddressAwareWin32& operator = (const LargeAddressAwareWin32&) = delete;

	void SetLargeAddressAware();
	void CloseRegestry();
	void GetCurrentOSInfo();
	bool GetNtPrivilege();
	bool IsElevated();
	bool RegestryIdentify();
	bool RegestryQueryValue(const HKEY& handle, const TCHAR* value, TCHAR* out_buffer);
	bool SetLargeAddressAwareRegestry(bool is_large);
	void GetOSVersionStr(const OSVERSIONINFOEX& info);

private:
	bool large_address_aware_ = false;
	HKEY key_handel_ = nullptr;
	OSType os_type_ = OSType::OS_END;
	std::string os_version_ = "";
};
