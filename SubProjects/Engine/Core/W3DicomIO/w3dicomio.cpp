#include "w3dicomio.h"
#include "../../Common/Common/W3Memory.h"
#include "W3DicomIOImpl.h"
#include <QByteArray>

CW3DicomIO::CW3DicomIO(void)
	: m_pImpl(new CW3DicomIOImpl())
{

}

CW3DicomIO::~CW3DicomIO(void)
{
	SAFE_DELETE_OBJECT(m_pImpl);
}

/*****************************
	Interfaces for Reading.
	******************************/
const bool CW3DicomIO::setFiles(const std::vector<std::string> &fileNames)
{
	return m_pImpl->setFiles(fileNames);
}

const std::vector<std::string>& CW3DicomIO::GetDICOMFiles() {
	return m_pImpl->GetDICOMFiles();
}

void CW3DicomIO::setArea(QRect rect)
{
	return m_pImpl->setArea(rect);
}

const bool CW3DicomIO::setDirectory(const std::string &dirPath)
{
	return m_pImpl->setDirectory(dirPath);
}

const bool CW3DicomIO::getHeaderInfoFromFile(const std::string& filePath, HeaderList &listHeader, bool bSimple)
{
	return m_pImpl->getHeaderInfoFromFile(filePath, listHeader, bSimple);
}

bool CW3DicomIO::getImageBufFromFile(const std::string& filePath, short *pImageBuf)
{
	return m_pImpl->getImageBufFromFile(filePath, pImageBuf);
}

/*
	Get # of series in files | directory.
*/
const unsigned int CW3DicomIO::getNumberOfSeries(void)
{
	return m_pImpl->getNumberOfSeries();
}
/*
	Get header information for (index)th volume among series.
*/

//const bool CW3DicomIO::getHeaderInfo(const int index, std::vector<std::pair<std::string, std::string>> &listHeader)
//{
//	return m_pImpl->getHeaderInfo(index, listHeader);
//}
/*
	Get volume data for (index)th volume among series.
*/
const bool CW3DicomIO::readVolume(const int index, CW3Image3D *&vol)
{
	return m_pImpl->readVolume(index, vol);
}

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
const bool CW3DicomIO::writeDicom(const std::string& dirPath, HeaderList listHeader, unsigned char *data)
{
	return m_pImpl->writeDicom(dirPath, listHeader, data);
}

const bool CW3DicomIO::writeDicom(const std::string& dirPath, HeaderList listHeader, unsigned short *data)
{
	return m_pImpl->writeDicom(dirPath, listHeader, data);
}

const bool CW3DicomIO::WriteDicom(const std::string& dir_path, unsigned short* data, const int instance_num, const int rows, const int columns)
{
	return m_pImpl->WriteDicom(dir_path, data, instance_num, rows, columns);
}

const bool CW3DicomIO::WriteDicomRGB(const std::string& dir_path, unsigned char* data, const int instance_num, const int rows, const int columns)
{
	return m_pImpl->WriteDicomRGB(dir_path, data, instance_num, rows, columns);
}

const bool CW3DicomIO::CheckFile(const QString& file_path)
{
	QByteArray ba = file_path.toLocal8Bit();
	std::string file_path_std_string = ba.constData();
	return m_pImpl->CheckFile(file_path_std_string);
}

const bool CW3DicomIO::CheckFile(const std::string& file_path)
{
	return m_pImpl->CheckFile(file_path);
}

void CW3DicomIO::CreateSeriesInstanceUID()
{
	m_pImpl->CreateSeriesInstanceUID();
}

void CW3DicomIO::InitSeriesInstanceUID()
{
	m_pImpl->InitSeriesInstanceUID();
}
