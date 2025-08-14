#include "tmj_arrow_item.h"

#include <qmath.h>
#include <QGraphicsSceneMouseEvent>

#include "../../Common/Common/W3Memory.h"
#include "W3LineItem.h"

TmjArrowItem::TmjArrowItem(QGraphicsItem* parent)
	: QGraphicsItem(parent)
{
	QPen pen(Qt::yellow);
	pen.setWidthF(4.0f);
	QBrush brush(Qt::yellow);

	this->setAcceptHoverEvents(true);
	
	line_.reset(new CW3LineItem(0, this));
	line_->setPen(pen);
	line_->setZValue(1.0f);
	line_->setVisible(false);

	for (int i = 0; i < 2; i++)
	{
		head[i].reset(new QGraphicsPolygonItem(this));
		head[i]->setPen(pen);
		head[i]->setBrush(brush);
		head[i]->setZValue(2.0f);
		head[i]->setVisible(false);
	}

	connect(line_.get(), &CW3LineItem::sigTranslateLine, this, &TmjArrowItem::sigTranslateLine);
}

TmjArrowItem::~TmjArrowItem()
{
}

QRectF TmjArrowItem::boundingRect() const {
	return line_->boundingRect();
}

void TmjArrowItem::SetArrowHead()
{
	double angle = ::acos(line_->line().dx() / line_->line().length());
	if (line_->line().dy() >= 0)
		angle = (M_PI * 2) - angle;

	float arrowSize = 20.0f;
	QPointF arrowP1 = line_->line().p1() + QPointF(sin(angle + M_PI / 3) * arrowSize,
		cos(angle + M_PI / 3) * arrowSize);
	QPointF arrowP2 = line_->line().p1() + QPointF(sin(angle + M_PI - M_PI / 3) * arrowSize,
		cos(angle + M_PI - M_PI / 3) * arrowSize);

	QPolygonF arrowHead;
	arrowHead << line_->line().p1() << arrowP1 << arrowP2;
	head[0]->setPolygon(arrowHead);

	arrowP1 = line_->line().p2() + QPointF(sin(angle - M_PI / 3) * arrowSize,
		cos(angle - M_PI / 3) * arrowSize);
	arrowP2 = line_->line().p2() + QPointF(sin(angle - M_PI + M_PI / 3) * arrowSize,
		cos(angle - M_PI + M_PI / 3) * arrowSize);

	arrowHead.clear();
	arrowHead << line_->line().p2() << arrowP1 << arrowP2;
	head[1]->setPolygon(arrowHead);
}

QPainterPath TmjArrowItem::shape() const {
	QPainterPath path(QPointF(line_->line().x1(), line_->line().y1()));
	path.lineTo(QPointF(line_->line().x2(), line_->line().y2()));
	QPainterPathStroker stroker;
	stroker.setWidth(2.0f);
	stroker.setJoinStyle(Qt::PenJoinStyle::BevelJoin);
	return stroker.createStroke(path);
}

void TmjArrowItem::hoverEnterEvent(QGraphicsSceneHoverEvent * event) {
	QGraphicsItem::hoverEnterEvent(event);
	is_hovered = true;
	emit sigHover(true);
}

void TmjArrowItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * event) {
	QGraphicsItem::hoverLeaveEvent(event);
	is_hovered = false; 
	emit sigHover(false);
}

void TmjArrowItem::TransformItems(const QTransform& transform) {
	QLineF old_line = line_->line();
	QLineF new_line = QLineF(transform.map(old_line.p1()), transform.map(old_line.p2()));
	line_->setLine(new_line);

	SetArrowHead();
}

void TmjArrowItem::SetPen(const QPen & pen) {
	pen_ = pen;
	line_->setPen(pen);
	for (int i = 0; i < 2; i++) {
		if(head[i])
			head[i]->setPen(pen);
	}
}

void TmjArrowItem::SetBrush(const QBrush & brush) {
	brush_ = brush;

	for (int i = 0; i < 2; i++) {
		if (head[i])
			head[i]->setBrush(brush);
	}
}

void TmjArrowItem::SetPos(QPointF pos)
{

}

void TmjArrowItem::SetStart(QPointF pos)
{
	QLineF line = line_->line();
	line.setP1(pos);
	line_->setLine(line);

	SetArrowHead();
}

void TmjArrowItem::SetEnd(QPointF pos)
{
	QLineF line = line_->line();
	line.setP2(pos);
	line_->setLine(line);

	SetArrowHead();
}

void TmjArrowItem::SetVisible(bool visible)
{
	line_->setVisible(visible);
	for (int i = 0; i < 2; i++)
		head[i]->setVisible(visible);
}

void TmjArrowItem::setZValue(qreal z) {
	QGraphicsItem::setZValue(z);
	line_->setZValue(z);
	for (int i = 0; i < 2; i++) {
		head[i]->setZValue(z);
	}
}

QPointF TmjArrowItem::GetStart() { return line_->line().p1(); }
QPointF TmjArrowItem::GetEnd() { return line_->line().p2(); }
bool TmjArrowItem::IsVisible() { return line_->isVisible(); }

bool TmjArrowItem::IsHovered() { return is_hovered; }
