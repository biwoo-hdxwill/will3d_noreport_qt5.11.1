#include "W3SurfacePlaneItem.h"

#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/Common/W3ElementGenerator.h"
#include "../../Common/Common/W3Logger.h"

using namespace UIGLObjects;
using namespace common;

using glm::mat4;
using glm::vec3;
using glm::vec4;

CW3SurfacePlaneItem::CW3SurfacePlaneItem() {
	m_vaoPlane = 0;
	m_vaoOutline = 0;
	m_model = mat4(1.0f);
	m_isReady = false;

	m_volTexBias = mat4(
		vec4(-0.5f, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, 0.5f, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, 0.5f, 0.0f),
		vec4(0.5f, 0.5f, 0.5f, 1.0f));
}
CW3SurfacePlaneItem::~CW3SurfacePlaneItem() {
	if (m_vboPlane.size()) {
		glDeleteBuffers(m_vboPlane.size(), &m_vboPlane[0]);
		m_vboPlane.clear();
	}
	if (m_vaoPlane) {
		glDeleteVertexArrays(1, &m_vaoPlane);
		m_vaoPlane = 0;
	}
	if (m_vboOutline.size()) {
		glDeleteBuffers(m_vboOutline.size(), &m_vboOutline[0]);
		m_vboOutline.clear();
	}

	if (m_vaoOutline) {
		glDeleteVertexArrays(1, &m_vaoOutline);
		m_vaoOutline = 0;
	}
}
void CW3SurfacePlaneItem::draw(GLuint program) {
	if (!m_isReady) {
		createRect();
		m_isReady = true;
	}

	GLboolean enableEnable = GL_FALSE;
	glGetBooleanv(GL_BLEND, &enableEnable);

	for (const auto& elem : m_planeEquations) {
		if (!m_isVisible[elem.first])
			continue;

		float d = elem.second.w;
		glm::mat4 rotMat = rotVec2Vec(m_oriNormal, vec3(elem.second));
		m_model = rotMat;

		this->setTransformMat(glm::translate(mat3(m_transform.scale)*vec3(elem.second)*d), TRANSLATE);
		WGLSLprogram::setUniform(program, "VolTexTransformMat", this->getVolTexTransformMat());
		setMatrix(program);

		glBindVertexArray(m_vaoPlane);
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_ONE);

			m_material.Ka = vec3(0.1f);
			m_material.Ks = vec3(0.1f);
			m_material.Kd = vec3(0.1f);
			m_material.Shininess = 3.0f;
			setUniformColor(program);

			WGLSLprogram::setUniform(program, "alpha", 0.2f);
			glDrawElements(
				GL_TRIANGLES,
				m_indicesPlane.size(),
				GL_UNSIGNED_INT,
				(void*)0
			);
			glDisable(GL_BLEND);
		}
		glBindVertexArray(0);

		glBindVertexArray(m_vaoOutline);
		{
			glLineWidth(2.0);

			m_material.Ka = vec3(0.5f);
			m_material.Ks = vec3(0.0f, 0.0f, 0.0f);
			m_material.Kd = vec3(0.0f, 0.0f, 0.0f);
			m_material.Shininess = 1.0f;
			setUniformColor(program);

			glDrawElements(
				GL_LINES,      // mode
				m_outlineIndices.size(),    // count
				GL_UNSIGNED_INT,   // type
				(void*)0           // elem.secondent array buffer offset
			);

		}
		glBindVertexArray(0);
	}

	if (enableEnable)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
}
void CW3SurfacePlaneItem::drawSliceTexture(GLuint progSlice) {
	if (!m_isReady) {
		createRect();
		m_isReady = true;
	}

	this->pushModelMat();
	glm::mat4 model = m_model;
	for (const auto& elem : m_planeEquations) {
		if (!m_isVisible[elem.first])
			continue;

		setPlaneMatrix(elem.first);
		m_model = mat4(1.0f);

		WGLSLprogram::setUniform(progSlice, "invModel", m_volTexBias*this->getVolTexTransformMat());
		WGLSLprogram::setUniform(progSlice, "VolTexTransformMat", m_volTexBias*this->getVolTexTransformMat());
		setMatrix(progSlice);
		WGLSLprogram::setUniform(progSlice, "alpha", 0.8f);

		glBindVertexArray(m_vaoPlane);
		{
			glDrawElements(
				GL_TRIANGLES,
				m_indicesPlane.size(),
				GL_UNSIGNED_INT,
				(void*)0
			);
		}
		glBindVertexArray(0);
	}
	m_model = model;
	this->popModelMat();
}
void CW3SurfacePlaneItem::drawSliceTexture(const QString & planeName, GLuint progSlice) {
	if (!checkErrPlane(planeName))
		return;

	if (!m_isReady) {
		createRect();
		m_isReady = true;
	}

	this->pushModelMat();
	glm::mat4 model = m_model;

	setPlaneMatrix(planeName);
	m_model = mat4(1.0f);

	WGLSLprogram::setUniform(progSlice, "VolTexTransformMat", m_volTexBias*this->getVolTexTransformMat());
	WGLSLprogram::setUniform(progSlice, "invModel", m_volTexBias*this->getVolTexTransformMat());
	setMatrix(progSlice);
	WGLSLprogram::setUniform(progSlice, "alpha", 0.8f);

	glBindVertexArray(m_vaoPlane);
	{
		glDrawElements(
			GL_TRIANGLES,
			m_indicesPlane.size(),
			GL_UNSIGNED_INT,
			(void*)0
		);
	}
	glBindVertexArray(0);

	m_model = model;
	this->popModelMat();
}
void CW3SurfacePlaneItem::drawOutline(GLuint progSurface) {
	if (!m_isReady) {
		createRect();
		m_isReady = true;
	}

	this->pushModelMat();
	glm::mat4 model = m_model;
	for (const auto& elem : m_planeEquations) {
		if (!m_isVisible[elem.first])
			continue;

		setPlaneMatrix(elem.first);
		m_model = mat4(1.0f);

		WGLSLprogram::setUniform(progSurface, "VolTexTransformMat", this->getVolTexTransformMat());
		setMatrix(progSurface);

		glBindVertexArray(m_vaoOutline);
		{
			glLineWidth(2.0);

			m_material.Ka = vec3(0.47f, 0.63f, 1.0f);
			m_material.Ks = vec3(0.0f, 0.0f, 0.0f);
			m_material.Kd = vec3(0.0f, 0.0f, 0.0f);
			m_material.Shininess = 1.0f;
			setUniformColor(progSurface);

			glDrawElements(
				GL_LINES,      // mode
				m_outlineIndices.size(),    // count
				GL_UNSIGNED_INT,   // type
				(void*)0           // elem.secondent array buffer offset
			);
		}
		glBindVertexArray(0);
	}
	m_model = model;
	this->popModelMat();
}
void CW3SurfacePlaneItem::drawOutline(const QString & planeName, GLuint progSurface) {
	if (!checkErrPlane(planeName))
		return;

	if (!m_isReady) {
		createRect();
		m_isReady = true;
	}

	this->pushModelMat();
	glm::mat4 model = m_model;

	setPlaneMatrix(planeName);
	m_model = mat4(1.0f);

	WGLSLprogram::setUniform(progSurface, "VolTexTransformMat", this->getVolTexTransformMat());
	setMatrix(progSurface);

	glBindVertexArray(m_vaoOutline);
	{
		glLineWidth(2.0);

		m_material.Ka = vec3(0.47f, 0.63f, 1.0f);
		m_material.Ks = vec3(0.0f, 0.0f, 0.0f);
		m_material.Kd = vec3(0.0f, 0.0f, 0.0f);
		m_material.Shininess = 1.0f;
		setUniformColor(progSurface);

		glDrawElements(
			GL_LINES,      // mode
			m_outlineIndices.size(),    // count
			GL_UNSIGNED_INT,   // type
			(void*)0           // array buffer offset
		);
	}
	glBindVertexArray(0);

	m_model = model;
	this->popModelMat();
}
void CW3SurfacePlaneItem::clearVAOVBO() {
	m_isReady = false;

	if (m_vboPlane.size()) {
		glDeleteBuffers(m_vboPlane.size(), &m_vboPlane[0]);
		m_vboPlane.clear();
	}
	if (m_vaoPlane) {
		glDeleteVertexArrays(1, &m_vaoPlane);
		m_vaoPlane = 0;
	}
	if (m_vboOutline.size()) {
		glDeleteBuffers(m_vboOutline.size(), &m_vboOutline[0]);
		m_vboOutline.clear();
	}
	if (m_vaoOutline) {
		glDeleteVertexArrays(1, &m_vaoOutline);
		m_vaoOutline = 0;
	}
}

