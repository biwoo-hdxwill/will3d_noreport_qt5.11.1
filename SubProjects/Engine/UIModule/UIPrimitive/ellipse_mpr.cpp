#include "ellipse_mpr.h"
#include <qpen.h>
#include <qgraphicsscene.h>

EllipseMPR::EllipseMPR(const QPointF& pt, QGraphicsItem* parent) :
	circle_(new CW3EllipseItem(pt)), center_(new CW3EllipseItem(pt)) {

	QPen pen_c(Qt::white);
	pen_c.setCosmetic(true);
	pen_c.setWidthF(2.0f);

	circle_->setZValue(5.0f);
	circle_->setPen(pen_c);
	circle_->setPos(pt);
	circle_->setRect(-10, -10, 20, 20);
	circle_->setAcceptHoverEvents(true);

	QPen pen_pt(Qt::white);
	pen_pt.setCosmetic(true);
	pen_pt.setWidthF(3.0f);

	center_->setZValue(5.0f);
	center_->setPen(pen_pt);
	center_->setPos(pt);
	center_->setRect(-0.5f, -0.5f, 1.0f, 1.0f);
	center_->setAcceptHoverEvents(true);
}

EllipseMPR::~EllipseMPR() {}

void EllipseMPR::SetVisible(bool visibility) {
	circle_->setVisible(visibility);
	center_->setVisible(visibility);
}

void EllipseMPR::SetVisibleCircle(bool visibility){
	circle_->setVisible(visibility);
}
void EllipseMPR::SetVisibleCenter(bool visibility){
	center_->setVisible(visibility);
}

void EllipseMPR::AddToScene(QGraphicsScene* scene) {
	scene->addItem(circle_);
	scene->addItem(center_);
}

void EllipseMPR::InitItems(const QPointF& center, float diameter) {
	circle_->setRect(-diameter / 2.0f, -diameter / 2.0f, diameter, diameter);
	circle_->setPos(center);
	circle_->setVisible(false);

	center_->setPos(center);
	center_->setVisible(false);
}

void EllipseMPR::SetPos(const QPointF& pt) {
	circle_->setPos(pt);
	center_->setPos(pt);
}

QPointF EllipseMPR::Pos() {
	return circle_->pos();
}
