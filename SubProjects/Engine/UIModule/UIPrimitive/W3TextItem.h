#pragma once
/*=========================================================================

File:			class CW3TextItem
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-12-22
Modify Date:	2016-09-26

=========================================================================*/
#include "uiprimitive_global.h"
#include <QGraphicsTextItem>
#include <QFont>
#include <QPen>
#include <QBrush>

class QPainterPath;

/*
	class CW3TextItem
	 : Re-implementation of QGraphicsTextItem
*/
class UIPRIMITIVE_EXPORT CW3TextItem : public QGraphicsTextItem
{
	Q_OBJECT
public:
	CW3TextItem(const bool interactive, QGraphicsItem *parent = 0);
	CW3TextItem(QGraphicsItem *parent = 0);
	CW3TextItem(const QFont& font,
				const QString& txt, // plane text
				const QColor& c, // text color
				const bool bHover = false, // hover enable flag
				QGraphicsItem* pParent = nullptr);

	~CW3TextItem();

public:
	void setHighlightEffect(const bool bFlag);
	void setTextColor(const QColor &c);
	void setTextHighlightColor(const QColor &c);
	void setPixelSize(float size);
	void setPointSize(float size);
	void setTextBold(bool enable);

	void setBackground(const QColor &c);
	void setBorder(const QColor &c, int lineWidth);

	inline void setFixedWidth(int width) { m_nFixedWidth = width; }
	inline void setHoverEnabled(bool b) { m_bHoverEnabled = b; }

	void setFont(const QFont& aFont);

	inline void setAntialiasing(bool bEnable) { m_bAntialiasing = bEnable; }
	inline bool isHovered() { return m_bHovered; }

	virtual void paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget) override;
	virtual QPainterPath shape() const;
	virtual QRectF boundingRect() const;

signals:
	void sigPressed(void);
	void sigReleased(void);
	void sigHoverText(const bool bHovered);

protected:
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

protected:
	QFont	m_font;
	QColor	m_textColor;
	QColor  m_textHLColor;

	bool	m_bHovered = false;
	bool	m_bHoverEnabled = true;

	bool	m_drawBackground = false;
	bool	m_drawBorder = false;

	bool m_bAntialiasing = true;

	QBrush	m_brushBackground;
	QPen	m_penBorder;

	int m_nFixedWidth = 0;
};

