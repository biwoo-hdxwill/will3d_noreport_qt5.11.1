#pragma once
/*=========================================================================
File:			base_setting_widget.h
Language:		C++11
Library:        Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-10-13
Last modify:	2021-10-13

Copyright (c) 2018 HDXWILL. All rights reserved.
=========================================================================*/

#include <map>

#include <QWidget>

#include "uiframe_global.h"

namespace
{
	const QMargins kMarginZero(0, 0, 0, 0);
}

class QVBoxLayout;
class QSlider;
class UIFRAME_EXPORT BaseSettingWidget : public QWidget
{
	Q_OBJECT
public:
	explicit BaseSettingWidget(QWidget* parent = 0);
	virtual ~BaseSettingWidget() = 0;

	BaseSettingWidget(const BaseSettingWidget&) = delete;
	const BaseSettingWidget& operator=(const BaseSettingWidget&) = delete;

signals:
	void sigSliderUpdate(const int id, const int value);

protected:
	QVBoxLayout* CreateSliderLayout(const int id, const QString& name, const int min, const int max, const int init_value = 0);
	const int GetSliderValue(const int id) const;
	void SetSliderValue(const int id, const int value, bool signal = false);

	inline QVBoxLayout* main_layout() { return main_layout_; }

private:
	void Initialize();
	void ClearMap();
	bool CheckSlider(const int id) const;

private:
	QVBoxLayout* main_layout_ = nullptr;

	std::map<int, QSlider*> slider_map_;
};
