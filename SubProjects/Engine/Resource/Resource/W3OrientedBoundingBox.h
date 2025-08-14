#pragma once
/*=========================================================================

File:			class CW3OrientedBoundingBox
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun, Hong Jung
First date:		2016-03-30
Last modify:	2016-05-20

=========================================================================*/
#if defined(__APPLE__)
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Eigenvalues>
#else
#include <eigen/Eigen/Dense>
#include <eigen/Eigen/Eigenvalues>
#endif
using Eigen::Matrix3f;
using Eigen::Matrix3cf;
using Eigen::MatrixXcf;
using Eigen::Vector3f;
using Eigen::Vector3cf;
using Eigen::EigenSolver;

#include <QList>
#include "../../Common/Common/common.h"
#include "../../Common/Common/W3Memory.h"

#include "../../Common/GLfunctions/WGLHeaders.h"
#include "../../Common/GLfunctions/W3GLTypes.h"

#include "resource_global.h"

///////////////////////////////////////////////////////////////////////////
//
//	* CW3OrientedBoundingBox
//	OBB 객체
//	vertex 를 받아 OBB 를 만들고 충돌 검사에 필요한 데이터를 가지고 있음
//	트리의 노드 당 하나의 객체가 존재
//
///////////////////////////////////////////////////////////////////////////

class RESOURCE_EXPORT CW3OrientedBoundingBox {
public:
	CW3OrientedBoundingBox(QList<Triangle *> *triangles, bool isAxisAligned);
	~CW3OrientedBoundingBox();

	inline const vec4& getTreeSeparatePlane() const noexcept { return m_vHalfPlaneLongedtAxis; }

	inline vec3 getMean() { return glm::vec3(m_mu2.x, m_mu2.y, m_mu2.z); }

	inline vec3 getAxis(int i) {
		return glm::vec3(m_axis2[i].x, m_axis2[i].y, m_axis2[i].z);
	}

	inline vec3 getHalfDim() { return m_halfDim; }

	void setMVP(const mat4&  mvp);
	bool setModelScaled(const mat4&  model);

private:
	void createOBB();
	void createABB();
	void axisOrdering();
	glm::vec4 getPlaneEquation(glm::vec3 P, glm::vec3 Q, glm::vec3 R);

private:
	QList<Triangle *> *m_pgvTriangles;	// triangle mesh vertex
	std::vector<float> m_vVert;

	vec4 m_vHalfPlaneLongedtAxis;	// 최장축을 수직하게 반으로 나누는 평면

	vec4 m_mu;	// OBB의 중심
	vec4 m_axis[3];	// OBB의 면들에 수직인 각 축
	vec3 m_halfDim;	// OBB의 각 변 길이의 반

	vec4 m_mu2;	// transformed OBB의 중심
	vec4 m_axis2[3];	// transformed OBB의 면들에 수직인 각 축

	vec4 m_OBBPoint[8];	// mvp 적용을 위한 OBB 좌표

	float m_fMaxPointLength;

	int m_AxisOrder[3];
};
