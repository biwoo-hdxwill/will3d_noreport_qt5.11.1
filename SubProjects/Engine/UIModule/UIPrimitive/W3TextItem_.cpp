#include "W3TextItem.h"
/*=========================================================================

File:			class CW3TextItem
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-12-22
Modify Date:	2015-12-22

=========================================================================*/

#include <QStyleOptionGraphicsItem>

CW3TextItem::CW3TextItem(QGraphicsItem *parent, W3BOOL useDefaultFont)
	: QGraphicsTextItem(parent)
{
	if (useDefaultFont)
	{
		QFont font;
		font.setFamily(font.defaultFamily());
		font.setPixelSize(font.pixelSize() + 2);
		this->setFontNormal(font);
		this->setFontHighlight(font);
	}
	else
	{
		QFont font("Times", 12, QFont::Normal);
		this->setFontNormal(font);
		this->setFontHighlight(font);
	}


	m_textColor = Qt::white; //normal color
	m_textHLColor = Qt::red; //highlight color

	this->setDefaultTextColor(m_textColor);
	this->setAcceptHoverEvents(true);
	this->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	this->setFlag(QGraphicsTextItem::ItemIsSelectable, true);
	this->setZValue(50);

	m_bHovered = false;
	m_bHoverEnabled = true;
}

CW3TextItem::~CW3TextItem(void)
{
}

////////////////////////////////////////////////////////////////////////////////////
// public functions
////////////////////////////////////////////////////////////////////////////////////
void CW3TextItem::setHighlightEffect(const W3BOOL bFlag)
{

	if (bFlag && !m_bHovered)
	{
		this->setDefaultTextColor(m_textHLColor);
		setFont(m_fontHL);
		m_bHovered = true;
	}
	else
	{
		this->setDefaultTextColor(m_textColor);
		setFont(m_font);
		m_bHovered = false;
	}

}
void CW3TextItem::setTextColor(const QColor &c)
{
	m_textColor = c;
	this->setDefaultTextColor(m_textColor);
}

void CW3TextItem::setTextHighlightColor(const QColor &c)
{
	m_textHLColor = c;
	this->update();
}
void CW3TextItem::setFontNormal(const QFont & aFont)
{
	m_font = aFont;
	this->setFont(m_font);
}
void CW3TextItem::setFontHighlight(const QFont & aFont)
{
	m_fontHL = aFont;
	this->update();
}
////////////////////////////////////////////////////////////////////////////////////
// protected functions
////////////////////////////////////////////////////////////////////////////////////

void CW3TextItem::paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget)
{
	QStyleOptionGraphicsItem* style = const_cast<QStyleOptionGraphicsItem*>(pOption);

	// Remove the HasFocus style state to prevent the dotted line from being drawn.
	style->state &= ~QStyle::State_HasFocus;
	style->state &= ~QStyle::State_Selected;

	QGraphicsTextItem::paint(pPainter, pOption, pWidget);
}

void CW3TextItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	if(m_bHoverEnabled)
	{
		QGraphicsTextItem::hoverEnterEvent(event);

		setHighlightEffect(true);

		emit sigHoverText(true);
	}
	
}


void CW3TextItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	if (m_bHoverEnabled)
	{
		QGraphicsTextItem::hoverLeaveEvent(event);

		setHighlightEffect(false);

		emit sigHoverText(false);
	}
	
}

void CW3TextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if (m_bHoverEnabled)
	{
		QGraphicsTextItem::mousePressEvent(event);
		emit sigPressed();
	}
	
}
