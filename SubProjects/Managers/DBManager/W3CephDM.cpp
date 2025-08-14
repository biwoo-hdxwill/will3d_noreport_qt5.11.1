#include "W3CephDM.h"
#include <iostream>
#include <qmath.h>

#include "../../Engine/Common/Common/W3Define.h"
#include "../../Engine/Common/Common/W3Logger.h"
#include "../../Engine/Resource/Resource/W3Image3D.h"
#include "../../Engine/Resource/ResContainer/resource_container.h"

#include "W3DBM.h"

namespace {
const float kDegreeToRadian = 180.0f / M_PI;
} // end of namespace

CW3CephDM::CW3CephDM(QObject * parent)
	: QObject(parent) {}

CW3CephDM::~CW3CephDM() {}

void CW3CephDM::InitFromDatabase(const QString& studyID) {
	try {
		CW3DBM* dbm = CW3DBM::getInstance();
		std::map<QString, glm::vec3> landmarks;
		dbm->getStudyLandmarkRecord(studyID, landmarks);

		if (landmarks.empty())
			return;

		this->Initialize(landmarks);
	} catch (std::runtime_error& e) {
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR,
										  "CW3CephDM::InitFromDatabase: " + err_msg);
	}
}

void CW3CephDM::InitFromProjectFile(const std::map<QString, glm::vec3>& landmarks) {
	if (landmarks.empty())
		return;

	try {
		this->Initialize(landmarks);
	} catch (std::runtime_error& e) {
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR,
										  "CW3CephDM::InitFromProjectFile: " + err_msg);
	}
}

void CW3CephDM::clear() {
	m_volLandmarks.clear();
	m_actualLandmarks.clear();
	m_isTrunOn = false;
	emit sigTurnOnEnable();
}

void CW3CephDM::addLandmark(const QString & landmarkName, const glm::vec3 & landmarkPos) {
	m_volLandmarks[landmarkName] = landmarkPos;
	m_isTrunOn = false;
}

void CW3CephDM::endEditLandmark() {
	mapVolToActualForLandmark(m_volLandmarks, m_actualLandmarks);

	m_isTrunOn = true;
	emit sigTurnOnEnable();
}

void CW3CephDM::updateLandmarkPositions(const std::map<QString, glm::vec3>& landmarks) {
	try {
		for (const auto& elem : landmarks) {
			if (m_volLandmarks.find(elem.first) == m_volLandmarks.end())
				throw std::runtime_error((elem.first + QString("does not exsist.")).toStdString().c_str());

			m_volLandmarks[elem.first] = elem.second;
		}

		this->mapVolToActualForLandmark(m_volLandmarks, m_actualLandmarks);

		emit sigUpdateLandmarkPositions();
	} catch (const std::runtime_error& e) {
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR,
										  "CW3CephDM::updateLandmarkPositions: " + err_msg);
	}
}
void CW3CephDM::setCoordinateSystem(const std::vector<glm::vec3>& coordSysPoints) {
	try {
		if (coordSysPoints.size() != 6) {
			throw std::runtime_error("not formatted coordinate system");
		}
		m_coordSysVolPoints.clear();

		glm::vec3 reorienZ = glm::normalize(glm::cross((coordSysPoints[1] - coordSysPoints[2]), (coordSysPoints[0] - coordSysPoints[2])));
		glm::vec3 reorienX = glm::normalize(glm::cross((coordSysPoints[4] - coordSysPoints[5]), reorienZ));
		glm::vec3 reorienY = glm::normalize(glm::cross(reorienZ, reorienX));

		m_coordSysVolMatrix = glm::mat4(glm::vec4(reorienX, 0.0), glm::vec4(reorienY, 0.0),
										glm::vec4(reorienZ, 0.0), glm::vec4(0.0, 0.0, 0.0, 1.0));
		m_coordSysVolPoints.assign(coordSysPoints.begin(), coordSysPoints.end());

		const CW3Image3D& vol = ResourceContainer::GetInstance()->GetMainVolume();
		const int width = vol.width();
		const int height = vol.height();
		const int depth = vol.depth();
		const float pixelSpacing = vol.pixelSpacing();
		const float sliceSpacing = vol.sliceSpacing();

		const glm::vec3 posOrigin = m_coordSysVolPoints.back();

		for (const auto& elem : m_coordSysVolPoints) {
			glm::vec3 actPos(
				-(elem.x - posOrigin.x) * width * pixelSpacing*0.5,// gl좌표계에서 volume x축은 뒤집혀 있다.
				(elem.y - posOrigin.y) * height * pixelSpacing*0.5,
				-(elem.z - posOrigin.z) * depth * sliceSpacing*0.5); //데카르트 좌표계 Z

			m_coordSysActPoints.push_back(actPos);
		}

		glm::vec3 az = glm::normalize(glm::cross((m_coordSysActPoints[1] - m_coordSysActPoints[2]), (m_coordSysActPoints[0] - m_coordSysActPoints[2])));
		glm::vec3 ax = glm::normalize(glm::cross((m_coordSysActPoints[4] - m_coordSysActPoints[5]), az));
		glm::vec3 ay = glm::normalize(glm::cross(az, ax));

		m_coordSysActMatrix = glm::mat4(glm::vec4(ax, 0.0), glm::vec4(ay, 0.0),
										glm::vec4(az, 0.0), glm::vec4(0.0, 0.0, 0.0, 1.0));
		//m_coordSysVolMatrix = (mat4(vec4(-ax, 0.0), vec4(ay, 0.0), vec4(-az, 0.0), vec4(0.0, 0.0, 0.0, 1.0)));
		m_isSetCoordSys = true;
	} catch (std::runtime_error& e) {
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR,
										  "CW3CephDM::setCoordinateSystem: " + err_msg);
	}
}

