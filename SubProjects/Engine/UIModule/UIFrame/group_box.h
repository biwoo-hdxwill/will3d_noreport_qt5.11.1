#pragma once

/*=========================================================================

File:			class GroupBox
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-04-05
Last modify:	2016-04-05

=========================================================================*/
#include <QFrame>

#include <Engine/UIModule/UITools/tool_box.h>

#include "uiframe_global.h"

class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;
class QLabel;

class UIFRAME_EXPORT GroupBox : public ToolBox
{
	Q_OBJECT
public:
	GroupBox(QFrame* parent = 0);
	~GroupBox();

public:
	void SetCaptionName(const QString& name, Qt::Alignment align = Qt::AlignLeft);
	void SetContentsMargins(int left, int top, int right, int bottom);
	void AddWidget(QWidget* widget);
	void AddLayout(QVBoxLayout* layout);
	void AddLayout(QHBoxLayout* layout);
	void AddLayout(QGridLayout* layout);
};
