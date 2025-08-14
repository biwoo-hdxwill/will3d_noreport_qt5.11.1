#pragma once

/*=========================================================================

File:			class CW3LabelItem_anno
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			YouSong kim, JUNG DAE GUN
First Date:		2015-06-17
Modify Date:	2016-05-31
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include <qstringlist.h>
#include <QGraphicsTextItem>
#include <qpoint.h>

#include "uiprimitive_global.h"

class UIPRIMITIVE_EXPORT CW3LabelItem_anno : public QGraphicsTextItem
{
	Q_OBJECT
public:
	CW3LabelItem_anno(QGraphicsItem *parent = 0);
	~CW3LabelItem_anno(void);

public:
	virtual void paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget) override;
	void SetTextColor(QColor color);
	void setPlainText(const QString& text);

signals:
	void sigPressed(void);
	void sigDoubleClicked();	// by jdk 160603
	void sigHoverLabel(bool);
	void sigTranslateLabel(QPointF);

protected:
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;	// by jdk 160603

private:
	QPointF m_ptScene;
	QColor color_;
	QStringList shadow_texts_;
};

