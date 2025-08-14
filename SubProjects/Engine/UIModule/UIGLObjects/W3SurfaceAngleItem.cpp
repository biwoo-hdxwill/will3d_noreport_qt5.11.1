#include "W3SurfaceAngleItem.h"

#include <vector>
#include <qmath.h>

#include "../../Common/Common/W3Logger.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"

#define DRAW_ARCH_GL_LINES 1

namespace
{
	const int kArchPointCount = 50;
}

CW3SurfaceAngleItem::CW3SurfaceAngleItem(QGraphicsScene* pScene, SHAPE shape)
	: CW3SurfaceTextLineItem(pScene, shape, true)
{
	m_vaoArc = 0;
}
CW3SurfaceAngleItem::~CW3SurfaceAngleItem()
{
	release();
}

void CW3SurfaceAngleItem::draw(GLuint program)
{
	if (!visible_)
	{
		return;
	}

	if (!m_vaoArc)
	{
		createLine();
	}

	CW3SurfaceTextLineItem::draw(program);

	//drawing arch line
	m_model = glm::mat4(1.0f);

	WGLSLprogram::setUniform(program, "VolTexTransformMat", getVolTexTransformMat());
	setMatrix(program);

	WGLSLprogram::setUniform(program, "Material.Ka", vec3(1.0f, 1.0f, 0.0f));
	WGLSLprogram::setUniform(program, "Material.Ks", vec3(0.0));
	WGLSLprogram::setUniform(program, "Material.Kd", vec3(0.0));
	WGLSLprogram::setUniform(program, "Material.Shininess", 1.0f);
	WGLSLprogram::setUniform(program, "alpha", m_alpha);

	glBindVertexArray(m_vaoArc);
	{
#if !DRAW_ARCH_GL_LINES
		glDrawElements(
			GL_TRIANGLES,
			m_indicesArc.size(),
			GL_UNSIGNED_INT,
			(void*)0
		);
#else
		glLineWidth(2.0f);
		glDrawArrays(GL_LINES, 0, kArchPointCount);
#endif
	}
	glBindVertexArray(0);
}

void CW3SurfaceAngleItem::addPoint(const glm::vec3& point)
{
	CW3SurfaceLineItem::addPoint(point);

	SetAngleLabel();
}

void CW3SurfaceAngleItem::editPoint(int idx, const glm::vec3& point)
{
	CW3SurfaceLineItem::editPoint(idx, point);

	SetAngleLabel();
}

void CW3SurfaceAngleItem::clear()
{
	CW3SurfaceTextLineItem::clear();
	release();
}

void CW3SurfaceAngleItem::clearVAOVBO()
{
	CW3SurfaceTextLineItem::clearVAOVBO();

	if (m_vboArc.size())
	{
		glDeleteBuffers(m_vboArc.size(), &m_vboArc[0]);
		m_vboArc.clear();
	}
	if (m_vaoArc)
	{
		glDeleteVertexArrays(1, &m_vaoArc);
		m_vaoArc = 0;
	}
}

void CW3SurfaceAngleItem::setAnglePoints(const glm::vec3 & ori, const glm::vec3 & p1, const glm::vec3 & p2)
{
	std::vector<glm::vec3> points = { p1, ori, p2 };
	this->setPoints(points);

	glm::vec3 o = m_points[1];
	glm::vec3 v1 = m_points[0] - o;
	glm::vec3 v2 = m_points[2] - o;

	float degree = acos(glm::dot(v1, v2) / (glm::length(v1)*glm::length(v2)))*(180.0f / M_PI);
	this->setText(QString("%1 deg").arg(degree, 0, 'f', 1));
}

void CW3SurfaceAngleItem::setAnglePoints(const glm::vec3 & ori, const glm::vec3 & p1, const glm::vec3 & p2, float degree)
{
	std::vector<glm::vec3> points = { p1, ori, p2 };
	this->setPoints(points);
	this->setText(QString("%1 deg").arg(degree, 0, 'f', 1));
}

void CW3SurfaceAngleItem::createLine()
{
	switch (m_points.size())
	{
	case 2:
		CW3SurfaceTextLineItem::createLine();
		break;
	case 3:
		CW3SurfaceLineItem::createLine();
		CreateArch();
		break;
	default:
		break;
	}
}

void CW3SurfaceAngleItem::release()
{
	if (m_vboArc.size())
	{
		glDeleteBuffers(m_vboArc.size(), &m_vboArc[0]);
		m_vboArc.clear();
	}
	if (m_vaoArc)
	{
		glDeleteVertexArrays(1, &m_vaoArc);
		m_vaoArc = 0;
	}
}

void CW3SurfaceAngleItem::SetAngleLabel()
{
	if (m_points.size() < 3)
	{
		return;
	}

	glm::vec3 o = m_points[1];
	glm::vec3 v1 = m_points[0] - o;
	glm::vec3 v2 = m_points[2] - o;

	float z_correction_factor = slice_thickness_ / pixel_spacing_;
	v1 *= z_correction_factor;
	v2 *= z_correction_factor;

	float degree = acos(glm::dot(v1, v2) / (glm::length(v1) * glm::length(v2))) * (180.0f / M_PI);

	setText(QString("%1 [deg]").arg(degree, 0, 'f', 1));
}

void CW3SurfaceAngleItem::CreateArch()
{
	glm::vec3 o = m_points[1];
	glm::vec3 v1 = m_points[0] - o;
	glm::vec3 v2 = m_points[2] - o;

	float degree = acos(glm::dot(v1, v2) / (glm::length(v1)*glm::length(v2)))*(180.0f / M_PI);

	m_textGLpos = o;

	glm::vec3 cross = glm::cross(v1, v2);
	cross = glm::normalize(cross);

	glm::vec3 arcVector;

	float l1 = glm::length(v1);
	float l2 = glm::length(v2);

	float rad = degree*(M_PI / 180.0f);

	if (l1 < l2)
	{
		arcVector = v1*0.15f;
	}
	else
	{
		arcVector = v2*0.15f;
		rad = -rad;
	}

	std::vector<glm::vec3> arcPoints;
	arcPoints.push_back(arcVector + o);
#if !DRAW_ARCH_GL_LINES
	m_indicesArc.clear();
#endif
	for (int i = 1; i < kArchPointCount; i++)
	{
		glm::mat4 rot = glm::rotate(rad * ((float)i / kArchPointCount), cross);
		arcPoints.push_back(glm::vec3(rot * vec4(arcVector, 1.0)) + o);
#if !DRAW_ARCH_GL_LINES
		m_indicesArc.push_back(i);
		m_indicesArc.push_back(i - 1);
#endif
	}

	release();
#if !DRAW_ARCH_GL_LINES
	m_vboArc.resize(2, 0);
	CW3GLFunctions::initVAOVBO(&m_vaoArc, &m_vboArc[0], arcPoints, m_indicesArc);
#else
	m_vboArc.resize(1, 0);
	CW3GLFunctions::InitVAOVBO(&m_vaoArc, &m_vboArc[0], arcPoints);
#endif
}
