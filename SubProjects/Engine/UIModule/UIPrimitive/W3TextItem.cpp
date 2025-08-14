#include "W3TextItem.h"
/*=========================================================================

File:			class CW3TextItem
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-12-22
Modify Date:	2016-09-26

=========================================================================*/
#include <QStyleOptionGraphicsItem>

#include <QPainter>
#include <QPainterPath>
#include <QApplication>

namespace {
QColor kOutlineColor(Qt::black);
QPen kPenOutline(kOutlineColor, 2.5f);
} // end of namespace

CW3TextItem::CW3TextItem(const bool interactive, QGraphicsItem *parent)
	: QGraphicsTextItem(parent) {
	QFont font = QApplication::font();
	this->setFont(font);

	m_textColor = Qt::white; //normal color
	m_textHLColor = QColor("#FF78A3FF"); //highlight color

	this->setDefaultTextColor(m_textColor);
	this->setAcceptHoverEvents(interactive);
	this->setFlag(QGraphicsTextItem::ItemIgnoresTransformations, true);
	this->setFlag(QGraphicsTextItem::ItemIsSelectable, interactive);
	this->setFlag(QGraphicsTextItem::ItemIsMovable, interactive);
	this->setZValue(10.0f);

	m_bHoverEnabled = interactive;
}

CW3TextItem::CW3TextItem(QGraphicsItem *parent)
	: QGraphicsTextItem(parent) {
	QFont font = QApplication::font();
	this->setFont(font);

	m_textColor = Qt::white; //normal color
	m_textHLColor = QColor("#FF78A3FF"); //highlight color

	this->setDefaultTextColor(m_textColor);
	this->setAcceptHoverEvents(true);
	this->setFlag(QGraphicsTextItem::ItemIgnoresTransformations, true);
	this->setFlag(QGraphicsTextItem::ItemIsSelectable, true);
	this->setZValue(10.0f);

	m_bHoverEnabled = true;
}

CW3TextItem::CW3TextItem(const QFont& font, const QString& txt,
						 const QColor& c, const bool bHover,
						 QGraphicsItem* pParent) :
	QGraphicsTextItem(pParent) {
	this->setFont(font);
	this->setPlainText(txt);
	m_textColor = c;

	this->setDefaultTextColor(m_textColor);
	this->setAcceptHoverEvents(true);
	this->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	this->setFlag(QGraphicsTextItem::ItemIsSelectable, true);
	this->setZValue(10.0f);

	m_bHoverEnabled = bHover;
}

CW3TextItem::~CW3TextItem() {}

////////////////////////////////////////////////////////////////////////////////////
// public functions
////////////////////////////////////////////////////////////////////////////////////
void CW3TextItem::setHighlightEffect(const bool bFlag) {
	if (bFlag && !m_bHovered) {
		this->setDefaultTextColor(m_textHLColor);
		setFont(font());
		m_bHovered = true;
	} else {
		this->setDefaultTextColor(m_textColor);
		setFont(font());
		m_bHovered = false;
	}
}
void CW3TextItem::setTextColor(const QColor &c) {
	m_textColor = c;
	this->setDefaultTextColor(m_textColor);
}

void CW3TextItem::setTextHighlightColor(const QColor &c) {
	m_textHLColor = c;
	this->update();
}
void CW3TextItem::setPixelSize(float size) {
	m_font.setPixelSize(size);
	this->setFont(m_font);
}

void CW3TextItem::setPointSize(float size)
{
	m_font.setPointSizeF(size);
	setFont(m_font);
}

void CW3TextItem::setTextBold(bool enable) {
	m_font.setBold(enable);
	this->setFont(m_font);
}
void CW3TextItem::setBackground(const QColor & c) {
	m_brushBackground = QBrush(c);
	m_drawBackground = true;
}
void CW3TextItem::setBorder(const QColor & c, int lineWidth) {
	m_penBorder = QPen(c, lineWidth);
	m_drawBorder = true;
}
void CW3TextItem::setFont(const QFont & aFont) {
	m_font = aFont;
	QGraphicsTextItem::setFont(m_font);
}
////////////////////////////////////////////////////////////////////////////////////
// protected functions
////////////////////////////////////////////////////////////////////////////////////

void CW3TextItem::paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget) {
	QStyleOptionGraphicsItem* style = const_cast<QStyleOptionGraphicsItem*>(pOption);

	// Remove the HasFocus style state to prevent the dotted line from being drawn.
	style->state &= ~QStyle::State_HasFocus;
	style->state &= ~QStyle::State_Selected;

	QString plane_text = toPlainText();
	QFontMetrics fm(font());
	QRect rect = fm.boundingRect(this->boundingRect().toRect(), Qt::AlignCenter, plane_text);

	QPainterPath path;
	path.addText(rect.left(), rect.top() + fm.ascent(), font(), plane_text);
	pPainter->setPen(kPenOutline);
	pPainter->drawPath(path);

	if (m_bAntialiasing)
		pPainter->setRenderHint(QPainter::TextAntialiasing, true);
	else
		pPainter->setRenderHint(QPainter::TextAntialiasing, false);

	if (m_drawBackground && m_drawBorder) {
		pPainter->save();
		pPainter->setBrush(m_brushBackground);
		pPainter->setPen(m_penBorder);
		pPainter->drawRoundedRect(this->boundingRect(), 2, 2);
		pPainter->restore();
	} else if (m_drawBackground) {
		pPainter->save();
		pPainter->setBrush(m_brushBackground);
		pPainter->setPen(QPen("#00000000"));
		pPainter->drawRoundedRect(this->boundingRect(), 2, 2);
		pPainter->restore();
	} else if (m_drawBorder) {
		pPainter->save();
		pPainter->setBrush(QBrush("#00000000"));
		pPainter->setPen(m_penBorder);
		pPainter->drawRoundedRect(this->boundingRect(), 2, 2);
		pPainter->restore();
	}

	QGraphicsTextItem::paint(pPainter, pOption, pWidget);
}

QRectF CW3TextItem::boundingRect() const {
	QRectF rect = QGraphicsTextItem::boundingRect();

	if (m_nFixedWidth != 0.0)
		rect.setWidth(m_nFixedWidth);

	return rect;
}
QPainterPath CW3TextItem::shape() const {
	QPainterPath path;
	QRectF rect = boundingRect();
	path.addRect(rect);
	return path;
}

void CW3TextItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
	if (m_bHoverEnabled) {
		QGraphicsTextItem::hoverEnterEvent(event);
		setHighlightEffect(true);
		emit sigHoverText(true);
	}
}

void CW3TextItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
	if (m_bHoverEnabled) {
		QGraphicsTextItem::hoverLeaveEvent(event);
		setHighlightEffect(false);
		emit sigHoverText(false);
	}
}

void CW3TextItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	if (m_bHoverEnabled) {
		QGraphicsTextItem::mousePressEvent(event);
		emit sigPressed();
	}
}

void CW3TextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	if (m_bHoverEnabled) {
		QGraphicsTextItem::mouseReleaseEvent(event);
		emit sigReleased();
	}
}
