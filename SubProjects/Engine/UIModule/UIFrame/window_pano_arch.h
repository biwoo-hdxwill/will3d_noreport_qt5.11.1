#pragma once

/**=================================================================================================

Project:		UIFrame
File:			window_pano_arch.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-02-12
Last modify: 	2018-02-12

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <QSpinbox>
#include <QDoubleSpinBox>

#include "window.h"

class UIFRAME_EXPORT WindowPanoArch : public Window 
{
	Q_OBJECT

public:
	WindowPanoArch(bool is_maximize = false, QWidget *parent = 0);
	virtual ~WindowPanoArch() override;

	WindowPanoArch(const WindowPanoArch&) = delete;
	WindowPanoArch& operator=(const WindowPanoArch&) = delete;

public:
	void SetRange(const double& range_mm);
	QDoubleSpinBox* GetRange() { return range_.get(); }

private:
	virtual void Initialize() override;

private:
	void InitSpinboxes();

private:
	std::unique_ptr<QDoubleSpinBox> range_;
};
