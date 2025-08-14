#pragma once
/*=========================================================================

File:		class CW3ImageHeader
Language:	C++11
Library:	Qt 5.2.1, Standard C++ Library

=========================================================================*/
#include <vector>
#include <map>

#include <qstring.h>

#include "resource_global.h"

namespace dcm
{
	namespace tags
	{
		/* const std::string for dicom_info header */
		const std::string kPatientID("PatientID");
		const std::string kPatientName("PatientName");
		const std::string kPatientGender("PatientSex");
		const std::string kPatientAge("PatientAge");
		const std::string kPatientBirthDate("PatientBirthDate");
		const std::string kStudyInstanceUID("StudyInstanceUID");
		const std::string kSeriesInstanceUID("SeriesInstanceUID");
		const std::string kStudyDate("StudyDate");
		const std::string kSeriesDate("SeriesDate");
		const std::string kStudyTime("StudyTime");
		const std::string kSeriesTime("SeriesTime");
		const std::string kModality("Modality");
		const std::string kKVP("KVP");
		const std::string kXRayTubeCurrent("XRayTubeCurrent");
		const std::string kExposureTime("ExposureTime");
		const std::string kPixelSpacing("PixelSpacing");
		const std::string kSliceThickness("SliceThickness");
		const std::string kBitsAllocated("BitsAllocated");
		const std::string kBitsStored("BitsStored");
		const std::string kPixelRepresentation("PixelRepresentation");
		const std::string kRescaleIntercept("RescaleIntercept");
		const std::string kRescaleSlope("RescaleSlope");
		const std::string kRows("Rows");
		const std::string kColumns("Columns");
		const std::string kWindowCenter("WindowCenter");
		const std::string kWindowWidth("WindowWidth");
		const std::string kStudyDescription("StudyDescription");
		const std::string kImagePositionPatient("ImagePositionPatient");
		const std::string kInstanceNumber("InstanceNumber");
		const std::string kSliceLocation("SliceLocation");
	} // end of namespace tags
} // end of namespace dcm

/*
	* class CW3ImageHeader
		- contains image's header information. (Volume header)
		- handled by class CW3Image3D
	* Purpose:
		- maintains header information as string pair. (to display on 2D-view)
		- handle all Dicom header information
		- use when Dicom exporting
*/
class RESOURCE_EXPORT CW3ImageHeader
{
public:
	//CW3ImageHeader(void);
	explicit CW3ImageHeader(std::map<std::string, QString>& list) { m_listHeaderCore = list; }
	~CW3ImageHeader(void) {};

public:
	// public functions.
	inline const std::map<std::string, QString>& getListCore(void) const { return m_listHeaderCore; }

	QString getStudyID();
	QString getSeriesID();
	QString getPatientID();
	QString getPatientName();
	QString getPatientBirthDate();
	QString getPatientAge();
	QString getPatientSex();
	QString getStudyDate();
	QString getStudyTime();
	QString getSeriesDate();
	QString getSeriesTime();
	QString getDescription();

	QString getKVP();
	QString getModality();
	QString getXRayTubeCurrent();

private:
	QString findHeaderValue(const QString& tag);

private:
	// private member fields.
	std::map<std::string, QString>	m_listHeaderCore;
};
