#pragma once

/**=================================================================================================

Project:		UIPrimitive
File:			view_border_item.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-02-06
Last modify: 	2018-02-06

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/


#include "uiprimitive_global.h"

#include <memory>
#include <QObject>
#include <QGraphicsItem>
#include <QPen>


////////////////////////////////////////////////////////////////////////////////////
//ViewBorderItem
////////////////////////////////////////////////////////////////////////////////////

class UIPRIMITIVE_EXPORT ViewBorderItem : public QObject, public QGraphicsItem
{
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)
public:
	ViewBorderItem(QGraphicsItem* parent = nullptr);
	~ViewBorderItem();

	ViewBorderItem(const ViewBorderItem&) = delete;
	ViewBorderItem& operator=(const ViewBorderItem&) = delete;
public:
	void SetColor(const QColor& color);
	void SetRect(const QRectF& rect);
	

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) { };

	virtual QRectF boundingRect() const { return rect_; }

private:
	std::unique_ptr<QGraphicsRectItem> border_;
	std::unique_ptr<QGraphicsRectItem> border_shadow_;
	QRectF rect_;
};


