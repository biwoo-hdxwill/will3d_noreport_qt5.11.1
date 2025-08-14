#include "pano_orientatoin_roi_item.h"

#include "W3LineItem.h"


PanoOrientationROIItem::PanoOrientationROIItem() {
	this->setZValue(5);
	this->CreateLines();
}

PanoOrientationROIItem::~PanoOrientationROIItem() {

}

void PanoOrientationROIItem::SetROILine(EROI type, double scene_x1, double scene_x2, double scene_y) {
	lines_[type]->setLine(scene_x1, scene_y, scene_x2, scene_y);
}

void PanoOrientationROIItem::SetMinMax(double min, double max) {
	roi_.min = min;
	roi_.max = max;
}
bool PanoOrientationROIItem::IsHoveredLines() {
	
	for (int i = 0; i < EROI_LINE_END; i++) {
		if (lines_[i]->isHovered())
			return true;
	}

	return false;
}
void PanoOrientationROIItem::CreateLines() {

	for (int i = 0; i < EROI_LINE_END; i++) {
		lines_[i].reset(new CW3LineItem(i, this));
		connect(lines_[i].get(),
				SIGNAL(sigTranslateLine(QPointF, int)), this, SLOT(slotTranslateLineItem(QPointF, int)));
		connect(lines_[i].get(),
				SIGNAL(sigMouseReleased()), this, SIGNAL(sigMouseReleased()));

		lines_[i]->SetFlagHighlight(true);
		lines_[i]->SetFlagMovable(false);
	}

	lines_[EROI_TOP]->setPen(QPen(Qt::red));
	lines_[EROI_BOTTOM]->setPen(QPen(Qt::red));
	lines_[EROI_MID]->setPen(QPen(Qt::yellow));

}
void PanoOrientationROIItem::TransformLine(int line_id, const QTransform & transform) {
	QLineF old_line = lines_[line_id]->line();
	QLineF new_line = QLineF(transform.map(old_line.p1()), transform.map(old_line.p2()));
	lines_[line_id]->setLine(new_line);
}
void PanoOrientationROIItem::slotTranslateLineItem(const QPointF& trans, int id) {
	double y_transed = lines_[id]->line().y1() + trans.y();
	double y_valid_trans;

	double min, max;

	switch ((EROI)id) {
		case EROI_MID:
			min = lines_[EROI_TOP]->line().y1();
			max = lines_[EROI_BOTTOM]->line().y1();
			break;
		case EROI_TOP:
			min = roi_.min;
			max = lines_[EROI_MID]->line().y1();
			break;
		case EROI_BOTTOM:
			min = lines_[EROI_MID]->line().y1();
			max = roi_.max;
			break;

		default:
			break;
	}

	if (y_transed > max)
	{
		y_valid_trans = max - lines_[id]->line().y1();
	}
	else if (y_transed < min)
	{
		y_valid_trans = min - lines_[id]->line().y1();
	}
	else
	{
		y_valid_trans = trans.y();
	}

	QLineF old_line = lines_[id]->line();
	QLineF new_line = QLineF(old_line.x1(), old_line.y1() + y_valid_trans,
							 old_line.x2(), old_line.y2() + y_valid_trans);

	lines_[id]->setLine(new_line);

	emit sigTranslateLine(id, (float)y_valid_trans);
}
