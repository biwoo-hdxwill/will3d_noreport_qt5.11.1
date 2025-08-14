#include "W3LineItem.h"
/*=========================================================================

File:			class CW3LineItem
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Libraryqt Item Coordinate Cache
Author:			TaeHoon Yoo
First Date:		2015-08-13
Modify Date:	2015-09-05
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneEvent>
#include <QPainter>
#include <QDebug>

CW3LineItem::CW3LineItem(int nID, QGraphicsItem *parent) :
	id_(nID),
	QGraphicsLineItem(parent) {
	this->setFlag(CW3LineItem::ItemIsSelectable, true);
	this->setFlag(CW3LineItem::ItemIsMovable, false);

	m_penLine = QPen(QColor(0, 255, 0), 1, Qt::SolidLine);
	m_penLine.setCosmetic(true);

	this->setPen(m_penLine);
	this->setAcceptHoverEvents(true);
	this->setZValue(5);
}

CW3LineItem::~CW3LineItem() {
}

//////////////////////////////////////////////////////////////////////////////////////
// public functions
//////////////////////////////////////////////////////////////////////////////////////

void CW3LineItem::SetFlagHighlight(const bool& edited) {
	flag_highlight_ = edited;
}

void CW3LineItem::SetFlagMovable(const bool& movable) {
	flag_movable_ = movable;
	//this->setFlag(CW3LineItem::ItemIsMovable, movable);
}

void CW3LineItem::setPen(const QPen& pen) {
	m_penLine = pen;
	m_penLine.setCosmetic(true);
	QGraphicsLineItem::setPen(m_penLine);
}

void CW3LineItem::SetHighlightEffect(bool is_highlight) {
	if (is_highlight) {
		QPen hoverPen = QPen(m_penLine.color(), m_penLine.width() + 1, m_penLine.style(), m_penLine.capStyle());
		hoverPen.setCosmetic(true);
		this->setPen(hoverPen);
	} else {
		QPen pen = QPen(m_penLine.color(), m_penLine.width() - 1, m_penLine.style(), m_penLine.capStyle());
		pen.setCosmetic(true);
		this->setPen(pen);
	}
}

//////////////////////////////////////////////////////////////////////////////////////
// protected functions
//////////////////////////////////////////////////////////////////////////////////////

void CW3LineItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
	QGraphicsLineItem::hoverEnterEvent(event);

	if (flag_highlight_ && !m_bHovered) {
		this->SetHighlightEffect(true);
		emit sigHighLightEvent(true);
	}

	emit sigHoveredEvent(true);
	m_bHovered = true;
}

void CW3LineItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
	QGraphicsLineItem::hoverLeaveEvent(event);

	if (flag_highlight_ && m_bHovered) {
		this->SetHighlightEffect(false);
		emit sigHighLightEvent(false);
	}
	emit sigHoveredEvent(false);
	m_bHovered = false;
}

void CW3LineItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsLineItem::mouseMoveEvent(event);

#if 1
	QPointF trans = event->scenePos() - event->lastScenePos();
	emit sigTranslateLine(trans, id_);
#else
	QLineF line = this->line();
	float delta_x = event->scenePos().x() - current_pos_.x();
	float translate_x = 0.0f;
	if (abs(delta_x) >= interval_)
	{
		QPointF p1 = line.p1();
		QPointF p2 = line.p2();
		if (delta_x > 0)
		{
			translate_x = interval_;
		}
		else
		{
			translate_x = -interval_;
		}
		p1.setX(p1.x() + translate_x);
		p2.setX(p2.x() + translate_x);
		line.setP1(p1);
		line.setP2(p2);
		
		//setLine(line);
		qDebug() << event->scenePos() - event->lastScenePos();

		current_pos_ = event->scenePos();
		QPointF trans(translate_x, 0.0f);
		emit sigTranslateLine(trans, id_);
	}
	else
	{
		setLine(line);
	}
#endif
}
void CW3LineItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsLineItem::mousePressEvent(event);
	this->setSelected(true);

	current_pos_ = event->scenePos();
}

void CW3LineItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsLineItem::mouseReleaseEvent(event);
	this->setSelected(false);

	emit sigMouseReleased();
}

void CW3LineItem::paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget) {
	QStyleOptionGraphicsItem* style
		= const_cast<QStyleOptionGraphicsItem*>(pOption);

	// Remove the HasFocus style state to prevent the dotted line from being drawn.
	style->state &= ~QStyle::State_HasFocus;
	style->state &= ~QStyle::State_Selected;

	if (m_bAntialiasing)
		pPainter->setRenderHint(QPainter::Antialiasing, true);
	else
		pPainter->setRenderHint(QPainter::Antialiasing, false);

	QGraphicsLineItem::paint(pPainter, pOption, pWidget);
}

QPainterPath CW3LineItem::shape() const {
	QPainterPath path(QPointF(this->line().x1(), this->line().y1()));
	path.lineTo(QPointF(this->line().x2(), this->line().y2()));
	QPainterPathStroker stroker;
	stroker.setWidth(25.0f);
	stroker.setJoinStyle(Qt::PenJoinStyle::BevelJoin);
	return stroker.createStroke(path);
}
