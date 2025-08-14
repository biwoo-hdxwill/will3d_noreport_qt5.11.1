#include "windows_bit_check.h"

#include <windows.h>

bool WindowsBitCheck::Is64BitWindows()
{
	if (IsCurrentProcess64bit())
	{
		return true;
	}

	return IsCurrentProcessWow64();
}

bool WindowsBitCheck::IsCurrentProcess64bit()
{
#if defined(_WIN64)
	return true;
#else
	return false;
#endif
}

// WOW64 환경에서 동작중인지 확인하는 함수입니다. WOW64란 "Windows on Windows 64-bit" 의 약자로 
// 64bit 윈도우에서 32bit 어플리케이션이 동작하게 해주는 서브 시스템을 환경을 얘기합니다.
// 이 함수는 현재 윈도우가 64bit 이고 현재 프로세스가 32bit 일 경우에만 TRUE를 리턴하게 되며 
// 내부적으로는 IsWow64Process API 함수를 이용하여 구현되어 있습니다
bool WindowsBitCheck::IsCurrentProcessWow64()
{
	BOOL is_wow64 = FALSE;

	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS IsWow64Process;

	IsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

	if (!IsWow64Process)
	{
		return false;
	}

	IsWow64Process(GetCurrentProcess(), &is_wow64);
	return is_wow64;
}
