#pragma once

/*=========================================================================

File:			class CW3TitleBar
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-03-23
Last modify:	2016-03-23

=========================================================================*/

#include <QWidget>
#include <QMouseEvent>
#include <qaction.h>

#include "uiframe_global.h"

class QMenuBar;
class QMenu;
class QToolButton;

class UIFRAME_EXPORT CW3TitleBar : public QWidget
{
	Q_OBJECT
public:
	CW3TitleBar(QWidget* parent = 0);
	virtual ~CW3TitleBar();

	void SetMaximized(bool maximized);

public slots:
	void slotMinimizeClicked();
	void slotNormalMaximizeClicked();
	void slotCloseClicked();

signals:
	void sigOpenFolder();
	void sigClose();
	void sigShowMaxRestore(bool);

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent* pEvent);

private:
	void SetNormalMaximizeButtonState();
	void ToggleNormalMaximizeState();

private:
	QToolButton *m_btnMinimize = nullptr;
	QToolButton *m_btnRestore = nullptr;
	QToolButton *m_btnClose = nullptr;

	QMenuBar *m_pMenuBar = nullptr;

	QMenu* m_pMenuFile = nullptr;
	QMenu* m_pMenuEdit = nullptr;
	QMenu* m_pMenuHelp = nullptr;

	bool maximized_ = false;
	bool clicked_ = false;
	QPoint clickPos;
};
