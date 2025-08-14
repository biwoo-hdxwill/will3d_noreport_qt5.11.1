#pragma once
/*=========================================================================

File:			class CW3GuideCrossLine
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun
First date:		2016-05-12
Last modify:	2016-05-12

=========================================================================*/
#include <qpoint.h>

#include "uiprimitive_global.h"

class QGraphicsScene;
class QGraphicsLineItem;

class UIPRIMITIVE_EXPORT CW3GuideCrossLine
{
public:
	CW3GuideCrossLine(QGraphicsScene* pScene);
	~CW3GuideCrossLine();

	void setPos(QPointF pos);
	void setVisible(bool visible);

private:
	QGraphicsScene *m_pgScene;

	QGraphicsLineItem *m_pHorizontal;
	QGraphicsLineItem *m_pVertical;
};
