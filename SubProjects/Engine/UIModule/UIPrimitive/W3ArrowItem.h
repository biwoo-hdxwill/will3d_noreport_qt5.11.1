#pragma once
/*=========================================================================

File:			class CW3GuideCrossLine
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun
First date:		2016-05-12
Last modify:	2016-05-12

=========================================================================*/
#include <QGraphicsLineItem>

#include "uiprimitive_global.h"

class QGraphicsScene;

class UIPRIMITIVE_EXPORT ArrowItem
{
public:
	ArrowItem(QGraphicsScene* pScene, QGraphicsItem* parent = 0);
	~ArrowItem();

	void setPos(QPointF pos);
	void setStart(QPointF pos);
	void setEnd(QPointF pos);
	void setVisible(bool visible);
	void addItemAll();	
	void removeItemAll();	

	inline QPointF getStart() { return m_pLine->line().p1(); }
	inline QPointF getEnd() { return m_pLine->line().p2(); }
	inline bool isVisible() { return m_pLine->isVisible(); }

private:
	void setArrowHead();

private:
	QGraphicsScene *m_pgScene;

	QGraphicsLineItem *m_pLine;
	QGraphicsPolygonItem *head[2];
};

