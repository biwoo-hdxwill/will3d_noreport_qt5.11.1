
#pragma once

/**=================================================================================================

Project: 			UIPrimitive
File:				W3WorldAxisItem.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo, Hong Jung
First Date:			2016-05-19
Last modify:		2017-08-01

Comment:			
thyoo 170801. view 리펙토링이 일차적으로 끝나면 전반적으로 수정해야함.					

*===============================================================================================**/
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif

#include <QObject>
#include <QGraphicsItem>

#include "uiprimitive_global.h"

class QGraphicsTextItem;
class QGraphicsLineItem;
class QGraphicsEllipseItem;
class QPainter;

class UIPRIMITIVE_EXPORT CW3WorldAxisItem : public QObject , public QGraphicsItem
{
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)
public:
	CW3WorldAxisItem(bool is2D, bool isPano);
	~CW3WorldAxisItem();

signals:
	void sigRotateMatrix(const glm::mat4& roate);
public:

	/////////////////////////////////////////////////////////////////////////
	// @param matrix: view의 camera matrix가 들어오면 된다.
	/////////////////////////////////////////////////////////////////////////
	void setDirection(const glm::mat4& cameraMat);
	void setDirection(const glm::mat4 &rotateMat, const glm::mat4& viewMat);

	void setBases(glm::mat4 *rotate);
	void setVectors(const glm::vec3 &right, const glm::vec3 &bottom, const glm::vec3 &up);

	/////////////////////////////////////////////////////////////////////////
	// @param size: 선의 길이
	// description:
	//		WorldAxis 선의 길이를 설정하고, 그 선의 길이를 기준으로 나머지 사이즈들이 결정된다
	/////////////////////////////////////////////////////////////////////////
	void setSize(const int size);

	virtual QRectF boundingRect() const { return m_rect; }

	inline bool isHovered() { return m_isHovered; }

	bool rotateCompass(glm::mat4& T, const QPointF &curPos, const QPointF &lastPos);

	void setDarkMode(bool isDark);	

	inline int getSize() { return (int)m_length; }

protected:
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
	virtual void paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget) {};

	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:
	glm::vec3 ArcBallVector(QPointF &v);
	void setColor();	

private:
	enum AXIS { X, Y, Z };

	QList<QGraphicsTextItem*> m_texts;
	QList<QGraphicsLineItem*> m_lines;
	QGraphicsEllipseItem* m_opacityEllipse;
	QGraphicsEllipseItem* m_centerEllipse;

	glm::mat4 m_dir;	// reorientation 전

	glm::mat4 m_dirLast;	// hover rotation에 사용
	glm::mat4 m_ViewPlaneMatrix;
	glm::mat4 m_scaleMatrix;
	glm::mat4 m_scaleMatrix3D;
	glm::mat4 m_bases;	// reorientation 후
	glm::mat4 m_basesInverse;

	float m_length;
	QRectF m_rect;
	bool m_isHovered;
	bool m_is2D;
	
	bool m_isDarkMode;	
	glm::mat4 m_viewMatrix;
};
