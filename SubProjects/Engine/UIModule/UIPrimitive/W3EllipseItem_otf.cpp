#include "W3EllipseItem_otf.h"
#include <qpen.h>

CW3EllipseItem_otf::CW3EllipseItem_otf(const QPointF& point)
	: CW3EllipseItem(point) {
	QPen pen(Qt::green);
	pen.setCosmetic(true);
	QBrush brush(Qt::green);

	this->setAcceptHoverEvents(true);
	this->setAcceptedMouseButtons(Qt::MouseButton::LeftButton);
	this->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	this->setZValue(10.0f);
	this->setParentItem(nullptr);
	this->setPen(pen);
	this->setBrush(brush);
	this->setPos(point);
	this->setRect(-5, -5, 10, 10);

	m_bIsHover = false;
}

void CW3EllipseItem_otf::activate(void) {
	this->setRect(-7, -7, 14, 14);
}

bool CW3EllipseItem_otf::contains(const QPointF& point) {
	return m_bIsHover;
}

void CW3EllipseItem_otf::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
	QGraphicsEllipseItem::hoverEnterEvent(event);
	this->setRect(-7, -7, 14, 14);
	m_bIsHover = true;
}

void CW3EllipseItem_otf::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
	QGraphicsEllipseItem::hoverLeaveEvent(event);
	this->setRect(-5, -5, 10, 10);
	m_bIsHover = false;
}

void CW3EllipseItem_otf::deactivate(void) {
	this->setRect(-5, -5, 10, 10);
}
