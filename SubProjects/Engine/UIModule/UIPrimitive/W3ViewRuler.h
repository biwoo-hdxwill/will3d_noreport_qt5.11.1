#pragma once

/*=========================================================================

File:			class CW3ViewRuler
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun, Tae Hoon Yoo
First date:		2016-06-02
Last modify:	2017-07-28

Copyright (c) 2016 All rights reserved by HDXWILL.
=========================================================================*/
#include <QObject>
#include <QGraphicsItem>
#include <QColor>

#include "uiprimitive_global.h"

class QGraphicsScene;
class QGraphicsLineItem;
class CW3TextItem;
////////////////////////////////////////////////////////////////
//
// * view 눈금자, 단위 : 1cm
//
////////////////////////////////////////////////////////////////

class UIPRIMITIVE_EXPORT CW3ViewRuler : public QObject, public QGraphicsItem {
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)

private:
	/*
		View Width(mm) 에 따라 눈금을 표시하는 방법이 바뀐다.
		LV_1 : Width <= 50
		LV_2 : 50 < Width <= 250
		LV_3 : Width > 250
	*/
	enum class DetailLevel { LV_1, LV_2, LV_3 };

public:
	CW3ViewRuler(const QColor& color, QObject* parent = nullptr);
	explicit CW3ViewRuler(QObject* parent = nullptr);
	virtual ~CW3ViewRuler();

	void Disable();
	void SetColor(const QColor& color);

	void setViewRuler(float viewWidth, float viewHeight,
					  float widthLength, float heightLength,
					  const QPointF& viewCenterInScene, bool show_detail_length = false);
	void SetItemColor(const QColor& color);

	void setWidthLabelPos(float x, float y);
	void setHeightLabelPos(float x, float y);

	void setVisible(bool visible);

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
					   QWidget *widget) {};
	virtual QRectF boundingRect() const { return QRectF(); }

private:
	void InitUI();
	void ClearLines();
	void setRulerLine();
	void setVisibleRulerWidth(bool visible);
	void setVisibleRulerHeight(bool visible);
	void setVisibleRulerText(bool visible);

	float GetGradationLength(int index, const DetailLevel& detail_level);

private:
	QColor color_ = QColor(Qt::gray);
	QList<QGraphicsLineItem *> w_lines_;
	QList<QGraphicsLineItem *> h_lines_;

	CW3TextItem *org_text_;
	CW3TextItem *h_length_text_;
	CW3TextItem *v_length_text_;
	std::vector<CW3TextItem*> h_detail_length_text_;

	float view_width_length_ = 0.0f;
	float view_height_length_ = 0.0f;

	float view_width_in_scene_ = 0.0f;
	float view_height_in_scene_ = 0.0f;

	QPointF view_center_in_scene_;
	bool ruler_enable_ = true;

	bool show_detail_length_ = false;
};
