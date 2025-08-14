#pragma once
/*=========================================================================
File:			pacs_3d_setting_widget.h
Language:		C++11
Library:        Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-07-21
Last modify:	2021-09-30

Copyright (c) 2018 HDXWILL. All rights reserved.
=========================================================================*/

#include <QWidget>

#include "uiframe_global.h"

class QVBoxLayout;
class QButtonGroup;
class UIFRAME_EXPORT Pacs3DSettingWidget : public QWidget
{
	Q_OBJECT
	enum class RotationType { horizontal, vertical };
	enum class RotationDir { anterior, posterior };

public:
	explicit Pacs3DSettingWidget(QWidget* parent = 0);
	virtual ~Pacs3DSettingWidget();

	Pacs3DSettingWidget(const Pacs3DSettingWidget&) = delete;
	const Pacs3DSettingWidget& operator=(const Pacs3DSettingWidget&) = delete;

signals:
	void sigRotationTypeChange();
	void sigRotationDirChange();
	void sigUpdateAngle(const int angle);			//degree

private slots:
	void slotRotationTypeChange(const int button_id);
	void slotRotationDirChange(const int button_id);

private:
	void Initialize();
	void SetLayout();

	QVBoxLayout* CreateVRSettingLayout();

private:
	QVBoxLayout* main_layout_ = nullptr;

	QButtonGroup* rotation_type_button_group_ = nullptr;
	QButtonGroup* rotation_dir_button_group_ = nullptr;

	RotationType rot_type = RotationType::horizontal;
	RotationDir rot_dir = RotationDir::anterior;
};
