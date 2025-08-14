#include "W3TextItem_sequence.h"

/*=========================================================================

File:			class CW3TextItem_sequence
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-12-22
Modify Date:	2016-05-20

=========================================================================*/
#include <QSignalMapper>
#include <QApplication>

#include "../../Common/Common/W3Memory.h"
#include "W3TextItem.h"

CW3TextItem_sequence::CW3TextItem_sequence(QGraphicsItem* parent)
	: QGraphicsItem(parent) {
	this->setAcceptHoverEvents(true);
	this->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	this->setZValue(10.0f);

	m_orientation = ORIENTATION::horizontal;
	m_nTextMargin = 0;

	m_rect = QRectF(0.0f, 0.0f, 0.0f, 0.0f);

	m_font = QApplication::font();
	m_font.setPixelSize(m_font.pixelSize() - 1);

	m_textColor = Qt::white;
	m_textHIColor = QColor("#FF78A3FF");

	m_pMapper = new QSignalMapper(this);
	connect(m_pMapper, SIGNAL(mapped(QString)), this, SIGNAL(sigReleased(QString)));
}

CW3TextItem_sequence::~CW3TextItem_sequence() {
	while (m_lstTextItem.size()) {
		auto iter = m_lstTextItem.begin();
		SAFE_DELETE_OBJECT(*iter);
		m_lstTextItem.erase(iter);
	}
}

////////////////////////////////////////////////////////////////////////////////////
// public functions
////////////////////////////////////////////////////////////////////////////////////

void CW3TextItem_sequence::clear() {
	while (m_lstTextItem.size()) {
		auto iter = m_lstTextItem.begin();
		SAFE_DELETE_OBJECT(*iter);
		m_lstTextItem.erase(iter);
	}
}

void CW3TextItem_sequence::addText(const QString& str) {
	CW3TextItem* text = new CW3TextItem(this);
	text->setPlainText(str);
	text->setFont(m_font);
	text->setTextColor(m_textColor);
	text->setTextHighlightColor(m_textHIColor);
	text->setBackground(QColor("#FF5D5D5D"));
	text->setBorder(QColor("#FF4D4D4D"), 1);
	text->setFixedWidth(100);
	connect(text, SIGNAL(sigReleased()), m_pMapper, SLOT(map()));

	m_pMapper->setMapping(text, str);

	m_lstTextItem.push_back(text);
}

void CW3TextItem_sequence::setHighlightEffects(bool bFlag) {
	for (const auto &i : m_lstTextItem)
		i->setHighlightEffect(false);
}

void CW3TextItem_sequence::setFont(const QFont & font) {
	m_font = font;
	for (const auto &i : m_lstTextItem)
		i->setFont(m_font);
}

void CW3TextItem_sequence::setTextColor(const QColor& color) {
	m_textColor = color;

	for (const auto &i : m_lstTextItem)
		i->setTextColor(m_textColor);
}

void CW3TextItem_sequence::setTextHighlightColor(const QColor& color) {
	m_textHIColor = color;

	for (const auto &i : m_lstTextItem)
		i->setTextHighlightColor(m_textHIColor);
}

int CW3TextItem_sequence::getCurrentPressed(void) {
	for (int i = 0; i < m_lstTextItem.size(); i++) {
		if (m_lstTextItem.at(i)->isSelected()) {
			return i;
		}
	}
	return -1;
}

bool CW3TextItem_sequence::isSelected(void) {
	for (const auto &i : m_lstTextItem) {
		if (i->isSelected())
			return true;
	}
	return false;
}

QRectF CW3TextItem_sequence::boundingRect() const {
	int nTextLen = 0;
	int nTextCnt = m_lstTextItem.size();
	int nTextMargin = this->getTextMargin();

	QRectF rect;
	if (this->getOrientation() == ORIENTATION::vertical) {
		int nTextMaxWidth = 0;

		for (const auto &i : m_lstTextItem) {
			nTextLen += i->boundingRect().height();

			int width = i->boundingRect().width();
			if (nTextMaxWidth < width)
				nTextMaxWidth = width;
		}

		rect = QRectF(0, 0, nTextMaxWidth, nTextMargin*nTextCnt + nTextLen);

		int nPreLen = 0;
		for (int i = 0; i < nTextCnt; i++) {
			m_lstTextItem.at(i)->setPos(0, nPreLen + nTextMargin * i);
			nPreLen += m_lstTextItem.at(i)->boundingRect().height();
		}
	} else if (this->getOrientation() == ORIENTATION::horizontal) {
		int nTextMaxHeight = 0;

		for (const auto &i : m_lstTextItem) {
			nTextLen += i->boundingRect().width();

			int height = i->boundingRect().height();

			if (nTextMaxHeight < height)
				nTextMaxHeight = height;
		}

		rect = QRectF(0, 0, nTextMargin*nTextCnt + nTextLen, nTextMaxHeight);

		int nPreLen = 0;
		for (int i = 0; i < nTextCnt; i++) {
			m_lstTextItem.at(i)->setPos(nPreLen + nTextMargin * i, 0);
			nPreLen += m_lstTextItem.at(i)->boundingRect().width();
		}
	}

	return rect;
}
