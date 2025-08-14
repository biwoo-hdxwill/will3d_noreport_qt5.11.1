#pragma once
/*=========================================================================

File:			class CW3DicomIO
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-11-21
Last date:	2015-11-21

Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
// C++11 Standard Libraries.
#include <map>
#include <string>
#include <vector>

#include <qrect.h>
#include <QString>

#include "../../Common/Common/W3Enum.h"

#include "W3DicomIOException.h"

#include "w3dicomio_global.h"

class CW3Image3D;

typedef std::map<std::string, QString>	HeaderList;
class CW3DicomIOImpl;

/*
	* class CW3DicomIO
		- Wrapper class for GDCM Library.
		- Interface for Users.
		- Implementation : CW3DicomIOImpl
	* NOTE FOR IMPLEMENTORS:
		* Either of two functions must be called before actual reading process.
			setFiles()		: set series of files to read.
			setDirectory	: set directory to read.
	* CURRENTLY, SUPPORTS ONLY FOR SINGLE-SERIES DICOM FILES.
		* CALL setFiles() AND readVolume() SEQUENTIALLY.
*/

class W3DICOMIO_EXPORT CW3DicomIO
{
public:
	CW3DicomIO();
	~CW3DicomIO();

	/*
		Dicom File Type ID definition.
	*/
	//enum class EFILE_TYPE {
	//	SINGLE,		// single frame.
	//	MULTI,		// multi frame.
	//};
	// public functions.
	/*****************************
		Interfaces for Reading.
	******************************/
	const bool setFiles(const std::vector<std::string> &fileNames);
	const std::vector<std::string>& GetDICOMFiles();
	void setArea(QRect rect);
	const bool setDirectory(const std::string &dirPath);

	const bool getHeaderInfoFromFile(const std::string& filePath, HeaderList &listHeader, bool bSimple = true);
	bool getImageBufFromFile(const std::string& filePath, short *ImageBuf);

	/*
		Get # of series in files | directory.
	*/
	const unsigned int getNumberOfSeries(void);
	/*
		Get header information for (index)th volume among series.
	*/
	//const bool getHeaderInfo(const int index, HeaderList &listHeader);	
	/*
		Get volume data for (index)th volume among series.
		OUTPUT : vol
		second (vol) parameter must be "nullptr"
	*/
	const bool readVolume(const int index, CW3Image3D *&vol);

	/*****************************
		Interfaces for Writing.
		(exporting Dicom files)
	******************************/
	/*
		Export Dicom File(s).

			dirPath		: Directory Path for Writing Dicom
			listHeader	: Dicom Header Information
			ppData		: Volume Data
			type		: Specifies Dicom File Format. (single | multi frame)
	*/
	//template<typename TYPE>
	const bool writeDicom(const std::string& dirPath, HeaderList listHeader, unsigned char *data);
	const bool writeDicom(const std::string& dirPath, HeaderList listHeader, unsigned short *data);
	const bool WriteDicom(const std::string& dir_path, unsigned short* data, const int instance_num, const int rows, const int columns);
	const bool WriteDicomRGB(const std::string& dir_path, unsigned char* data, const int instance_num, const int rows, const int columns);
	//template<typename TYPE>
	//const bool writeDicom(std::string &dirPath, HeaderList &listHeader, TYPE **ppData, EFILE_TYPE type = EFILE_TYPE::SINGLE);

	const bool CheckFile(const QString& file_path);
	const bool CheckFile(const std::string& file_path);

	void CreateSeriesInstanceUID();
	void InitSeriesInstanceUID();

private:
	CW3DicomIOImpl	*m_pImpl; // "Pointer-to-Implementation"	
};

