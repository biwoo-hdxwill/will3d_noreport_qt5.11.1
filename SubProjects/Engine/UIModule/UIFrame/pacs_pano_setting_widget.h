#pragma once
/*=========================================================================
File:			pacs_pano_setting_widget.h
Language:		C++11
Library:        Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-08-03
Last modify:	2021-10-14

Copyright (c) 2018 HDXWILL. All rights reserved.
=========================================================================*/

#include "base_setting_widget.h"

namespace
{
	enum class PanoSliderID
	{
		FILTER,
		THICKNESS,
		//INTERVAL,
		//EXTENT,
		END
	};
}

class QVBoxLayout;
class UIFRAME_EXPORT PacsPanoSettingWidget : public BaseSettingWidget
{
	Q_OBJECT
public:
	explicit PacsPanoSettingWidget(QWidget* parent = 0);
	virtual ~PacsPanoSettingWidget();

	PacsPanoSettingWidget(const PacsPanoSettingWidget&) = delete;
	const PacsPanoSettingWidget& operator=(const PacsPanoSettingWidget&) = delete;

	void RestoreSliderValue();
	void SetFilterValue(const int filter_level);
	void SetThicknessValue(const int thickness);

	const int GetFilterValue() const;
	const int GetThicknessValue() const;

signals:
	void sigUpdateFilter(const int);
	//void sigUpdateInterval(const int);
	void sigUpdateThickness(const int);

private:
	void SetLayout();
	void Connection();

	QVBoxLayout* CreateSettingLayout();

private:
	int prev_filter_ = 0;
	int prev_thickness_ = 0;
};
