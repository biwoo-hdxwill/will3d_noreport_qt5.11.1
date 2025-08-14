#include "large_address_aware_win32.h"

#include <winerror.h>
#include "windows_bit_check.h"

namespace
{
	const TCHAR* kLargeAddressAware = TEXT("LargeAddressAware");
	const TCHAR* kTrue = TEXT("TRUE");
	const TCHAR* kFalse = TEXT("FALSE");
}

LargeAddressAwareWin32::LargeAddressAwareWin32()
{
	GetCurrentOSInfo();
	SetLargeAddressAware();
}

LargeAddressAwareWin32::~LargeAddressAwareWin32()
{
	CloseRegestry();
}

LargeAddressAwareWin32* LargeAddressAwareWin32::GetInstance()
{
	static LargeAddressAwareWin32 instacne;
	return &instacne;
}

void LargeAddressAwareWin32::SetLargeAddressAware()
{
	if (WindowsBitCheck::Is64BitWindows())
	{
		return;
	}

	if (os_version_.compare("Windows 7") != 0 &&
		os_version_.compare("Windows 8") != 0 &&
		os_version_.compare("Windows 8.1") != 0 &&
		os_version_.compare("Windows 10") != 0)
	{
		return;
	}

	RegestryIdentify();
	if (large_address_aware_)
	{
		CloseRegestry();
		return;
	}

	TCHAR file_path[_MAX_FNAME] = { 0, };
	GetSystemDirectory(file_path, _MAX_FNAME);
	wcscat(file_path, TEXT("\\bcdedit.exe"));

	BOOL is_shell_execute = FALSE;
	SHELLEXECUTEINFO shell_info = { sizeof(SHELLEXECUTEINFO) };

	shell_info.lpVerb = TEXT("runas");							// 관리자 권한 실행
	shell_info.lpFile = file_path;								// 권리자 권한으로 실행시킬 파일
	shell_info.lpParameters = TEXT("/set IncreaseUserVa 3072");

	is_shell_execute = ShellExecuteEx(&shell_info);
	if (!is_shell_execute)
	{
		//GetLastError();
		//ERROR_CANCELLED		// 사용자의 권한 상승 거절
		//ERROR_FILE_NOT_FOUND	// lpFile로 지정한 파일이 존재하지 않음
		CloseRegestry();
		return;
	}

	SetLargeAddressAwareRegestry(true);

	if (os_type_ == OSType::OS_NT_PREV)
	{
		ExitWindowsEx(EWX_FORCE | EWX_REBOOT, 0);
	}
	else if (os_type_ == OSType::OS_NT)
	{
		if (GetNtPrivilege())
		{
			ExitWindowsEx(EWX_FORCE | EWX_REBOOT, 0);
		}
		else
		{
			// SE_SHUTDOWN_NAME 레벨의 권한 등급을 얻지 못함.	
			CloseRegestry();
		}
	}
}

void LargeAddressAwareWin32::CloseRegestry()
{
	if (key_handel_)
	{
		RegCloseKey(key_handel_);
		key_handel_ = nullptr;
	}
}

