#include "polygon_item.h"

#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneEvent>
#include <QPainter>

PolygonItem::PolygonItem(QGraphicsItem* parent)
	: QGraphicsPolygonItem(parent) {
	this->setFlag(PolygonItem::ItemIsSelectable, true);
	this->setFlag(PolygonItem::ItemIsMovable, false);

	pen_ = QPen(QColor(0, 255, 0), 2, Qt::SolidLine);
	pen_.setCosmetic(true);

	this->setPen(pen_);
	this->setAcceptHoverEvents(true);
	this->setZValue(2.0f);

	is_highlighted_ = false;
	hovered_ = false;
	is_antialiasing_ = true;
}

PolygonItem::~PolygonItem() {
}

//////////////////////////////////////////////////////////////////////////////////////
// public functions
//////////////////////////////////////////////////////////////////////////////////////

void PolygonItem::TransformItems(const QTransform & transform) {	
	this->setPolygon(transform.map(this->polygon()));
}

void PolygonItem::SetHighlighted(const bool edited) {
	is_highlighted_ = edited;
}

void PolygonItem::SetPen(const QPen& pen) {
	pen_ = pen;
	pen_.setCosmetic(true);
	QGraphicsPolygonItem::setPen(pen_);
}

//////////////////////////////////////////////////////////////////////////////////////
// protected functions
//////////////////////////////////////////////////////////////////////////////////////

void PolygonItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
	QGraphicsPolygonItem::hoverEnterEvent(event);

	if (is_highlighted_ && !hovered_) {
		QPen hoverPen = QPen(pen_.color(), pen_.width() + 1, pen_.style());
		hoverPen.setCosmetic(true);
		this->setPen(hoverPen);
	}
	hovered_ = true;
}

void PolygonItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
	QGraphicsPolygonItem::hoverLeaveEvent(event);

	if (is_highlighted_ && hovered_) {
		QPen pen = QPen(pen_.color(), pen_.width() - 1, pen_.style());
		pen.setCosmetic(true);
		this->setPen(pen);
	}
	hovered_ = false;
}

void PolygonItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsPolygonItem::mouseMoveEvent(event);

	QPointF trans = event->scenePos() - event->lastScenePos();

	if (event->buttons() == Qt::LeftButton)
		emit sigTranslateRect(trans);
}
void PolygonItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsPolygonItem::mousePressEvent(event);
	this->setSelected(true);
}

void PolygonItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsPolygonItem::mouseReleaseEvent(event);
	this->setSelected(false);
}

void PolygonItem::paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget) {
	QStyleOptionGraphicsItem* style
		= const_cast<QStyleOptionGraphicsItem*>(pOption);

	// Remove the HasFocus style state to prevent the dotted Rect from being drawn.
	style->state &= ~QStyle::State_HasFocus;
	style->state &= ~QStyle::State_Selected;

	if (is_antialiasing_)
		pPainter->setRenderHint(QPainter::Antialiasing, true);
	else
		pPainter->setRenderHint(QPainter::Antialiasing, false);

	QGraphicsPolygonItem::paint(pPainter, pOption, pWidget);
}
