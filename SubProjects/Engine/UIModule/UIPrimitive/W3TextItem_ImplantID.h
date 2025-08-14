#pragma once
/*=========================================================================

File:			class CW3TextItem_ImplantID
Language:		C++11
Library:		Qt 5.4.0
Author:			Sang Keun Park
First date:		2016-06-30
Last modify:

Copyright (c) 2016 All rights reserved by HDXWILL.
=========================================================================*/
#include <QObject>
#include <QGraphicsTextItem>

#include "uiprimitive_global.h"

class QGraphicsScene;

class CW3TextItemImplant : public QGraphicsTextItem {
public:
	CW3TextItemImplant(const QString &text) :
		QGraphicsTextItem(text) {}

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *w) {
		QGraphicsTextItem::paint(painter, o, w);
	}
};

class UIPRIMITIVE_EXPORT CW3TextItem_ImplantID : public QObject {
	Q_OBJECT

public:
	CW3TextItem_ImplantID();
	~CW3TextItem_ImplantID();

public:
	void addItems(QGraphicsScene *pScene);
	void setVisible(bool bVisble);
	void setVisible(int index, bool bVisble);
	void setPos(int index, const QPointF &pos);
	void setPos(const QPointF &pos);
	void setID(int index, int id);
	QRectF sceneBoundingRect(int index);

	void transformItems(const QTransform& transform);

	void ResetHoverIDs() noexcept;
	inline const int hovered_id() const noexcept { return hovered_id_; }
	inline const int prehovered_id() const noexcept { return prehovered_id_; }
	inline void set_hovered_id(int id) noexcept { hovered_id_ = id; }
	inline void set_prehovered_id(int id) noexcept { prehovered_id_ = id; }

private:
	CW3TextItemImplant* text_item_ = nullptr;
	int hovered_id_ = -1; // 현재 hover 된 implant id
	int prehovered_id_ = -1; // 이전 hover 된 implant id
};
