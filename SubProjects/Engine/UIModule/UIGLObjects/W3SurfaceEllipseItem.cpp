#include "W3SurfaceEllipseItem.h"

#include <iostream>

#include <qmath.h>
#include <QDebug>

#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/Common/W3ElementGenerator.h"

using glm::mat4;
using glm::vec3;
using glm::vec4;
using std::runtime_error;
using std::cout;
using std::endl;

namespace {
	const float kSimpleEllipseSize = 5.0f;
	const float kSimpleEllipseBorderWidth = 4.0f;
const unsigned char kUnknownPickID = 255;
const glm::mat4 kIMat = glm::mat4(1.0f);
} // end of namespace

CW3SurfaceEllipseItem::CW3SurfaceEllipseItem(Shape shape) 
	: shape_(shape), m_curPickType(kUnknownPickID) {
	m_ellipseColor = QColor(Qt::green);
	m_brushColor = QColor(Qt::red);
	m_material.Ka = vec3(0.0);
	m_material.Ks = vec3(0.0);
	m_material.Kd = vec3(0.0);
	m_material.Shininess = 1.0f;
}
CW3SurfaceEllipseItem::~CW3SurfaceEllipseItem() {
	clearVAOVBO();
}

void CW3SurfaceEllipseItem::clear() {
	m_points.clear();
	m_pickType.clear();
}

void CW3SurfaceEllipseItem::clearVAOVBO() {
	if (m_vbo.size()) {
		glDeleteBuffers(m_vbo.size(), &m_vbo[0]);
		m_vbo.clear();
	}
	if (m_vao) {
		glDeleteVertexArrays(1, &m_vao);
		m_vao = 0;
	}
}

void CW3SurfaceEllipseItem::draw(GLuint program) {
	if (!m_vao)
		this->createEllipse();

	glm::mat4 pushProjection = m_projection;

	float maxScale = (m_transform.scale[0][0] > m_transform.scale[1][1]) ? m_transform.scale[0][0] : m_transform.scale[1][1];
	maxScale = (maxScale > m_transform.scale[2][2]) ? maxScale : m_transform.scale[2][2];
	mat4 ratioMat = glm::inverse(glm::scale(vec3(m_transform.scale[0][0], m_transform.scale[1][1], m_transform.scale[2][2]) / maxScale));

	glm::mat4 invRotArc = glm::inverse(m_transform.arcball);
	glm::mat4 invRotView = glm::inverse(m_view);

	glm::vec3 eyeToCenter = -vec3(invRotView[2]);
	glm::vec3 vertEyeToCenter = vec3(0.0f, 0.0f, -1.0f);
	float degree = acos(glm::dot(eyeToCenter, vertEyeToCenter))*180.0f / M_PI;
	vec3 rotAxis = glm::cross(vertEyeToCenter, eyeToCenter);

	glm::mat4 rot2D = (degree == 0.0f) ? kIMat : glm::rotate(degree, rotAxis);

	glBindVertexArray(m_vao);
	{
		for (int i = 0; i < m_points.size(); i++) {
			if (shape_ == Shape::CIRCLE)
			{
				glm::mat4 scale = m_pickType[i] ? glm::scale(vec3(1.5f)) : kIMat;
				m_projection = pushProjection;

				//draw ellipse
				m_model = glm::translate(m_points[i])*ratioMat*invRotArc*rot2D*scale;
				m_projection = glm::translate(vec3(0.0f, 0.0f, -0.01f))*m_projection;
				WGLSLprogram::setUniform(program, "VolTexTransformMat", m_model);
				setMatrix(program);
				m_material.Ka = vec3(m_ellipseColor.red(), m_ellipseColor.green(), m_ellipseColor.blue());
				setUniformColor(program);

				glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, (void*)0);

				//draw brush
				float brushSize = static_cast<float>(m_brushSize) / m_ellipseSize;
				m_model = glm::translate(m_points[i])*ratioMat*glm::scale(vec3(brushSize))*invRotArc*rot2D*scale;
				m_projection = glm::translate(vec3(0.0f, 0.0f, -0.01f))*m_projection;
				WGLSLprogram::setUniform(program, "VolTexTransformMat", m_model);
				setMatrix(program);

				m_material.Ka = vec3(m_brushColor.red(), m_brushColor.green(), m_brushColor.blue());
				setUniformColor(program);

				glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, (void*)0);
			}
			else if (shape_ == Shape::SIMPLE_POINT)
			{
				m_model = glm::translate(m_points[i]);

				setMatrix(program);
				m_material.Ka = vec3(m_ellipseColor.red(), m_ellipseColor.green(), m_ellipseColor.blue());
				setUniformColor(program);

				const float ellipse_size = m_pickType[i] ? kSimpleEllipseSize * 1.5f : kSimpleEllipseSize;

				glPointSize(ellipse_size + kSimpleEllipseBorderWidth);
				//glEnable(GL_POINT_SMOOTH);
				glDrawElements(GL_POINTS, m_indices.size(), GL_UNSIGNED_INT, (void*)0);
				//glDisable(GL_POINT_SMOOTH);

				m_material.Ka = vec3(m_brushColor.red(), m_brushColor.green(), m_brushColor.blue());
				setUniformColor(program);

				glPointSize(ellipse_size);
				//glEnable(GL_POINT_SMOOTH);
				glDrawElements(GL_POINTS, m_indices.size(), GL_UNSIGNED_INT, (void*)0);
				//glDisable(GL_POINT_SMOOTH);
			}
		}
	}glBindVertexArray(0);
	m_model = kIMat;
	m_projection = pushProjection;
}

