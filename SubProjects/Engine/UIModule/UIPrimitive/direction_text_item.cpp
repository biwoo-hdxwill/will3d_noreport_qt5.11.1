#include "direction_text_item.h"
#include <QApplication>
DirectionTextItem::DirectionTextItem() {
	QFont font = QApplication::font();
	font.setPixelSize(30);

	this->setTextColor(QColor(127, 127, 127));
	this->setFont(font);
	this->setHoverEnabled(false);
}

DirectionTextItem::~DirectionTextItem() {
}

void DirectionTextItem::setVisible(bool visible) {
	CW3TextItem::setVisible(visible);
}

void DirectionTextItem::SetText(const QString & text, bool is_align_left) {
	this->setPlainText(text);
	is_align_left_ = is_align_left;
}

void DirectionTextItem::SetSceneSize(const double & scene_width, const double & scene_height) {
	scene_width_ = scene_width;
	scene_height_ = scene_height;
}

void DirectionTextItem::UpdatePosition() {
	float d = scene_width_*0.05;

	if (is_align_left_)
		this->setPos(d, scene_height_*0.5 - this->boundingRect().height()*0.5);
	else
		this->setPos(scene_width_ - d - this->boundingRect().width(),
					 scene_height_*0.5 - this->boundingRect().height()*0.5);
}
