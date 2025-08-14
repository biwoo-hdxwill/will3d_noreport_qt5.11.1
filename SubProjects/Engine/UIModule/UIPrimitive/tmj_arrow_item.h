#pragma once

/**=================================================================================================

Project:		UIPrimitive
File:			tmj_arrow_item.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-11-26
Last modify: 	2018-11-26

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include <memory>
#include <qpen.h>
#include <qbrush.h>
#include <QGraphicsItem>
#include "uiprimitive_global.h"

class CW3LineItem;
class QGraphicsPolygonItem;

class UIPRIMITIVE_EXPORT TmjArrowItem : public QObject, public QGraphicsItem
{
	Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
signals:
	void sigHover(bool is_hover);
	void sigTranslateLine(const QPointF& translate);
public:
	explicit TmjArrowItem(QGraphicsItem* parent = 0);
	~TmjArrowItem();

	void TransformItems(const QTransform& transform);
	void SetPen(const QPen& pen);
	void SetBrush(const QBrush& brush);
	void SetPos(QPointF pos);
	void SetStart(QPointF pos);
	void SetEnd(QPointF pos);
	void setZValue(qreal z);
	void SetVisible(bool visible);

	QPointF GetStart();
	QPointF GetEnd();
	bool IsVisible();
	bool IsHovered();

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) { };
	virtual QRectF boundingRect() const;
private:
	void SetArrowHead();
	virtual QPainterPath shape() const override;
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private slots:

private:

	std::unique_ptr<CW3LineItem> line_;
	std::unique_ptr<QGraphicsPolygonItem> head[2];

	QPen pen_;
	QBrush brush_;
	bool is_hovered = false;
};