void CW3CephDM::mapVolToActualForLandmark(const std::map<QString, glm::vec3>& volLandmarks, std::map<QString, glm::vec3>& actualLandmarks) {
	try {
		if (!m_isSetCoordSys)
			throw std::runtime_error("coordinate system not enabled");

		const glm::vec3 posOrigin = m_coordSysVolPoints.back();

		const CW3Image3D& vol = ResourceContainer::GetInstance()->GetMainVolume();
		const int width = vol.width();
		const int height = vol.height();
		const int depth = vol.depth();
		const float pixelSpacing = vol.pixelSpacing();
		const float sliceSpacing = vol.sliceSpacing();

		for (const auto& elem : volLandmarks) {
			glm::vec3 landmarkPos(-(elem.second.x - posOrigin.x) * width * pixelSpacing*0.5,// gl좌표계에서 volume x축은 뒤집혀 있다.
				(elem.second.y - posOrigin.y) * height * pixelSpacing*0.5,
								  -(elem.second.z - posOrigin.z) * depth * sliceSpacing*0.5); //데카르트 좌표계 Z

			actualLandmarks[elem.first] = glm::vec3(landmarkPos.x, landmarkPos.y, landmarkPos.z);
		}
	} catch (std::runtime_error& e) {
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR,
										  "CW3CephDM::mapVolToActualForLandmark: " + err_msg);
	}
}

glm::vec3 CW3CephDM::mapActualToVolForLandmark(const glm::vec3 & actualLandmark) {
	try {
		if (!m_isSetCoordSys)
			throw std::runtime_error("coordinate system not enabled");

		const glm::vec3 posOrigin = m_coordSysVolPoints.back();

		const CW3Image3D& vol = ResourceContainer::GetInstance()->GetMainVolume();
		const int width = vol.width();
		const int height = vol.height();
		const int depth = vol.depth();
		const float pixelSpacing = vol.pixelSpacing();
		const float sliceSpacing = vol.sliceSpacing();
		return glm::vec3(-actualLandmark.x / (width * pixelSpacing) * 2.0f + posOrigin.x,
						 actualLandmark.y / (height * pixelSpacing)*2.0f + posOrigin.y,
						 -actualLandmark.z / (depth*sliceSpacing)*2.0f + posOrigin.z);
	} catch (std::runtime_error& e) {
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR,
										  "CW3CephDM::mapActualToVolForLandmark: " + err_msg);
		return glm::vec3(0.0f);
	}
}

glm::vec4 CW3CephDM::getReferencePlane(const QString& referencePlane) {
	try {
		if (!m_isTrunOn && !m_isSetCoordSys)
			throw std::runtime_error("must be initialized..");

		return this->funcReferencePlane(m_coordSysVolMatrix, m_coordSysVolPoints, m_volLandmarks, referencePlane);
	} catch (std::runtime_error& e) {
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR,
										  "CW3CephDM::getReferencePlane: " + err_msg);
		return glm::vec4(0.0f);
	}
}

float CW3CephDM::getMeasurementValue(const QString& measure) {
	try {
		if (!m_isTrunOn)
			throw std::runtime_error("must be initialized..");

		return this->funcMeasurement(measure);
	} catch (std::runtime_error& e) {
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR,
										  "CW3CephDM::getMeasurementValue: " + err_msg);
		return 0.0f;
	}

	return this->funcMeasurement(measure);
}

