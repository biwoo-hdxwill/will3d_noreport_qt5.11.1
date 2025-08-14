#pragma once
/**=================================================================================================

Project: 			UITools
File:				tool_factory.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-09-17
Last modify:		2018-09-17

 *===============================================================================================**/
#include <QObject>
#include "uitools_global.h"

class OTFTool;

class ToolFactory {

public:
	explicit ToolFactory() {}
	~ToolFactory() {}

	ToolFactory(const ToolFactory&) = delete;
	ToolFactory& operator=(const ToolFactory&) = delete;

	static OTFTool* CreateToolOTF(QObject* parent);
};
