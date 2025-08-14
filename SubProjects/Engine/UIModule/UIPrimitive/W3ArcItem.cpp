#include "W3ArcItem.h"

#include <QPainter>

CW3ArcItem::CW3ArcItem(QGraphicsItem *parent)
	: QGraphicsEllipseItem(parent) {}

CW3ArcItem::CW3ArcItem(float x, float y, float width, float height, QGraphicsItem *parent)
	: QGraphicsEllipseItem(x, y, width, height, parent) {}

CW3ArcItem::~CW3ArcItem() {}

void CW3ArcItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	painter->setPen(pen());
	painter->drawArc(rect(), startAngle(), spanAngle());
}
