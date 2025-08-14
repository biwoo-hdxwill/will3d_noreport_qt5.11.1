#pragma once
/*=========================================================================

File:		class CW3InputDialog
Language:	C++11
Library:	Qt 5.4 , Standard C++ Library
Author:			Sang Keun Park
First date:		2016-05-30
Last modify:	2016-05-30

comment:
Text 입력용 Dialog
=========================================================================*/
#include "../../Common/Common/W3Dialog.h"

#include "uiprimitive_global.h"

class QVBoxLayout;
class QHBoxLayout;
class QToolButton;
class QLineEdit;
class QDoubleSpinBox;

class UIPRIMITIVE_EXPORT CW3InputDialog : public CW3Dialog
{
	Q_OBJECT
public:
	enum InputMode { TextInput, DoubleInput };

	CW3InputDialog(InputMode mode = InputMode::TextInput, QDialog *parent = 0);
	~CW3InputDialog();

	void setTitle(const QString &strTitle);
	void setText(const QString &strText);

	void setSize(int w, int h);

	QString getInputText();
	double getInputDouble();

	void setRange(double min, double max);
	void setDecimals(int prec);
	void setSingleStep(double val);
	void setValue(double val);
	void setSuffix(const QString &suffix);

private:
	void connections();

private:
	QVBoxLayout*	m_pLayoutTop;
	QHBoxLayout*	m_pLayoutButton;

	QToolButton*	m_pBtn_OK;
	QToolButton*	m_pBtn_Cancel;
	QLineEdit*		m_pLineEdit = nullptr;
	QDoubleSpinBox*	m_pDoubleSpinBox = nullptr;
};

