#pragma once
/*=========================================================================

File:			class CW3TextItem
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-12-22
Modify Date:	2016-06-20

=========================================================================*/

#include "uiprimitive_global.h"
#include "../../Common/Common/W3Types.h"
#include <QGraphicsSceneEvent>
#include <QGraphicsTextItem>
#include <QFont>
/*
	class CW3TextItem
	 : Re-implementation of QGraphicsTextItem
*/
class UIPRIMITIVE_EXPORT CW3TextItem : public QGraphicsTextItem
{
	Q_OBJECT
public:
	CW3TextItem(QGraphicsItem *parent = 0, W3BOOL useDefaultFont = false);
	~CW3TextItem(void);

	void setHighlightEffect(const W3BOOL bFlag);
	void setTextColor(const QColor &c);
	void setTextHighlightColor(const QColor &c);

	inline void setHoverEnabled(W3BOOL b) { m_bHoverEnabled = b; }

	void setFontNormal(const QFont& aFont);
	void setFontHighlight(const QFont& aFont);

	inline void setHighlighted(const W3BOOL bFlag) { m_bHighlighted = bFlag; }
	inline W3BOOL isHovered() { return m_bHovered; }

	virtual void paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget) override;
signals:
	void sigPressed(void);
	void sigHoverText(const W3BOOL bHovered);

protected:
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

protected:

	QFont	m_font;
	QFont	m_fontHL;
	QColor	m_textColor;
	QColor  m_textHLColor;

	W3BOOL	m_bHovered;
	W3BOOL	m_bHoverEnabled;

	W3BOOL	m_bHighlighted;
};

