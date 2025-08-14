#include "W3SurfaceLineItem.h"

#include "qmath.h"
#include "../UIPrimitive/W3TextItem.h"

#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/Common/W3ElementGenerator.h"
#include "../../Common/Common/common.h"
#include <Engine/Common/Common/global_preferences.h>

using namespace std;
using glm::vec3;

namespace
{
	const float kLineWidth = 2.0f;
	const int kMeshCirclePoints = 8;
	const float kMeshCircleSegAngle = (2.0f*M_PI) / kMeshCirclePoints;

	void samplingPoints(const float divideLen, std::vector<glm::vec3>& points)
	{
		std::vector<glm::vec3> equiSpliPoints;
		equiSpliPoints.push_back(points.front());
		for (int i = 1; i < points.size(); i++)
		{
			glm::vec3 p0 = equiSpliPoints.back();
			glm::vec3 p1 = points[i];

			glm::vec3 v = p1 - p0;
			int length = glm::length(v);

			v /= length;

			for (int j = 0; j < length; j++)
				equiSpliPoints.push_back(p0 + v*((float)j + 1));
		}

		if (equiSpliPoints.size() > divideLen)
		{
			int splitStep;
			splitStep = (int)divideLen;

			points.clear();

			for (int i = 0; i < equiSpliPoints.size(); i += splitStep)
				points.push_back(equiSpliPoints[i]);

			glm::vec3 tailVec = points.back() - equiSpliPoints.back();
			int tailLen = glm::length(tailVec);
			if (tailLen <= splitStep)
			{
				points.pop_back();
				points.push_back((points.back() + equiSpliPoints.back())*0.5f);
			}

			points.push_back(equiSpliPoints.back());
		}
	}

	vec3 GetOrthoVector(const vec3 & dir, const vec3& unit)
	{
		vec3 tmp = normalize(unit);
		const float offset = 0.00001f;

		float dot = glm::dot(dir, tmp);

		if (dot <= offset && dot >= -offset)
			tmp.z += offset;
		else if (dot <= 1 + offset && dot >= 1 - offset)
			tmp.z += offset;

		return normalize(cross(dir, tmp));
	}
}
CW3SurfaceLineItem::CW3SurfaceLineItem(SHAPE shape, bool bDrawEllipse)
	: m_shape(shape), m_bDrawEllipse(bDrawEllipse),
	CW3SurfaceEllipseItem(shape == SHAPE::SIMPLE_LINE ? CW3SurfaceEllipseItem::Shape::SIMPLE_POINT : CW3SurfaceEllipseItem::Shape::CIRCLE)
{
	m_lineColor = QColor(Qt::green);
	m_vaoLine = 0;
}
CW3SurfaceLineItem::~CW3SurfaceLineItem()
{
	clearVAOVBO();
}

void CW3SurfaceLineItem::clear()
{
	CW3SurfaceEllipseItem::clear();
	m_isCreateLine = false;
}
void CW3SurfaceLineItem::clearVAOVBO()
{
	CW3SurfaceEllipseItem::clearVAOVBO();

	if (m_vaoLine)
	{
		glDeleteVertexArrays(1, &m_vaoLine);
		m_vaoLine = 0;
	}
	if (m_vboLine.size())
	{
		glDeleteBuffers(m_vboLine.size(), &m_vboLine[0]);
		m_vboLine.clear();
	}
	cnt_indices_ = 0;
}

