#include "W3TextItem_sequence.h"

/*=========================================================================

File:			class CW3TextItem_sequence
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-12-22
Modify Date:	2016-05-20

=========================================================================*/

#include "../../Common/Common/W3Memory.h"
#include <QSignalMapper>

CW3TextItem_sequence::CW3TextItem_sequence(QGraphicsItem* parent)
	: QGraphicsItem(parent)
{
	this->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	this->setZValue(50);

	m_orientation = ORIENTATION::horizontal;
	m_nTextMargin = 0;

	m_rect = QRectF(0.0f, 0.0f, 0.0f, 0.0f);

	QFont font;
	font.setFamily(font.defaultFamily());
	//font.setPixelSize(font.pixelSize() + 2);

	font.setPixelSize(12);

	m_font = font;

	m_textColor = Qt::green;
	m_textHIColor = Qt::red;

	m_pMapper = new QSignalMapper(this);
	connect(m_pMapper, SIGNAL(mapped(const QString&)), this, SIGNAL(sigPressed(const QString&)));

}


CW3TextItem_sequence::~CW3TextItem_sequence()
{
	while (m_lstTextItem.size())
	{
		auto iter = m_lstTextItem.begin();
		SAFE_DELETE_OBJECT(*iter);
		m_lstTextItem.erase(iter);
	}
}

////////////////////////////////////////////////////////////////////////////////////
// public functions
////////////////////////////////////////////////////////////////////////////////////

void CW3TextItem_sequence::addText(const QString& str)
{
	CW3TextItem* text = new CW3TextItem(this);
	text->setPlainText(str);
	text->setFont(m_font);
	text->setTextColor(m_textColor);
	text->setTextHighlightColor(m_textHIColor);
	connect(text, SIGNAL(sigPressed()), m_pMapper, SLOT(map()));
	m_pMapper->setMapping(text, str);

	m_lstTextItem.push_back(text);

	W3INT nTextLen = 0;
	W3INT nTextCnt = m_lstTextItem.size();
	W3INT nTextMargin = this->getTextMargin();


	if (this->getOrientation() == ORIENTATION::vertical)
	{

		W3INT nTextMaxWidth = 0;

		for (const auto &i : m_lstTextItem)
		{
			nTextLen += i->boundingRect().height();

			W3INT width = i->boundingRect().width();
			if (nTextMaxWidth < width)
				nTextMaxWidth = width;
		}

		m_rect = QRectF(0, 0, nTextMaxWidth, nTextMargin*nTextCnt + nTextLen);

		W3INT nPreLen = 0;

		for (W3INT i = 0; i < nTextCnt; i++)
		{
			m_lstTextItem.at(i)->setPos(0, nPreLen + nTextMargin*i);
			nPreLen += m_lstTextItem.at(i)->boundingRect().height();
		}
	}
	else if (this->getOrientation() == ORIENTATION::horizontal)
	{

		W3INT nTextMaxHeight = 0;


		for (const auto &i : m_lstTextItem)
		{
			nTextLen += i->boundingRect().width();

			W3INT height = i->boundingRect().height();

			if (nTextMaxHeight < height)
				nTextMaxHeight = height;
		}


		m_rect = QRectF(0, 0, nTextMargin*nTextCnt + nTextLen, nTextMaxHeight);

		W3INT nPreLen = 0;

		for (W3INT i = 0; i < nTextCnt; i++)
		{
			m_lstTextItem.at(i)->setPos(nPreLen + nTextMargin*i, 0);
			nPreLen += m_lstTextItem.at(i)->boundingRect().width();
		}
	}

}

void CW3TextItem_sequence::setFont(const QFont& font)
{
	m_font = font;

	for (const auto &i : m_lstTextItem)
		i->setFont(m_font);
}

void CW3TextItem_sequence::setTextColor(const QColor& color)
{
	m_textColor = color;

	for (const auto &i : m_lstTextItem)
		i->setTextColor(m_textColor);
}

void CW3TextItem_sequence::setTextHighlightColor(const QColor& color)
{
	m_textHIColor = color;

	for (const auto &i : m_lstTextItem)
		i->setTextHighlightColor(m_textHIColor);
}
W3INT CW3TextItem_sequence::getCurrentPressed(void)
{
	W3INT nIndex = -1;
	for (W3INT i = 0; i < m_lstTextItem.size(); i++)
	{
		if (m_lstTextItem.at(i)->isSelected())
		{
			nIndex = i;
			break;
		}
	}
	return nIndex;

}
W3BOOL CW3TextItem_sequence::isSelected(void)
{
	for (const auto &i : m_lstTextItem)
	{
		if (i->isSelected())
			return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////
// private slots
////////////////////////////////////////////////////////////////////////////////////
