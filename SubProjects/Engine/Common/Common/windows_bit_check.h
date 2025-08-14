#pragma once
/*=========================================================================
File:			windows_bit_check.h
Language:		C++11
Library:		Qt 5.8, Standard C++ Library
Author:			LIM TAE KYUN
First Date:		2020-08-18
Last Modify:	2020-08-18

Copyright (c) 2020 All rights reserved by HDXWILL.
=========================================================================*/

class WindowsBitCheck
{
public:
	static bool Is64BitWindows();

private:
	WindowsBitCheck(const WindowsBitCheck&) = delete;
	const WindowsBitCheck& operator = (const WindowsBitCheck&) = delete;

	static bool IsCurrentProcess64bit();
	static bool IsCurrentProcessWow64();
};
