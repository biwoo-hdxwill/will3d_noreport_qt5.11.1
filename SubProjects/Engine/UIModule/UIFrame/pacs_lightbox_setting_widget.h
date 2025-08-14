#pragma once
/*=========================================================================
File:			pacs_lightbox_setting_widget.h
Language:		C++11
Library:        Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-10-14
Last modify:	2021-10-14

Copyright (c) 2018 HDXWILL. All rights reserved.
=========================================================================*/

#include "base_setting_widget.h"

namespace
{
	enum class LightboxSliderID
	{
		FILTER,
		THICKNESS
	};
}

class QVBoxLayout;
class UIFRAME_EXPORT PacsLightboxSettingWidget : public BaseSettingWidget
{
	Q_OBJECT
public:
	explicit PacsLightboxSettingWidget(QWidget* parent = 0);
	virtual ~PacsLightboxSettingWidget();

	PacsLightboxSettingWidget(const PacsLightboxSettingWidget&) = delete;
	const PacsLightboxSettingWidget& operator=(const PacsLightboxSettingWidget&) = delete;

	void SetFilterValue(const int filter_level);
	void SetThicknessValue(const int thickness);

	const int GetFilterValue() const;
	const int GetThicknessValue() const;

private:
	void SetLayout();

	QVBoxLayout* CreateSettingLayout();
};
