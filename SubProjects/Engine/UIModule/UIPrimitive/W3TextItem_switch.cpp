#include "W3TextItem_switch.h"

#include <Engine/Common/Common/language_pack.h>

/*=========================================================================

File:			class CW3TextItem_switch
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-12-22
Modify Date:	2015-12-22

=========================================================================*/

CW3TextItem_switch::CW3TextItem_switch(const QString& str)
	:m_strSwitch(str), CW3TextItem(nullptr) {
	m_bState = false;
	QString strText = m_strSwitch + QString(" : " + lang::LanguagePack::txt_off());
	this->setPlainText(strText);
}

CW3TextItem_switch::~CW3TextItem_switch() {}

void CW3TextItem_switch::setCurrentState(bool isOn) {
	m_bState = isOn;

	if (m_bState) {
		QString strText = m_strSwitch + QString(" : " + lang::LanguagePack::txt_on());
		this->setPlainText(strText);
	} else {
		QString strText = m_strSwitch + QString(" : " + lang::LanguagePack::txt_off());
		this->setPlainText(strText);
	}

	emit sigState(m_bState);
}
void CW3TextItem_switch::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsTextItem::mousePressEvent(event);

	m_bState = !m_bState;

	if (m_bState) {
		QString strText = m_strSwitch + QString(" : " + lang::LanguagePack::txt_on());
		this->setPlainText(strText);
	} else {
		QString strText = m_strSwitch + QString(" : " + lang::LanguagePack::txt_off());
		this->setPlainText(strText);
	}

	emit sigState(m_bState);
}
