#pragma once

/**=================================================================================================

Project:		UIPrimitive
File:			polygon_item.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-11-22
Last modify: 	2018-11-22

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include <QObject>
#include <QPen>
#include <QGraphicsPolygonItem>

#include "uiprimitive_global.h"

class UIPRIMITIVE_EXPORT PolygonItem : public QObject, public QGraphicsPolygonItem {
	Q_OBJECT

public:
	explicit PolygonItem(QGraphicsItem* parent = 0);
	~PolygonItem(void);

	PolygonItem(const PolygonItem&) = delete;
	PolygonItem& operator=(const PolygonItem&) = delete;
public:

	void TransformItems(const QTransform& transform);
	void SetHighlighted(const bool edited);
	virtual void SetPen(const QPen& pen);
	inline void SetAntialiasing(bool enable) { is_antialiasing_ = enable; }
	inline bool IsHovered(void) const { return hovered_; }

signals:
	void sigTranslateRect(const QPointF& trans);

protected:
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

	void paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget);

private:
	QPen pen_;

	bool is_highlighted_;
	bool hovered_;
	bool is_antialiasing_;
};
