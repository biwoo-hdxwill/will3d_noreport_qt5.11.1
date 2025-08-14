#pragma once
/*=========================================================================

File:			class CW3SurfaceEllipseItem
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-02-22
Last modify:	2016-02-24

=========================================================================*/
#include <qcolor.h>
#include <qpoint.h>
#include "W3SurfaceItem.h"

class UIGLOBJECTS_EXPORT CW3SurfaceEllipseItem : public CW3BaseSurfaceItem
{
public:
	enum class Shape {CIRCLE, SIMPLE_POINT};

public:
	CW3SurfaceEllipseItem(Shape shape = Shape::CIRCLE);
	virtual ~CW3SurfaceEllipseItem();
	 
public:
	void clear();
	void clearVAOVBO();
	void pick(const QPointF& pickPoint, bool* isUpdateGL, GLuint program);

	virtual void draw(GLuint program);
	void draw(unsigned int nIndex, GLuint program);
	int getPickIndex();
	glm::vec3 getPointPosition(int idx);

	void addPoint(const glm::vec3& point);
	void erasePoint(int index);
	
	inline void editPoint(int idx, const glm::vec3& point) {
		if (idx >= 0 && idx < m_points.size()) m_points[idx] = point;
	}
	
	void setPoints(const std::vector<glm::vec3>& points);
	void setEllipseSize(int size);

	void SetAllPointsSelected(const bool selected);

	inline void setEllipseColor(const QColor& color) { m_ellipseColor = color; }
	inline void setBrushColor(const QColor& color) { m_brushColor = color; }
	inline void setbrushSize(int size) { m_brushSize = size; }

	inline QColor getBrushColor() { return m_brushColor; }

	inline const std::vector<glm::vec3>& points() { return m_points; }

protected:
	Shape shape_ = Shape::CIRCLE;

	std::vector<glm::vec3> m_points;

private:
	void createEllipse();

private:
	unsigned int m_vao = 0;
	std::vector<unsigned int> m_vbo;
	std::vector<unsigned int> m_indices;
	std::vector<bool> m_pickType;

	glm::vec3 m_pickPos;

	QColor m_ellipseColor;
	QColor m_brushColor;

	int m_ellipseSize = 7;
	int m_brushSize = 4;
	unsigned char m_curPickType = 0;

	bool m_isPickItem = false;
};