void LargeAddressAwareWin32::GetCurrentOSInfo()
{
	OSVERSIONINFOEX os_info;			// 윈도우즈 버전에 관련된 정보를 얻어올 변수
										// OSVERSIONINFOEX 구조체 값을 얻었는지 OSVERSIONINFO 구조체값을 얻었는지를	
	BOOL os_version_ex_flag = FALSE;

	ZeroMemory(&os_info, sizeof(OSVERSIONINFOEX));
	os_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);	// 현재 운영체제의 버전을 알수 없기 때문에 OSVERSIONINFOEX 속성으로 버전정보를 얻습니다.
															// OSVERSIONINFOEX 값을 설정하고 호출했기 때문에 윈도우 NT 4.0 SP6 이상 및 그 이후 버전의 윈도우가 아니라면 이 함수는 실패할 것이라고 함. 확인 x	
	os_version_ex_flag = GetVersionEx((OSVERSIONINFO*)&os_info);
	if (os_version_ex_flag != TRUE)
	{
		// 윈도우즈 버전이 낮아서 OSVERSIONINFOEX 형식으로 값을 얻을수 없는 경우 OSVERSIONINFO 형식으로 재설정하고 값을 얻습니다.
		os_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (!GetVersionEx((OSVERSIONINFO*)&os_info))
		{
			// 윈도우즈 9x 기반의 운영체제 Windows 95, Windows 98 또는 그 이전 버전의 운영체제의 경우 이곳으로 들어오는지 확인 x
			os_type_ = OSType::OS_NT_PREV;
			return;
		}
	}

	if (os_info.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		os_type_ = OSType::OS_NT;
	}
	else
	{
		// VER_PLATFORM_WIN32s			윈도우즈 3.1
		// VER_PLATFORM_WIN32_WINDOWS	윈도우즈 9x 기반의 운영체제 Windows 95, Windows 98
		os_type_ = OSType::OS_NT_PREV;
	}

	GetOSVersionStr(os_info);
}

bool LargeAddressAwareWin32::GetNtPrivilege()
{
	// 현재 운영체제가 NT 계열인 경우, SE_SHUTDOWN_NAME 레벨의 권한 등급을
	// 가지고 있어야 하기 때문에 아래의 함수를 이용하여 권한을 얻습니다.
	HANDLE token;
	TOKEN_PRIVILEGES token_privilege;

	// 현재 프로세스의 권한과 관련된 정보를 변경하기 위해 토큰정보를 엽니다.
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token))
	{
		// 현재 프로세스가 SE_SHUTDOWN_NAME 권한을 사용할수 있도록 설정한다.
		LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &token_privilege.Privileges[0].Luid);

		token_privilege.PrivilegeCount = 1;
		token_privilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		// 지정한 값으로 권한을 조정한다.
		if (AdjustTokenPrivileges(token, FALSE, &token_privilege, 0, (PTOKEN_PRIVILEGES)NULL, 0))
		{
			return true;
		}
	}

	return false;
}

// 응용 프로그램이 관리자 권한을 얻었는지 확인하는 용도.
bool LargeAddressAwareWin32::IsElevated()
{
	BOOL ret = FALSE;
	HANDLE token = nullptr;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
	{
		TOKEN_ELEVATION elevation;
		DWORD cb_size = sizeof(TOKEN_ELEVATION);
		if (GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &cb_size))
		{
			ret = elevation.TokenIsElevated;
		}
	}

	if (token)
	{
		CloseHandle(token);
	}

	return ret;
}

bool LargeAddressAwareWin32::RegestryIdentify()
{
	CloseRegestry();

	LSTATUS status = RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\Will3D"), 0, KEY_ALL_ACCESS, &key_handel_);
	if (status == NO_ERROR)
	{
		TCHAR buffer[_MAX_FNAME];
		memset(buffer, 0, sizeof(TCHAR) * _MAX_FNAME);
		if (RegestryQueryValue(key_handel_, kLargeAddressAware, buffer))
		{
			if (wcscmp(buffer, kTrue) == 0)
			{
				large_address_aware_ = true;
				return true;
			}
		}
		return false;
	}
	else if (status == ERROR_FILE_NOT_FOUND)
	{
		RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE"), 0, KEY_ALL_ACCESS, &key_handel_);
		RegCreateKey(key_handel_, TEXT("Will3D"), &key_handel_);

		return SetLargeAddressAwareRegestry(false);
	}

	return false;
}

bool LargeAddressAwareWin32::RegestryQueryValue(const HKEY& handle, const TCHAR* value, TCHAR* out_buffer)
{
	DWORD data_type = 0L;
	DWORD data_size = sizeof(TCHAR) * _MAX_FNAME;

	// RegQueryValueEx parameter
	// 1. RegOpenKeyEx에의해 얻어진 키 핸들
	// 2. 키 안에 값 이름
	// 3. 옵션 항상 0
	// 4. 얻어진 데이터 타입
	// 5. 얻어진 데이터
	// 6. 얻어진 데이터 크기
	LSTATUS status = RegQueryValueEx(handle, value, 0, &data_type, (BYTE*)out_buffer, &data_size);
	if (status == NO_ERROR)
	{
		return true;
	}

	return false;
}

