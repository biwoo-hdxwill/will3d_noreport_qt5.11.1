#pragma once
/*=========================================================================

File:			class CW3LineItem
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-08-13
Modify Date:	2015-09-05
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include <QObject>
#include <QPen>
#include <QGraphicsLineItem>
#include <qpoint.h>

#include "uiprimitive_global.h"

class UIPRIMITIVE_EXPORT CW3LineItem : public QObject , public QGraphicsLineItem
{
	Q_OBJECT

public:
	CW3LineItem(int nID = 0, QGraphicsItem *parent = 0);
	~CW3LineItem();

	void SetFlagHighlight(const bool& edited);
	void SetFlagMovable(const bool & movable);
	void SetHighlightEffect(bool is_highlight);
	virtual void setPen(const QPen& pen);
	inline void setAntialiasing(bool bEnable) { m_bAntialiasing = bEnable; }
	inline bool isHovered(void) const { return m_bHovered; }
	inline int id() const { return id_; }
	inline bool flag_highlight() const { return flag_highlight_; }
	inline bool flag_movable() const { return flag_movable_; }

signals:
	void sigTranslateLine(const QPointF ptTrans, int nID);
	void sigHighLightEvent(bool is_highlight);
	void sigHoveredEvent(bool is_hovered);
	void sigMouseReleased();

protected:
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

	void paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget);
	virtual QPainterPath shape() const override;

private:
	QPen m_penLine;

	bool flag_highlight_ = false;
	bool flag_movable_ = false;
	bool m_bHovered = false;
	bool m_bAntialiasing = true;
	bool is_pressed_right = false;

	int id_;

	float interval_ = 50.0f;
	QPointF current_pos_;
};
