#pragma once
 
/**=================================================================================================

Project:		UIGLObjects
File:			view_navigator.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-03-20
Last modify: 	2018-03-20

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <memory>
#if defined(__APPLE__)
#include <glm/detail/type_mat4x4.hpp>
#else
#include <GL/glm/detail/type_mat4x4.hpp>
#endif

#include <qobject.h>
#include <QGraphicsItem>
#include <QPixmap>

#include "uiviewcontroller_global.h"
class QGraphicsPixmapItem;

class UIVIEWCONTROLLER_EXPORT ViewNavigatorItem : public QObject, public QGraphicsItem {
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)
public:
	ViewNavigatorItem(QObject* parent = nullptr);
	~ViewNavigatorItem();

	ViewNavigatorItem(const ViewNavigatorItem&) = delete;
	ViewNavigatorItem& operator=(const ViewNavigatorItem&) = delete;

	const glm::mat4& GetRotateMatrix();

public:
	void SetWorldAxisDirection(const glm::mat4& rot_mat, const glm::mat4& view_mat);
	void SetSize(int width, int height);
	virtual QRectF boundingRect() const override { return rect_; }
	virtual void paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget) override;;
private:
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
private:

	QPixmap img_;
	QRectF rect_;
	QSize size_;
};
