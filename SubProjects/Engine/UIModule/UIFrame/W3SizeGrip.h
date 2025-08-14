#pragma once
/**=================================================================================================

Project: 			UIFrame
File:				W3SizeGrip.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-05-29
Last modify:		2017-05-29

 *===============================================================================================**/
#include <QSizeGrip>
#include "uiframe_global.h"

class UIFRAME_EXPORT CW3SizeGrip : public QSizeGrip {
public:
	CW3SizeGrip(QWidget *parent);
	~CW3SizeGrip();

protected:
	virtual void	mouseMoveEvent(QMouseEvent * event);
	virtual void	enterEvent(QEvent * event);
	virtual void	leaveEvent(QEvent * event);
};
