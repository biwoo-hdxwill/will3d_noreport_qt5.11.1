#pragma once
/*=========================================================================
File:			pacs_pano_view_widget.h
Language:		C++11
Library:        Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-07-29
Last modify:	2021-08-20

Copyright (c) 2018 HDXWILL. All rights reserved.
=========================================================================*/

#include <QWidget>

#include "uiframe_global.h"

class QVBoxLayout;
class QLabel;

class PACSViewPano;
class CW3DicomLoaderScrollbar;
class UIFRAME_EXPORT PacsPanoViewWidget : public QWidget
{
	Q_OBJECT
public:
	explicit PacsPanoViewWidget(QWidget* parent = 0);
	virtual ~PacsPanoViewWidget();

	PacsPanoViewWidget(const PacsPanoViewWidget&) = delete;
	const PacsPanoViewWidget& operator=(const PacsPanoViewWidget&) = delete;
	
	void PanoUpdate();
	void InitPanoView(bool nerve, bool implant);	
	void EmitCreateDCMFile();
	
signals:
	void sigPanoUpdate(int value);
	void sigCreateDCMFiles_ushort(unsigned short* data, const QString& path, const int instance_number, const int rows, const int columns);
	void sigCreateDCMFiles_uchar(unsigned char* data, const QString& path, const int instance_number, const int rows, const int columns);

public slots:
	void slotNerveVisibility(bool is_visible);
	void slotImplantVisibility(bool is_visible);
	void slotUpdateFilter(const int filter);

private slots:
	void slotSliderValueChange(int index, float value);

private:
	void Initialize();
	void SetLayout();
	void Connections();

	QVBoxLayout* CreateContentsLayout();

	void SetTextImageCntLabel();

private:
	QVBoxLayout* main_layout_ = nullptr;
	PACSViewPano* pacs_view_pano_[3] = { nullptr, };
	CW3DicomLoaderScrollbar* slider_ = nullptr;
	QLabel* label_image_cnt_ = nullptr;

	int prev_value_[3] = { -1, -1, -1 };
};
