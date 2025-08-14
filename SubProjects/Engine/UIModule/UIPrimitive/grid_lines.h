#pragma once
/*=========================================================================

File:			class CW3ViewRuler
Language:		C++11
Library:		Qt 5.4.0
Author:			Seo Seok Man
First date:		2018-04-12
Last modify:	2018-04-12

Copyright (c) 2016 ~ 2018 All rights reserved by HDXWILL.
=========================================================================*/
#include <QObject>
#include <QGraphicsItem>
#include <qlist.h>
#include "uiprimitive_global.h"

class CW3LineItem;

class UIPRIMITIVE_EXPORT GridLines : public QObject, public QGraphicsItem {
	Q_INTERFACES(QGraphicsItem)

public:
	GridLines(QObject* parent = nullptr);
	virtual ~GridLines();

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
					   QWidget *widget) {};
	virtual QRectF boundingRect() const { return QRectF(); }

public:
	void SetGrid(float view_width, float view_height,
				 float width_length, float height_length,
				 const QPointF& view_center_in_scene,
				 float grid_step_size_mm);
	void SetGridSpacing(float spacing);

private:
	void ClearGridLines();
	void SetGridLines();

private:
	QList<CW3LineItem *> w_lines_;
	QList<CW3LineItem *> h_lines_;

	float view_width_length_ = 0.0f;
	float view_height_length_ = 0.0f;

	float view_width_in_scene_ = 0.0f;
	float view_height_in_scene_ = 0.0f;

	float grid_step_size_mm_ = 1.0f; // mm 단위

	QPointF view_center_in_scene_;
};
