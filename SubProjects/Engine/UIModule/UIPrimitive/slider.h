#pragma once

/**=================================================================================================

Project:		UIPrimitive
File:			slider.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-12-13
Last modify: 	2018-12-13

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include <QSlider>
#include "uiprimitive_global.h"

class UIPRIMITIVE_EXPORT Slider : public QSlider {

public:
	explicit Slider(QWidget *parent = 0);
	~Slider();


	Slider(const Slider&) = delete;
	Slider& operator=(const Slider&) = delete;

protected:
	virtual void mouseMoveEvent(QMouseEvent *ev) override;
	virtual void mousePressEvent(QMouseEvent * ev) override;

	virtual void enterEvent(QEvent * event) override;
	virtual void leaveEvent(QEvent * event) override;
	bool IsUnderHandle(const QPointF& pt) const;
private:
	bool is_hover_handle_ = false;
	QCursor prev_cursor_;
};
