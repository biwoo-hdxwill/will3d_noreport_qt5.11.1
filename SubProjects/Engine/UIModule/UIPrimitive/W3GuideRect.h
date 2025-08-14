#pragma once
/*=========================================================================

File:			class CW3GuideRect
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun
First date:		2016-09-29
Last modify:	2016-09-29

=========================================================================*/
#include "uiprimitive_global.h"

#include <QGraphicsItem>

class QPainter;

enum EGUIDERECT_ELEMENT
{
	NONE = -1,
	CENTER,
	LEFT,
	RIGHT,
	TOP,
	BOTTOM,
	TOP_LEFT,
	TOP_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_RIGHT
};

class UIPRIMITIVE_EXPORT CW3GuideRect : public QGraphicsItem
{
	Q_INTERFACES(QGraphicsItem)
public:
	CW3GuideRect();
	~CW3GuideRect();

	virtual QRectF boundingRect() const { return m_rect; }
	void setRect(const QRectF rect, const bool change_dimension);
	EGUIDERECT_ELEMENT getElement(const QPointF &point);

protected:
	virtual void paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget) {};
	//virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
	//virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);

private:
	QRectF m_rect;

	QGraphicsLineItem *m_lpCenterCross[2];
	QGraphicsRectItem *m_pRect;
	QGraphicsLineItem *m_lpTopLeftEdge[2];
	QGraphicsLineItem *m_lpTopRightEdge[2];
	QGraphicsLineItem *m_lpBottomLeftEdge[2];
	QGraphicsLineItem *m_lpBottomRightEdge[2];
	
	int m_nSolidLineLength;

	//EGUIDERECT_ELEMENT m_eSelectedElement;
};

