#pragma once
/**=================================================================================================

Project:		UIFrame
File:			window_3d_implant.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-03-02
Last modify: 	2018-03-02

Copyright (c) 2018 HDXWILL. All rights reserved.

*===============================================================================================**/
#include <qspinbox.h>

#include "window.h"
#include "uiframe_global.h"

class UIFRAME_EXPORT WindowImplantSagittal : public Window {
	Q_OBJECT

public:
	explicit WindowImplantSagittal(QWidget *parent = 0);
	virtual ~WindowImplantSagittal();

	WindowImplantSagittal(const WindowImplantSagittal&) = delete;
	WindowImplantSagittal& operator=(const WindowImplantSagittal&) = delete;

public:
	QDoubleSpinBox * GetRotate() { return rotate_.get(); }

private:
	void Initialize() override;

private:
	std::unique_ptr<QDoubleSpinBox> rotate_ = nullptr;
};
