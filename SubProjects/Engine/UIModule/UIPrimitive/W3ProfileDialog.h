#pragma once
/*=========================================================================

File:			class CW3ProfileDialog
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			YouSong kim, Seo Seok Man
First Date:		2015-09-17
Modify Date:	2018-05-11
Version:		2.0

Copyright (c) 2015 ~ 2018 All rights reserved by HDXWILL.

=========================================================================*/
#include <vector>
#include "../../Common/Common/W3Dialog.h"

#include "uiprimitive_global.h"

class QTableWidget;
class PlotterModule;
class QToolButton;

class UIPRIMITIVE_EXPORT CW3ProfileDialog : public CW3Dialog
{
	Q_OBJECT
public:
	CW3ProfileDialog(const QString& name, QWidget *parent = 0);
	~CW3ProfileDialog(void);

	void initialize(
		const QPointF& start_pos, 
		const QPointF& end_pos,
		const std::vector<short>& data,
		short min, 
		short max,
		float pixel_pitch,
		float length
	);

	void GetMinMax(short& min, short& max);
	
signals:
	void sigPlotterWasClosed();
	void sigChangeLengthStartPos(const float percent_to_start);
	void sigChangeLengthEndPos(const float percent_to_start);

public slots:
	void slotCaptureBtnPressed();

private slots:
	void slotDisplayInfo(int profile_index, short hu_value);

protected:
	void closeEvent(QCloseEvent *event);


private:
	QToolButton *capture_;
	QToolButton *close_;	
	
	PlotterModule *plotter_;
	QTableWidget* info_display_;

	// curr pos 계산용
	QPointF start_pos_;
	QPointF end_pos_;
	int profile_length_;
};