void CW3CephDM::getMeasurementInfo(const QString & measure, MEASURE_INFO & info) {
	glm::vec4 MSPlane = funcReferencePlane(m_coordSysActMatrix, m_coordSysActPoints,
										   m_actualLandmarks, "Mid-Sagittal Plane");
	glm::vec4 OCPlane = funcReferencePlane(m_coordSysActMatrix, m_coordSysActPoints,
										   m_actualLandmarks, "Occlusal Plane R");
	glm::vec4 FHPlane = funcReferencePlane(
		m_coordSysActMatrix, 
		m_coordSysActPoints,
		m_actualLandmarks, 
		"FH Plane"
	);

	if (measure == "ANB Ang_2D") {
		angle_3point_measureInfo(getActualLandmarkCoord("N"), getActualLandmarkCoord("A"),
								 getActualLandmarkCoord("B"), MSPlane, info);
	} else if (measure == "SNA Ang_2D") {
		angle_3point_measureInfo(getActualLandmarkCoord("N"), getActualLandmarkCoord("Sella"),
								 getActualLandmarkCoord("A"), MSPlane, info);
	} else if (measure == "SNB Ang_2D") {
		angle_3point_measureInfo(getActualLandmarkCoord("N"), getActualLandmarkCoord("Sella"),
								 getActualLandmarkCoord("B"), MSPlane, info);
	} else if (measure == "Pog to NB_2D") {
		dist_point2line_measureInfo(getActualLandmarkCoord("Pog"), getActualLandmarkCoord("B"),
									getActualLandmarkCoord("N"), MSPlane, info);
	} else if (measure == "GoGn to SN Ang_2D") {
		angle_4point_measureInfo(getActualLandmarkCoord("Go_R"), getActualLandmarkCoord("Gn"),
								 getActualLandmarkCoord("Sella"), getActualLandmarkCoord("N"), MSPlane, info);
	} else if (measure == "Y-(growth) Axis Ang_2D") {
		angle_4point_measureInfo(getActualLandmarkCoord("Sella"), getActualLandmarkCoord("Gn"),
								 getActualLandmarkCoord("Po_R"), getActualLandmarkCoord("Or_R"), MSPlane, info);
	} else if (measure == "OP to SN Ang_2D") {
		angle_line2plane_measureInfo(getActualLandmarkCoord("Sella"), getActualLandmarkCoord("N"), OCPlane, MSPlane, info);
	} else if (measure == "U1 to NA_2D") {
		dist_point2line_measureInfo(getActualLandmarkCoord("Ullabial_R"), getActualLandmarkCoord("A"),
									getActualLandmarkCoord("N"), MSPlane, info);
	} else if (measure == "U1 to NB_2D") {
		dist_point2line_measureInfo(getActualLandmarkCoord("Ullabial_R"), getActualLandmarkCoord("B"),
									getActualLandmarkCoord("N"), MSPlane, info);
	} else if (measure == "U1 to NA Ang_2D") {
		angle_4point_measureInfo(getActualLandmarkCoord("Ulroot_R"), getActualLandmarkCoord("Ulcrown_R"),
								 getActualLandmarkCoord("N"), getActualLandmarkCoord("A"), MSPlane, info);
	} else if (measure == "U1 to L1 Ang_2D") {
		angle_4point_measureInfo(getActualLandmarkCoord("Ulroot_R"), getActualLandmarkCoord("Ulcrown_R"),
								 getActualLandmarkCoord("Llroot_R"), getActualLandmarkCoord("Llcrown_R"), MSPlane, info);
	} else if (measure == "L1 to NB Ang_2D") {
		angle_4point_measureInfo(getActualLandmarkCoord("Llroot_R"), getActualLandmarkCoord("Llcrown_R"),
								 getActualLandmarkCoord("B"), getActualLandmarkCoord("N"), MSPlane, info);
	} else if (measure == "Lw Lip to E-Pln_2D") {
		dist_point2line_measureInfo(getActualLandmarkCoord("Li"), getActualLandmarkCoord("Pn"),
									getActualLandmarkCoord("Soft_Pog"), MSPlane, info);
	} else if (measure == "Lw Lip to H-Line 2D") {
		dist_point2line_measureInfo(getActualLandmarkCoord("Li"), getActualLandmarkCoord("Ls"),
									getActualLandmarkCoord("Soft_Pog"), MSPlane, info);
	} else if (measure == "Up Lip to E-Pln_2D") {
		dist_point2line_measureInfo(getActualLandmarkCoord("Ls"), getActualLandmarkCoord("Pn"),
									getActualLandmarkCoord("Soft_Pog"), MSPlane, info);
	} else if (measure == "L1 to NB_2D") {
		dist_point2line_measureInfo(getActualLandmarkCoord("Lllabial_R"), getActualLandmarkCoord("B"),
									getActualLandmarkCoord("N"), MSPlane, info);
	} else if (measure == "L1 to APog_2D") {
		(dist_point2line_measureInfo(getActualLandmarkCoord("Lllabial_R"), getActualLandmarkCoord("A"),
									 getActualLandmarkCoord("Pog"), MSPlane, info));
	} else if (measure == "IMPA(L1-MP) Ang_2D") {
		angle_4point_measureInfo(getActualLandmarkCoord("Llcrown_R"), getActualLandmarkCoord("Llroot_R"),
								 getActualLandmarkCoord("Go_R"), getActualLandmarkCoord("Me"), MSPlane, info);
	} else if (measure == "FMIA(L1-FH) Ang_2D") {
		angle_4point_measureInfo(getActualLandmarkCoord("Llcrown_R"), getActualLandmarkCoord("Llroot_R"),
								 getActualLandmarkCoord("Or_R"), getActualLandmarkCoord("Po_R"), MSPlane, info);
	} else if (measure == "FMA(MP-FH) Ang_2D") {
		angle_4point_measureInfo(getActualLandmarkCoord("Go_R"), getActualLandmarkCoord("Me"),
								 getActualLandmarkCoord("Po_R"), getActualLandmarkCoord("Or_R"), MSPlane, info);
	} else if (measure == "MP-OP Ang_2D") {
		angle_line2plane_measureInfo(getActualLandmarkCoord("Go_R"), getActualLandmarkCoord("Me"),
									 OCPlane, MSPlane, info);
	} else if (measure == "MP-SN Ang_2D") {
		angle_4point_measureInfo(getActualLandmarkCoord("Go_R"), getActualLandmarkCoord("Me"),
								 getActualLandmarkCoord("Sella"), getActualLandmarkCoord("N"), MSPlane, info);
	} else if (measure == "U1 to SN Ang_2D") {
		angle_4point_measureInfo(getActualLandmarkCoord("Ulcrown_R"), getActualLandmarkCoord("Ulroot_R"),
								 getActualLandmarkCoord("Sella"), getActualLandmarkCoord("N"), MSPlane, info);
	} else if (measure == "Ang of Convexity_2D") {
		angle_4point_measureInfo(getActualLandmarkCoord("N"), getActualLandmarkCoord("A"),
								 getActualLandmarkCoord("A"), getActualLandmarkCoord("Pog"), MSPlane, info);
	} else 0.0;

#if 1
	if (measure == "SNA")
	{
		info.type = CephMeasureType::ANGLE;
		info.points.push_back(glm::vec3(0.0f));
		info.points.push_back(glm::vec3(0.0f));
		info.val = 0.0f;
	}
	else if (measure == "SNB")
	{
		angle_3point_measureInfo(
			getActualLandmarkCoord("N"), 
			getActualLandmarkCoord("Sella"),
			getActualLandmarkCoord("B point"), 
			MSPlane, 
			info
		);
	}
	else if (measure == "ANB")
	{
		info.type = CephMeasureType::ANGLE;
		info.points.push_back(glm::vec3(0.0f));
		info.points.push_back(glm::vec3(0.0f));
		info.val = 0.0f;
	}
	else if (measure == "Wits appraisal")
	{
		info.type = CephMeasureType::DISTANCE;
		info.points.push_back(glm::vec3(0.0f));
		info.points.push_back(glm::vec3(0.0f));
		info.val = 0.0f;
	}
	else if (measure == "Min. Body length Lt.")
	{
		dist_2point_measureInfo(
			getActualLandmarkCoord("Go_mid_L"),
			getActualLandmarkCoord("M_e"),
			MSPlane,
			info
		);
	}
	else if (measure == "Min. Body length Rt.")
	{
		dist_2point_measureInfo(
			getActualLandmarkCoord("Go_mid_R"),
			getActualLandmarkCoord("M_e"),
			MSPlane,
			info
		);
	}
	else if (measure == "Nperp to A")
	{
		info.type = CephMeasureType::DISTANCE;
		info.points.push_back(glm::vec3(0.0f));
		info.points.push_back(glm::vec3(0.0f));
		info.val = 0.0f;
	}
	else if (measure == "Nperp to Pog")
	{
		dist_point2line_measureInfo(
			getActualLandmarkCoord("Pog"),
			getActualLandmarkCoord("N"),
			getActualLandmarkCoord("N") +
			perp_to_line_on_plane(
				getActualLandmarkCoord("Sella"),
				getActualLandmarkCoord("N"),
				MSPlane
			),
			MSPlane,
			info
		);
	}
	else if (measure == "Ramus height Lt.")
	{
		dist_2point_measureInfo(
			getActualLandmarkCoord("Go_mid_L"),
			getActualLandmarkCoord("Con_L"),
			MSPlane,
			info
		);
	}
	else if (measure == "Ramus height Rt.")
	{
		dist_2point_measureInfo(
			getActualLandmarkCoord("Go_mid_R"),
			getActualLandmarkCoord("Con_R"),
			MSPlane,
			info
		);
	}
	else if (measure == "Gonial angle Lt.")
	{
		angle_3point_measureInfo(
			getActualLandmarkCoord("Go_mid_L"),
			getActualLandmarkCoord("Con_L"),
			getActualLandmarkCoord("M_e"),
			MSPlane,
			info
		);
	}
	else if (measure == "Gonial angle Rt.")
	{
		angle_3point_measureInfo(
			getActualLandmarkCoord("Go_mid_R"),
			getActualLandmarkCoord("Con_R"),
			getActualLandmarkCoord("M_e"),
			MSPlane,
			info
		);
	}
	else if (measure == "Anterior facial height")
	{
		dist_2point_measureInfo(
			getActualLandmarkCoord("N"),
			getActualLandmarkCoord("M_e"),
			MSPlane,
			info
		);
	}
	else if (measure == "Posterior facial height Lt.")
	{
		dist_2point_measureInfo(
			getActualLandmarkCoord("Sella"),
			getActualLandmarkCoord("Go_mid_L"),
			MSPlane,
			info
		);
	}
	else if (measure == "Posterior facial height Rt.")
	{
		dist_2point_measureInfo(
			getActualLandmarkCoord("Sella"),
			getActualLandmarkCoord("Go_mid_R"),
			MSPlane,
			info
		);
	}
	else if (measure == "Palatal plane angle")
	{
		info.type = CephMeasureType::ANGLE;
		info.points.push_back(glm::vec3(0.0f));
		info.points.push_back(glm::vec3(0.0f));
		info.val = 0.0f;
	}
	else if (measure == "Mandibular plane angle")
	{
		angle_line2plane_measureInfo(
			getActualLandmarkCoord("Go_mid_L") + getActualLandmarkCoord("Go_mid_R") * 0.5f,
			getActualLandmarkCoord("M_e"),
			FHPlane,
			MSPlane,
			info
		);
	}
	else if (measure == "Upper occlusal plane angle")
	{
		info.type = CephMeasureType::ANGLE;
		info.points.push_back(glm::vec3(0.0f));
		info.points.push_back(glm::vec3(0.0f));
		info.val = 0.0f;
	}
	else if (measure == "Lower occlusal plane angle")
	{
		info.type = CephMeasureType::ANGLE;
		info.points.push_back(glm::vec3(0.0f));
		info.points.push_back(glm::vec3(0.0f));
		info.val = 0.0f;
	}
	else if (measure == "Occlusal plane angle")
	{
		info.type = CephMeasureType::ANGLE;
		info.points.push_back(glm::vec3(0.0f));
		info.points.push_back(glm::vec3(0.0f));
		info.val = 0.0f;
	}
#endif
}
glm::vec3 CW3CephDM::getActualLandmarkCoord(const QString & str) {
	if (m_actualLandmarks.find(str) != m_actualLandmarks.end())
		return m_actualLandmarks[str];
	else
		return glm::vec3();
}
void CW3CephDM::Initialize(const std::map<QString, glm::vec3>& landmarks) {
	QStringList tracingTasks;
	CW3DBM* dbm = CW3DBM::getInstance();
	dbm->getTracingtasksName(tracingTasks);

	if (tracingTasks.size() < 6)
		throw std::runtime_error("tacingtask smaller than six.");

	std::vector<glm::vec3> coordSysPoints;
	coordSysPoints.reserve(6);
	for (int i = 0; i < 6; i++) {
		TracingTaskInfo taskInfo;
		taskInfo.name = tracingTasks[i];
		dbm->getTracingtaskRecordFromName(taskInfo);
		
		auto iter = landmarks.find(taskInfo.landmarks[0]);
		if (iter == landmarks.end())
			return;

		coordSysPoints.push_back(iter->second);
	}

	setCoordinateSystem(coordSysPoints);

	m_volLandmarks.clear();
	m_actualLandmarks.clear();
	m_volLandmarks.insert(landmarks.begin(), landmarks.end());
	mapVolToActualForLandmark(m_volLandmarks, m_actualLandmarks);

	m_isTrunOn = true;

	emit sigTurnOnEnable();
}
glm::vec4 CW3CephDM::funcReferencePlane(const glm::mat4& coordMat4, const std::vector<glm::vec3>& coordSysPts,
										std::map<QString, glm::vec3> landmarks, const QString& plane) {
	if (coordSysPts.size() < 6)
		return glm::vec4(0.0);

	if (plane == "Mid-Sagittal Plane") {
		glm::mat3 coordMat(coordMat4);
		return glm::vec4(coordMat[0], (glm::dot(coordMat[0], coordSysPts[5])));
	} else if (plane == "FH Plane") {
		return plane_3point(landmarks["Or_L"], landmarks["Po_R"], landmarks["Po_L"]);
	} else if (plane == "Frontal Plane") {
#if 0
		glm::mat3 coordMat(coordMat4);
		return glm::vec4(coordMat[1], (glm::dot(coordMat[1], coordSysPts[5])));
#else
		return plane_1pointPerp2(landmarks["N"], funcReferencePlane(coordMat4, coordSysPts, landmarks, "FH Plane"), funcReferencePlane(coordMat4, coordSysPts, landmarks, "Mid-Sagittal Plane"));
#endif
	} else if (plane == "Frankfort Horizontal Plane R") {
		return plane_3point(landmarks["Or_R"], landmarks["Po_R"], landmarks["Po_L"]);
	} else if (plane == "Maxillary Plane") {
		return plane_2pointPerp(landmarks["ANS"], landmarks["PNS"], funcReferencePlane(coordMat4, coordSysPts, landmarks, "Mid-Sagittal Plane"));
	} else if (plane == "Ba-N Plane") {
		return plane_2pointPerp(landmarks["Ba"], landmarks["N"], funcReferencePlane(coordMat4, coordSysPts, landmarks, "Mid-Sagittal Plane"));
	} else if (plane == "A FH Perp") {
		return plane_1pointPerp2(landmarks["ANS"], funcReferencePlane(coordMat4, coordSysPts, landmarks, "Frankfort Horizontal Plane R"), funcReferencePlane(coordMat4, coordSysPts, landmarks, "Mid-Sagittal Plane"));
	} else if (plane == "Occlusal Plane R") {
		return plane_4pointPerp(landmarks["LMcusp_R"], landmarks["UMcusp_R"], landmarks["Llcrown_R"], landmarks["Ulcrown_R"],
								funcReferencePlane(coordMat4, coordSysPts, landmarks, "Mid-Sagittal Plane"));
	} else if (plane == "N-Occl Perp") {
		return plane_1pointPerp2(landmarks["N"], funcReferencePlane(coordMat4, coordSysPts, landmarks, "Mid-Sagittal Plane"), funcReferencePlane(coordMat4, coordSysPts, landmarks, "Occlusal Plane R"));
	} else if (plane == "N Ba-N Perp") {
		return plane_1pointPerp2(landmarks["N"], funcReferencePlane(coordMat4, coordSysPts, landmarks, "Ba-N Plane"), funcReferencePlane(coordMat4, coordSysPts, landmarks, "Mid-Sagittal Plane"));
	} else if (plane == "N FH Perp") {
		return plane_1pointPerp2(landmarks["N"], funcReferencePlane(coordMat4, coordSysPts, landmarks, "Frankfort Horizontal Plane R"), funcReferencePlane(coordMat4, coordSysPts, landmarks, "Mid-Sagittal Plane"));
	} else if (plane == "OLs") {
		return plane_2pointPerp(landmarks["Ulcrown_R"], landmarks["UMcusp_R"], funcReferencePlane(coordMat4, coordSysPts, landmarks, "Mid-Sagittal Plane"));
	} else if (plane == "OLi") {
		return plane_2pointPerp(landmarks["Llcrown_R"], landmarks["LMcusp_R"], funcReferencePlane(coordMat4, coordSysPts, landmarks, "Mid-Sagittal Plane"));
	} else if (plane == "ML") {
		return plane_3point(landmarks["Me"], landmarks["Go_L"], landmarks["Go_R"]);
	} else if (plane == "B Perp MP") {
		return plane_1pointPerp2(landmarks["B"], funcReferencePlane(coordMat4, coordSysPts, landmarks, "Mid-Sagittal Plane"), funcReferencePlane(coordMat4, coordSysPts, landmarks, "Mandibular Plane"));
	} else if (plane == "Mandibular Plane") {
		return plane_3point(landmarks["Me"], landmarks["Go_L"], landmarks["Go_R"]);
	} else return glm::vec4(0.0);
}
float CW3CephDM::funcMeasurement(const QString& measure)
{
	glm::vec4 MSPlane = funcReferencePlane(m_coordSysActMatrix, m_coordSysActPoints,
		m_actualLandmarks, "Mid-Sagittal Plane");
	glm::vec4 OCPlane = funcReferencePlane(m_coordSysActMatrix, m_coordSysActPoints,
		m_actualLandmarks, "Occlusal Plane R");
	glm::vec4 FHPlane = funcReferencePlane(
		m_coordSysActMatrix,
		m_coordSysActPoints,
		m_actualLandmarks,
		"FH Plane"
	);

	if (measure == "ANB Ang_2D")
	{
		return angle_3point(getActualLandmarkCoord("N"), getActualLandmarkCoord("A"),
			getActualLandmarkCoord("B"), MSPlane);
	}
	else if (measure == "SNA Ang_2D")
	{
		return angle_3point(getActualLandmarkCoord("N"), getActualLandmarkCoord("Sella"),
			getActualLandmarkCoord("A"), MSPlane);
	}
	else if (measure == "SNB Ang_2D")
	{
		return angle_3point(getActualLandmarkCoord("N"), getActualLandmarkCoord("Sella"),
			getActualLandmarkCoord("B"), MSPlane);
	}
	else if (measure == "Pog to NB_2D")
	{
		return dist_point2line(getActualLandmarkCoord("Pog"), getActualLandmarkCoord("B"),
			getActualLandmarkCoord("N"), MSPlane);
	}
	else if (measure == "GoGn to SN Ang_2D")
	{
		return angle_4point(getActualLandmarkCoord("Go_R"), getActualLandmarkCoord("Gn"),
			getActualLandmarkCoord("Sella"), getActualLandmarkCoord("N"), MSPlane);
	}
	else if (measure == "Y-(growth) Axis Ang_2D")
	{
		return angle_4point(getActualLandmarkCoord("Sella"), getActualLandmarkCoord("Gn"),
			getActualLandmarkCoord("Po_R"), getActualLandmarkCoord("Or_R"), MSPlane);
	}
	else if (measure == "OP to SN Ang_2D")
	{
		return angle_line2plane(getActualLandmarkCoord("Sella"), getActualLandmarkCoord("N"), OCPlane, MSPlane);
	}
	else if (measure == "U1 to NA_2D")
	{
		return dist_point2line(getActualLandmarkCoord("Ullabial_R"), getActualLandmarkCoord("A"),
			getActualLandmarkCoord("N"), MSPlane);
	}
	else if (measure == "U1 to NB_2D")
	{
		return dist_point2line(getActualLandmarkCoord("Ullabial_R"), getActualLandmarkCoord("B"),
			getActualLandmarkCoord("N"), MSPlane);
	}
	else if (measure == "U1 to NA Ang_2D")
	{
		return angle_4point(getActualLandmarkCoord("Ulroot_R"), getActualLandmarkCoord("Ulcrown_R"),
			getActualLandmarkCoord("N"), getActualLandmarkCoord("A"), MSPlane);
	}
	else if (measure == "U1 to L1 Ang_2D")
	{
		return angle_4point(getActualLandmarkCoord("Ulroot_R"), getActualLandmarkCoord("Ulcrown_R"),
			getActualLandmarkCoord("Llroot_R"), getActualLandmarkCoord("Llcrown_R"), MSPlane);
	}
	else if (measure == "L1 to NB Ang_2D")
	{
		return angle_4point(getActualLandmarkCoord("Llroot_R"), getActualLandmarkCoord("Llcrown_R"),
			getActualLandmarkCoord("B"), getActualLandmarkCoord("N"), MSPlane);
	}
	else if (measure == "Lw Lip to E-Pln_2D")
	{
		return -dist_point2line(getActualLandmarkCoord("Li"), getActualLandmarkCoord("Pn"),
			getActualLandmarkCoord("Soft_Pog"), MSPlane);
	}
	else if (measure == "Lw Lip to H-Line 2D")
	{
		return -dist_point2line(getActualLandmarkCoord("Li"), getActualLandmarkCoord("Ls"),
			getActualLandmarkCoord("Soft_Pog"), MSPlane);
	}
	else if (measure == "Up Lip to E-Pln_2D")
	{
		return -dist_point2line(getActualLandmarkCoord("Ls"), getActualLandmarkCoord("Pn"),
			getActualLandmarkCoord("Soft_Pog"), MSPlane);
	}
	else if (measure == "L1 to NB_2D")
	{
		return dist_point2line(getActualLandmarkCoord("Lllabial_R"), getActualLandmarkCoord("B"),
			getActualLandmarkCoord("N"), MSPlane);
	}
	else if (measure == "L1 to APog_2D")
	{
		return abs(dist_point2line(getActualLandmarkCoord("Lllabial_R"), getActualLandmarkCoord("A"),
			getActualLandmarkCoord("Pog"), MSPlane));
	}
	else if (measure == "IMPA(L1-MP) Ang_2D")
	{
		return angle_4point(getActualLandmarkCoord("Llcrown_R"), getActualLandmarkCoord("Llroot_R"),
			getActualLandmarkCoord("Go_R"), getActualLandmarkCoord("Me"), MSPlane);
	}
	else if (measure == "FMIA(L1-FH) Ang_2D")
	{
		return angle_4point(getActualLandmarkCoord("Llcrown_R"), getActualLandmarkCoord("Llroot_R"),
			getActualLandmarkCoord("Or_R"), getActualLandmarkCoord("Po_R"), MSPlane);
	}
	else if (measure == "FMA(MP-FH) Ang_2D")
	{
		return angle_4point(getActualLandmarkCoord("Go_R"), getActualLandmarkCoord("Me"),
			getActualLandmarkCoord("Po_R"), getActualLandmarkCoord("Or_R"), MSPlane);
	}
	else if (measure == "MP-OP Ang_2D")
	{
		return angle_line2plane(getActualLandmarkCoord("Go_R"), getActualLandmarkCoord("Me"), OCPlane, MSPlane);
	}
	else if (measure == "MP-SN Ang_2D")
	{
		return angle_4point(getActualLandmarkCoord("Go_R"), getActualLandmarkCoord("Me"),
			getActualLandmarkCoord("Sella"), getActualLandmarkCoord("N"), MSPlane);
	}
	else if (measure == "U1 to SN Ang_2D")
	{
		return angle_4point(getActualLandmarkCoord("Ulcrown_R"), getActualLandmarkCoord("Ulroot_R"),
			getActualLandmarkCoord("Sella"), getActualLandmarkCoord("N"), MSPlane);
	}
	else if (measure == "Ang of Convexity_2D")
	{
		return angle_4point(getActualLandmarkCoord("N"), getActualLandmarkCoord("A"),
			getActualLandmarkCoord("A"), getActualLandmarkCoord("Pog"), MSPlane);
	}
	else if (measure == "Holdaway Ratio_2D")
	{
		return abs(dist_point2line(getActualLandmarkCoord("Ullabial_R"), getActualLandmarkCoord("B"), getActualLandmarkCoord("N"), MSPlane)
			/ dist_point2line(getActualLandmarkCoord("Pog"), getActualLandmarkCoord("B"), getActualLandmarkCoord("N"), MSPlane));
	}
#if 1
	else if (measure == "SNA")
	{
		return 0.0f;
	}
	else if (measure == "SNB")
	{
		return angle_3point(
			getActualLandmarkCoord("N"),
			getActualLandmarkCoord("Sella"),
			getActualLandmarkCoord("B point"),
			MSPlane
		);
	}
	else if (measure == "ANB")
	{
		return 0.0f;
	}
	else if (measure == "Wits appraisal")
	{
		return 0.0f;
	}
	else if (measure == "Min. Body length Lt.")
	{
		return dist_2point(
			getActualLandmarkCoord("Go_mid_L"),
			getActualLandmarkCoord("M_e"),
			MSPlane
		);
	}
	else if (measure == "Min. Body length Rt.")
	{
		return dist_2point(
			getActualLandmarkCoord("Go_mid_R"),
			getActualLandmarkCoord("M_e"),
			MSPlane
		);
	}
	else if (measure == "Nperp to A")
	{
		return 0.0f;
	}
	else if (measure == "Nperp to Pog")
	{
		return dist_point2line(
			getActualLandmarkCoord("Pog"),
			getActualLandmarkCoord("N"),
			getActualLandmarkCoord("N") +
			perp_to_line_on_plane(
				getActualLandmarkCoord("Sella"),
				getActualLandmarkCoord("N"),
				MSPlane
			),
			MSPlane
		);
	}
	else if (measure == "Ramus height Lt.")
	{
		return dist_2point(
			getActualLandmarkCoord("Go_mid_L"),
			getActualLandmarkCoord("Con_L"),
			MSPlane
		);
	}
	else if (measure == "Ramus height Rt.")
	{
		return dist_2point(
			getActualLandmarkCoord("Go_mid_R"),
			getActualLandmarkCoord("Con_R"),
			MSPlane
		);
	}
	else if (measure == "Gonial angle Lt.")
	{
		return angle_3point(
			getActualLandmarkCoord("Go_mid_L"),
			getActualLandmarkCoord("Con_L"),
			getActualLandmarkCoord("M_e"),
			MSPlane
		);
	}
	else if (measure == "Gonial angle Rt.")
	{
		return angle_3point(
			getActualLandmarkCoord("Go_mid_R"),
			getActualLandmarkCoord("Con_R"),
			getActualLandmarkCoord("M_e"),
			MSPlane
		);
	}
	else if (measure == "Anterior facial height")
	{
		return dist_2point(
			getActualLandmarkCoord("N"),
			getActualLandmarkCoord("M_e"),
			MSPlane
		);
	}
	else if (measure == "Posterior facial height Lt.")
	{
		return dist_2point(
			getActualLandmarkCoord("Sella"),
			getActualLandmarkCoord("Go_mid_L"),
			MSPlane
		);
	}
	else if (measure == "Posterior facial height Rt.")
	{
		return dist_2point(
			getActualLandmarkCoord("Sella"),
			getActualLandmarkCoord("Go_mid_R"),
			MSPlane
		);
	}
	else if (measure == "Palatal plane angle")
	{
		return 0.0f;
	}
	else if (measure == "Mandibular plane angle")
	{
		return angle_line2plane(
			getActualLandmarkCoord("Go_mid_L") + getActualLandmarkCoord("Go_mid_R") * 0.5f,
			getActualLandmarkCoord("M_e"),
			FHPlane,
			MSPlane
		);
	}
	else if (measure == "Upper occlusal plane angle")
	{
		return 0.0f;
	}
	else if (measure == "Lower occlusal plane angle")
	{
		return 0.0f;
	}
	else if (measure == "Occlusal plane angle")
	{
		return 0.0f;
	}
#endif
	else
	{
		return 0.0;
	}
}
glm::vec3 CW3CephDM::proj(const glm::vec3& point, const glm::vec4& plane) {
	glm::vec3 planeNorm(plane.x, plane.y, plane.z);
	return point - (glm::dot(point, planeNorm) - plane.w)*planeNorm;
}
float CW3CephDM::angle_3point(const glm::vec3& point1, const glm::vec3& point2, const glm::vec3& point3,
							  const glm::vec4& projPlane) {
	glm::vec3 o = proj(point1, projPlane);
	glm::vec3 v1 = proj(point2, projPlane) - o;
	glm::vec3 v2 = proj(point3, projPlane) - o;

	return acos(glm::dot(v1, v2) / (glm::length(v1)*glm::length(v2)))*kDegreeToRadian;
}
float CW3CephDM::angle_4point(const glm::vec3& point1, const glm::vec3& point2, const glm::vec3& point3, const glm::vec3& point4,
							  const glm::vec4& projPlane) {
	glm::vec3 v1 = proj(point2, projPlane) - proj(point1, projPlane);
	glm::vec3 v2 = proj(point4, projPlane) - proj(point3, projPlane);
	return acos(glm::dot(v1, v2) / (glm::length(v1)*glm::length(v2)))*kDegreeToRadian;
}

