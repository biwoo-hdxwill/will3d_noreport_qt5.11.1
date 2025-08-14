#pragma once
/*=========================================================================

File:		class CW3FloatingMenuBar
Language:	OpenCL 1.1, C++11
Library:	Standard C++ Library

=========================================================================*/
#include <QWidget>
#include "uiframe_global.h"
#include "GeneratedFiles\ui_W3FloatingMenuBar.h"

class UIFRAME_EXPORT CW3FloatingMenuBar : public QWidget
{
	Q_OBJECT

public:
	CW3FloatingMenuBar(QWidget *parent = 0);
	~CW3FloatingMenuBar();

private:
	Ui::CW3FloatingMenuBar ui;
};
