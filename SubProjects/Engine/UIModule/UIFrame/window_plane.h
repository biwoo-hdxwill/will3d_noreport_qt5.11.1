#pragma once
/**=================================================================================================

Project:		UIFrame
File:			window_plane.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-03-08
Last modify: 	2018-03-08

Copyright (c) 2018 HDXWILL. All rights reserved.

*===============================================================================================**/
#include <qstring.h>

#include "window.h"
#include "uiframe_global.h"

class UIFRAME_EXPORT WindowPlane : public Window {
	Q_OBJECT

public:
	explicit WindowPlane(const QString& title, QWidget *parent = 0);
	virtual ~WindowPlane();

	WindowPlane(const WindowPlane&) = delete;
	WindowPlane& operator=(const WindowPlane&) = delete;
	
	void AddMaximizeButton();

private:
	void Initialize() override;
};
