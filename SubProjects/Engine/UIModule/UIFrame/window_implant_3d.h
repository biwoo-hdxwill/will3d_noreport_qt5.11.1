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
#include <qtoolbutton.h>
#include "window.h"
#include "uiframe_global.h"

class UIFRAME_EXPORT WindowImplant3D : public Window {
	Q_OBJECT

public:
	explicit WindowImplant3D(QWidget *parent = 0);
	virtual ~WindowImplant3D();

	WindowImplant3D(const WindowImplant3D&) = delete;
	WindowImplant3D& operator=(const WindowImplant3D&) = delete;

signals:
	void sigClip3DOnOff(bool clip_on);

public:
	QToolButton * GetClip() { return clip_.get(); }

private slots :
	void slotClip3DOnOff(bool);

private:
	void Initialize() override;

private:
	std::unique_ptr<QToolButton> clip_ = nullptr;
	std::vector<QString> switch_icon_path_ = {
		":/image/viewmenu/toggle_on.png",
		":/image/viewmenu/toggle_off.png"
	};
};
