#include "W3Slider_2DView.h"
#include <qstyle.h>
#include <QStyleOptionSlider>
#include <QMouseEvent>
CW3Slider_2DView::CW3Slider_2DView(QWidget *parent)
	: Slider(parent) {
	this->setAttribute(Qt::WA_TranslucentBackground);
	this->setObjectName("2DView");
}

CW3Slider_2DView::~CW3Slider_2DView() {
}

void CW3Slider_2DView::mousePressEvent(QMouseEvent * ev) {
	Slider::mousePressEvent(ev);
	pressed_ = true;
}

void CW3Slider_2DView::mouseReleaseEvent(QMouseEvent * ev) {
	Slider::mouseReleaseEvent(ev);
	pressed_ = false;
}

void CW3Slider_2DView::enterEvent(QEvent * event) {
	hovered_ = true;
	Slider::enterEvent(event);
}

void CW3Slider_2DView::leaveEvent(QEvent * event) {
	hovered_ = false;
	Slider::leaveEvent(event);
}
