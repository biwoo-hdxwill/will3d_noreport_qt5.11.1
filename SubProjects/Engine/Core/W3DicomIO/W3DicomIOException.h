#pragma once
/*=========================================================================

File:		class CW3DicomIOException
Language:	C++11
Library:	GDCM 2.4 (Grassroots DICOM Library), Standard C++ Library

=========================================================================*/
#include <exception>
#include "W3dicomio_global.h"
/*
	CW3DicomIOException class's Exception Class.
	[ ERROR_ID ]
		INVALID_PATH			: invalid input path.
		INVALID_FILE_FORMAT		: invalid file format.
		READ_ERROR				: exception in file read.
		WRITE_ERROR				: exception in file write.
		UNKNOWN					: unknown.
*/
class W3DICOMIO_EXPORT CW3DicomIOException : public std::exception
{
public:
	// define exception ID.
	enum class EID {
		INVALID_PATH,
		INVALID_FILE_FORMAT,
		INVALID_VOLUME,
		READ_ERROR,
		WRITE_ERROR,
		INSUFFICIENT_MEMORY,
		UNKNOWN
	};
public:
	CW3DicomIOException(EID eid = EID::UNKNOWN, std::string str = nullptr) {
		m_eID = eid;
		m_strErr = str;
	}
	inline const char* what() const throw() override { return m_strErr.c_str(); }
	inline const EID ErrorType(void) const { return m_eID; }

private:
	EID m_eID;
	std::string	m_strErr;
};
