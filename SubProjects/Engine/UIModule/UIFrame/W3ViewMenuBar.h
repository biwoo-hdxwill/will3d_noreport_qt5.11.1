#pragma once
/*=========================================================================

File:		class CW3ViewMenuBar
Language:	C++11
Library:	Qt 5.2.1, Standard C++ Library

=========================================================================*/
#include "uiframe_global.h"

#include <QWidget>
#include <qpushbutton.h>

#include "GeneratedFiles\ui_W3ViewMenuBar.h"
#include "../../Common/Common/W3Types.h"
#include "../../Common/Common/W3Enum.h"

#define BTN_SIZE_HEIGHT			22
#define BTN_SIZE_WIDTH			22
#define CBX_SIZE_HEIGHT			19
#define CBX_SIZE_WIDTH			70
#define VIEWMENU_SIZE_HEIGHT	22
#define VIEWMENU_SPACING		0

class UIFRAME_EXPORT CW3ViewMenuBar : public QWidget
{
	Q_OBJECT

public:
	CW3ViewMenuBar(EVIEW_TYPE eViewType, QWidget *parent = 0);
	~CW3ViewMenuBar();

	void setBtnMaximize(W3BOOL bFlag);
	void setVisibleExpandButton(W3BOOL bExpand);
	void setExpandButton(W3BOOL bExpand);

	inline EVIEW_TYPE getViewType(void) const { return m_eViewType; }
	virtual void setViewType(EVIEW_TYPE);

signals:
	void sigSwapWindow(EVIEW_TYPE eThis, EVIEW_TYPE eSWap);
	void sigSetPanning(W3BOOL bFlag);
	void sigSetZooming(W3BOOL bFlag);
	void sigSetMaximize(EVIEW_TYPE);
	void sigReset();
	void sigFit(void);
	void sigSetOverlay(W3BOOL bFlag);
	void sigCapture();
	void sigExpand(W3BOOL);

public slots:
	//btn -> this
	virtual void slotSwapWindow(int);
	void slotSetPanning(void);
	void slotSetZooming(void);
	void slotSetMaximize(void);
	void slotOverlay(void);
	void slotExpand(void);

	//sync button status 
	void slotSyncPanning(W3BOOL);
	void slotSyncZooming(W3BOOL);
	void slotSyncOverlay(W3BOOL);

protected:
	virtual void setConnections(void);
	virtual void setDisconnections(void);

	virtual void displayViewType(void);

protected:
	Ui::W3ViewMenuBar ui;
	EVIEW_TYPE		m_eViewType;
	QHBoxLayout*	m_pLayout;
	QComboBox*      m_pCbxViewType;
private:
	W3BOOL m_bPanning;
	W3BOOL m_bZooming;
	W3BOOL m_bMaximize;
	W3BOOL m_bOverlay;
	W3BOOL m_bExpand;
};
