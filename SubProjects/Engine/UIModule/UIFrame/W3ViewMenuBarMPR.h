#pragma once
/*=========================================================================

File:		class CW3ViewMenuBar
Language:	C++11
Library:	Qt 5.2.1, Standard C++ Library

=========================================================================*/
#include "uiframe_global.h"
#include "W3ViewMenuBar.h"

class UIFRAME_EXPORT CW3ViewMenuBarMPR : public CW3ViewMenuBar
{
	Q_OBJECT

public:
	CW3ViewMenuBarMPR(EVIEW_TYPE eViewType, QWidget *parent = 0);
	~CW3ViewMenuBarMPR();

	inline void setLineOnVisible(W3BOOL bVisible) { m_btnLineOnOff->setVisible(bVisible); }

	void setViewType(EVIEW_TYPE);

signals:
	void sigLineOn(W3BOOL);

public slots:
	virtual void slotSwapWindow(int);
	void slotLineOn(void);
	void slotSyncLineOn(W3BOOL);

protected:
	virtual void setConnections(void);
	virtual void setDisconnections(void);
	virtual void displayViewType(void);

private:
	W3BOOL			m_bLineOn;
	QPushButton*	m_btnLineOnOff;
};
