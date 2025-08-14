#include "W3EllipseItem.h"
/*=========================================================================

File:			class CW3EllipseItem
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-06-17
Modify Date:	2016-05-31
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include <qmath.h>
#include <qvector2d.h>
#include <QGraphicsSceneEvent>
#include <QStyleOptionGraphicsItem>

CW3EllipseItem::CW3EllipseItem(QGraphicsItem* parent)
	: QGraphicsEllipseItem(parent) {
	this->Initialize();
}
CW3EllipseItem::CW3EllipseItem(const QPointF& point, QGraphicsItem *parent)
	: QGraphicsEllipseItem(parent) {
	setPos(point);
	this->Initialize();
}

/*
Implant handle의 rotation을 위해 만든 생성자.
InteractionType을 ROTATE로 적용하여 sigTranslateEllipse 대신
sigRotateEllipse를 내보내게 한다.
*/
CW3EllipseItem::CW3EllipseItem(const QPointF& point, const QRectF &rect,
							   const QRectF& hover_rect, QGraphicsItem *parent)
	: QGraphicsEllipseItem(parent) {
	setPos(point);

	this->Initialize();

	rect_normal_ = rect;
	rect_hover_ = hover_rect;
	setRect(rect);

	interaction_type_ = InteractionType::ROTATE;
}

////////////////////////////////////////////////////////////////////////////////////
// public functions
////////////////////////////////////////////////////////////////////////////////////

void CW3EllipseItem::SetFlagHighlight(const bool& edited) {
	flag_highlight_ = edited;
}

void CW3EllipseItem::SetFlagMovable(const bool& movable) {
	flag_movable_ = movable;
	this->setFlag(CW3EllipseItem::ItemIsMovable, movable);
}

void CW3EllipseItem::SetHighlight(bool is_highlight) {
	if (is_highlight) {
		QGraphicsEllipseItem::setPen(pen_hover_);
		this->setRect(rect_hover_);
		is_highlight_ = true;
	} else {
		QGraphicsEllipseItem::setPen(pen_normal_);
		this->setRect(rect_normal_);
		is_highlight_ = false;
	}
}

void CW3EllipseItem::setPenColor(const QColor& color) {
	QPen& p = this->pen();
	p.setColor(color);
	this->setPen(p);
}

void CW3EllipseItem::setBrushColor(const QColor& color) {
	QBrush& b = this->brush();
	b.setColor(color);
	this->setBrush(b);
}

void CW3EllipseItem::setVisible(bool is_visible) {
	if (is_hovered_) {
		if (flag_highlight_) {
			this->SetHighlight(false);
		}

		emit sigHoverEllipse(false);
		is_hovered_ = false;
	}
	QGraphicsEllipseItem::setVisible(is_visible);
}

void CW3EllipseItem::SetToRotateHandle() {
	interaction_type_ = InteractionType::ROTATE_HANDLE;
}

void CW3EllipseItem::setPen(const QPen& pen) {
	pen_normal_ = pen_hover_ = pen;
	QGraphicsEllipseItem::setPen(pen_normal_);
}

void CW3EllipseItem::setHoveredPen(const QPen& pen) {
	pen_hover_ = pen;
	QGraphicsEllipseItem::setPen(pen_hover_);
}

void CW3EllipseItem::setPens(const QPen & normal, const QPen & hovered) {
  pen_normal_ = normal;
  pen_hover_ = hovered;
  QGraphicsEllipseItem::setPen(pen_normal_);
}

////////////////////////////////////////////////////////////////////////////////////
// protected functions
////////////////////////////////////////////////////////////////////////////////////
void CW3EllipseItem::paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget) {
	QStyleOptionGraphicsItem* style = const_cast<QStyleOptionGraphicsItem*>(pOption);

	// Remove the HasFocus style state to prevent the dotted line from being drawn.
	style->state &= ~QStyle::State_HasFocus;
	style->state &= ~QStyle::State_Selected;

	QGraphicsEllipseItem::paint(pPainter, pOption, pWidget);
}

void CW3EllipseItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
	QGraphicsEllipseItem::hoverEnterEvent(event);

	if (flag_highlight_ && !is_hovered_) {
		this->SetHighlight(true);
	}
	emit sigHoverEllipse(true);
	is_hovered_ = true;
}

void CW3EllipseItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
	QGraphicsEllipseItem::hoverLeaveEvent(event);

	if (flag_highlight_ && is_hovered_) {
		this->SetHighlight(false);
	}

	emit sigHoverEllipse(false);
	is_hovered_ = false;
}

void CW3EllipseItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsEllipseItem::mouseMoveEvent(event);

	if (event->buttons() == Qt::LeftButton) {
		m_bLeftClicked = true;

		if (interaction_type_ == InteractionType::TRANSLATE) {
			QPointF trans = event->scenePos() - event->lastScenePos();
			emit sigTranslateEllipse(trans);
		} else if(interaction_type_ == InteractionType::ROTATE) {
			QPointF pt_org = mapToScene(pos());
			QVector2D prev_vec = QVector2D(event->lastScenePos() - pt_org).normalized();
			QVector2D curr_vec = QVector2D(event->scenePos() - pt_org).normalized();
			float delta_angle = -std::asin(prev_vec.x()*curr_vec.y() -
										   prev_vec.y()*curr_vec.x())*(180.0f / M_PI);
			emit sigRotateEllipse(delta_angle);
		} else if (interaction_type_ == InteractionType::ROTATE_HANDLE) {
			emit sigRotateWithHandle(event->lastScenePos(), event->scenePos());
		}
	}
}

void CW3EllipseItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsEllipseItem::mouseReleaseEvent(event);

	this->setSelected(true);
	emit sigMousePressed();
}

void CW3EllipseItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsEllipseItem::mouseReleaseEvent(event);

	this->setSelected(false);
	if (m_bLeftClicked) {
		m_bLeftClicked = false;
		emit sigTranslatedEllipse(event->scenePos());
	}
	emit sigMouseReleased();
}

QPainterPath CW3EllipseItem::shape() const {
	QPainterPath path;
	path.addEllipse(rect_normal_.adjusted(-3, -3, 6, 6));
	return path;
}

////////////////////////////////////////////////////////////////////////////////////
// private functions
////////////////////////////////////////////////////////////////////////////////////

void CW3EllipseItem::Initialize() {
	setFlag(CW3EllipseItem::ItemIsSelectable, true);
	setFlag(CW3EllipseItem::ItemIsMovable, false);
	setAcceptHoverEvents(true); 
	setZValue(11);

	QRectF curr_rect(-3, -3, 6, 6);
	rect_normal_ = curr_rect;
	rect_hover_ = curr_rect.adjusted(-1.5, -1.5, 3, 3);
	setRect(curr_rect);

	pen_normal_ = pen_hover_ = QPen(QColor(128, 255, 0), 2, Qt::SolidLine);
	setPen(pen_normal_);
	this->pen().setCosmetic(true);
}
