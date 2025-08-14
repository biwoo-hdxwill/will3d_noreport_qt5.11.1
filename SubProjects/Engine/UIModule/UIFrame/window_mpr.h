#pragma once
/**=================================================================================================

Project:		UIFrame
File:			window_mpr.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-03-02
Last modify: 	2018-03-02

Copyright (c) 2018 HDXWILL. All rights reserved.

*===============================================================================================**/
#include <QDoubleSpinBox>

#include <Engine/Common/Common/W3Enum.h>
#include "window.h"
#include "uiframe_global.h"

class UIFRAME_EXPORT WindowMPR : public Window 
{
	Q_OBJECT

public:
	explicit WindowMPR(const MPRViewType& view_type, QWidget* parent = 0);
	virtual ~WindowMPR();

	WindowMPR(const WindowMPR&) = delete;
	WindowMPR& operator=(const WindowMPR&) = delete;

signals:
	void sigChangeThickness(double);
	void sigChangeInterval(double);
	void sigLightboxOn(int row, int col, float interval, float thickness);

public:
	void SyncThicknessValue(const float thickness);
	void InitInterval(const float interval, const float minimum);

private slots:
	virtual void slotSelectLayout(int row, int column) override;
	void slotIntervalChanged(double slider_value);

private:
	virtual void Initialize() override;

private:
	std::unique_ptr<QDoubleSpinBox> thickness_;
	std::unique_ptr<QDoubleSpinBox> interval_;
};
