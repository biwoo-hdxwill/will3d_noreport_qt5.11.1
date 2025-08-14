#pragma once
/*=========================================================================

File:		class CW3LineItem_otf
Language:	C++11
Library:	Qt 5.2.0

=========================================================================*/
#include <qgraphicsitem.h>
#include <qpoint.h>

#include "uiprimitive_global.h"
/*
	class CW3LineItem_otf
	 : Re-implementation of QGraphicsLineItem
*/
class UIPRIMITIVE_EXPORT CW3LineItem_otf : public QGraphicsLineItem
{
public:
	CW3LineItem_otf(const QPointF, const QPointF);
	~CW3LineItem_otf(void);
	/**************************
		public interface.
	***************************/
	void activate(void);
	void deactivate(void);
	virtual QPainterPath shape() const override;
};

