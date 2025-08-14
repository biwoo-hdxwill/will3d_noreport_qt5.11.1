#include "W3SurfaceAxesItem.h"
#include <qmath.h>

#include <QDebug>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"

#include "surface_axes_item_geometry.h"
#include "W3VBOs.h"

namespace {
const vec3 kCoefKa(0.1f);
const float kCoefKd = 1.0f;
const mat4 kIModel(1.0f);
const char* kUniformIndex = "index";
const float kRad90 = glm::radians(90.0f);
const float kRadm90 = glm::radians(-90.0f);
} // end of namespace

CW3SurfaceAxesItem::CW3SurfaceAxesItem() {
	m_material.Ks = vec3(0.55f);
	m_material.Shininess = 15.0f;

	//initializeAxes();
}

CW3SurfaceAxesItem::~CW3SurfaceAxesItem() {
}

void CW3SurfaceAxesItem::clearVAOVBO() {
	if (m_pArrow)
		m_pArrow->clearVAOVBO();
	if (m_pTorus)
		m_pTorus->clearVAOVBO();
}

void CW3SurfaceAxesItem::initializeAxes() {
	// 세 아이템의 lifetime 이 같기 때문에 하나만 존재해도 리턴한다.
	if (m_pArrow || m_pTorus)
		return;

	m_pArrow = SurfaceAxesItemGeometry::GetInstance()->arrow();
	m_pTorus = SurfaceAxesItemGeometry::GetInstance()->torus();
	//m_pHgTorus = new CW3VBOSTL(QString(":/stl/Axes/torus_h.stl"));
}

void CW3SurfaceAxesItem::draw(GLuint program) {
	//qDebug() << "start CW3SurfaceAxesItem::draw";

	initializeAxes();

	//qDebug() << "1 CW3SurfaceAxesItem::draw";

	//draw X-axis
	m_model = kIModel;
	setMatrix(program);
	
	vec3 color = vec3(1.0f, 0.1f, 0.03f);
	m_material.Kd = color * kCoefKd;
	m_material.Ka = (m_itemType == X_AXIS_ARROW) ? color : kCoefKa;
	setUniformColor(program);
	m_pArrow->render();

	//qDebug() << "2 CW3SurfaceAxesItem::draw";

	m_model = glm::rotate(kRadm90, vec3(0.0f, 1.0f, 0.0f));
	setMatrix(program);

	if (m_itemType == X_AXIS_TORUS) {
		m_material.Ka = color;
		setUniformColor(program);
		m_pTorus->render();
	} else {
		m_material.Ka = kCoefKa;
		setUniformColor(program);
		m_pTorus->quarterRender();
	}
	
	//qDebug() << "3 CW3SurfaceAxesItem::draw";

	//draw Y-axis
	color = vec3(0.0f, 0.65f, 0.31f);
	m_material.Kd = color * kCoefKd;
	m_material.Ka = (m_itemType == Y_AXIS_ARROW) ? color : kCoefKa;
	setUniformColor(program);

	m_model = glm::rotate(kRad90, vec3(0.0f, 0.0f, 1.0f));
	setMatrix(program);
	m_pArrow->render();

	m_model = glm::rotate(kRad90, vec3(1.0f, 0.0f, 0.0f));
	setMatrix(program);
	if (m_itemType == Y_AXIS_TORUS) {
		m_material.Ka = color;
		setUniformColor(program);
		m_pTorus->render();
	} else {
		m_material.Ka = kCoefKa;
		setUniformColor(program);
		m_pTorus->quarterRender();
	}
	m_pTorus->quarterRender();
	
	//draw Z-axis
	color = vec3(0.09f, 0.01f, 0.9f);
	m_material.Kd = color * kCoefKd;
	m_material.Ka = (m_itemType == Z_AXIS_ARROW) ? color : kCoefKa;
	setUniformColor(program);

	m_model = glm::rotate(kRadm90, vec3(0.0f, 1.0f, 0.0f));
	setMatrix(program);
	m_pArrow->render();

	m_model = kIModel;
	setMatrix(program);

	if (m_itemType == Z_AXIS_TORUS) {
		m_material.Ka = color;
		setUniformColor(program);
		m_pTorus->render();
	} else {
		m_material.Ka = kCoefKa;
		setUniformColor(program);
		m_pTorus->quarterRender();
	}

	//qDebug() << "end CW3SurfaceAxesItem::draw";
}
void CW3SurfaceAxesItem::render_for_pick(GLuint program) {
	initializeAxes();

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	const mat4 kScale = glm::scale(vec3(1.0f, 6.0f, 6.0f)); //for hightlight

	//draw X-axis
	m_model = kScale;
	setMatrix(program);
	WGLSLprogram::setUniform(program, kUniformIndex, X_AXIS_ARROW);
	m_pArrow->render();

	mat4 rotMat = glm::rotate(kRadm90, vec3(0.0f, 1.0f, 0.0f));
	m_model = rotMat;
	setMatrix(program);
	WGLSLprogram::setUniform(program, kUniformIndex, X_AXIS_TORUS);
	//m_pHgTorus->render();
	m_pTorus->render();

	//draw Y-axis
	rotMat = glm::rotate(kRad90, vec3(0.0f, 0.0f, 1.0f));
	m_model = rotMat*kScale;
	setMatrix(program);
	WGLSLprogram::setUniform(program, kUniformIndex, Y_AXIS_ARROW);
	m_pArrow->render();

	rotMat = glm::rotate(kRad90, vec3(1.0f, 0.0f, 0.0f));
	m_model = rotMat;
	setMatrix(program);
	WGLSLprogram::setUniform(program, kUniformIndex, Y_AXIS_TORUS);
	//m_pHgTorus->render();
	m_pTorus->render();

	//draw Z-axis
	rotMat = glm::rotate(kRadm90, vec3(0.0f, 1.0f, 0.0f));
	m_model = rotMat*kScale;
	setMatrix(program);
	WGLSLprogram::setUniform(program, kUniformIndex, Z_AXIS_ARROW);
	m_pArrow->render();

	m_model = kIModel;
	setMatrix(program);
	WGLSLprogram::setUniform(program, kUniformIndex, Z_AXIS_TORUS);
	//m_pHgTorus->render();
	m_pTorus->render();
}

