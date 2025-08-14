#pragma once
/*=========================================================================

File:			class CW3DicomLoaderThumbnailView
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			YouSong kim, JUNG DAE GUN
First Date:
Modify Date:	2016-06-01
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.
=========================================================================*/
#include <QGraphicsView>

#include "../UIPrimitive/W3GuideRect.h"

#include "uiframe_global.h"

class QGraphicsPixmapItem;

class UIFRAME_EXPORT CW3DicomLoaderThumbnailView : public QGraphicsView {
	Q_OBJECT
public:
	CW3DicomLoaderThumbnailView(int);
	~CW3DicomLoaderThumbnailView();
	
	void setImage(short *pImage, int width, int height, int winWidth, int winLevel, const QRectF& giudeRect = QRectF());
	void setEmpty();

	QRect getSelectedRect();

	inline int left_max() const { return left_max_; };
	inline int right_min() const { return right_min_; };
	inline int top_max() const { return top_max_; };
	inline int bottom_min() const { return bottom_min_; };

signals:
	void sigSyncThumbnailRect(QRectF, int);
	void sigWheelTranslate(int, int);

public slots:
	void slotSyncThumbnailRect(QRectF rect, int index);
	void slotSetGuideRectLeft(int left);
	void slotSetGuideRectRight(int right);
	void slotSetGuideRectTop(int top);
	void slotSetGuideRectBottom(int bottom);

private:
	void updateLUT();
	void limiteRectLeft(QRectF &rect, const int &minRectSize);
	void limiteRectRight(QRectF &rect, const int &minRectSize);
	void limiteRectTop(QRectF &rect, const int &minRectSize);
	void limiteRectBottom(QRectF &rect, const int &minRectSize);

	virtual void resizeEvent(QResizeEvent *event);
	virtual void wheelEvent(QWheelEvent *event);
	virtual void leaveEvent(QEvent* event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);

private:
	QImage *m_pImage = nullptr;
	QGraphicsPixmapItem *m_pRenderPixmap = nullptr;

	bool m_bDisplayed = false;

	int m_nControlPos;	//left, center, right를 생성자에서 전달받는다.
	int m_nWidth = 0;
	int m_nHeight = 0;
	int m_nWindowingLevel = 0;
	int m_nWindowingWidth = 0;

	//QRect m_rectGuide[2];

	QGraphicsLineItem *m_lpCenterCross[2];

	QPointF m_ptScene;
	QPointF m_ptOld;

	unsigned char *m_pLUT = nullptr;
	unsigned char *m_pgResultLUT = nullptr;

	CW3GuideRect *m_pGuideRect = nullptr;
	QPointF m_viewMargin;

	EGUIDERECT_ELEMENT m_eSelectedElement = EGUIDERECT_ELEMENT::NONE;

	int m_nPreWidth = 0;
	int m_nPreHeight = 0;

	int left_max_ = 0;
	int right_min_ = 0;
	int top_max_ = 0;
	int bottom_min_ = 0;

	float rect_size_min_ = 0;
};
