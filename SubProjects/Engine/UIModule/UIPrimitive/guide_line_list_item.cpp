#include "guide_line_list_item.h"

#include "W3LineItem.h"

GuideLineListItem::GuideLineListItem(ELINE_TYPE line_type, QGraphicsItem* parent)
	: type_(line_type), LineListItem(parent) {
	this->SetHighlight(false);
	this->SetMovable(false);
}

GuideLineListItem::~GuideLineListItem() {}

void GuideLineListItem::SetRangeScene(const double& min, const double& max) {
	min_scene_ = min;
	max_scene_ = max;
}
void GuideLineListItem::TransformItems(const QTransform& transform) {
	LineListItem::TransformItems(transform);

	switch (type_) {
		case GuideLineListItem::HORIZONTAL:
			min_scene_ = transform.map(QPointF(0.0, min_scene_)).y();
			max_scene_ = transform.map(QPointF(0.0, max_scene_)).y();
			break;
		case GuideLineListItem::VERTICAL:
			min_scene_ = transform.map(QPointF(min_scene_, 0.0)).x();
			max_scene_ = transform.map(QPointF(max_scene_, 0.0)).x();
			break;
		default:
			break;
	}
}

void GuideLineListItem::SetVisibleSelectedLine(const int& line_id, const bool& visible) {
	for (auto& line : lines()) {
		line.second->setVisible(line.first == line_id ? visible : false);
	}
}

void GuideLineListItem::TranslateVerticalLine(const QPointF & trans, const int& id) {
	double x_valid_trans;
	bool is_over = false;
	for (auto& elem : lines()) {
		double x_trans = elem.second->pos().x() + trans.x();
		double x_line = GetLineCenterPosition(elem.first).x();
		double x_coord = x_line + x_trans;

		if (x_coord < min_scene_) {
			x_valid_trans = min_scene_ - x_line - elem.second->pos().x();
			is_over = true;
			break;
		}
		if (x_coord > max_scene_) {
			x_valid_trans = max_scene_ - x_line - elem.second->pos().x();
			is_over = true;
		}
	}

	double translate_x = is_over ? x_valid_trans : trans.x();

	if (flag_movable()) {
		QTransform transform;
		transform.translate(translate_x, 0.0);
		for (auto& elem : lines()) {
			this->TransformLine(elem.first, transform);
		}
	}

	emit sigTranslateLines(QPointF(translate_x, 0.0));
}

void GuideLineListItem::TranslateHorizontalLine(const QPointF & trans, const int& id) {
	double y_valid_trans;
	bool is_over = false;
	for (auto& elem : lines()) {
		double y_trans = elem.second->pos().y() + trans.y();
		double y_line = GetLineCenterPosition(elem.first).y();
		double y_coord = y_line + y_trans;

		if (y_coord < min_scene_) {
			y_valid_trans = min_scene_ - y_line - elem.second->pos().y();
			is_over = true;
			break;
		}
		if (y_coord > max_scene_) {
			y_valid_trans = max_scene_ - y_line - elem.second->pos().y();
			is_over = true;
		}
	}

	double translate_y = is_over ? y_valid_trans : trans.y();

	if (flag_movable()) {
		QTransform transform;
		transform.translate(0.0, translate_y);
		for (auto& elem : lines()) {
			this->TransformLine(elem.first, transform);
		}
	}

	emit sigTranslateLines(QPointF(0.0, translate_y));
}

void GuideLineListItem::SetHighlight(const bool& is_highlight) {
	LineListItem::SetHighlight(is_highlight);
	this->SetMovable(false);
}

void GuideLineListItem::slotTranslateLine(const QPointF& trans, int id) {
	
	if (!flag_highlight())
		return;

	switch (type_) {
		case GuideLineListItem::HORIZONTAL:
			this->TranslateHorizontalLine(trans, id);
			break;
		case GuideLineListItem::VERTICAL:
			this->TranslateVerticalLine(trans, id);
			break;
		default:
			break;
	}
}
