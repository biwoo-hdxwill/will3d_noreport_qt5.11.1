#pragma once
/*=========================================================================

File:			class CW3RectItem
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-11-17
Modify Date:	2015-11-17
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include <QObject>
#include <QPen>
#include <QGraphicsRectItem>

#include "uiprimitive_global.h"

class UIPRIMITIVE_EXPORT CW3RectItem : public QObject, public QGraphicsRectItem {
	Q_OBJECT

public:
	CW3RectItem(int nID = 0, QGraphicsItem* parent = 0);
	~CW3RectItem(void);

	void TransformItems(const QTransform& transform);
	void setHighlighted(const bool edited);
	virtual void setPen(const QPen& pen);
	inline void setAntialiasing(bool bEnable) { m_bAntialiasing = bEnable; }
	inline bool isHovered(void) const { return m_bHovered; }

signals:
	void sigTranslateRect(const QPointF ptTrans, const int nID);

protected:
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

	void paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget);

private:
	QPen m_penRect;

	bool m_bHighlighted;
	bool m_bHovered;
	bool m_bAntialiasing;
	int m_nID;
};
