#pragma once
/*=========================================================================

File:			class CW3CephDM
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-06-28
Last modify:	2016-06-28

=========================================================================*/
#include <typeinfo>
#include <typeindex>

#if defined(__APPLE__)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#else
#include <GL/glm/glm.hpp>
#include <GL/glm/gtc/matrix_transform.hpp>
#endif

#include <QObject>

#include "dbmanager_global.h"

class DBMANAGER_EXPORT CW3CephDM : public QObject {
	Q_OBJECT

public:
	enum class CephMeasureType { ANGLE, DISTANCE };

	typedef struct _MEASURE_INFO {
		CephMeasureType type;
		std::vector<glm::vec3> points;
		float val = 0.0f;
	}MEASURE_INFO;

public:
	CW3CephDM(QObject * parent = nullptr);
	~CW3CephDM();

public:
	void InitFromDatabase(const QString& studyID);
	void InitFromProjectFile(const std::map<QString, glm::vec3>& landmarks);
	void clear();

	void addLandmark(const QString& landmarkName, const glm::vec3& landmarkPos);

	void endEditLandmark();

	void updateLandmarkPositions(const std::map<QString, glm::vec3>& landmarks);

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// Sets coordinate system.
	/// @param	coordSysPoints	  	The coordinate system points.
	/// 							0:
	/// 							1:
	/// 							2:
	/// 							3:
	///								4:
	/// 							5: origin point
	///////////////////////////////////////////////////////////////////////////////////////////////////
	void setCoordinateSystem(const std::vector<glm::vec3>& coordSysPoints);
	void mapVolToActualForLandmark(const std::map<QString, glm::vec3>& volLandmarks,
								   std::map<QString, glm::vec3>& actualLandmarks);
	glm::vec3 mapActualToVolForLandmark(const glm::vec3& actualLandmark);

	glm::vec4 getReferencePlane(const QString& referencePlane);
	float getMeasurementValue(const QString& measure);
	void getMeasurementInfo(const QString& measure, MEASURE_INFO& info);

	glm::vec3 getActualLandmarkCoord(const QString& str);
	inline const std::map<QString, glm::vec3>& getVolumeLandmarks() { return m_volLandmarks; }
	inline const std::map<QString, glm::vec3>& getActualLandmarks() { return m_actualLandmarks; }
	//inline std::map<QString, glm::vec4> getReferencePlanes() { return m_referencePlanes; }

	inline glm::mat4 getCoordSysMatrix() const { return m_coordSysVolMatrix; }
	inline bool isSetCoordSystem() const { return m_isSetCoordSys; }
	inline bool isTurnOn() const { return m_isTrunOn; }

signals:
	void sigTurnOnEnable();
	void sigUpdateLandmarkPositions();

private:
	void Initialize(const std::map<QString, glm::vec3>& landmarks);
	glm::vec4 funcReferencePlane(const glm::mat4& coordMat4, const std::vector<glm::vec3>& coordSysPts,
								 std::map<QString, glm::vec3> landmarks, const QString& plane);
	float funcMeasurement(const QString& measure);

	float angle_3point(const glm::vec3& point1, const glm::vec3& point2,
					   const glm::vec3& point3, const glm::vec4& projPlane);
	float angle_4point(const glm::vec3& point1, const glm::vec3& point2,
					   const glm::vec3& point3, const glm::vec3& point4,
					   const glm::vec4& projPlane);
	float dist_2point(const glm::vec3& point1, const glm::vec3& point2, const glm::vec4& proj_plane);
	float dist_point2line(const glm::vec3& point, const glm::vec3& linePoint1,
						  const glm::vec3& linePoint2, const glm::vec4& projPlane);
	float angle_line2plane(const glm::vec3& point1, const glm::vec3& point2,
						   const glm::vec4& plane, const glm::vec4& projPlane);

	void angle_3point_measureInfo(const glm::vec3& point1, const glm::vec3& point2, const glm::vec3& point3,
								  const glm::vec4& projPlane, MEASURE_INFO& info);
	void angle_4point_measureInfo(const glm::vec3& point1, const glm::vec3& point2,
								  const glm::vec3& point3, const glm::vec3& point4,
								  const glm::vec4& projPlane, MEASURE_INFO& info);
	void dist_2point_measureInfo(const glm::vec3& point1, const glm::vec3& point2, const glm::vec4& proj_plane, MEASURE_INFO& info);
	void dist_point2line_measureInfo(const glm::vec3& point, const glm::vec3& linePoint1, const glm::vec3& linePoint2,
									 const glm::vec4& projPlane, MEASURE_INFO& info);
	void angle_line2plane_measureInfo(const glm::vec3& point1, const glm::vec3& point2, const glm::vec4& plane,
									  const glm::vec4& projPlane, MEASURE_INFO& info);

	glm::vec4 plane_3point(const glm::vec3& point1, const glm::vec3& point2, const glm::vec3& point3);
	glm::vec4 plane_2pointPerp(const glm::vec3& point1, const glm::vec3& point2, const glm::vec4& plane);
	glm::vec4 plane_4pointPerp(const glm::vec3& point1, const glm::vec3& point2,
							   const glm::vec3& point3, const glm::vec3& point4,
							   const glm::vec4& plane);
	glm::vec4 plane_1pointPerp2(const glm::vec3& point, const glm::vec4& plane1, const glm::vec4& plane2);

	glm::vec3 perp_to_line_on_plane(const glm::vec3& point1, const glm::vec3& point2, const glm::vec4& plane);

	glm::vec3 proj(const glm::vec3& point, const glm::vec4& plane);

private:
	std::map<QString, glm::vec3> m_volLandmarks;
	/// actual length.
	std::map<QString, glm::vec3> m_actualLandmarks;

	std::vector<glm::vec3> m_coordSysVolPoints;
	std::vector<glm::vec3> m_coordSysActPoints;
	glm::mat4 m_coordSysVolMatrix = glm::mat4(1.0f);
	glm::mat4 m_coordSysActMatrix;
	bool m_isSetCoordSys = false;
	bool m_isTrunOn = false;
};
