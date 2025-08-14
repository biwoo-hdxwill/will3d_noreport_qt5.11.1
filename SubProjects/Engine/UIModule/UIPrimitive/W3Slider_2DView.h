#pragma once
/*=========================================================================

File:			class CW3Slider_2DView
Language:		C++11
Library:        Qt 5.4.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2016-11-17
Modify Date:	2016-11-17
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include "slider.h"
#include "uiprimitive_global.h"

class UIPRIMITIVE_EXPORT CW3Slider_2DView : public Slider {
	Q_OBJECT

public:
	CW3Slider_2DView(QWidget *parent = 0);
	~CW3Slider_2DView();

public:
	inline bool pressed() const noexcept { return pressed_; }
	inline bool hovered() const noexcept { return hovered_; }

private:
	virtual void mousePressEvent(QMouseEvent *ev) override;
	virtual void mouseReleaseEvent(QMouseEvent *ev) override;
	virtual void enterEvent(QEvent *event) override;
	virtual void leaveEvent(QEvent *event) override;

private:
	bool pressed_ = false;
	bool hovered_ = false;
};
