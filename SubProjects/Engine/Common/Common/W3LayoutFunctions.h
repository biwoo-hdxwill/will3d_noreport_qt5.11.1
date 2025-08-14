#pragma once
/**=================================================================================================

Project: 			Common
File:				CW3LayoutFunctions.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-07-11
Last modify:		2017-07-11

 *===============================================================================================**/
#include <QLayout>
#include "common_global.h"

class COMMON_EXPORT CW3LayoutFunctions {
public:
	static void setVisibleWidgets(QLayout* layout, bool isVisible);

	//Layout 안에 있는 가장 하위의 Widget까지 재귀적으로 모두 뺀다.
	static void RemoveWidgetsAll(QLayout * layout);
	//현재 Layout에 있는 Widget만 뺀다.
	static void RemoveWidgets(QLayout * layout);
};
