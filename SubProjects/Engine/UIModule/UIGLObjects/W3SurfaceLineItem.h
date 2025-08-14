#pragma once
/*=========================================================================

File:			class CW3SurfaceLineItem
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-09-20
Last modify:	2016-09-20

=========================================================================*/
#include "W3SurfaceEllipseItem.h"

#include <qcolor.h>

class CW3TextItem;

class UIGLOBJECTS_EXPORT CW3SurfaceLineItem : public CW3SurfaceEllipseItem
{
public:
	enum SHAPE { CURVE, SPLINE, SIMPLE_LINE };

public:
	CW3SurfaceLineItem(SHAPE shape, bool bDrawEllipse);
	~CW3SurfaceLineItem();

public:
	virtual void clear();
	void clearVAOVBO();
	void pick(const QPointF& pickPoint, bool* isUpdateGL, GLuint program);

	virtual void draw(GLuint program);

	virtual void addPoint(const glm::vec3& point);
	void erasePoint(int index);
	virtual void editPoint(int idx, const glm::vec3& point);
	void setPoints(const std::vector<glm::vec3>& points);

	void SetSelected(const bool selected);

	inline void setLineColor(QColor color) { m_lineColor = color; }
	inline void set_line_width(float line_width) { line_width_ = line_width; }
	inline void changeShape(SHAPE shape) { m_shape = shape; m_isCreateLine = false; }

	inline QColor getLineColor() { return m_lineColor; }
	inline const float line_width() const { return line_width_; }
	inline const bool line_selected() const { return line_selected_; }

	virtual void SetVisible(const bool visible);

	virtual void ApplyPreferences();

	void SetNodeVisible(const bool visible);

protected:
	virtual void createLine();

	void ApplyNodeColor();
	void ApplyLineColor();

protected:
	unsigned int m_vaoLine;
	std::vector<unsigned int> m_vboLine;

	int cnt_indices_ = 0;
	float line_width_ = 1.0f;

	QColor m_lineColor;

	SHAPE m_shape;
	
	bool m_isCreateLine;
	bool m_bDrawEllipse;

	bool line_selected_ = false;

	bool visible_ = true;
};