void CW3SurfaceLineItem::draw(GLuint program)
{
	if (!visible_)
	{
		return;
	}

	if (m_points.size() < 2)
	{
		if (m_bDrawEllipse)
			CW3SurfaceEllipseItem::draw(program);

		return;
	}

	if (!m_isCreateLine)
	{
		this->createLine();
		m_isCreateLine = true;
	}

	if (!m_vaoLine)
	{
		this->createLine();
	}

	m_model = mat4(1.0f);

	//WGLSLprogram::setUniform(program, "VolTexTransformMat", this->getVolTexTransformMat());
	setMatrix(program);
	glm::vec3 color((float)m_lineColor.red() / 255.0f, (float)m_lineColor.green() / 255.0f, (float)m_lineColor.blue() / 255.0f);

	WGLSLprogram::setUniform(program, "Material.Ka", color);
	WGLSLprogram::setUniform(program, "Material.Ks", vec3(0.0));
	WGLSLprogram::setUniform(program, "Material.Kd", vec3(0.0));
	WGLSLprogram::setUniform(program, "Material.Shininess", 1.0f);
	WGLSLprogram::setUniform(program, "alpha", m_alpha);

	GLenum draw_mode = GL_TRIANGLES;
	if (m_shape == SIMPLE_LINE)
	{
		draw_mode = GL_LINES;
		glLineWidth(line_selected_ ? kLineWidth + 1.0f : kLineWidth);
	}

	glBindVertexArray(m_vaoLine);
	{
		glDrawElements(
			draw_mode,
			cnt_indices_,
			GL_UNSIGNED_INT,
			(void*)0
		);
	}
	glBindVertexArray(0);

	if (m_bDrawEllipse)
		CW3SurfaceEllipseItem::draw(program);
}

void CW3SurfaceLineItem::pick(const QPointF& pickPoint, bool* isUpdateGL, GLuint program)
{
	*isUpdateGL = false;

	if (m_bDrawEllipse)
	{
		CW3GLFunctions::clearView(true, GL_BACK);

		CW3SurfaceEllipseItem::pick(pickPoint, isUpdateGL, program);
		if (CW3SurfaceEllipseItem::getPickIndex() > -1)
		{
			if (line_selected_)
			{
				m_isCreateLine = false;
			}
			return;
		}
	}

	if (m_points.size() < 2)
	{
		return;
	}

	if (!m_isCreateLine)
	{
		this->createLine();
		m_isCreateLine = true;
	}

	if (!m_vaoLine)
	{
		this->createLine();
	}

	//CW3GLFunctions::clearView(true, GL_BACK);

	m_model = mat4(1.0f);

	setMatrix(program);
	WGLSLprogram::setUniform(program, "index", 0);

	GLenum draw_mode = GL_TRIANGLES;
	if (m_shape == SIMPLE_LINE)
	{
		draw_mode = GL_LINES;
		glLineWidth(kLineWidth + 2.0f);
	}

	glBindVertexArray(m_vaoLine);
	{
		glDrawElements(
			draw_mode,
			cnt_indices_,
			GL_UNSIGNED_INT,
			(void*)0
		);
	}
	glBindVertexArray(0);

	unsigned char id = 255;
	vec3 picked_pos;
	readPickInfo(pickPoint.x(), pickPoint.y(), &id, &picked_pos);

	bool old_line_selected = line_selected_;
	if (id == 0)
	{
		line_selected_ = true;
	}
	else
	{
		line_selected_ = false;
	}

	if (line_selected_)
	{
		SetAllPointsSelected(line_selected_);
	}

	bool update_line = old_line_selected != line_selected_;
	m_isCreateLine = !update_line;

	*isUpdateGL |= (old_line_selected != line_selected_);
}

void CW3SurfaceLineItem::addPoint(const glm::vec3 & point)
{
	CW3SurfaceEllipseItem::addPoint(point);
	m_isCreateLine = false;
}

