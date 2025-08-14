#pragma once 
/*=========================================================================

File:		Dock Widget
Language:	C++11
Library:	Qt Library

=========================================================================*/
#include "uiframe_global.h"

#include <QDockWidget>

class UIFRAME_EXPORT CW3DockWidget : public QDockWidget
{
	Q_OBJECT

public:
	CW3DockWidget(QWidget *parent = 0);
	~CW3DockWidget();

public slots:
	void showTitleBar(void);

protected:
	virtual void enterEvent(QEvent* event);
	virtual void leaveEvent(QEvent* event);

private:
	QWidget* m_pTitleBarWidget;
};