void CW3SurfacePlaneItem::deleteAllPlanes() {
	m_planeEquations.clear();
	m_isVisible.clear();
	m_planeRightVector.clear();
}

void CW3SurfacePlaneItem::addPlane(const QString& planeName, const glm::vec4 & eqa) {
	m_planeEquations[planeName] = eqa;
	m_isVisible[planeName] = true;
}

void CW3SurfacePlaneItem::editPlane(const QString & planeName, const glm::vec4 & eqa) {
	if (!checkErrPlane(planeName))
		return;

	m_planeEquations[planeName] = eqa;
}
void CW3SurfacePlaneItem::setPlaneRightVector(const QString & planeName, const glm::vec3 & rightVector) {
	m_planeRightVector[planeName] = rightVector;
}
void CW3SurfacePlaneItem::erasePlane(const QString& planeName) {
	m_planeEquations.erase(planeName);
	m_isVisible.erase(planeName);
	m_planeRightVector.erase(planeName);
}
glm::vec4 CW3SurfacePlaneItem::getPlaneEquation(const QString & planeName) {
	if (m_planeEquations.find(planeName) != m_planeEquations.end())
		return m_planeEquations[planeName];
	else
		return vec4(0.0);
}

glm::mat4 CW3SurfacePlaneItem::rotVec2Vec(const glm::vec3 & v1, const glm::vec3 & v2) {
	glm::vec3 nv1 = glm::normalize(v1);
	glm::vec3 nv2 = glm::normalize(v2);

	if (abs(glm::dot(nv1, nv2)) >= 1.0f)
		return mat4(1.0f);

	glm::vec3 h = glm::normalize(glm::cross(nv1, nv2));
	float th = acos(glm::clamp(glm::dot(nv1, nv2), -1.f, 1.f));
	return rotWithAxis(h, th);
}

