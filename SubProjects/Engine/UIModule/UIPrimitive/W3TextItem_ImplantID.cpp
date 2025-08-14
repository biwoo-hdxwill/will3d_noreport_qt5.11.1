/*=========================================================================

File:			class CW3TextItem_ImplantID
Language:		C++11
Library:		Qt 5.4.0
Author:			Sang Keun Park
First date:		2016-06-30
Last modify:

Copyright (c) 2016 All rights reserved by HDXWILL.
=========================================================================*/
#include <QApplication>
#include <qgraphicsscene.h>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Define.h"
#include "W3TextItem_ImplantID.h"

CW3TextItem_ImplantID::CW3TextItem_ImplantID()
{
	text_item_ = new CW3TextItemImplant("");
	text_item_->setPlainText(QString(""));
	text_item_->setDefaultTextColor(Qt::yellow);

	QFont font = QApplication::font();
	font.setWeight(QFont::Normal);

	text_item_->setFont(font);

	text_item_->setVisible(false);

	text_item_->setZValue(0.0f);
	text_item_->setFlag(QGraphicsItem::ItemIsSelectable, false);
	text_item_->setFlag(QGraphicsItem::ItemIsFocusable, false);
	text_item_->setAcceptHoverEvents(false);
}

CW3TextItem_ImplantID::~CW3TextItem_ImplantID()
{
	SAFE_DELETE_OBJECT(text_item_);
}

void CW3TextItem_ImplantID::addItems(QGraphicsScene *pScene)
{
	pScene->addItem(text_item_);
}

void CW3TextItem_ImplantID::setVisible(bool bVisble)
{
	text_item_->setVisible(bVisble);
}

void CW3TextItem_ImplantID::setVisible(int index, bool bVisble)
{
	text_item_->setVisible(bVisble);
}

void CW3TextItem_ImplantID::setPos(int index, const QPointF &pos)
{
	text_item_->setPos(pos);
}
void CW3TextItem_ImplantID::setPos(const QPointF &pos)
{
	text_item_->setPos(pos);
}

void CW3TextItem_ImplantID::setID(int index, int id)
{
	text_item_->setPlainText(QString("%1").arg(id));
}

QRectF CW3TextItem_ImplantID::sceneBoundingRect(int index)
{
	return text_item_->sceneBoundingRect();
}

void CW3TextItem_ImplantID::transformItems(const QTransform& transform)
{
	text_item_->setPos(transform.map(text_item_->pos()));
}

void CW3TextItem_ImplantID::ResetHoverIDs() noexcept
{
	hovered_id_ = -1;
	prehovered_id_ = -1;
}
