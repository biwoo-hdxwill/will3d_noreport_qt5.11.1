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
#include "W3InputDialog.h"

#include <qboxlayout.h>
#include <QToolButton>
#include <QLineEdit>
#include <QDoubleSpinBox>

#include "../../Common/Common/language_pack.h"

CW3InputDialog::CW3InputDialog(InputMode mode, QDialog *parent)
	: CW3Dialog(QString("Input"), parent) {
	//this->setFixedSize(300, 80);

	m_pBtn_OK = new QToolButton();
	m_pBtn_OK->setText(lang::LanguagePack::txt_ok());

	m_pBtn_Cancel = new QToolButton();
	m_pBtn_Cancel->setText(lang::LanguagePack::txt_cancel());

	m_pLayoutTop = new QVBoxLayout();
	m_pLayoutTop->setContentsMargins(15, 10, 15, 5);
	m_pLayoutTop->setSpacing(15);

	switch (mode) {
	case CW3InputDialog::TextInput:
		m_pLineEdit = new QLineEdit();
		m_pLayoutTop->addWidget(m_pLineEdit);
		connect(m_pLineEdit, SIGNAL(returnPressed()), this, SLOT(accept()));
		break;
	case CW3InputDialog::DoubleInput:
		m_pDoubleSpinBox = new QDoubleSpinBox();
		m_pLayoutTop->addWidget(m_pDoubleSpinBox);
		break;
	default:
		break;
	}

	m_pLayoutButton = new QHBoxLayout();
	m_pLayoutButton->setContentsMargins(15, 5, 15, 10);
	m_pLayoutButton->setSpacing(5);
	m_pLayoutButton->addWidget(m_pBtn_OK);
	m_pLayoutButton->addWidget(m_pBtn_Cancel);

	//m_pLayoutTop->addWidget(m_pLineEdit);
	m_pLayoutTop->addLayout(m_pLayoutButton);

	m_contentLayout->addLayout(m_pLayoutTop);

	connections();
}

CW3InputDialog::~CW3InputDialog() {
}

void CW3InputDialog::connections() {
	connect(m_pBtn_OK, SIGNAL(clicked()), this, SLOT(accept()));
	connect(m_pBtn_Cancel, SIGNAL(clicked()), this, SLOT(reject()));
}

void CW3InputDialog::setTitle(const QString &strTitle) {
	CW3Dialog::setTitle(strTitle);
	//this->setWindowTitle(strTitle);
	//m_pLineEdit->setText("");
}

void CW3InputDialog::setText(const QString &strText) {
	if (m_pLineEdit)
		m_pLineEdit->setText(strText);
}

void CW3InputDialog::setSize(int w, int h) {
	this->setFixedSize(w, h);
}

QString CW3InputDialog::getInputText() {
	return m_pLineEdit ? m_pLineEdit->text() : "";
}

double CW3InputDialog::getInputDouble() {
	return m_pDoubleSpinBox ? m_pDoubleSpinBox->value() : 0.0;
}

void CW3InputDialog::setRange(double min, double max) {
	if (m_pDoubleSpinBox)
		m_pDoubleSpinBox->setRange(min, max);
}

void CW3InputDialog::setDecimals(int prec) {
	if (m_pDoubleSpinBox)
		m_pDoubleSpinBox->setDecimals(prec);
}

void CW3InputDialog::setSingleStep(double val) {
	if (m_pDoubleSpinBox)
		m_pDoubleSpinBox->setSingleStep(val);
}

void CW3InputDialog::setValue(double val) {
	if (m_pDoubleSpinBox)
		m_pDoubleSpinBox->setValue(val);
}

void CW3InputDialog::setSuffix(const QString &suffix) {
	if (m_pDoubleSpinBox)
		m_pDoubleSpinBox->setSuffix(suffix);
}
