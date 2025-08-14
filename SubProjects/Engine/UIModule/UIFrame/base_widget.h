#pragma once
/*=========================================================================
File:			base_widget.h
Language:		C++11
Library:        Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-09-01
Last modify:	2021-09-01

Copyright (c) 2018 HDXWILL. All rights reserved.
=========================================================================*/

#include <QWidget>

#include "uiframe_global.h"

namespace
{
	const QMargins kMarginZero(0, 0, 0, 0);
	const int kSpacingZero = 0;
	const int kSpacing5 = 5;
	const int kSpacing10 = 10;

	const QMargins kContentsMargins = QMargins(15, 15, 15, 15);
	const QMargins kStepMargins = QMargins(20, 0, 20, 0);
}

class QFrame;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QToolButton;
class QRadioButton;
class QVBoxLayout;
class QHBoxLayout;
class UIFRAME_EXPORT BaseWidget : public QWidget
{
	Q_OBJECT
public:
	explicit BaseWidget(QWidget* parent = nullptr);
	virtual ~BaseWidget();
	BaseWidget(const BaseWidget&) = delete;
	const BaseWidget& operator = (const BaseWidget&) = delete;

protected:
	virtual void Reset() {};

	void ResetButtonDisable();

	QFrame* CreateHorizontalLine();
	QFrame* CreateVerticalLine();
	QComboBox* CreateComboBox(const QStringList& items, bool fixed_width = true);
	QLabel* CreateLabel(const QString& text, QSizePolicy::Policy hor = QSizePolicy::Preferred, QSizePolicy::Policy ver = QSizePolicy::Preferred, Qt::Alignment alignment = Qt::AlignTop);
	QLineEdit* CreateLineEdit(QSizePolicy::Policy hor = QSizePolicy::Preferred, QSizePolicy::Policy ver = QSizePolicy::Fixed);
	QToolButton* CreateColoredToolButton(QColor color, QSizePolicy::Policy hor = QSizePolicy::Fixed, QSizePolicy::Policy ver = QSizePolicy::Fixed);
	QToolButton* CreateTextToolButton(const QString& text, QSizePolicy::Policy hor = QSizePolicy::Fixed, QSizePolicy::Policy ver = QSizePolicy::Fixed);
	QRadioButton* CreateTextRadioBttton(const QString& text, QSizePolicy::Policy hor = QSizePolicy::Preferred, QSizePolicy::Policy ver = QSizePolicy::Fixed);
	QSpinBox* CreateSpinBox(int min, int max, int default_value);
	QDoubleSpinBox* CreateDoubleSpinBox(float min, float max, float default_value, QSizePolicy::Policy hor = QSizePolicy::Fixed, QSizePolicy::Policy ver = QSizePolicy::Fixed);
	
	QVBoxLayout* CreateResetButtonLayout();

	inline QVBoxLayout* contents_layout() { return contents_layout_; }

private:
	void SetLayout();

private:
	QVBoxLayout* cover_layout_ = nullptr;
	QVBoxLayout* contents_layout_ = nullptr;
};
