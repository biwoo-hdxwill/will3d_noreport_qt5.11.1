#include "W3DBM.h"

CW3DBM* CW3DBM::m_pInstance = nullptr;

#include <QtSql>
#include <QSqlTableModel>

#if defined(__APPLE__)
#include <QSqlQueryModel>
#endif

#include "../../Engine/Common/Common/W3Define.h"
#include "../../Engine/Common/Common/W3Logger.h"
#include <Engine/Common/Common/global_preferences.h>

using std::runtime_error;

CW3DBM::CW3DBM() 
{
	int port = GlobalPreferences::GetInstance()->preferences_.general.database.port;
	//int odbc_version = GlobalPreferences::GetInstance()->preferences_.general.database.version;
	   
	QSettings odbc_settings("C:/Windows/ODBCINST.INI", QSettings::IniFormat);
	QStringList odbc_group_list = odbc_settings.childGroups();
	QString driver_name = "";
	for (int i = 0; i < odbc_group_list.size(); ++i) 
	{
		if (odbc_group_list.at(i).indexOf("MariaDB ODBC") == 0)
		{
			driver_name = odbc_group_list.at(i);
		}
	}

	driver_name.remove(" (32 bit)");
	driver_name.remove(" (64 bit)");
	
#if defined(__APPLE__)
	m_db = QSqlDatabase::addDatabase("QMYSQL", "DBM");
	m_db.setHostName("127.0.0.1");
	m_db.setPort(port);
	m_db.setDatabaseName("Will3D");
	m_db.setUserName("root");
#else
	m_db = QSqlDatabase::addDatabase("QODBC", "DBM");
#if MARIA_DB
	m_db.setDatabaseName(QString("DRIVER={%1};SERVER=127.0.0.1;DATABASE=Will3D;Port=%2;UID=root;PWD=2002;WSID=.;").arg(driver_name).arg(port));
#else
	m_db.setDatabaseName("DRIVER={SQL Server};SERVER=127.0.0.1\\HDXWILL2014;DATABASE=Will3D;Port=1433;UID=sa;PWD=2002;WSID=.;");
#endif
#endif

	if (!m_db.open()) 
	{
		std::string err_msg = m_db.lastError().text().toStdString();
		common::Logger::instance()->Print(common::LogType::ERR, "CW3DBM::CW3DBM:" + err_msg);
	}
}

CW3DBM::~CW3DBM() {
	if (m_db.isOpen() && m_db.isValid())
		m_db.close();
}

void CW3DBM::setStudyLandmark(const QString& studyID, const std::map<QString, glm::vec3>& landmarkPos) {
	try {
		QSqlQuery q(m_db);

		q.exec(QString("SELECT * FROM StudyLandmark WHERE study_id = '%1'").arg(studyID));

		if (!q.isActive())
			throw runtime_error(q.lastError().text().toStdString().c_str());

		if (q.next())
			q.exec(QString("DELETE FROM StudyLandmark WHERE study_id = '%1'").arg(studyID));

		for (const auto& elem : landmarkPos) {
			QString str = QString(
				"INSERT INTO StudyLandmark (study_id, landmark_name, position) "
				"VALUES('%1', '%2', '%3, %4, %5')")
				.arg(studyID)
				.arg(elem.first)
				.arg(elem.second.x, 0, 'f', 3)
				.arg(elem.second.y, 0, 'f', 3)
				.arg(elem.second.z, 0, 'f', 3);
			q.exec(str);

			if (!q.isActive())
				throw runtime_error(q.lastError().text().toStdString().c_str());
		}
	}
	catch (runtime_error& e) {
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR,
			"CW3DBM::setStudyLandmark:" + err_msg);
	}
}

void CW3DBM::getTracingtasksName(QStringList& tasks) {
	try {
		QSqlQuery q(m_db);

		q.exec("SELECT Name FROM TracingTasks");

		if (!q.isActive())
			throw runtime_error(q.lastError().text().toStdString().c_str());

		tasks.clear();
		while (q.next()) {
			tasks.push_back(q.value(0).toString());
		}
	}
	catch (runtime_error& e) {
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR,
			"CW3DBM::getTracingtasksName:" + err_msg);
	}
}

