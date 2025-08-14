#pragma once
/*=========================================================================

File:			image_filter_selector.h
Language:		C++11
Library:		Qt 5.8.0, Standard C++ Library
Author:			JUNG DAE GUN
First Date:		2020-05-06
Last Modify:	2020-05-06

Copyright (c) 2018~2020 All rights reserved by HDXWILL.

=========================================================================*/

#include <QMouseEvent>
#include <QGraphicsItem>

#include "uiprimitive_global.h"

class CW3TextItem_sequence;
class CW3TextItem;

class UIPRIMITIVE_EXPORT ImageFilterSelector : public QObject, public QGraphicsItem
{
	Q_OBJECT
		Q_INTERFACES(QGraphicsItem)

public:
	ImageFilterSelector(QGraphicsItem* parent = nullptr);
	virtual ~ImageFilterSelector();

	bool IsHovered();

public:
	void SetTextWidth(int width);
	void SetFont(const QFont& font);
	void SetLevel(const int level);

protected:
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {};
	virtual QRectF boundingRect() const override;

signals:
	void sigPressed(const int);

private:
	void Initialize();
	void Clear();
	void SetText(const QString& text);
	void AddText(const QString& text);

private slots:
	void slotPressedSequenceText(const QString& text);

private:
	CW3TextItem* text_ = nullptr;
	CW3TextItem_sequence* sequence_text_ = nullptr;
};
