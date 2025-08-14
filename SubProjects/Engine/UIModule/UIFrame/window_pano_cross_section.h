#pragma once

/**=================================================================================================

Project:		UIFrame
File:			window_pano_cross_section.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-01-02
Last modify: 	2018-01-02

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include "window.h"
#include <QSpinbox>
#include <QDoubleSpinBox>

class UIFRAME_EXPORT WindowPanoCrossSection : public Window
{
	Q_OBJECT
public:
	explicit WindowPanoCrossSection(QWidget *parent = 0);

	virtual ~WindowPanoCrossSection() override;

	WindowPanoCrossSection(const WindowPanoCrossSection&) = delete;
	WindowPanoCrossSection& operator=(const WindowPanoCrossSection&) = delete;

public:
	inline QDoubleSpinBox* GetRotate() { return spin_box_.rotate.get(); }
	inline QDoubleSpinBox* GetInterval() { return spin_box_.interval.get(); }
	inline QDoubleSpinBox* GetThickness() { return spin_box_.thickness.get(); }

	inline int GetRotateValue() { return spin_box_.rotate.get()->value(); }
	inline int GetIntervalValue() { return spin_box_.interval.get()->value(); }
	inline int GetThicknessValue() { return spin_box_.thickness.get()->value(); }

	void ApplyPreferences();
	void SetIntervalMinimumValue(const float& value);

private:
	virtual void Initialize() override;

private:
	void InitSpinboxes();


	void ApplyIntervalIncrements();
	void ApplyThicknessIncrements();

private:
	struct SpinBox {
		std::unique_ptr<QDoubleSpinBox> rotate;
		std::unique_ptr<QDoubleSpinBox> interval;
		std::unique_ptr<QDoubleSpinBox> thickness;
	};

	SpinBox spin_box_;
};

