#pragma once
/*=========================================================================

File:		class CW3MaskException
Language:	C++11
Library:	Standard C++ Library

=========================================================================*/
#include <iostream>
#include <exception>
/*
	CW3Mask class's Exception Class.
*/
class RESOURCE_EXPORT CW3MaskException : public std::exception
{
public:
	// define exception ID.
	enum class EID {
		BUFFER_NULLPTR,	// buffer is nullptr.
		PARENT_NULLPTR,	// parent is nullptr.
		INVALID_SIZE,	// invalid mask size
		INDEX_OUT_OF_RANGE,	// index out-of-range
		UNKNOWN
	};
public:
	CW3MaskException(EID eid = EID::UNKNOWN, std::string str = nullptr) {
		m_eID = eid;
		m_strErr = str;
	}
	inline const char* what() const throw() override { return m_strErr.c_str(); }
	inline const EID ErrorType(void) { return m_eID; }

private:
	EID m_eID;
	std::string	m_strErr;
};
