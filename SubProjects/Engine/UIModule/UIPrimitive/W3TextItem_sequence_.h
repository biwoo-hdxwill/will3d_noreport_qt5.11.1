#pragma once

/*=========================================================================

File:			class CW3TextItem_sequence
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-12-22
Modify Date:	2016-05-20

=========================================================================*/

#include "uiprimitive_global.h"
#include <QPainter>
#include <QObject>
#include <QGraphicsItem>
#include "W3TextItem.h"

#include <QStyleOptionGraphicsItem>

class QSignalMapper;

class UIPRIMITIVE_EXPORT CW3TextItem_sequence : public QObject, public QGraphicsItem
{
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)
public:
	CW3TextItem_sequence(QGraphicsItem* parent = 0);
	~CW3TextItem_sequence(void);

public:
	enum ORIENTATION{
		vertical,
		horizontal
	};

	void addText(const QString& str);
	W3INT getCurrentPressed(void);

	void setFont(const QFont& font);
	void setTextColor(const QColor& color);
	void setTextHighlightColor(const QColor& color);

	virtual W3BOOL isSelected(void);

	inline void setOrientation(const ORIENTATION orien){ m_orientation = orien; }
	inline ORIENTATION getOrientation(void) const { return m_orientation; }

	inline void setTextMargin(const W3INT margin) { m_nTextMargin = margin; }
	inline W3INT getTextMargin(void) const { return m_nTextMargin; }

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {};
	virtual QRectF boundingRect() const { return m_rect; }

signals:
	void sigPressed(const QString&);
private:

	QList<CW3TextItem*> m_lstTextItem;
	ORIENTATION m_orientation;
	W3INT m_nTextMargin;
	QRectF m_rect;

	QFont m_font;
	QColor m_textColor;
	QColor m_textHIColor;

	QSignalMapper* m_pMapper;
};