void CW3SurfaceLineItem::erasePoint(int index)
{
	CW3SurfaceEllipseItem::erasePoint(index);
	m_isCreateLine = false;
}
void CW3SurfaceLineItem::editPoint(int idx, const glm::vec3 & point)
{
	CW3SurfaceEllipseItem::editPoint(idx, point);
	m_isCreateLine = false;
}
void CW3SurfaceLineItem::setPoints(const std::vector<glm::vec3>& points)
{
	CW3SurfaceEllipseItem::setPoints(points);
	m_isCreateLine = false;
}
void CW3SurfaceLineItem::createLine()
{
	if (m_points.size() < 2)
	{
		return;
	}

	std::vector<vec3> mesh_vertices, mesh_normals;
	std::vector<uint> mesh_indices;

	if (m_shape == SIMPLE_LINE)
	{
		const int point_count = m_points.size();
		mesh_vertices.reserve(point_count);
		mesh_normals.reserve(point_count);
		mesh_indices.reserve(point_count * 2 - 2);

		for (int i = 0; i < m_points.size(); ++i)
		{
			mesh_vertices.push_back(m_points.at(i));
			mesh_normals.push_back(m_points.at(i));

			if (i < m_points.size() - 1)
			{
				mesh_indices.push_back(i);
				mesh_indices.push_back(i + 1);
			}
		}
	}
	else
	{
		std::vector<vec3> tmp_points;
		if (m_shape == CURVE)
		{
			tmp_points.assign(m_points.begin(), m_points.end());
		}
		else if (m_shape == SPLINE)
		{
			Common::generateCubicSpline(m_points, tmp_points);
		}

		if (tmp_points.size() < 2)
		{
			return;
		}

		int vertices_size = tmp_points.size();
		mesh_vertices.reserve(vertices_size);
		mesh_indices.reserve(vertices_size * 6);
		mesh_normals.reserve(vertices_size);

		glm::vec3 tail = tmp_points.back() * 2.0f - tmp_points.at(tmp_points.size() - 2);
		tmp_points.push_back(tail);

		glm::vec3 unit = vec3(1.0f);
		for (int i = 0; i < (int)tmp_points.size() - 1; i++)
		{
			vec3 p1 = tmp_points[i];
			vec3 p2 = tmp_points[i + 1];

			vec3 dir = normalize(p2 - p1);
			vec3 ortho = GetOrthoVector(dir, unit);

			int index = kMeshCirclePoints*i;

			vec4 ortho_rotate = vec4(ortho, 0.0f);

			for (int r = 0; r < kMeshCirclePoints; r++)
			{
				ortho_rotate = rotate(kMeshCircleSegAngle, dir)*ortho_rotate;
				float line_width = line_selected_ ? line_width_ * 1.5f : line_width_;
				mesh_vertices.push_back(p1 + vec3(ortho_rotate) * line_width * 0.5f);
			}

			if (i > tmp_points.size() - 3)
				continue;

			for (int r = 0; r < kMeshCirclePoints; r++)
			{
				uint i0 = index + r;
				uint i1 = index + (r + 1) % kMeshCirclePoints;
				uint i2 = index + r + kMeshCirclePoints;
				uint i3 = index + (r + 1) % kMeshCirclePoints + kMeshCirclePoints;

				mesh_indices.push_back(i0);
				mesh_indices.push_back(i1);
				mesh_indices.push_back(i2);

				mesh_indices.push_back(i2);
				mesh_indices.push_back(i1);
				mesh_indices.push_back(i3);
			}
		}

		CW3ElementGenerator::GenerateSmoothNormals(mesh_vertices, mesh_indices, mesh_normals);
	}

	clearVAOVBO();
	m_vboLine.resize(3, 0);
	cnt_indices_ = mesh_indices.size();

	CW3GLFunctions::initVAOVBO(&m_vaoLine, &m_vboLine[0], mesh_vertices, mesh_normals, mesh_indices);
}

void CW3SurfaceLineItem::SetSelected(const bool selected)
{
	line_selected_ = selected;
	SetAllPointsSelected(selected);
}

void CW3SurfaceLineItem::SetVisible(const bool visible)
{
	visible_ = visible;
}

void CW3SurfaceLineItem::ApplyPreferences()
{
	ApplyNodeColor();
	ApplyLineColor();
}

void CW3SurfaceLineItem::ApplyNodeColor()
{
	QColor line_color = GlobalPreferences::GetInstance()->preferences_.objects.measure.line_color;
	setEllipseColor(line_color);
}

void CW3SurfaceLineItem::ApplyLineColor()
{
	QColor line_color = GlobalPreferences::GetInstance()->preferences_.objects.measure.line_color;
	setLineColor(line_color);
}

void CW3SurfaceLineItem::SetNodeVisible(const bool visible)
{
	m_bDrawEllipse = visible;
}
