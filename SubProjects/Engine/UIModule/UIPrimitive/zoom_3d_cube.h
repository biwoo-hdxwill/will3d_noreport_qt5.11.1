#pragma once
/*=========================================================================

File:			zoom_3d_cube.h
Language:		C++11
Library:		Qt 5.2.0
Author:			JUNG DAE GUN
First date:		2015-07-22
Last modify:	2015-07-24

=========================================================================*/
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif

#include <qpen.h>

#include "uiprimitive_global.h"

class QGraphicsScene;
class QGraphicsEllipseItem;
class QGraphicsLineItem;

class UIPRIMITIVE_EXPORT Zoom3DCube : public QObject {
	Q_OBJECT
public:
	Zoom3DCube(const QPointF &point);
	Zoom3DCube(const QPointF &point, const glm::vec3 &rightVec, const glm::vec3 &backVec);
	~Zoom3DCube(void);

	enum ZOOM3D_ELEMENT {
		ZOOM3D_NONE = -1,
		SPHERE,
		CUBE
	};

public:
	// functions
	void AddToScene(QGraphicsScene *scene);
	void RemoveFromScene(QGraphicsScene *scene);
	void setVisible(bool visible);
	void resize(const float &redius = 0.0f);
	void resize(const QPointF &point, const float &radius = 0.0f);
	void rotate(const glm::mat4 &rotMat, const glm::vec3 &rightVec, const glm::vec3 &backVec);
	QPointF center() const { return center_pos_; };
	void setCenter(const QPointF &center);
	inline float radius() const noexcept { return m_fRadius; }
	void GetMargin(float& view_margin);
	ZOOM3D_ELEMENT selectedElement(const QPointF &point);
	bool isDrawMode() const { return m_bIsDrawMode; };
	void drawStart() { m_bIsDrawMode = true; };
	void drawEnd() { m_bIsDrawMode = false; };
	void removeItemAll(QGraphicsScene *scene);
	void addItemAll(QGraphicsScene *scene);

	void transformItems(const QTransform& transform);

	void resetUI(void);
	void UpdateUI();

private:
	void init();
	void drawCube();

private:
	QPointF center_pos_;
	float m_fRadius;
	QGraphicsEllipseItem *ellipse_;
	QList<QGraphicsLineItem *> center_lines_;
	ZOOM3D_ELEMENT m_eCurrElement;
	bool m_bIsDrawMode;

	QList<glm::vec3> m_listPoint;
	QList<QGraphicsLineItem *> cube_lines_;

	glm::mat4 m_mRotMat;
	glm::vec3 m_vRightVec;
	glm::vec3 m_vBackVec;

	QPen m_penCube;
	QPen m_penEllipse;
	QPen m_penLine;
};