float CW3CephDM::dist_2point(const glm::vec3& point1, const glm::vec3& point2, const glm::vec4& proj_plane)
{
	glm::vec3 v = proj(point2, proj_plane) - proj(point1, proj_plane);

	return length(v);
}

float CW3CephDM::dist_point2line(const glm::vec3& point, const glm::vec3& linePoint1, const glm::vec3& linePoint2,
								 const glm::vec4& projPlane) {
	glm::vec3 v = proj(linePoint2, projPlane) - proj(linePoint1, projPlane);
	glm::vec3 p = proj(point, projPlane) - proj(linePoint2, projPlane);

	glm::vec3 planeNorm(projPlane.x, projPlane.y, projPlane.z);
	float sign = (glm::dot(p, glm::cross(planeNorm, v)) < 0) ? 1.0f : -1.0f;
	return sign * glm::length(p - ((glm::dot(v, p) / glm::dot(v, v))*v));
}

float CW3CephDM::angle_line2plane(const glm::vec3 & point1, const glm::vec3 & point2,
								  const glm::vec4 & plane, const glm::vec4 & projPlane) {
	glm::vec3 v1 = proj(point1, plane) - proj(point2, plane);
	v1 = proj(v1, projPlane);
	glm::vec3 v2 = proj(point1, projPlane) - proj(point2, projPlane);

	return acos(glm::dot(v1, v2) / (glm::length(v1)*glm::length(v2)))*kDegreeToRadian;
}
glm::vec4 CW3CephDM::plane_3point(const glm::vec3 & point1, const glm::vec3 & point2, const glm::vec3 & point3) {
	glm::vec3 v1 = point2 - point1;
	glm::vec3 v2 = point3 - point1;
	glm::vec3 n = glm::normalize(glm::cross(v1, v2));
	//return vec4(n, -glm::length(glm::dot(point1, n)));
	return glm::vec4(n, (glm::dot(point1, n)));
}

