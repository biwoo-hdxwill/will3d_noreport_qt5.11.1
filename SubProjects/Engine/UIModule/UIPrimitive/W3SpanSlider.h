#pragma once

/*=========================================================================

File:			class CW3SpanSlider
Language:		C++11
Library:        Qt 5.4.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2016-05-13
Modify Date:	2016-05-13
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include "uiprimitive_global.h"
#include <QSlider>
#include <QStyle>
#include <QObject>

QT_FORWARD_DECLARE_CLASS(QStylePainter)
QT_FORWARD_DECLARE_CLASS(QStyleOptionSlider)
class CW3SpanSliderPrivate;

class UIPRIMITIVE_EXPORT CW3SpanSlider : public QSlider {
	Q_OBJECT

		//QXT_DECLARE_PRIVATE(CW3SpanSlider)
		Q_PROPERTY(int lowerValue READ lowerValue WRITE setLowerValue)
		Q_PROPERTY(int upperValue READ upperValue WRITE setUpperValue)
		Q_PROPERTY(int lowerPosition READ lowerPosition WRITE setLowerPosition)
		Q_PROPERTY(int upperPosition READ upperPosition WRITE setUpperPosition)
		Q_PROPERTY(HandleMovementMode handleMovementMode READ handleMovementMode WRITE setHandleMovementMode)
		Q_ENUMS(HandleMovementMode)

public:
	explicit CW3SpanSlider(QWidget* parent = 0);
	explicit CW3SpanSlider(Qt::Orientation orientation, QWidget* parent = 0);
	virtual ~CW3SpanSlider();

	enum HandleMovementMode {
		FreeMovement,
		NoCrossing,
		NoOverlapping
	};

	enum SpanHandle {
		NoHandle,
		LowerHandle,
		UpperHandle
	};

	HandleMovementMode handleMovementMode() const;
	void setHandleMovementMode(HandleMovementMode mode);

	int lowerValue() const;
	int upperValue() const;

	int lowerPosition() const;
	int upperPosition() const;

	void stopChangeValue();

public slots:
	void setLowerValue(int lower);
	void setUpperValue(int upper);
	void setSpan(int lower, int upper);

	void setLowerPosition(int lower);
	void setUpperPosition(int upper);

signals:
	void sigSpanChanged(int lower, int upper);
	void sigLowerValueChanged(int lower);
	void sigUpperValueChanged(int upper);

	void sigLowerPositionChanged(int lower);
	void sigUpperPositionChanged(int upper);

	void sigSliderPressed(SpanHandle handle);
	void sigSliderReleased(SpanHandle handle);
	void sigStopChangeValue();

protected:
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void leaveEvent(QEvent * event);
	virtual void enterEvent(QEvent * event);
	virtual void wheelEvent(QWheelEvent *event);
	virtual void paintEvent(QPaintEvent* event);

private slots:
	void slotChangeTimeOut();

private:
	CW3SpanSliderPrivate* d_ptr;
	QTimer* change_timer_ = nullptr;
	friend class CW3SpanSliderPrivate;
};

class CW3SpanSliderPrivate : public QObject {
	Q_OBJECT
public:
	CW3SpanSliderPrivate();
	void initStyleOption(QStyleOptionSlider* option, CW3SpanSlider::SpanHandle handle = CW3SpanSlider::UpperHandle) const;
	int pick(const QPoint& pt) const {
		return q_ptr->orientation() == Qt::Horizontal ? pt.x() : pt.y();
	}
	int pixelPosToRangeValue(int pos) const;
	void handleMousePress(const QPoint& pos, QStyle::SubControl& control, int value, CW3SpanSlider::SpanHandle handle);
	void drawHandle(QStylePainter* painter, CW3SpanSlider::SpanHandle handle) const;
	void setupPainter(QPainter* painter, Qt::Orientation orientation, qreal x1, qreal y1, qreal x2, qreal y2) const;
	void drawSpan(QStylePainter* painter, const QRect& rect) const;
	void triggerAction(QAbstractSlider::SliderAction action, bool main);
	void swapControls();

	int lower;
	int upper;
	int lowerPos;
	int upperPos;
	int offset;
	int position;
	CW3SpanSlider::SpanHandle lastPressed;
	CW3SpanSlider::SpanHandle mainControl;
	QStyle::SubControl lowerPressed;
	QStyle::SubControl upperPressed;
	CW3SpanSlider::HandleMovementMode movement;
	bool firstMovement;
	bool blockTracking;

public slots:
	void updateRange(int min, int max);
	void movePressedHandle();

private:
	CW3SpanSlider* q_ptr;
	friend class CW3SpanSlider;
};
