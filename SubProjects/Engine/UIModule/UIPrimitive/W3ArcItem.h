#pragma once
/*=========================================================================

File:			class CW3ArcItem
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun, Hong Jung
First date:		2016-02-19
Last modify:	2016-04-20

Copyright (c) 2016 All rights reserved by HDXWILL.
=========================================================================*/
#include <QGraphicsEllipseItem>
#include "uiprimitive_global.h"

class UIPRIMITIVE_EXPORT CW3ArcItem : public QGraphicsEllipseItem
{
public:
	CW3ArcItem(QGraphicsItem *parent = 0);
	CW3ArcItem(float x, float y, float width, float height, QGraphicsItem *parent = 0);
	~CW3ArcItem();

protected:
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
};