glm::vec4 CW3CephDM::plane_2pointPerp(const glm::vec3 & point1, const glm::vec3 & point2, const glm::vec4 & plane) {
	glm::vec3 v = point2 - point1;
	glm::vec3 n = glm::normalize(glm::cross(v, glm::vec3(plane)));

	//return glm::vec4(n, -glm::length(glm::dot(n, point1)));
	return glm::vec4(n, (glm::dot(n, point1)));
}

glm::vec4 CW3CephDM::plane_4pointPerp(const glm::vec3 & point1, const glm::vec3 & point2,
									  const glm::vec3 & point3, const glm::vec3& point4, const glm::vec4& plane) {
	glm::vec3 v = ((point2 - point1)*0.5f + point1) - ((point4 - point3)*0.5f + point3);
	glm::vec3 n = glm::normalize(glm::cross(v, glm::vec3(plane)));
	//return vec4(n, -glm::length(glm::dot(n, ((point4 - point3)*0.5f + point3))));
	return glm::vec4(n, (glm::dot(n, ((point4 - point3)*0.5f + point3))));
}

glm::vec4 CW3CephDM::plane_1pointPerp2(const glm::vec3 & point, const glm::vec4 & plane1, const glm::vec4 & plane2) {
	glm::vec3 n = glm::normalize(glm::cross(glm::vec3(plane1), glm::vec3(plane2)));
	//return glm::vec4(n, -glm::length(glm::dot(n, point)));
	return glm::vec4(n, (glm::dot(n, point)));
}

