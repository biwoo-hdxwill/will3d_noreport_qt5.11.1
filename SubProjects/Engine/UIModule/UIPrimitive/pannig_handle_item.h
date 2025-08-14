#pragma once

/**=================================================================================================

Project:		UIPrimitive
File:			pannig_handle_item.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-03-15
Last modify: 	2018-03-15

	Copyright (c) 2018 HDXWILL. All rights reserved.

*===============================================================================================**/

#include <QObject>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include "uiprimitive_global.h"

class UIPRIMITIVE_EXPORT PanningHandleItem : public QObject, public QGraphicsPixmapItem {
	Q_OBJECT

signals :
	void sigTranslate(const QPointF& pt_trans);
	void sigMouseReleased();

public:
	explicit PanningHandleItem(QGraphicsItem* parent = 0);
	~PanningHandleItem(void);
	PanningHandleItem(const PanningHandleItem&) = delete;
	PanningHandleItem& operator=(const PanningHandleItem&) = delete;

public:

private:
	virtual void paint(QPainter * pPainter, const QStyleOptionGraphicsItem * pOption, QWidget * pWidget) override;
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

	QPainterPath shape() const;

private:
	QPixmap pixmap_prev_cursor;
};
