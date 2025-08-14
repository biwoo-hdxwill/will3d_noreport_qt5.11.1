#pragma once

/*=========================================================================

File:			class CW3TextItem_sequence
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-12-22
Modify Date:	2016-06-20

=========================================================================*/
#include <QPainter>
#include <QObject>
#include <QGraphicsItem>

#include "uiprimitive_global.h"

class QSignalMapper;
class QStyleOptionGraphicsItem;
class CW3TextItem;

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

	void clear();
	void addText(const QString& str);
	int getCurrentPressed(void);

	void setHighlightEffects(bool bFlag);
	void setFont(const QFont& font);
	void setTextColor(const QColor& color);
	void setTextHighlightColor(const QColor& color);

	virtual bool isSelected(void);

	inline void setOrientation(const ORIENTATION orien){ m_orientation = orien; }
	inline ORIENTATION getOrientation(void) const { return m_orientation; }

	inline void setTextMargin(const int margin) { m_nTextMargin = margin; }
	inline int getTextMargin(void) const { return m_nTextMargin; }
	inline int getCount() const { return m_lstTextItem.size(); }
	inline const QList<CW3TextItem*> getTexts() const { return m_lstTextItem; }

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {};
	virtual QRectF boundingRect() const;

signals:
	void sigReleased(const QString&);

private:
	QList<CW3TextItem*> m_lstTextItem;
	ORIENTATION m_orientation;
	int m_nTextMargin;
	QRectF m_rect;

	QFont m_font;
	QColor m_textColor;
	QColor m_textHIColor;

	QSignalMapper* m_pMapper;
};
