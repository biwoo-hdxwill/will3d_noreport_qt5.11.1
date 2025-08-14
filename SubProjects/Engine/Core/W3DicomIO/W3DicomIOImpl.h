#pragma once
/*=========================================================================

File:		class CW3DicomIOImpl
Language:	C++11
Library:	DCMTK 3.6.4, Standard C++ Library

=========================================================================*/
// C++11 Standard Libraries.

#include <map>
#include <string>
#include <vector>
#include <QRect>
//DCMTK
#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmdata/dcddirif.h"
#include "dcmtk/dcmjpeg/djrplol.h"
#include "dcmtk/dcmjpeg/djencode.h"
#include "dcmtk/dcmjpeg/djdecode.h"
#include "dcmtk/dcmimgle/dcmimage.h"
#include "dcmtk/dcmdata/dcpxitem.h"

#include "../../Common/Common/W3Enum.h"

class CW3Image3D;
class DcmDataset;
class DcmTagKey;

typedef std::map<std::string, QString>	HeaderList;

/*
	* class CW3DicomIO
		- Wrapper class for GDCM Library.
		- Implementation of Interface (CW3DicomIO)
*/
class CW3DicomIOImpl
{
	enum class EFILE_TYPE
	{
		SINGLE,		// single frame.
		MULTI,		// multi frame.
	};

public:
	CW3DicomIOImpl(void);
	~CW3DicomIOImpl(void);

public:
	// public functions.
	const bool setFiles(const std::vector<std::string> &fileNames);
	const std::vector<std::string>& GetDICOMFiles();
	void setArea(QRect Area);
	const bool setDirectory(const std::string &dirPath);
	const unsigned int getNumberOfSeries(void);
	const bool getHeaderInfoFromFile(const std::string& filePath, HeaderList &listHeader, bool bSimple = true);
	const bool getImageBufFromFile(const std::string& filePath, short *pImageBuf);
	const bool readVolume(const int index, CW3Image3D *&vol);
	const bool writeDicom(const std::string& dirPath, HeaderList listHeader, unsigned char* data);
	const bool writeDicom(const std::string& dirPath, HeaderList listHeader, unsigned short* data);
	const bool WriteDicom(const std::string& dir_path, unsigned short* data, const int instance_number, const int rows, const int columns);
	const bool WriteDicomRGB(const std::string& dir_path, unsigned char* data, const int instance_number, const int rows, const int columns);
	const bool CheckFile(const std::string& file_path);
	void CreateSeriesInstanceUID();
	void InitSeriesInstanceUID();

private:
	QString GetDicomInfo(DcmDataset* ds, DcmTagKey key);
	QString GetDicomInfo(DcmDataset* ds, DcmTagKey key, unsigned int pos, QString& title);
	std::vector<std::string> getHeader(HeaderList& listHeader, CW3Image3D*& vol);
	const bool isStatusBad(const OFCondition& status, const char* errMsg);
	void PostProcessing(CW3Image3D *&vol);
	void WriteHeader(DcmDataset*& dataset, HeaderList& header);
	bool ReadPixelData(CW3Image3D*& volume, const int x_start, const int y_start);
	bool GetPixelData(const std::string& file, ushort* output, const int bits_stored);
	template <typename T>
	void Read16BitSlices(CW3Image3D *vol,
		const std::vector<std::string> &file_names,
		const int &x_start, const int &y_start, const int &x_end,
		const int &y_end);
	void AutoWindowing(CW3Image3D* volume);

	bool PutAndInsertString(DcmDataset* dataset, HeaderList& header, const DcmTag& tag, const std::string& str);

private:
	// private member fields.
	int						m_nSeriesNum;		// # of input series.
	std::vector<EFILE_TYPE>		m_eSeriesFileType;	// file types of input series.
	std::vector<HeaderList>		m_listHeader;		// vector for "multiple series" of volumes

	std::vector<std::string>	m_strFileNames;

	bool						m_bProcessingAreaInputed;
	QRect						m_ProcessingArea;

	bool is_unicode_ = false;

	static char series_instance_uid_[100];
};
