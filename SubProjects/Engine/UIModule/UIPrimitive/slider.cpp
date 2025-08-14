#include "slider.h"
#include <Engine/Common/Common/W3Cursor.h>
#include <qapplication.h>
#include <qstyle.h>
#include <QMouseEvent>
#include <QStyleOptionSlider>

#include <iostream>
Slider::Slider(QWidget* parent) : QSlider(parent) {
  this->setMouseTracking(true);
}

Slider::~Slider() {}


void Slider::mouseMoveEvent(QMouseEvent* ev) {
  QSlider::mouseMoveEvent(ev);
  bool is_hover_handle = IsUnderHandle(ev->localPos());
  if (ev->buttons() != Qt::LeftButton) {
    if (this->orientation() == Qt::Horizontal) {
      if (is_hover_handle_ && !is_hover_handle) {
		QApplication::setOverrideCursor(prev_cursor_);
        is_hover_handle_ = false;
      } else if (!is_hover_handle_ && is_hover_handle) {
        is_hover_handle_ = true;
        QApplication::setOverrideCursor(CW3Cursor::SizeHorCursor());
      }
    } else {
      if (is_hover_handle_ && !is_hover_handle) {
		QApplication::setOverrideCursor(prev_cursor_);
        is_hover_handle_ = false;
      } else if (!is_hover_handle_ && is_hover_handle) {
        is_hover_handle_ = true;
        QApplication::setOverrideCursor(CW3Cursor::SizeVerCursor());
      }
    }
  }
  ev->accept();
}

void Slider::mousePressEvent(QMouseEvent * ev) {
  if (!IsUnderHandle(ev->localPos()))
	return QWidget::mousePressEvent(ev);
  QSlider::mousePressEvent(ev);
}

void Slider::enterEvent(QEvent* event) {
  is_hover_handle_ = false;
  prev_cursor_ = QCursor(*QApplication::overrideCursor());
  QSlider::enterEvent(event);
}

void Slider::leaveEvent(QEvent* event) {
  is_hover_handle_ = false; 
  QApplication::setOverrideCursor(prev_cursor_);
  //QApplication::restoreOverrideCursor();
  QSlider::leaveEvent(event);
}

bool Slider::IsUnderHandle(const QPointF& pt) const {
  QStyleOptionSlider opt;
  initStyleOption(&opt);
  QRect sr = this->style()->subControlRect(QStyle::CC_Slider, &opt,
                                           QStyle::SC_SliderHandle, this);

  return sr.contains((int)pt.x(), (int)pt.y());
}
