#pragma once
/*=========================================================================

File:		class CW3GeneralMenu
Language:	C++11
Library:	Qt 5.2.1, Standard C++ Library

=========================================================================*/
#include "uiframe_global.h"

#include <QWidget>

#include "GeneratedFiles\ui_W3GeneralMenu.h"
#include "../../Common/Common/W3Types.h"
#include "../../Common/Common/W3Enum.h"
#include "../../Common/Common/W3Point2D.h"
#include "W3SelectDicomHeaderDlg.h"
#include "W3SelectViewLayout.h"

class UIFRAME_EXPORT CW3GeneralMenu : public QWidget
{
	Q_OBJECT

public:
	CW3GeneralMenu(QWidget *parent = 0);
	~CW3GeneralMenu();

signals:
	void sigChangeLayout(EVIEW_LAYOUT_TYPE, W3INT, W3INT);
	void sigInfoDisplay(std::vector<W3BOOL> bList);
	void sigCapture(void);
	void sigResetMPR(void);
	void sigResourceView(CW3Point2D pt, W3BOOL);

public slots:
	void slotChangeLayout(void);
	void slotInfoDisplay(void);
	void slotCapture(void);
	void slotResourceView(void);
	void slotSetMenuSize(W3INT);

private:
	void setConnections(void);

private:
	Ui::GeneralMenu ui;

	CW3SelectDicomHeaderDlg*	m_dlg;
	CW3SelectViewLayout*		m_dlgSelectLayout;

	W3BOOL m_bChangeLayout;
	W3BOOL m_bCapture;
	W3BOOL m_bMinimized;
	W3BOOL m_bResourceView;
};
