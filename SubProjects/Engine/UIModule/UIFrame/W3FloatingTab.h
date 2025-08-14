#pragma once
/*=========================================================================

File:		class CW3FloatingTab
Language:	OpenCL 1.1, C++11
Library:	Standard C++ Library

=========================================================================*/

#include <QWidget>
#include "uiframe_global.h"
#include "GeneratedFiles\ui_W3FloatingTab.h"

#include "../../Common/Common/W3Types.h"

class CW3ViewLayout;
class CW3GeneralMenu;
class CW3FloatingMenuBar;

class UIFRAME_EXPORT CW3FloatingTab : public QWidget
{
	Q_OBJECT

public:
	CW3FloatingTab(QWidget *parent);
	~CW3FloatingTab();

	CW3ViewLayout* getViewLayout(void);

signals:
	void sigClosed(void);

public slots:
	void slotSetDisplayList(std::vector<W3BOOL>);

private:
	void initLayout(void);
	void connections(void);
	void connections_seg(void);
	void disconnections_seg(void);

protected:
	virtual void closeEvent(QCloseEvent* pEvent);

private:
	Ui::CW3FloatingTab	ui;
	QWidget*			m_pParent;

	CW3ViewLayout*		m_viewLayoutFloating;
	CW3GeneralMenu*		m_generalMenuFloating;
	CW3FloatingMenuBar*	m_menuBarFloating;
};

