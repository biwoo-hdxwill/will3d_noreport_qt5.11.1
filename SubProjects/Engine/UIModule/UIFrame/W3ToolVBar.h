#pragma once

/*=========================================================================

File:			class CW3ToolVBar
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-04-05
Last modify:	2016-04-11

=========================================================================*/
#include "uiframe_global.h"

#include <memory>

#include <QScrollArea>

#include "../../Common/Common/W3Enum.h"

class QVBoxLayout;

class UIFRAME_EXPORT CW3ToolVBar : public QScrollArea {

public:
	CW3ToolVBar(QWidget* parent = 0);
	~CW3ToolVBar();

public:
	void setTabTools(const TabType& tab_type);

private:
	std::unique_ptr<QVBoxLayout> main_layout_ = nullptr;
};
