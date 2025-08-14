#pragma once
/*=========================================================================

File:			class CW3SurfaceDistanceLineItem
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-11-15
Last modify:	2016-11-15

=========================================================================*/
#include "W3SurfaceTextLineItem.h"

class QGraphicsScene;

class UIGLOBJECTS_EXPORT CW3SurfaceDistanceItem : public CW3SurfaceTextLineItem
{

public:
	CW3SurfaceDistanceItem(QGraphicsScene* pScene, SHAPE shape = CURVE);
	~CW3SurfaceDistanceItem();
	 
public:
	virtual void draw(GLuint program);

	virtual void addPoint(const glm::vec3& point) override;
	virtual void editPoint(int idx, const glm::vec3& point) override;

	void setDistancePoint(const std::vector<glm::vec3>& points, float distance);

	inline void set_volume_range(const glm::vec3& range) { volume_range_ = range; }
	inline void set_pixel_spacing(const float pixel_spacing) { pixel_spacing_ = pixel_spacing; }
	inline void set_slice_thickness(const float slice_thickness) { slice_thickness_ = slice_thickness; }

	void SetDistanceLabel();

protected:
	virtual void createLine() override;

private:
	glm::vec3 MapVertexToVolume(glm::vec3 vertex_position);

private:
	glm::vec3 volume_range_ = glm::vec3(1.0f);
	float pixel_spacing_ = 1.0f;
	float slice_thickness_ = 1.0f;
};