glm::vec3 CW3CephDM::perp_to_line_on_plane(const glm::vec3& point1, const glm::vec3& point2, const glm::vec4& plane)
{
	glm::vec3 v = proj(point2, plane) - proj(point1, plane);

	return glm::cross(v, glm::vec3(plane));
}

void CW3CephDM::angle_3point_measureInfo(const glm::vec3& point1, const glm::vec3& point2, const glm::vec3& point3,
										 const glm::vec4& projPlane, MEASURE_INFO& info) {
	glm::vec3 o = proj(point1, projPlane);

	glm::vec3 v1 = proj(point2, projPlane) - o;
	glm::vec3 v2 = proj(point3, projPlane) - o;

	std::vector<glm::vec3> points = {
		o, proj(point2, projPlane), proj(point3, projPlane)
	};

	for (auto& elem : points)
		elem = mapActualToVolForLandmark(elem);

	info.type = CephMeasureType::ANGLE;
	info.points = points;
	info.val = acos(glm::dot(v1, v2) / (glm::length(v1)*glm::length(v2)))*kDegreeToRadian;
}
void CW3CephDM::angle_4point_measureInfo(const glm::vec3& point1, const glm::vec3& point2, const glm::vec3& point3, const glm::vec3& point4,
										 const glm::vec4& projPlane, MEASURE_INFO& info) {
	glm::vec3 v1 = proj(point2, projPlane) - proj(point1, projPlane);
	glm::vec3 v2 = proj(point4, projPlane) - proj(point3, projPlane);

	std::vector<glm::vec3> points = {
		proj(point1, projPlane), //origin
		proj(point2, projPlane),
		proj(point4, projPlane) - (proj(point3, projPlane) - proj(point1, projPlane))
	};

	for (auto& elem : points)
		elem = mapActualToVolForLandmark(elem);

	info.type = CephMeasureType::ANGLE;
	info.points = points;
	info.val = acos(glm::dot(v1, v2) / (glm::length(v1)*glm::length(v2)))*kDegreeToRadian;
}

