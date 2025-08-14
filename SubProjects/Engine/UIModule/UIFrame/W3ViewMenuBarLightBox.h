#pragma once
/*=========================================================================

File:		class CW3ViewMenuBar
Language:	C++11
Library:	Qt 5.2.1, Standard C++ Library
Author:		kys

=========================================================================*/
#include "uiframe_global.h"
#include "W3ViewMenuBar.h"

class UIFRAME_EXPORT CW3ViewMenuBarLightBox : public CW3ViewMenuBar
{
	Q_OBJECT

public:
	CW3ViewMenuBarLightBox(EMPR_VIEW_TYPE eViewType, QWidget *parent = 0);
	~CW3ViewMenuBarLightBox();

	void setViewType(EVIEW_TYPE);

public slots:
	virtual void slotSwapWindow(int);
	
protected:
	virtual void displayViewType(void);	
};
