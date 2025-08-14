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
class UIFRAME_EXPORT WindowMPR3D : public Window {
	Q_OBJECT
public:
	enum class MPR3DMode { VR, ZOOM3D };

public:
	explicit WindowMPR3D(MPR3DMode mode, QWidget *parent = 0);
	virtual ~WindowMPR3D();

	WindowMPR3D(const WindowMPR3D&) = delete;
	WindowMPR3D& operator=(const WindowMPR3D&) = delete;

signals:
	void sig3DCutReset();
	void sig3DCutUndo();
	void sig3DCutRedo();

public:
	void Set3DCutMode(const bool& on);

private:
	void Initialize() override;

private:
	MPR3DMode mode_;
	std::unique_ptr<QToolButton> reset_ = nullptr;
	std::unique_ptr<QToolButton> undo_ = nullptr;
	std::unique_ptr<QToolButton> redo_ = nullptr;
};