void CW3CephDM::dist_2point_measureInfo(const glm::vec3& point1, const glm::vec3& point2, const glm::vec4& proj_plane, MEASURE_INFO& info)
{
	glm::vec3 v = proj(point2, proj_plane) - proj(point1, proj_plane);

	std::vector<glm::vec3> points = {
		proj(point1, proj_plane),
		proj(point2, proj_plane)
	};


	for (auto& elem : points)
	{
		elem = mapActualToVolForLandmark(elem);
	}

	info.type = CephMeasureType::DISTANCE;
	info.points = points;
	info.val = length(v);
}

void CW3CephDM::dist_point2line_measureInfo(const glm::vec3& point, const glm::vec3& linePoint1, const glm::vec3& linePoint2,
											const glm::vec4& projPlane, MEASURE_INFO& info) {
	glm::vec3 v = proj(linePoint2, projPlane) - proj(linePoint1, projPlane);
	glm::vec3 p = proj(point, projPlane) - proj(linePoint2, projPlane);

	glm::vec3 planeNorm(projPlane.x, projPlane.y, projPlane.z);
	float sign = (glm::dot(p, glm::cross(planeNorm, v)) < 0) ? 1.0f : -1.0f;

	std::vector<glm::vec3> points = {
		proj(point, projPlane),
		proj(point, projPlane) - (p - ((glm::dot(v, p) / glm::dot(v, v))*v))
	};

	for (auto& elem : points)
		elem = mapActualToVolForLandmark(elem);

	info.type = CephMeasureType::DISTANCE;
	info.points = points;
	info.val = sign * glm::length(p - ((glm::dot(v, p) / glm::dot(v, v))*v));
}

void CW3CephDM::angle_line2plane_measureInfo(const glm::vec3 & point1, const glm::vec3 & point2,
											 const glm::vec4 & plane, const glm::vec4 & projPlane, MEASURE_INFO& info) {
	glm::vec3 v1 = proj(point1, plane) - proj(point2, plane);
	v1 = proj(v1, projPlane);
	glm::vec3 v2 = proj(point1, projPlane) - proj(point2, projPlane);

	std::vector<glm::vec3> points = { glm::vec3(0.0f), v1, v2 };

	for (auto& elem : points)
		elem = mapActualToVolForLandmark(elem);

	info.type = CephMeasureType::ANGLE;
	info.points = points;
	info.val = acos(glm::dot(v1, v2) / (glm::length(v1)*glm::length(v2)))*kDegreeToRadian;
}
