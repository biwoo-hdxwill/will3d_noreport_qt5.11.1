#pragma once
/*=========================================================================

File:			class CW3PathItem
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-06-17
Modify Date:	2015-09-05
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include <QObject>
#include <QGraphicsPathItem>
#include <QPen>

#include "uiprimitive_global.h"

class UIPRIMITIVE_EXPORT CW3PathItem : public QObject, public QGraphicsPathItem
{
	Q_OBJECT
public:
	CW3PathItem(QGraphicsItem* pParent = 0);
	CW3PathItem(const QPen& pen, QGraphicsItem* pParent = 0);
	~CW3PathItem();

public:
	void setHighlighted(const bool highlighted);
	void setOpacityColor(bool bEnable, uchar alpha = 255);
	inline void setAntialiasing(bool bEnable) { m_bAntialiasing = bEnable; }

	inline bool isHighlight(void) const { return m_bHighlighted; }
	void drawingPath(const std::vector<QPointF>& points);
	virtual void setPen(QPen& pen);

	void setLineStatic(bool lineStatic);	
	inline void setMovable(bool movable) noexcept { m_bIsMovable = movable; }
	
	void transformItem(const QTransform& transform);

protected:
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
	void paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget);
	virtual QPainterPath shape() const override;

signals:
	void sigHoverPath(const bool bHovered);
	void sigTranslatePath(const QPointF& ptTrans);
	void sigMouseReleased();

private:
	QPainterPath* m_path = nullptr;
	QPen m_penLine;

	bool m_bHighlighted = false;
	bool m_bHovered = false;
	bool m_bAntialiasing = true;
	bool m_bIsMovable = false;
};