void CW3DBM::getTracingtaskRecordFromName(TracingTaskInfo& taskInfo) {
	try {
		if (taskInfo.name.isEmpty())
			throw runtime_error("taskInfo name is empty");

		taskInfo.landmarks.clear();

		QSqlQuery q(m_db);

		QString q_str = QString("SELECT * FROM TracingTasks WHERE Name = '%1'").arg(taskInfo.name);
		q.exec(q_str);

		if (!q.isActive())
			throw runtime_error(q.lastError().text().toStdString().c_str());

		if (!q.next())
			throw runtime_error("empty a record list");

		QSqlRecord rec = q.record();
		taskInfo.id = q.value(rec.indexOf("ID")).toInt();
		taskInfo.type = q.value(rec.indexOf("Type")).toString();
		taskInfo.landmarks = (q.value(rec.indexOf("Landmarks")).toString()).split(",");
		taskInfo.volumePreset = q.value(rec.indexOf("VolumePreset")).toString();
		taskInfo.clippingIsOn = q.value(rec.indexOf("ClippingIsOn")).toBool();
		taskInfo.clippingIsFlip = q.value(rec.indexOf("ClippingIsFlip")).toBool();
		taskInfo.clippingPosition = q.value(rec.indexOf("ClippingPosition")).toFloat();

		QString plane = q.value(rec.indexOf("ClippingPlane")).toString();

		if (plane == QString("Axial")) {
			taskInfo.clippingPlane = MPRClipID::AXIAL;
		}
		else if (plane == QString("Coronal")) {
			taskInfo.clippingPlane = MPRClipID::CORONAL;
		}
		else if (plane == QString("Sagittal")) {
			taskInfo.clippingPlane = MPRClipID::SAGITTAL;
		}
		else {
			taskInfo.clippingPlane = MPRClipID::UNKNOWN;
		}

		QStringList rotAxis = (q.value(rec.indexOf("RotateMatrix")).toString()).split(",");

		for (auto& elem : taskInfo.landmarks)
			elem = elem.trimmed();

		if (rotAxis.size() == 9) {
			for (int i = 0; i < 9; i++)
				taskInfo.rotateMatrix[i] = rotAxis[i].toFloat();
		}

		if (q.next())
			throw runtime_error("more than one record set");
	}
	catch (runtime_error& e) {
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR,
			"CW3DBM::getTracingtaskRecordFromName:" + err_msg);
	}
}

void CW3DBM::getStudyLandmarkRecord(const QString& studyID, std::map<QString, glm::vec3>& landmarkPos) {
	try {
		if (landmarkPos.size() != 0)
			throw runtime_error("input landmark is not empty");

		QString query = QString("SELECT landmark_name, position FROM studyLandmark WHERE study_id = '%1'").arg(studyID);
		QSqlQuery q(m_db);
		q.exec(query);

		if (!q.isActive())
			throw runtime_error(q.lastError().text().toStdString().c_str());

		QSqlRecord rec = q.record();
		int idxName = rec.indexOf("Landmark_Name");
		int idxPosition = rec.indexOf("Position");
		while (q.next()) {
			QStringList strPosition = q.value(idxPosition).toString().split(",");
			glm::vec3 position(strPosition[0].trimmed().toFloat(),
				strPosition[1].trimmed().toFloat(),
				strPosition[2].trimmed().toFloat());

			landmarkPos[q.value(idxName).toString()] = position;
		}
	}
	catch (runtime_error& e) {
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR,
			"CW3DBM::getStudyLandmarkRecord:" + err_msg);
	}
}

void CW3DBM::getLandmarkGroupFromLandmark(const QString& landmark, QString& group) {
	try {
		QString query = QString("SELECT \"group\" FROM landmark WHERE name = '%1'").arg(landmark);
		QSqlQuery q(m_db);
		q.exec(query);

		if (!q.isActive())
			throw runtime_error(q.lastError().text().toStdString().c_str());

		if (q.next()) {
			group = q.value(0).toString();
		}
		else {
			throw runtime_error("landmark is not exist in landmark table");
		}
	}
	catch (runtime_error& e) {
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR,
			"CW3DBM::getLandmarkGroupFromLandmark:" + err_msg);
	}
}

