#pragma once
/*=========================================================================

File:			class CW3FilteredTextItem
Language:		C++11
Library:		Qt 5.4.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2016-05-19
Modify Date:	2016-06-20
Version:		1.0

Copyright (c) 2016 All rights reserved by HDXWILL.

=========================================================================*/
#include <QMouseEvent>
#include <QGraphicsItem>

#include "uiprimitive_global.h"

class CW3TextItem_sequence;
class CW3TextItem;

namespace ui_primitive {
const QString kReconFilterMPR("MPR");
const QString kReconFilterMIP("MIP");
const QString kReconFilterXRay("X-Ray");
const QString kReconFilterVR("VR : Shaded");
const QString kReconFilterVRUnshade("VR : Shaded");
}

class UIPRIMITIVE_EXPORT CW3FilteredTextItem : public QObject, public QGraphicsItem {
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)

public:
	CW3FilteredTextItem(const QString& text, QGraphicsItem* parent = 0);
	CW3FilteredTextItem(QGraphicsItem* parent = 0);
	~CW3FilteredTextItem();

	void setText(const QString &text);
	void setReconType(const QString &text_type);

	bool isHovered();

public:
	void clear();
	void addText(const QString& text);
	void changeText(const QString & text);
	void setTextWidth(int width);
	void setFont(const QFont& afont);

protected:
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {};
	virtual QRectF boundingRect() const override;

signals:
	void sigPressed(const QString&);

private:
	void Initialize();

private slots:
	void slotPressedSequenceText(const QString&);

private:
	CW3TextItem* m_pText;
	CW3TextItem_sequence* m_pSequenceText;
};