void CW3SurfaceEllipseItem::draw(unsigned int nIndex, GLuint program) {
	if (!m_vao)
		this->createEllipse();

	try {
		if (nIndex >= m_points.size())
			throw runtime_error("ellipse index error.");

		glm::mat4 pushProjection = m_projection;

		float maxScale = (m_transform.scale[0][0] > m_transform.scale[1][1]) ? m_transform.scale[0][0] : m_transform.scale[1][1];
		maxScale = (maxScale > m_transform.scale[2][2]) ? maxScale : m_transform.scale[2][2];
		mat4 ratioMat = glm::inverse(glm::scale(vec3(m_transform.scale[0][0], m_transform.scale[1][1], m_transform.scale[2][2]) / maxScale));

		glm::mat4 invRotArc = glm::inverse(m_transform.arcball);
		glm::mat4 invRotView = glm::inverse(m_view);

		glm::vec3 eyeToCenter = -vec3(invRotView[2]);
		glm::vec3 vertEyeToCenter = vec3(0.0f, 0.0f, -1.0f);
		float degree = acos(glm::dot(eyeToCenter, vertEyeToCenter))*180.0f / M_PI;
		vec3 rotAxis = glm::cross(vertEyeToCenter, eyeToCenter);

		glm::mat4 rot2D = (degree == 0.0f) ? kIMat : glm::rotate(degree, rotAxis);

		glBindVertexArray(m_vao);
		{
			if (shape_ == Shape::CIRCLE)
			{
				glm::mat4 scale = m_pickType[nIndex] ? glm::scale(vec3(1.35f)) : kIMat;
				m_projection = pushProjection;

				//draw ellipse
				m_model = glm::translate(m_points[nIndex])*ratioMat*invRotArc*rot2D*scale;
				m_projection = glm::translate(vec3(0.0f, 0.0f, -0.01f))*m_projection;
				WGLSLprogram::setUniform(program, "VolTexTransformMat", m_model);
				setMatrix(program);
				m_material.Ka = vec3(m_ellipseColor.red(), m_ellipseColor.green(), m_ellipseColor.blue());
				setUniformColor(program);

				glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, (void*)0);

				//draw brush
				float brushSize = static_cast<float>(m_brushSize) / m_ellipseSize;
				m_model = glm::translate(m_points[nIndex])*ratioMat*glm::scale(vec3(brushSize))*invRotArc*rot2D*scale;
				m_projection = glm::translate(vec3(0.0f, 0.0f, -0.01f))*m_projection;
				WGLSLprogram::setUniform(program, "VolTexTransformMat", m_model);
				setMatrix(program);

				m_material.Ka = vec3(m_brushColor.red(), m_brushColor.green(), m_brushColor.blue());
				setUniformColor(program);

				glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, (void*)0);
			}
			else if (shape_ == Shape::SIMPLE_POINT)
			{
				m_model = glm::translate(m_points[nIndex]);

				setMatrix(program);
				m_material.Ka = vec3(m_ellipseColor.red(), m_ellipseColor.green(), m_ellipseColor.blue());
				setUniformColor(program);

				const float ellipse_size = m_pickType[nIndex] ? kSimpleEllipseSize * 1.5f : kSimpleEllipseSize;

				glPointSize(ellipse_size + kSimpleEllipseBorderWidth);
				//glEnable(GL_POINT_SMOOTH);
				glDrawElements(GL_POINTS, m_indices.size(), GL_UNSIGNED_INT, (void*)0);
				//glDisable(GL_POINT_SMOOTH);

				m_material.Ka = vec3(m_brushColor.red(), m_brushColor.green(), m_brushColor.blue());
				setUniformColor(program);

				glPointSize(ellipse_size);
				//glEnable(GL_POINT_SMOOTH);
				glDrawElements(GL_POINTS, m_indices.size(), GL_UNSIGNED_INT, (void*)0);
				//glDisable(GL_POINT_SMOOTH);
			}
		}glBindVertexArray(0);
		m_model = kIMat;
		m_projection = pushProjection;
	} catch (runtime_error& e) {
		std::cout << "CW3SurfaceEllipseItem::draw: " << e.what() << std::endl;
	}
}

