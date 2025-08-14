#pragma once

/**=================================================================================================

Project:		UIFrame
File:			window_pano.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-02-12
Last modify: 	2018-02-12

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/


#include "window.h"
#include <QSpinbox>
#include <QDoubleSpinBox>

class UIFRAME_EXPORT WindowPano : public Window
{
	Q_OBJECT
public:
	explicit WindowPano(QWidget *parent = 0);

	virtual ~WindowPano() override;

	WindowPano(const WindowPano&) = delete;
	WindowPano& operator=(const WindowPano&) = delete;

public:
	QDoubleSpinBox* GetThickness() { return thickness_.get(); }

	void ApplyPreferences();

private:
	virtual void Initialize() override;

private:
	void InitSpinboxes();

	void ApplyArchThicknessIncrements();

private:

	std::unique_ptr<QDoubleSpinBox> thickness_;
};

