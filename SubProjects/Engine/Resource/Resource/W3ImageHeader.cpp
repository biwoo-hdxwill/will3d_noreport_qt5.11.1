#include "W3ImageHeader.h"

#include "../../Common/Common/W3Logger.h"

using namespace dcm::tags;

QString CW3ImageHeader::getStudyID() { return findHeaderValue("StudyInstanceUID"); }
QString CW3ImageHeader::getSeriesID() { return findHeaderValue("SeriesInstanceUID"); }
QString CW3ImageHeader::getPatientID() { return findHeaderValue("PatientID"); }
QString CW3ImageHeader::getPatientName() { return findHeaderValue("PatientName"); }
QString CW3ImageHeader::getPatientBirthDate() { return findHeaderValue("PatientBirthDate"); }
QString CW3ImageHeader::getPatientAge() { return findHeaderValue("PatientAge"); }
QString CW3ImageHeader::getPatientSex() { return findHeaderValue("PatientSex"); }
QString CW3ImageHeader::getStudyDate() { return findHeaderValue("StudyDate"); }
QString CW3ImageHeader::getStudyTime() { return findHeaderValue("StudyTime"); }
QString CW3ImageHeader::getSeriesDate() { return findHeaderValue("SeriesDate"); }
QString CW3ImageHeader::getSeriesTime() { return findHeaderValue("SeriesTime"); }
QString CW3ImageHeader::getDescription() { return findHeaderValue("StudyDescription"); }

QString CW3ImageHeader::getKVP() { return findHeaderValue("KVP"); }
QString CW3ImageHeader::getModality() { return findHeaderValue("Modality"); }
QString CW3ImageHeader::getXRayTubeCurrent() { return findHeaderValue("XRayTubeCurrent"); }

QString CW3ImageHeader::findHeaderValue(const QString & tag)
{
	using common::Logger;
	using common::LogType;

	const auto& mapIter = m_listHeaderCore.find(tag.toStdString());
	if (mapIter != m_listHeaderCore.end())
	{
#if 0
		return QString::fromLocal8Bit(mapIter->second.c_str());
#else
		return mapIter->second;
#endif
	}

	Logger::instance()->Print(LogType::ERR,
		"CW3ImageHeader::findHeaderValue: not found");

	return QString();
}
