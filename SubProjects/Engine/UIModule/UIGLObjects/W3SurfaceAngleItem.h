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

class UIGLOBJECTS_EXPORT CW3SurfaceAngleItem : public CW3SurfaceTextLineItem
{

public:
	CW3SurfaceAngleItem(QGraphicsScene* pScene, SHAPE shape = CURVE);
	~CW3SurfaceAngleItem();

public:
	virtual void draw(GLuint program) override;

	virtual void addPoint(const glm::vec3& point) override;
	virtual void editPoint(int idx, const glm::vec3& point) override;

	void clear();
	void clearVAOVBO();
	void setAnglePoints(const glm::vec3& ori, const glm::vec3& p1, const glm::vec3& p2);
	void setAnglePoints(const glm::vec3& ori, const glm::vec3& p1, const glm::vec3& p2, float degree);

	inline void set_pixel_spacing(const float pixel_spacing) { pixel_spacing_ = pixel_spacing; }
	inline void set_slice_thickness(const float slice_thickness) { slice_thickness_ = slice_thickness; }
	 
	void SetAngleLabel();

protected:
	virtual void createLine() override;

private:
	void release();
	void CreateArch();

private:
	unsigned int m_vaoArc;
	std::vector<unsigned int> m_vboArc;

	std::vector<unsigned int> m_indicesArc;

	float pixel_spacing_ = 1.0f;
	float slice_thickness_ = 1.0f;
};