bool LargeAddressAwareWin32::SetLargeAddressAwareRegestry(bool is_large)
{
	const TCHAR* value;
	if (is_large)
	{
		value = kTrue;
	}
	else
	{
		value = kFalse;
	}

	LSTATUS status = RegSetValueEx(key_handel_, kLargeAddressAware, 0, REG_SZ, (BYTE*)value, wcslen(value) * 2);
	if (status == NO_ERROR)
	{
		return true;
	}

	return false;
}

/*
In Windows 8.1 and Windows 10, the GetVersion and GetVersionEx functions have been deprecated.
In Windows 10, the VerifyVersionInfo function has also been deprecated.
While you can still call the deprecated functions, if your application does not specifically
target Windows 8.1 or Windows 10, the functions will return the Windows 8 version (6.2).

In order for your app to target Windows 8.1 or Windows 10, you'll need to include an
app (executable) manifest for the app's executable. Then, in the <compatibility> section of the manifest,
you'll need to add a <supportedOS> element for each Windows version you want to declare that your app supports.

https://docs.microsoft.com/ko-kr/windows/win32/sysinfo/targeting-your-application-at-windows-8-1?redirectedfrom=MSDN
*/
void LargeAddressAwareWin32::GetOSVersionStr(const OSVERSIONINFOEX& info)
{
	if (info.dwMajorVersion == 5)
	{
		if (info.dwMinorVersion == 0)
		{
			os_version_ = "Windows 2000";
		}
		else if (info.dwMinorVersion == 1)
		{
			os_version_ = "Windows XP";
		}
		else if (info.dwMinorVersion == 2)
		{
			if (info.wProductType == VER_NT_WORKSTATION)
			{
				os_version_ = "Windows XP Professional x64 Edition";
			}
			else if (GetSystemMetrics(SM_SERVERR2) == 0)
			{
				os_version_ = "Windows Server 2003";
			}
			else if (info.wSuiteMask & VER_SUITE_WH_SERVER)
			{
				os_version_ = "Windows Home Server";
			}
			else if (GetSystemMetrics(SM_SERVERR2) != 0)
			{
				os_version_ = "Windows Server 2003 R2";
			}
		}
	}
	else if (info.dwMajorVersion == 6)
	{
		if (info.dwMinorVersion == 0)
		{
			if (info.wProductType == VER_NT_WORKSTATION)
			{
				os_version_ = "Windows Vista";
			}
			else if (info.wProductType != VER_NT_WORKSTATION)
			{
				os_version_ = "Windows Server 2008";
			}
		}
		else if (info.dwMinorVersion == 1)
		{
			if (info.wProductType != VER_NT_WORKSTATION)
			{
				os_version_ = "Windows Server 2008 R2";
			}
			else if (info.wProductType == VER_NT_WORKSTATION)
			{
				os_version_ = "Windows 7";
			}
		}
		else if (info.dwMinorVersion == 2)
		{
			if (info.wProductType != VER_NT_WORKSTATION)
			{
				os_version_ = "Windows Server 2012";
			}
			else if (info.wProductType == VER_NT_WORKSTATION)
			{
				os_version_ = "Windows 8";
			}
		}
		else if (info.dwMinorVersion == 3)
		{
			if (info.wProductType != VER_NT_WORKSTATION)
			{
				os_version_ = "Windows Server 2012 R2";
			}
			else if (info.wProductType == VER_NT_WORKSTATION)
			{
				os_version_ = "Windows 8.1";
			}
		}
	}
	else if (info.dwMajorVersion == 10)
	{
		if (info.wProductType != VER_NT_WORKSTATION)
		{
			os_version_ = "Windows Server 2016";
		}
		else
		{
			os_version_ = "Windows 10";
		}
	}
}