void CW3DBM::getMeasurementNameFromAnalysis(const QString& analysis, QList<QStringList>& measurement)//Analysis table ¸¸μe¸e ¾ø¾iAu.
{
	try {
		QSqlQuery q(m_db);
		q.exec(QString("SELECT * FROM Analysis WHERE Name = '%1'").arg(analysis));

		if (!q.isActive())
			throw runtime_error(q.lastError().text().toStdString().c_str());

		if (!q.next())
			throw runtime_error(QString("%1 is not exist in analysis table").arg(analysis).toStdString().c_str());

		QStringList measIndices = q.value(q.record().indexOf("Measurements")).toString().split(",");
		for (int i = 0; i < measIndices.size(); i++) {
			QSqlQuery subQ(m_db);
			subQ.exec(QString("SELECT * FROM Measurement WHERE ID = '%1'").arg(measIndices[i].toInt()));

			if (!subQ.isActive())
				throw runtime_error(q.lastError().text().toStdString().c_str());

			if (!subQ.next())
				throw runtime_error(QString("measurement ID(%1) is not exist in measurement table").arg(measIndices[i].toInt()).toStdString().c_str());

			QSqlRecord rec = subQ.record();

			// tjsm : 170512
			QStringList row = {
				subQ.value(rec.indexOf("Name")).toString(),
				subQ.value(rec.indexOf("Group")).toString(),
				subQ.value(rec.indexOf("Type")).toString(),
				subQ.value(rec.indexOf("Projection")).toString() };
			//row.push_back(subQ.value(rec.indexOf("Name")).toString());
			//row.push_back(subQ.value(rec.indexOf("Group")).toString());
			//row.push_back(subQ.value(rec.indexOf("Type")).toString());
			//row.push_back(subQ.value(rec.indexOf("Projection")).toString());

			measurement.push_back(row);
		}
	}
	catch (runtime_error& e) {
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR,
			"CW3DBM::getMeasurement:" + err_msg);
	}
}

void CW3DBM::getNormDataRace(QStringList & races) {
	try {
		QSqlQuery q(m_db);
		q.exec(QString("SELECT DISTINCT Race FROM NormData"));

		if (!q.isActive())
			throw runtime_error(q.lastError().text().toStdString().c_str());

		while (q.next()) {
			races.push_back(q.value(0).toString());
		}
	}
	catch (runtime_error& e) {
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR,
			"CW3DBM::getNormDataRace:" + err_msg);
	}
}

bool CW3DBM::getNormDataRecordFromMeasurement(const QString& measurement, const QString& race, float& mean, float& SD) {
	try {
		QSqlQuery q(m_db);
		q.exec(QString("SELECT * FROM NormData WHERE Measurement_Name = '%1' and Race = '%2'")
			.arg(measurement)
			.arg(race));

		if (!q.isActive())
			throw runtime_error(q.lastError().text().toStdString().c_str());

		QSqlRecord rec = q.record();
		int idxMean = rec.indexOf("Mean");
		int idxSD = rec.indexOf("SD");

		if (q.next()) {
			mean = q.value(idxMean).toFloat();
			SD = q.value(idxSD).toFloat();
			return true;
		}
		else
			return false;
	}
	catch (runtime_error& e) {
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR,
			"CW3DBM::getNormDataRecordFromMeasurement:" + err_msg);
		return false;
	}
}

void CW3DBM::getAnalysisAll(std::map<QString, QStringList>& analysesInfo) {
	try {
		QSqlQuery q(m_db);
		q.exec("SELECT * FROM Analysis");

		if (!q.isActive())
			throw runtime_error(q.lastError().text().toStdString().c_str());

		QSqlRecord rec = q.record();
		int idxName = rec.indexOf("Name");
		int idxMeasurements = rec.indexOf("Measurements");

		while (q.next()) {
			QString analysis = q.value(idxName).toString();

			QStringList measIndices = (q.value(idxMeasurements).toString()).split(",");
			QStringList measurements;
			for (int i = 0; i < measIndices.size(); i++) {
				QSqlQuery subQ(m_db);
				subQ.exec(QString("SELECT Name FROM Measurement WHERE ID = '%1'").arg(measIndices[i].toInt()));

				if (!subQ.isActive())
					throw runtime_error(q.lastError().text().toStdString().c_str());

				if (!subQ.next())
					throw runtime_error(QString("measurement ID(%1) is not exist in measurement table").arg(measIndices[i].toInt()).toStdString().c_str());

				measurements.push_back(subQ.value(0).toString());
			}

			analysesInfo[analysis] = measurements;
		}
	}
	catch (runtime_error& e) {
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR,
			"CW3DBM::getAnalysisAll:" + err_msg);
	}
}

void CW3DBM::getReferenceName(QStringList & references) {
	try {
		QSqlQuery q(m_db);
		q.exec("SELECT Name FROM Reference");

		if (!q.isActive())
			throw runtime_error(q.lastError().text().toStdString().c_str());

		QSqlRecord rec = q.record();
		int idxName = rec.indexOf("Name");

		while (q.next()) {
			references.push_back(q.value(idxName).toString());
		}
	}
	catch (runtime_error& e) {
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR,
			"CW3DBM::getReferenceName:" + err_msg);
	}
}

