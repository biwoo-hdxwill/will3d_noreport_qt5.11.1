#pragma once

/*=========================================================================

File:			class CW3DicomLoaderScrollbar
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			YouSong kim, JUNG DAE GUN
First Date:
Modify Date:	2016-06-01
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.
=========================================================================*/
#include <QGraphicsView>
#include "uiprimitive_global.h"

class UIPRIMITIVE_EXPORT CW3DicomLoaderScrollbar : public QGraphicsView
{
	Q_OBJECT
public:
	CW3DicomLoaderScrollbar(QWidget* parent = nullptr);
	~CW3DicomLoaderScrollbar(void);

	void setRange(float first, float last);
	void setValidRange(float first, float last, float start, float end);
	void setStart(float start);
	void setMiddle(float mid);
	void setEnd(float end);
	void Reset();

	inline float getFirst() { return m_fFirst; }
	inline float getLast() { return m_fLast; }
	inline float getStart() { return m_fStart; }
	inline float getMiddle() { return m_fMiddle; }
	inline float getEnd() { return m_fEnd; }
	
signals:
	void sigScrollTranslated(int, float);

public slots:
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);

protected:
	virtual void resizeEvent(QResizeEvent *event);

private:
	void drawForeground(QPainter *painter, const QRectF &rect);	

private:
	enum { LEFT = 0, CENTER, RIGHT };

	int m_nSelectedCtrl;
	float m_fPos[3];
	float m_fNodeWidth;
	float m_fStart, m_fMiddle, m_fEnd;		//선택된 시작과 끝
	float m_fFirst, m_fLast;	//시작, 끝

	QPointF m_ptClickedPos;

	int m_nPreWidth;

	float bar_height_ = 0.0f;

	QRectF bar_rect_;
	QRectF left_node_rect_;
	QRectF center_node_rect_;
	QRectF right_node_rect_;
};
