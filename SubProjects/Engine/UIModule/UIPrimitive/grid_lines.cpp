#include "grid_lines.h"
#include <qpen.h>
#include "../../Common/Common/W3Memory.h"
#include "W3LineItem.h"

namespace {
const QColor kGridLineColor(127, 127, 127, 127);
} // end of namespace

GridLines::GridLines(QObject* parent) : QObject(parent) {
	this->setFlag(QGraphicsItem::ItemIsSelectable, false);
	this->setFlag(QGraphicsItem::ItemIsMovable, false);
	this->setZValue(0);
}

GridLines::~GridLines() {}

void GridLines::SetGrid(float view_width, float view_height,
						float width_length, float height_length,
						const QPointF & view_center_in_scene,
						float grid_step_size_mm) {
	if ((int)width_length == 0 ||
		(int)height_length == 0)
		return;

	view_width_in_scene_ = view_width;
	view_height_in_scene_ = view_height;
	view_width_length_ = width_length;
	view_height_length_ = height_length;
	view_center_in_scene_ = view_center_in_scene;
	grid_step_size_mm_ = grid_step_size_mm;

	SetGridLines();
}

//void GridLines::DisplayGrid(bool visible_status) {
//	bool is_display = grid_visible_ & visible_status;
//
//	QGraphicsItem::setVisible(is_display);
//
//	if (w_lines_.empty() || h_lines_.empty())
//		return;
//
//	if (is_display == w_lines_[0]->isVisible() ||
//		is_display == h_lines_[0]->isVisible())
//		return;
//
//	for (auto &i : w_lines_)
//		i->setVisible(is_display);
//
//	for (auto &i : h_lines_)
//		i->setVisible(is_display);
//}

void GridLines::SetGridSpacing(float spacing) {
	grid_step_size_mm_ = spacing;
	SetGridLines();
}

void GridLines::ClearGridLines() {
	for (auto &i : w_lines_)
		SAFE_DELETE_OBJECT(i);

	for (auto &i : h_lines_)
		SAFE_DELETE_OBJECT(i);

	w_lines_.clear();
	h_lines_.clear();
}

void GridLines::SetGridLines() {
	ClearGridLines();

	QPen pen(kGridLineColor);
	pen.setCosmetic(true);

	float lineGap = view_width_in_scene_ / view_width_length_ * grid_step_size_mm_;
	int stepX = view_width_in_scene_ / lineGap;
	int stepY = view_height_in_scene_ / lineGap;

	float widthX = view_center_in_scene_.x() - view_width_in_scene_ * 0.5f;
	float widthY = view_center_in_scene_.y() - view_height_in_scene_ * 0.5f;
	for (int i = 0; i <= stepX; i++) {
		CW3LineItem *line = new CW3LineItem(0, this);
		line->setLine(widthX, widthY,
					  widthX, view_height_in_scene_);
		line->setAntialiasing(false);
		line->setFlag(QGraphicsLineItem::ItemIgnoresTransformations, true);
		line->setPen(pen);
		line->setZValue(0);
		w_lines_.push_back(line);

		widthX += lineGap;
	}

	float heightX = view_center_in_scene_.x() - view_width_in_scene_ * 0.5f;
	float heightY = view_center_in_scene_.y() - view_height_in_scene_ * 0.5f;
	for (int i = 0; i <= stepY; i++) {
		CW3LineItem *line = new CW3LineItem(0, this);
		line->setLine(heightX, heightY,
					  view_width_in_scene_, heightY);
		line->setAntialiasing(false);
		line->setFlag(QGraphicsLineItem::ItemIgnoresTransformations, true);
		line->setPen(pen);
		line->setZValue(0);
		h_lines_.push_back(line);

		heightY += lineGap;
	}
}
