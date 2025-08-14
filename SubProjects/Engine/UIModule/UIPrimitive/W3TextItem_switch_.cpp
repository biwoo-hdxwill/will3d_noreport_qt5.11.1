#include "W3TextItem_switch.h"

/*=========================================================================

File:			class CW3TextItem_switch
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-12-22
Modify Date:	2015-12-22

=========================================================================*/

CW3TextItem_switch::CW3TextItem_switch(const QString& str)
	:m_strSwitch(str)
{
	m_bState = false;
	QString strText = m_strSwitch + QString(": Off");
	this->setPlainText(strText);

	m_font = QFont("Times", 12, QFont::Normal);
	m_fontHL = QFont("Times", 13, QFont::Bold);
	m_textColor = QColor(0, 255, 0); //normal color
	m_textHLColor = QColor(255, 0, 0); //highlight color

	this->setFont(m_font);
	this->setDefaultTextColor(m_textColor);
}


CW3TextItem_switch::~CW3TextItem_switch()
{

}
void CW3TextItem_switch::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsTextItem::mousePressEvent(event);

	m_bState = !m_bState;

	if (m_bState)
	{
		QString strText = m_strSwitch + QString(": On");
		this->setPlainText(strText);
	}
	else
	{
		QString strText = m_strSwitch + QString(": Off");
		this->setPlainText(strText);
	}

	emit sigState(m_bState);
}
