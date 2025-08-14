#include "W3RectItem.h"

/*=========================================================================

File:			class CW3RectItem
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-11-17
Modify Date:	2015-11-17
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneEvent>
#include <QPainter>

CW3RectItem::CW3RectItem(int nID, QGraphicsItem* parent)
	: m_nID(nID), QGraphicsRectItem(parent) {
	this->setFlag(CW3RectItem::ItemIsSelectable, true);
	this->setFlag(CW3RectItem::ItemIsMovable, false);

	m_penRect = QPen(QColor(0, 255, 0), 2, Qt::SolidLine);
	m_penRect.setCosmetic(true);

	this->setPen(m_penRect);
	this->setAcceptHoverEvents(true);
	this->setZValue(2.0f);

	m_bHighlighted = false;
	m_bHovered = false;
	m_bAntialiasing = true;
}

CW3RectItem::~CW3RectItem() {
}

//////////////////////////////////////////////////////////////////////////////////////
// public functions
//////////////////////////////////////////////////////////////////////////////////////

void CW3RectItem::TransformItems(const QTransform & transform) {
	QPointF pt_left_top = transform.map(this->rect().topLeft());
	QPointF vec_rect = transform.map(this->rect().bottomRight()) - pt_left_top;
	this->setPos(0.0, 0.0);
	this->setRect(pt_left_top.x(), pt_left_top.y(),
								vec_rect.x(),
								vec_rect.y());
}

void CW3RectItem::setHighlighted(const bool edited) {
	m_bHighlighted = edited;

	/*if (m_bHighlighted)
		this->setFlag(CW3RectItem::ItemIsMovable, true);
	else
		this->setFlag(CW3RectItem::ItemIsMovable, false);*/
}

void CW3RectItem::setPen(const QPen& pen) {
	m_penRect = pen;
	m_penRect.setCosmetic(true);
	QGraphicsRectItem::setPen(m_penRect);
}

//////////////////////////////////////////////////////////////////////////////////////
// protected functions
//////////////////////////////////////////////////////////////////////////////////////

void CW3RectItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
	QGraphicsRectItem::hoverEnterEvent(event);

	if (m_bHighlighted && !m_bHovered) {
		QPen hoverPen = QPen(m_penRect.color(), m_penRect.width() + 1, m_penRect.style());
		hoverPen.setCosmetic(true);
		this->setPen(hoverPen);
	}
	m_bHovered = true;
}

void CW3RectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
	QGraphicsRectItem::hoverLeaveEvent(event);

	if (m_bHighlighted && m_bHovered) {
		QPen pen = QPen(m_penRect.color(), m_penRect.width() - 1, m_penRect.style());
		pen.setCosmetic(true);
		this->setPen(pen);
	}
	m_bHovered = false;
}

void CW3RectItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsRectItem::mouseMoveEvent(event);

	QPointF trans = event->scenePos() - event->lastScenePos();

	if (event->buttons() == Qt::LeftButton)
		emit sigTranslateRect(trans, m_nID);
}
void CW3RectItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsRectItem::mousePressEvent(event);
	this->setSelected(true);
}

void CW3RectItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsRectItem::mouseReleaseEvent(event);
	this->setSelected(false);
}

void CW3RectItem::paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget) {
	QStyleOptionGraphicsItem* style
		= const_cast<QStyleOptionGraphicsItem*>(pOption);

	// Remove the HasFocus style state to prevent the dotted Rect from being drawn.
	style->state &= ~QStyle::State_HasFocus;
	style->state &= ~QStyle::State_Selected;

	if (m_bAntialiasing)
		pPainter->setRenderHint(QPainter::Antialiasing, true);
	else
		pPainter->setRenderHint(QPainter::Antialiasing, false);

	QGraphicsRectItem::paint(pPainter, pOption, pWidget);
}
