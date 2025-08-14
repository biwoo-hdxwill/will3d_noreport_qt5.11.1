#pragma once

/*=========================================================================

File:			class CW3Styles
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-03-23
Last modify:	2016-03-23

=========================================================================*/
#include <map>

#include <QObject>
#include <qproxystyle.h>
#include <QPainter>

#include "common_global.h"

//tab position must be set WEST and this class is change the text horizontally
class COMMON_EXPORT CW3TabWestStyle : public QProxyStyle {
	Q_OBJECT

public:
	virtual void drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled,
							  const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const override;
};

class COMMON_EXPORT CW3ToolIconButtonStyle : public QProxyStyle {
	Q_OBJECT

public:
	void drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled,
					  const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const override;
};

class COMMON_EXPORT CWToolMenuIconStyle : public QProxyStyle {
	Q_OBJECT
public:
	int pixelMetric(PixelMetric metric, const QStyleOption * option = 0, const QWidget * widget = 0) const {
		int s = QProxyStyle::pixelMetric(metric, option, widget);
		if (metric == QStyle::PM_SmallIconSize) {
			s = 40;
		}
		return s;
	}
};

class COMMON_EXPORT CW3ToolTaskStyle : public QProxyStyle {
	Q_OBJECT

public:
	void drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled,
					  const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const override;
};

class COMMON_EXPORT CW3CephTracingTaskToolStyle : public QProxyStyle {
	Q_OBJECT

public:
	void drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled,
					  const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const override;
};

class COMMON_EXPORT CW3CephIndicatorTabStyle : public QProxyStyle {
	Q_OBJECT

public:
	enum TAB_STYLE {
		DEFUALT,
		LANDMARK,
		MEASUREMENT,
		REFERENCE,
		ANALYSIS
	};
	CW3CephIndicatorTabStyle(QStyle* style = 0) :QProxyStyle(style) { m_style = DEFUALT; }

	void drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled,
					  const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const override;

	inline void setColumnWidth(int nIndex, int width) { m_columnWidth[nIndex] = width; }
	inline void setTabStyle(TAB_STYLE style) { m_style = style; }

private:
	std::map<int, int> m_columnWidth;
	TAB_STYLE m_style;
};

class COMMON_EXPORT ViewSpinBoxStyle : public QProxyStyle {
	Q_OBJECT

public:
	ViewSpinBoxStyle() {};
	~ViewSpinBoxStyle() {}

	virtual void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
							   QPainter *painter, const QWidget *widget) const override;

	virtual QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *option, SubControl sc, const QWidget *widget) const override;

	virtual void drawComplexControl(ComplexControl which,
									const QStyleOptionComplex* option,
									QPainter* painter,
									const QWidget* widget) const override;

private:
	void drawSpinButton(SubControl which,
						const QStyleOptionComplex* option, QPainter* painter, const QWidget *widget) const;
};
