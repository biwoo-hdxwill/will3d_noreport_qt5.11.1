#include "W3SurfaceDistanceItem.h"

CW3SurfaceDistanceItem::CW3SurfaceDistanceItem(QGraphicsScene* pScene, SHAPE shape)
	: CW3SurfaceTextLineItem(pScene, shape, true)
{
}

CW3SurfaceDistanceItem::~CW3SurfaceDistanceItem()
{
}

void CW3SurfaceDistanceItem::draw(GLuint program)
{
	if (!visible_)
	{
		return;
	}

	CW3SurfaceTextLineItem::draw(program);
}

void CW3SurfaceDistanceItem::addPoint(const glm::vec3& point)
{
	CW3SurfaceLineItem::addPoint(point);

	SetDistanceLabel();
}

void CW3SurfaceDistanceItem::editPoint(int idx, const glm::vec3& point)
{
	CW3SurfaceLineItem::editPoint(idx, point);

	SetDistanceLabel();
}

void CW3SurfaceDistanceItem::setDistancePoint(const std::vector<glm::vec3>& points, float distance)
{
	setPoints(points);
	setText(QString("%1 mm").arg(distance, 0, 'f', 2));
}

void CW3SurfaceDistanceItem::createLine()
{
	CW3SurfaceTextLineItem::createLine();
}

void CW3SurfaceDistanceItem::SetDistanceLabel()
{
	if (m_points.size() < 2)
	{
		return;
	}

	float distance = 0.0f;

	for (int i = 1; i < m_points.size(); ++i)
	{
		glm::vec3 vertex_start_point = m_points.at(i - 1);
		glm::vec3 vertex_end_point = m_points.at(i);

		glm::vec3 volume_start_point = MapVertexToVolume(vertex_start_point);
		glm::vec3 volume_end_point = MapVertexToVolume(vertex_end_point);

		glm::vec3 distance_vector = volume_end_point - volume_start_point;

		distance_vector.x *= -pixel_spacing_;
		distance_vector.y *= pixel_spacing_;
		distance_vector.z *= slice_thickness_;

		distance = glm::length(distance_vector);
	}

	setText(QString("%1 [mm]").arg(distance, 0, 'f', 2));
}

glm::vec3 CW3SurfaceDistanceItem::MapVertexToVolume(glm::vec3 vertex_position)
{
	return (vertex_position + 1.0f) * 0.5f * volume_range_;
}