void CW3SurfaceEllipseItem::pick(const QPointF& pickPoint, bool* isUpdateGL, GLuint program) {
	if (!m_vao)
		this->createEllipse();

	if (m_points.size() == 0)
		return;

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	float maxScale = (m_transform.scale[0][0] > m_transform.scale[1][1]) ? m_transform.scale[0][0] : m_transform.scale[1][1];
	maxScale = (maxScale > m_transform.scale[2][2]) ? maxScale : m_transform.scale[2][2];
	mat4 ratioMat = glm::inverse(
		glm::scale(vec3(m_transform.scale[0][0],
						m_transform.scale[1][1],
						m_transform.scale[2][2]) / maxScale));

	glm::mat4 invRotArc = glm::inverse(m_transform.arcball);
	glm::mat4 invRotView = glm::inverse(m_view);

	glm::vec3 eyeToCenter = -vec3(invRotView[2]);
	glm::vec3 vertEyeToCenter = vec3(0.0f, 0.0f, -1.0f);
	float degree = acos(glm::dot(eyeToCenter, vertEyeToCenter))*180.0f / M_PI;
	vec3 rotAxis = glm::cross(vertEyeToCenter, eyeToCenter);

	glm::mat4 rot2D = (degree == 0.0f) ? kIMat : glm::rotate(degree, rotAxis);
	glm::mat4 scale = glm::mat4(1.5f);

	//draw ellipse
	glBindVertexArray(m_vao);
	for (int i = 0; i < m_points.size(); i++)
	{
		if (shape_ == Shape::CIRCLE)
		{
			m_model = glm::translate(m_points[i])*ratioMat*invRotArc*rot2D*scale;
			setMatrix(program);
			WGLSLprogram::setUniform(program, "index", i + 1);
			glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, (void*)0);
		}
		else if (shape_ == Shape::SIMPLE_POINT)
		{
			m_model = glm::translate(m_points[i]);

			setMatrix(program);
			WGLSLprogram::setUniform(program, "index", i + 1);

			glPointSize(kSimpleEllipseSize * 1.5f);
			//glEnable(GL_POINT_SMOOTH);
			glDrawElements(GL_POINTS, m_indices.size(), GL_UNSIGNED_INT, (void*)0);
			//glDisable(GL_POINT_SMOOTH);
		}
	}
	glBindVertexArray(0);

	m_model = kIMat;
	unsigned char id = kUnknownPickID;
	vec3 pickPos;
	readPickInfo(pickPoint.x(), pickPoint.y(), &id, &pickPos);

	if (id != m_curPickType) {
		*isUpdateGL = true;
		m_curPickType = id;
	} else {
		*isUpdateGL = false;
	}

	if (id > 0 && id < kUnknownPickID) {
		m_pickType.assign(m_pickType.size(), false);
		m_pickType[id - 1] = true;
		m_isPickItem = true;
	} else {
		m_pickType.assign(m_pickType.size(), false);
		m_isPickItem = false;
	}
}

int CW3SurfaceEllipseItem::getPickIndex() {
	if (m_isPickItem) {
		for (int i = 0; i < m_pickType.size(); i++) {
			if (m_pickType[i]) {
				return i;
			}
		}
	}

	return -1;
}
glm::vec3 CW3SurfaceEllipseItem::getPointPosition(int idx) {
	if (idx >= 0 && idx < m_points.size())
		return m_points[idx];
	else
		return vec3(-1, -1, -1);
}

void CW3SurfaceEllipseItem::addPoint(const glm::vec3& point) {
	m_points.push_back(point);
	m_pickType.push_back(false);
}

void CW3SurfaceEllipseItem::erasePoint(int index) {
	m_points.erase(m_points.begin() + index);
	m_pickType.erase(m_pickType.begin() + index);
}

void CW3SurfaceEllipseItem::setPoints(const std::vector<glm::vec3>& points) {
	m_points.assign(points.begin(), points.end());
	m_pickType.resize(m_points.size(), false);
}

void CW3SurfaceEllipseItem::setEllipseSize(int size) {
	m_ellipseSize = size;
	this->createEllipse();
}

void CW3SurfaceEllipseItem::SetAllPointsSelected(const bool selected)
{
	m_pickType.assign(m_pickType.size(), selected);
	m_isPickItem = selected;
}

void CW3SurfaceEllipseItem::createEllipse() {
	std::vector<vec3> vert, norm;
	m_indices.clear();

	if (shape_ == Shape::CIRCLE)
	{
		CW3ElementGenerator::generateCircleFace(vec3(0.0f), m_ellipseSize*0.0025f, 18, vert, norm, m_indices);
	}
	else if (shape_ == Shape::SIMPLE_POINT)
	{
		vert.push_back(vec3(0.0f));
		norm.push_back(vec3(0.0f));
		m_indices.push_back(0);
	}

	clearVAOVBO();
	m_vbo.resize(3, 0);

	CW3GLFunctions::initVAOVBO(&m_vao, &m_vbo[0], vert, norm, m_indices);
}
