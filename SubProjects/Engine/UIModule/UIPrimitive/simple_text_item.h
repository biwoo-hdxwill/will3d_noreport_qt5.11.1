#pragma once
/**=================================================================================================

Project: 			UIPrimitive
File:				simple_text_item.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-05-25
Last modify:		2018-05-25

Copyright (c) 2017 ~ 2018 All rights reserved by HDXWILL.

*===============================================================================================**/
#include <qstringlist.h>
#include <QGraphicsTextItem>

#include "uiprimitive_global.h"

class UIPRIMITIVE_EXPORT SimpleTextItem : public QGraphicsTextItem {
	Q_OBJECT

public:
	SimpleTextItem(QGraphicsItem *parent = 0);
	~SimpleTextItem();

public:
	void setPosCenter(const QPointF& scene_pos);
	void SetText(const QString& text);
	void setTextColor(const QColor &c);

	virtual void paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption,
					   QWidget *pWidget) override;

private:
	QStringList shadow_texts_;
};
