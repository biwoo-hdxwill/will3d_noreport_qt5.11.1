#pragma once

/*=========================================================================

File:			class AboutDialog
Language:		C++11
Library:        Qt 5.8.0
Author:			Jung Dae Gun
First date:		2018-05-09
Last modify:	2016-05-09

=========================================================================*/

#include <Engine/Common/Common/W3Dialog.h>

#include "uiframe_global.h"

class QHBoxLayout;

class UIFRAME_EXPORT AboutDialog : public CW3Dialog
{
	Q_OBJECT

public:
	AboutDialog(QWidget* parent = 0);
	virtual ~AboutDialog();

private:
	void SetLayout();
	QVBoxLayout* CreateContentsLayout();
	QHBoxLayout* CreateButtonLayout();
};
