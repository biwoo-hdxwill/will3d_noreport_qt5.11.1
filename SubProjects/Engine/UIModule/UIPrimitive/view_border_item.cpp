#include "view_border_item.h"

////////////////////////////////////////////////////////////////////////////////////
//ViewBorderItem
////////////////////////////////////////////////////////////////////////////////////

ViewBorderItem::ViewBorderItem(QGraphicsItem* parent)
	:QGraphicsItem(parent)
{
	this->setFlag(QGraphicsItem::ItemIsSelectable, false);
	this->setFlag(QGraphicsItem::ItemIsMovable, false);
	this->setZValue(0);

	QPen pen;
	pen.setCosmetic(true);
	pen = QPen(QColor(0, 255, 0), 2.0, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin);
	border_.reset(new QGraphicsRectItem(this));	
	border_->setPen(pen);
	border_->setZValue(0);

	border_shadow_.reset(new QGraphicsRectItem(this));
	border_shadow_->setPen(QPen(QColor(0, 0, 0, 127), 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	border_shadow_->setZValue(2);
}

ViewBorderItem::~ViewBorderItem()
{
}

void ViewBorderItem::SetColor(const QColor& color) {
	QPen pen = border_->pen();
	pen.setColor(color);
	border_->setPen(pen);
}

void ViewBorderItem::SetRect(const QRectF& rect) {
	//rect_ = rect;
	border_->setRect(rect.adjusted(2, 2, -3, -3));
	border_shadow_->setRect(rect);
}