void CW3SurfaceAxesItem::pick(const QPointF& pickPoint, bool* isUpdateGL, GLenum format, GLenum type) {
	unsigned char id;
	vec3 pickPos;
	readPickInfo(pickPoint.x(), pickPoint.y(), &id, &pickPos, format, type);

	if (id > ITEM_UNKNOWN_TYPE && id < ITEM_TYPE_END) {
		if (m_itemType != static_cast<eItemType>(id))
			*isUpdateGL = true;
		else
			*isUpdateGL = false;

		m_itemType = static_cast<eItemType>(id);
		m_pickPos = pickPos;
	} else {
		if (m_itemType != static_cast<eItemType>(id) && m_itemType != static_cast<eItemType>(id - 256))
			*isUpdateGL = true;
		else
			*isUpdateGL = false;

		m_itemType = ITEM_UNKNOWN_TYPE;
	}
}

const bool CW3SurfaceAxesItem::isSelectTranslate(void) const {
	switch (m_itemType) {
	case X_AXIS_ARROW:
	case Y_AXIS_ARROW:
	case Z_AXIS_ARROW:
		return true;
	default:
		return false;
	}
}

const bool CW3SurfaceAxesItem::isSelectRotate(void) const {
	switch (m_itemType) {
	case X_AXIS_TORUS:
	case Y_AXIS_TORUS:
	case Z_AXIS_TORUS:
		return true;
	default:
		return false;
	}
}
glm::vec3 CW3SurfaceAxesItem::translate(const glm::vec3& transGLCoord, bool is_move_axes) {
	vec4 vAxis;
	bool isTranslate;
	switch (m_itemType) {
	case X_AXIS_ARROW:
		vAxis = glm::normalize(m_transform.rotate[0]);
		isTranslate = true;
		break;
	case Y_AXIS_ARROW:
		vAxis = glm::normalize(m_transform.rotate[1]);
		isTranslate = true;
		break;
	case Z_AXIS_ARROW:
		vAxis = glm::normalize(m_transform.rotate[2]);
		isTranslate = true;
		break;
	default:
		isTranslate = false;
	}

	if (isTranslate) {
		mat4 iRVP = glm::inverse(m_projection*m_view*m_transform.arcball);
		vec4 trans = iRVP * vec4(transGLCoord, 1.0f);

		float dot = glm::dot(vAxis, trans);
		vec3 vAxisT = vec3(vAxis*dot);

		if (is_move_axes)
			m_transform.translate *= glm::translate(vAxisT);

		return vAxisT;
	} else {
		return vec3(0.0);
	}
}

QPair<float, glm::vec3> CW3SurfaceAxesItem::rotate(const glm::vec3& curGLCoord,
												   const glm::vec3& lastGLCoord,
												   bool is_move_axes) {
	bool isRotate;
	vec3 vRotAxis;
	switch (m_itemType) {
	case X_AXIS_TORUS:
		vRotAxis = glm::normalize(vec3(m_transform.rotate[0]));
		isRotate = true;
		break;
	case Y_AXIS_TORUS:
		vRotAxis = glm::normalize(vec3(m_transform.rotate[1]));
		isRotate = true;
		break;
	case Z_AXIS_TORUS:
		vRotAxis = glm::normalize(vec3(m_transform.rotate[2]));
		isRotate = true;
		break;
	default:
		isRotate = false;
	}

	if (isRotate) {
		mat4 iRVP = glm::inverse(m_projection*m_view*m_transform.arcball);

		vec4 applyD_CGL(curGLCoord.x, curGLCoord.y, m_pickPos.z, 1.0f);
		vec4 applyD_LGL(lastGLCoord.x, lastGLCoord.y, m_pickPos.z, 1.0f);

		vec4 v0 = m_transform.translate*vec4(0.0f, 0.0f, 0.0f, 1.0f);
		vec4 v1 = glm::normalize(iRVP*applyD_LGL - v0);
		vec4 v2 = glm::normalize(iRVP*applyD_CGL - v0);

		float sign = (glm::dot(glm::cross(vec3(v1), vec3(v2)), vRotAxis) > 0) ? 1.0f : -1.0f;
		float rotAngle = sign * glm::radians(std::acos(std::min(1.0f, glm::dot(v1, v2)))*180.0f / M_PI);

		if (is_move_axes)
			m_transform.rotate = glm::rotate(rotAngle, vRotAxis)*m_transform.rotate;

		return qMakePair(rotAngle, vRotAxis);
	} else {
		return qMakePair(0.0f, vec4(0.0f));
	}
}
