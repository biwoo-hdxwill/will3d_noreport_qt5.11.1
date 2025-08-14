#pragma once

/*=========================================================================

File:			class CW3RectItem_anno
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			YouSong kim, JUNG DAE GUN
First Date:		2015-06-17
Modify Date:	2016-05-31
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include <qobject.h>
#include <qpoint.h>
#include <QGraphicsRectItem>
#include <QPen>

#include "uiprimitive_global.h"

class UIPRIMITIVE_EXPORT CW3RectItem_anno : public QObject, public QGraphicsRectItem
{
	Q_OBJECT
public:
	CW3RectItem_anno(const QPointF point);
	~CW3RectItem_anno();

public:
	void setHighlightEffect(bool bFlag);
	void setPenColor(QColor color);
	void setBrushColor(QColor color);

protected:
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
	void paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget);

signals:
	void sigTranslateRect(QPointF);
	void sigHoverRect(const bool);
	
private:
	QPointF m_ptScene;
	QPen	m_pen;
	QPen	m_hoverPen;
	QBrush	m_brush;
	QColor	m_penColor;
	bool	m_bHighlight;
};
