#pragma once
/*=========================================================================

File:			class CW3PlotterModule
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			YouSong kim, Seo Seok Man
First Date:		2015-09-17
Modify Date:	2018-05-11
Version:		2.0

Copyright (c) 2015 ~ 2018 All rights reserved by HDXWILL.

=========================================================================*/
#include <vector>
#include <qwidget.h>
#include "uiprimitive_global.h"

class QLineF;

class UIPRIMITIVE_EXPORT PlotterModule : public QWidget
{
	Q_OBJECT
public:
	PlotterModule(QWidget *parent = 0);
	~PlotterModule(void);

	void initialize(
		const std::vector<short>& data,
		short min, 
		short max, 
		float pixel_pitch,
		float length
	);
	void captureImage();

	inline short min_hu() const noexcept { return min_hu_; }
	inline short max_hu() const noexcept { return max_hu_; }

signals:
	void sigDisplayInfo(int profile_index, short hu_value);
	void sigChangeLengthStartPos(const float percent_to_start);
	void sigChangeLengthEndPos(const float percent_to_start);

private:
	void InitProfilePos();

	void drawChart(QPainter *painter);

	void adjust();
	void adjustAxis(float &min, float &max, int &numThicks);

	void DrawCurrentValue(const QPointF& pos);
	void SetLengthStartLine(const float start);
	void SetLengthEndLine(const float end);
	void SetLengthLines(const float start, const float end);
	void SetLengthLines();

protected:
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	void paintEvent(QPaintEvent* event);

private:
	enum Item
	{
		NONE,
		LENGTH_START_LINE,
		LENGTH_END_LINE
	};

	short min_hu_, max_hu_;
	float span_x_, m_fChartMinY, m_fChartMaxY;
	int ticks_x_, ticks_y_;
	std::vector<short> profile_;
	std::vector<QPointF> profile_pos_;
	float pixel_pitch_ = 0.0f;
	bool no_value_ = false;
	QRect draw_rect_ = QRect();

	QPoint curr_pos_ = QPoint(-1, -1);
	int pressed_profile_index_ = -1;

	QLineF length_start_line_;
	QLineF length_end_line_;
	QLineF length_range_line_;

	Item selected_item_ = Item::NONE;

	float length_ = 0.0f;

	float start_pos_percent_ = 0.0f;
	float end_pos_percent_ = 1.0f;
};
