#pragma once

/**=================================================================================================

Project:		UIFrame
File:			window_tmj_lateral.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-11-28
Last modify: 	2018-11-28

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include "window.h"
#include <QSpinbox>
#include <QDoubleSpinBox>

#include "../../Common/Common/W3Enum.h"
class UIFRAME_EXPORT WindowTmjLateral : public Window
{
	Q_OBJECT
public:
	explicit WindowTmjLateral(const TMJDirectionType& type, QWidget *parent = 0);
	virtual ~WindowTmjLateral() override;

	WindowTmjLateral(const WindowTmjLateral&) = delete;
	WindowTmjLateral& operator=(const WindowTmjLateral&) = delete;

public:
	QDoubleSpinBox* GetInterval() { return  spin_box_.interval.get(); }
	QDoubleSpinBox* GetThickness() { return  spin_box_.thickness.get(); }

  void SetIntervalMinimumValue(const float& value);
	void ApplyPreferences();

private:
	virtual void Initialize() override;

private:
	void InitSpinboxes();

private:
	struct SpinBox {
		std::unique_ptr<QDoubleSpinBox> interval;
		std::unique_ptr<QDoubleSpinBox> thickness;
	};

	SpinBox spin_box_;
	TMJDirectionType type_;
};

