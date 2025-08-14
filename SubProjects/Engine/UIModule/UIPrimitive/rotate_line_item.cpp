#include "rotate_line_item.h"
#include <QGraphicsSceneEvent>
#include <qmath.h>

RotateLineItem::RotateLineItem() {
	setZValue(5);
	SetFlagHighlight(true);
	SetFlagMovable(true);
	this->setFlag(CW3LineItem::ItemIsMovable, false);
}

RotateLineItem::~RotateLineItem() {}

void RotateLineItem::SetHighlight(const bool & is_highlight) {
	SetFlagHighlight(is_highlight);
	SetFlagMovable(is_highlight);
	this->setFlag(CW3LineItem::ItemIsMovable, false);
}

QPointF RotateLineItem::TransformItems(const QTransform& transform) {
	QLineF old_line = this->line();
	QLineF new_line = QLineF(transform.map(old_line.p1()), transform.map(old_line.p2()));
	setLine(new_line);

	center_position_ = (QPointF)(new_line.p1() + new_line.p2()) * 0.5f;
	line_vector_ = (QVector2D)(new_line.p1() - new_line.p2());

	return center_position_;
}

void RotateLineItem::SetLine(const QPointF & pos, const QVector2D & vector) {
	center_position_ = pos;
	line_vector_ = vector.normalized();
	UpdateLine();
}

void RotateLineItem::UpdateLine() {
	QVector2D nv = line_vector_ * length_*0.5f;

	QPointF p1 = center_position_ + QPointF(nv.x(), nv.y());
	QPointF p2 = center_position_ - QPointF(nv.x(), nv.y());

	setLine(p1.x(), p1.y(), p2.x(), p2.y());
}

QPointF RotateLineItem::p1() const {
	QVector2D nv = line_vector_ * length_*0.5f;
	return center_position_ + QPointF(nv.x(), nv.y());
}

QPointF RotateLineItem::p2() const {
	QVector2D nv = line_vector_ * length_*0.5f;
	return center_position_ - QPointF(nv.x(), nv.y());
}

void RotateLineItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event) {
	QGraphicsLineItem::mouseMoveEvent(event);

	if(event->buttons() == Qt::LeftButton)
		RotateLine(event->pos());
}

void RotateLineItem::mousePressEvent(QGraphicsSceneMouseEvent * event) {
	QGraphicsLineItem::mousePressEvent(event);
	mouse_prev_ = event->pos();
	this->setSelected(true);
}

void RotateLineItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event) {
	QGraphicsLineItem::mouseReleaseEvent(event);
	this->setSelected(false);
}

void RotateLineItem::slotRotateWithHandle(const QPointF& prev_pt, const QPointF& curr_pt) {
	if (!flag_movable())
		return;

	active_ = true;
	QVector2D prev_vec = QVector2D(prev_pt - center_position_).normalized();
	QVector2D curr_vec = QVector2D(curr_pt - center_position_).normalized();
	float delta_angle = std::asin(prev_vec.x()*curr_vec.y() - prev_vec.y()*curr_vec.x())*(180.0f / M_PI);
	mouse_prev_ = curr_pt;
	emit sigRotateLine(delta_angle);
}

void RotateLineItem::RotateLine(const QPointF & mouse_curr) {
	if (!flag_movable())
		return;

	active_ = true;
	QVector2D prev_vec = QVector2D(mouse_prev_ - center_position_).normalized();
	QVector2D curr_vec = QVector2D(mouse_curr - center_position_).normalized();
	float delta_angle = std::asin(prev_vec.x()*curr_vec.y() - prev_vec.y()*curr_vec.x())*(180.0f / M_PI);
	mouse_prev_ = mouse_curr;
	emit sigRotateLine(delta_angle);
}

