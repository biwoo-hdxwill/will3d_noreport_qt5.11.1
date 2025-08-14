#pragma once

/*=========================================================================

File:			class CW3DBM
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-05-11
Last modify:	2016-05-11

=========================================================================*/
#include <map>
#include <vector>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif
#include <QStringList>
#include <QtSql/qsqldatabase.h>

#include "../../Engine/Common/Common/W3Enum.h"
#include "dbmanager_global.h"

typedef struct _tracingTaskInfo {
	int id = 0;
	QString name;
	QString type;
	QStringList landmarks;
	QString volumePreset;
	bool clippingIsOn = false;
	bool clippingIsFlip = false;
	MPRClipID clippingPlane;
	float clippingPosition = false;
	float rotateMatrix[9] = { 0.0f };
} TracingTaskInfo;

class DBMANAGER_EXPORT CW3DBM {
public:
	CW3DBM();
	~CW3DBM();
	CW3DBM(const CW3DBM& t) {}

public:
	static CW3DBM* getInstance() {
		if (m_pInstance == nullptr) {
			m_pInstance = new CW3DBM();
		}
		return m_pInstance;
	}
	void destroy() {
		if (m_pInstance != nullptr) {
			delete m_pInstance;
			m_pInstance = nullptr;
		}
	}

	void setStudyLandmark(const QString& studyID, const std::map<QString, glm::vec3>& landmarkPos);
	void getTracingtasksName(QStringList& tasks);
	void getTracingtaskRecordFromName(TracingTaskInfo& taskInfo);
	void getStudyLandmarkRecord(const QString& studyID, std::map<QString, glm::vec3>& landmarkPos);
	void getLandmarkGroupFromLandmark(const QString& landmark, QString& group);
	void getMeasurementNameFromAnalysis(const QString& analysis, QList<QStringList>& measurement);
	void getNormDataRace(QStringList& races);
	bool getNormDataRecordFromMeasurement(const QString& measurement, const QString& race, float& mean, float& SD);
	void getAnalysisAll(std::map<QString, QStringList>& analysesInfo);
	void getReferenceName(QStringList& references);

	void insertTMJ(const QString &studyUID, const QString &seriesUID, const int &side, const std::vector<glm::vec3> &tmj);
	void insertOrthodontic(const QString &studyUID, const QString &seriesUID, const int &num, std::vector<glm::vec3> &ortho);
	void insertAirway(const QString &studyUID, const QString &seriesUID, const int &num, std::vector<glm::vec3> &airway);

private:	
	inline QString getWhereEqual(const QString& field, const QString& value) { return field + QString("='") + value + QString("'"); }

private:
	static CW3DBM* m_pInstance;
	QSqlDatabase m_db;
};