glm::mat4 CW3SurfacePlaneItem::rotWithAxis(const glm::vec3 & axis, float th) {
	glm::mat4 axis_crossmat = glm::mat4(0.f);
	axis_crossmat[1][0] = -axis[2];
	axis_crossmat[2][0] = axis[1];
	axis_crossmat[0][1] = axis[2];
	axis_crossmat[2][1] = -axis[0];
	axis_crossmat[0][2] = -axis[1];
	axis_crossmat[1][2] = axis[0];
	glm::mat4 axis_extmat(0.f);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			axis_extmat[i][j] = axis[i] * axis[j];
		}
	}
	glm::mat4 res = sin(th) * axis_crossmat + cos(th) * glm::mat4(1.f) + (1 - cos(th)) * axis_extmat;
	res[3][3] = 1.f;
	return res;
}

void CW3SurfacePlaneItem::createRect() {
	std::vector<vec3> vert, norm;
	m_indicesPlane.clear();

	std::vector<glm::vec3> lowerLine, upperLine;

	glm::mat3 invReorien = mat3(glm::inverse(m_transform.reorien));

	m_oriNormal = invReorien*vec3(0.0f, 0.0f, 1.0);
	m_oriRight = invReorien*vec3(1.0f, 0.0f, 0.0f);
	lowerLine.push_back(invReorien*vec3(-m_coordVertex, -m_coordVertex, 0.0f));
	lowerLine.push_back(invReorien*vec3(m_coordVertex, -m_coordVertex, 0.0f));
	upperLine.push_back(invReorien*vec3(-m_coordVertex, m_coordVertex, 0.0f));
	upperLine.push_back(invReorien*vec3(m_coordVertex, m_coordVertex, 0.0f));

	CW3ElementGenerator::generateRectFace(lowerLine, upperLine, vert, norm, m_indicesPlane);

	if (m_vboPlane.size()) {
		glDeleteBuffers(m_vboPlane.size(), &m_vboPlane[0]);
		m_vboPlane.clear();
	}
	if (m_vaoPlane) {
		glDeleteVertexArrays(1, &m_vaoPlane);
		m_vaoPlane = 0;
	}

	m_vboPlane.resize(3, 0);
	CW3GLFunctions::initVAOVBO(&m_vaoPlane, &m_vboPlane[0], vert, norm, m_indicesPlane);

	std::vector<vec3> outlineVerts;
	outlineVerts.push_back(lowerLine[0]);
	outlineVerts.push_back(lowerLine[1]);
	outlineVerts.push_back(upperLine[1]);
	outlineVerts.push_back(upperLine[0]);

	m_outlineIndices.clear();
	m_outlineIndices.push_back(0);
	m_outlineIndices.push_back(1);
	m_outlineIndices.push_back(1);
	m_outlineIndices.push_back(2);
	m_outlineIndices.push_back(2);
	m_outlineIndices.push_back(3);
	m_outlineIndices.push_back(3);
	m_outlineIndices.push_back(0);

	if (m_vboOutline.size()) {
		glDeleteBuffers(m_vboOutline.size(), &m_vboOutline[0]);
		m_vboOutline.clear();
	}

	if (m_vaoOutline) {
		glDeleteVertexArrays(1, &m_vaoOutline);
		m_vaoOutline = 0;
	}

	m_vboOutline.resize(2, 0);
	CW3GLFunctions::initVAOVBO(&m_vaoOutline, &m_vboOutline[0], outlineVerts, m_outlineIndices);
}

void CW3SurfacePlaneItem::setPlaneMatrix(const QString & planeName) {
	glm::vec4 plane = m_planeEquations[planeName];
	glm::mat4 rotMat = rotVec2Vec(m_oriNormal, plane.xyz);

	if (m_planeRightVector.find(planeName) != m_planeRightVector.end()) {
		glm::vec3 rotatedOriRight = mat3(rotMat)*m_oriRight;
		glm::mat4 rotRight = (rotVec2Vec(rotatedOriRight, m_planeRightVector[planeName]));
		rotMat = rotRight*rotMat;
	}

	this->setTransformMat(rotMat, ROTATE);

	float d = plane.w;
	this->setTransformMat(glm::translate(mat3(m_transform.scale)*plane.xyz*d), TRANSLATE);
}

bool CW3SurfacePlaneItem::checkErrPlane(const QString& planeName) {
	if (m_planeEquations.find(planeName) == m_planeEquations.end()) {
		auto logger = Logger::instance();
		logger->Print(LogType::ERR, QString("not found[% 1] plane.").arg(planeName).toStdString());
		return false;
	} else
		return true;
}
