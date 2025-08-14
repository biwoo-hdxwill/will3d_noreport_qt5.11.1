#pragma once
/*=========================================================================

File:		class CW3ViewMenuBarVR
Language:	C++11
Library:	Qt 5.2.1, Standard C++ Library

=========================================================================*/
#include "uiframe_global.h"
#include "W3ViewMenuBar.h"

class UIFRAME_EXPORT CW3ViewMenuBarVR : public CW3ViewMenuBar
{
	Q_OBJECT

public:
	CW3ViewMenuBarVR(EVIEW_TYPE eViewType, QWidget *parent = 0);
	~CW3ViewMenuBarVR();

	void setViewType(EVIEW_TYPE eType);

public slots:
	virtual void slotSwapWindow(int);

protected:
	virtual void setConnections(void);
	virtual void setDisconnections(void);
	virtual void displayViewType(void);

private:
};
