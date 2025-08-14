#pragma once

/*=========================================================================

File:			class CW3Dialog
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-06-13
Last modify:	2016-06-13

=========================================================================*/
#include <QDialog>
#include <QStyleOptionTitleBar>
#include <QPainter>

#include "common_global.h"

class QLabel;
class QAbstractButton;
class QToolButton;
class QVBoxLayout;
class QFrame;

class CW3DialogTitleBar : public QWidget {
	Q_OBJECT
public:
	CW3DialogTitleBar(const QString& strTitle, QWidget* parent = 0);
	~CW3DialogTitleBar() {}

	void setTitle(const QString& strTitle);

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

private slots:
	void parentClose();
private:
	QToolButton* m_btnClose;
	QLabel* m_lblCaption;

	QPoint startPos;
	QPoint clickPos;
};
class COMMON_EXPORT CW3Dialog : public QDialog {
	Q_OBJECT

public:
	enum class Theme { Dark, Light };

	CW3Dialog(const QString& strTitle, QWidget* parent = 0, const Theme theme = Theme::Dark);
	virtual ~CW3Dialog();

	void setTitle(const QString& strTitle);
	void SetContentLayout(QLayout* layout);

protected:
	void keyPressEvent(QKeyEvent* event) override;

	QFrame* CreateHorizontalLine();

protected:
	QFrame* m_contentWidget;
	QVBoxLayout* m_contentLayout;
	CW3DialogTitleBar* m_titleBar;

	QAbstractButton* positive_button_ = nullptr;
};