void CW3DBM::insertTMJ(const QString &studyUID, const QString &seriesUID, const int &side, const std::vector<glm::vec3> &tmj) {
	try {
		QSqlQuery q(m_db);

		q.exec(QString("SELECT * FROM dataTmj WHERE study_uid = '%1' AND series_uid = '%2' AND tmj_num = %3").arg(studyUID).arg(seriesUID).arg(side));

		if (!q.isActive())
			throw runtime_error(q.lastError().text().toStdString().c_str());

		if (q.next())
			q.exec(QString("DELETE FROM dataTmj WHERE study_uid = '%1' AND series_uid = '%2' AND tmj_num = %3").arg(studyUID).arg(seriesUID).arg(side));

		for (int i = 0; i < tmj.size(); i++) {
			QString str = QString(
				"INSERT INTO dataTmj (study_uid, series_uid, tmj_num, point_num, position) "
				"VALUES('%1', '%2', %3, %4, '%5, %6, %7')")
				.arg(studyUID)
				.arg(seriesUID)
				.arg(side)
				.arg(i)
				.arg(tmj.at(i).x, 0, 'f', 3)
				.arg(tmj.at(i).y, 0, 'f', 3)
				.arg(tmj.at(i).z, 0, 'f', 3);
			q.exec(str);

			if (!q.isActive())
				throw runtime_error(q.lastError().text().toStdString().c_str());
		}
	}
	catch (runtime_error& e) {
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR,
			"CW3DBM::insertTMJ:" + err_msg);
	}
}

void CW3DBM::insertOrthodontic(const QString &studyUID, const QString &seriesUID, const int &num, std::vector<glm::vec3> &ortho) {
	try {
		QSqlQuery q(m_db);

		q.exec(QString("SELECT * FROM dataOrthodontic WHERE study_uid = '%1' AND series_uid = '%2' AND ortho_num = %3").arg(studyUID).arg(seriesUID).arg(num));

		if (!q.isActive())
			throw runtime_error(q.lastError().text().toStdString().c_str());

		if (q.next())
			q.exec(QString("DELETE FROM dataOrthodontic WHERE study_uid = '%1' AND series_uid = '%2' AND ortho_num = %3").arg(studyUID).arg(seriesUID).arg(num));

		for (int i = 0; i < ortho.size(); i++) {
			QString str = QString(
				"INSERT INTO dataOrthodontic (study_uid, series_uid, ortho_num, point_num, position) "
				"VALUES('%1', '%2', %3, %4, '%5, %6, %7')")
				.arg(studyUID)
				.arg(seriesUID)
				.arg(num)
				.arg(i)
				.arg(ortho.at(i).x, 0, 'f', 3)
				.arg(ortho.at(i).y, 0, 'f', 3)
				.arg(ortho.at(i).z, 0, 'f', 3);
			q.exec(str);

			if (!q.isActive())
				throw runtime_error(q.lastError().text().toStdString().c_str());
		}
	}
	catch (runtime_error& e) {
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR,
			"CW3DBM::insertOrthodontic:" + err_msg);
	}
}

void CW3DBM::insertAirway(const QString &studyUID, const QString &seriesUID, const int &num, std::vector<glm::vec3> &airway) {
	try {
		QSqlQuery q(m_db);

		q.exec(QString("SELECT * FROM dataAirway WHERE study_uid = '%1' AND series_uid = '%2' AND airway_num = %3").arg(studyUID).arg(seriesUID).arg(num));

		if (!q.isActive())
			throw runtime_error(q.lastError().text().toStdString().c_str());

		if (q.next())
			q.exec(QString("DELETE FROM dataAirway WHERE study_uid = '%1' AND series_uid = '%2' AND airway_num = %3").arg(studyUID).arg(seriesUID).arg(num));

		for (int i = 0; i < airway.size(); i++) {
			QString str = QString(
				"INSERT INTO dataAirway (study_uid, series_uid, airway_num, point_num, position) "
				"VALUES('%1', '%2', %3, %4, '%5, %6, %7')")
				.arg(studyUID)
				.arg(seriesUID)
				.arg(num)
				.arg(i)
				.arg(airway.at(i).x, 0, 'f', 3)
				.arg(airway.at(i).y, 0, 'f', 3)
				.arg(airway.at(i).z, 0, 'f', 3);
			q.exec(str);

			if (!q.isActive())
				throw runtime_error(q.lastError().text().toStdString().c_str());
		}
	}
	catch (runtime_error& e) {
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR,
			"CW3DBM::insertAirway:" + err_msg);
	}
}
