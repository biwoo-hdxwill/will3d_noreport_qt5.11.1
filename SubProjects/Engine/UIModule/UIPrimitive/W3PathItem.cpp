#include "W3PathItem.h"
/*=========================================================================

File:			class CW3PathItem
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-06-17
Modify Date:	2015-09-05
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include <QPainter>
#include <QGraphicsSceneEvent>
#include <QStyleOptionGraphicsItem>

#include "../../Common/Common/define_ui.h"
#include "../../Common/Common/W3Memory.h"

CW3PathItem::CW3PathItem(QGraphicsItem* pParent)
	: QGraphicsPathItem(pParent) {
	this->setFlag(CW3PathItem::ItemIsSelectable, true);
	this->setFlag(CW3PathItem::ItemIsMovable, false);

	m_penLine = QPen(QColor(128, 255, 0),
					 common::ui_define::kToolLineWidth,
					 Qt::SolidLine, Qt::FlatCap);

	this->setPen(m_penLine);
	this->setAcceptHoverEvents(true);
	this->setZValue(5.0);
}

CW3PathItem::CW3PathItem(const QPen& pen, QGraphicsItem* pParent) : 
	QGraphicsPathItem(pParent), m_penLine(pen) {
	this->setFlag(CW3PathItem::ItemIsSelectable, true);
	this->setFlag(CW3PathItem::ItemIsMovable, false);
	this->setPen(m_penLine);
	this->setAcceptHoverEvents(true);
	this->setZValue(5.0);
}

CW3PathItem::~CW3PathItem() {
	SAFE_DELETE_OBJECT(m_path);
}

void CW3PathItem::setHighlighted(const bool edited) {
	m_bHighlighted = edited;

	if (!m_bIsMovable)
		return;

	this->setFlag(CW3PathItem::ItemIsMovable, m_bHighlighted);
}

void CW3PathItem::setOpacityColor(bool bEnable, uchar alpha) {
	QColor color = m_penLine.color();
	if (bEnable) {
		m_penLine.setColor(QColor(color.red(), color.green(), color.blue(), alpha));
	} else {
		m_penLine.setColor(QColor(color.red(), color.green(), color.blue(), color.alpha()));
	}
	this->setPen(m_penLine);
}

void CW3PathItem::drawingPath(const std::vector<QPointF>& points) {
	//this->setPos(0, 0);

	if (points.size() == 0)
		return;

	SAFE_DELETE_OBJECT(m_path);
	m_path = new QPainterPath(points[points.size() - 1]);

	for (int i = points.size() - 1; i >= 0; i--) {
		m_path->lineTo(points[i].x(), points[i].y());
	}
	this->setPath(*m_path);
}

void CW3PathItem::setPen(QPen& pen) {
	m_penLine = pen;
	m_penLine.setCosmetic(true);
	QGraphicsPathItem::setPen(m_penLine);
}

void CW3PathItem::paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget) {
	QStyleOptionGraphicsItem* style
		= const_cast<QStyleOptionGraphicsItem*>(pOption);

	// Remove the HasFocus style state to prevent the dotted line from being drawn.
	style->state &= ~QStyle::State_HasFocus;
	style->state &= ~QStyle::State_Selected;

	pPainter->setRenderHint(QPainter::Antialiasing, m_bAntialiasing);
	QGraphicsPathItem::paint(pPainter, pOption, pWidget);
}

QPainterPath CW3PathItem::shape() const {
	QPainterPath path = this->path();
	QPainterPathStroker stroker;
	//stroker.setWidth(20.0f);
	stroker.setWidth(this->pen().widthF() * 10.0f);
	stroker.setJoinStyle(Qt::PenJoinStyle::BevelJoin);
	return stroker.createStroke(path);
}

void CW3PathItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
	QGraphicsPathItem::hoverEnterEvent(event);

	if (m_bHighlighted && !m_bHovered) {
		QPen hoverPen = QPen(m_penLine.color(), m_penLine.width() + 1, m_penLine.style());
		hoverPen.setCosmetic(true);
		this->setPen(hoverPen);
		m_bHovered = true;
	}

	emit sigHoverPath(true);
}

void CW3PathItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
	QGraphicsPathItem::hoverLeaveEvent(event);

	if (m_bHighlighted && m_bHovered) {
		QPen pen = QPen(m_penLine.color(), m_penLine.width() - 1, m_penLine.style());
		pen.setCosmetic(true);
		this->setPen(pen);
		m_bHovered = false;
	}

	emit sigHoverPath(false);
}

void CW3PathItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsPathItem::mouseMoveEvent(event);

	if (!m_bHighlighted)
		return;

	if (event->buttons() == Qt::LeftButton) {
		//this->setFlag(CW3PathItem::ItemIsMovable, true);
		emit sigTranslatePath(event->scenePos() - event->lastScenePos());
	}
	//if (event->buttons() == (Qt::RightButton | Qt::LeftButton)) {
	//	this->setFlag(CW3PathItem::ItemIsMovable, false);
	//} else if (event->buttons() == Qt::LeftButton) {
	//	this->setFlag(CW3PathItem::ItemIsMovable, true);
	//	emit sigTranslatePath(event->scenePos() - event->lastScenePos());
	//}
}

void CW3PathItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsPathItem::mousePressEvent(event);

	if (!m_bHighlighted)
		return;

	this->setSelected(true);
}

void CW3PathItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsPathItem::mouseReleaseEvent(event);

	emit sigMouseReleased();

	if (!m_bHighlighted)
		return;

	this->setSelected(false);
}

void CW3PathItem::setLineStatic(bool lineStatic) {
	this->setFlag(CW3PathItem::ItemIsSelectable, !lineStatic);
	this->setFlag(CW3PathItem::ItemIsFocusable, !lineStatic);
	this->setAcceptHoverEvents(!lineStatic);
}

void CW3PathItem::transformItem(const QTransform & transform) {
	if (!m_path)
		return;

	for (int i = 0; i < m_path->elementCount(); i++) {
		QPointF pt = m_path->elementAt(i);
		pt = transform.map(pt);
		m_path->setElementPositionAt(i, pt.x(), pt.y());
	}

	this->setPath(*m_path);
}
