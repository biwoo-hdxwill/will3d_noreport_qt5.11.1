#pragma once
/*=========================================================================
File:			pacs_3d_view_widget.h
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
class QLabel;

class PACSView3D;
class CW3DicomLoaderScrollbar;

class UIFRAME_EXPORT Pacs3DViewWidget : public QWidget
{
	Q_OBJECT
public:
	explicit Pacs3DViewWidget(QWidget* parent = 0);
	virtual ~Pacs3DViewWidget();

	Pacs3DViewWidget(const Pacs3DViewWidget&) = delete;
	const Pacs3DViewWidget& operator=(const Pacs3DViewWidget&) = delete;

	void EmitCreateDCMFile();

signals:
	void sigCreateDCMFiles_uchar(unsigned char* data, const QString& path, const int instance_number, const int rows, const int columns);

	public slots:
	void slotRotationTypeChange();
	void slotRotationDirChange();
	void slotNerveVisibility(bool is_visible);
	void slotImplantVisibility(bool is_visible);
	void slotUpdateAngle(const int angle);			//degree

	private slots:
	void slotSliderValueChange(int index, float value);

private:
	void Initialize();
	void SetLayout();
	void Connections();

	QVBoxLayout* CreateMPRContentsLayout();

	void SetRotationSliderRange();
	void SetTextImageCntLabel();
	void UpdateView(const int index, const int value);

private:
	QVBoxLayout* main_layout_ = nullptr;
	PACSView3D* pacs_view_3d_[3] = { nullptr, };
	CW3DicomLoaderScrollbar* slider_ = nullptr;
	QLabel* label_image_cnt_ = nullptr;

	bool is_rot_horizontal_ = true;
	bool is_dir_anterior_ = true;

	int angle_ = 1;
};
