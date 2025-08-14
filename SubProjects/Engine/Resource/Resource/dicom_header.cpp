#include "dicom_header.h"
/*=========================================================================
Copyright (c) 2017 All rights reserved by HDXWILL.

File: dicom_header.cpp
Desc: In header.
=========================================================================*/

#include "../../Common/Common/W3Logger.h"

namespace resource
{
	QString DicomImageHeader::GetStudyID() { return FindHeaderValue("StudyInstanceUID"); }
	QString DicomImageHeader::GetSeriesID() { return FindHeaderValue("SeriesInstanceUID"); }
	QString DicomImageHeader::GetPatientID() { return FindHeaderValue("PatientID"); }
	QString DicomImageHeader::GetPatientName() { return FindHeaderValue("PatientName"); }
	QString DicomImageHeader::GetPatientBirthDate() { return FindHeaderValue("PatientBirthDate"); }
	QString DicomImageHeader::GetPatientAge() { return FindHeaderValue("PatientAge"); }
	QString DicomImageHeader::GetPatientSex() { return FindHeaderValue("PatientSex"); }
	QString DicomImageHeader::GetStudyDate() { return FindHeaderValue("StudyDate"); }
	QString DicomImageHeader::GetStudyTime() { return FindHeaderValue("StudyTime"); }
	QString DicomImageHeader::GetSeriesDate() { return FindHeaderValue("SeriesDate"); }
	QString DicomImageHeader::GetSeriesTime() { return FindHeaderValue("SeriesTime"); }
	QString DicomImageHeader::GetDescription() { return FindHeaderValue("StudyDescription"); }

	QString DicomImageHeader::GetKVP() { return FindHeaderValue("KVP"); }
	QString DicomImageHeader::GetModality() { return FindHeaderValue("Modality"); }
	QString DicomImageHeader::GetXRayTubeCurrent() { return FindHeaderValue("XRayTubeCurrent"); }

	QString DicomImageHeader::FindHeaderValue(const QString& tag)
	{
		const auto& mapIter = core_list_.find(tag.toStdString());
		if (mapIter != core_list_.end())
		{
			return QString::fromLocal8Bit(mapIter->second.c_str());
		}

		common::Logger::instance()->Print(common::LogType::ERR,
			"CW3ImageHeader::findHeaderValue: not found");

		return QString();
	}
} // end of namespace resource
