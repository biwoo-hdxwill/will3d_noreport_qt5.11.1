#pragma once
/*=========================================================================

File:		class CW3TextItem_otf
Language:	C++11
Library:	Qt 5.2.0

=========================================================================*/
#include <qgraphicsitem.h>
#include "uiprimitive_global.h"

/*
	class CW3TextItem_otf
	 : Re-implementation of QGraphicsTextItem
*/
class UIPRIMITIVE_EXPORT CW3TextItem_otf : public QGraphicsTextItem
{
public:
	CW3TextItem_otf(const QPointF pt, const float val);
	~CW3TextItem_otf(void);
};

