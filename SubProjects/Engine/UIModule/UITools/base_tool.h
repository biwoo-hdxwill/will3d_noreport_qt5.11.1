#pragma once
/**=================================================================================================

Project: 			UITools
File:				base_tool.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-09-17
Last modify:		2018-09-17

 *===============================================================================================**/
#include <memory>
#include <QObject>
#include "uitools_global.h"

class QWidget;

class UITOOLS_EXPORT BaseTool : public QObject {
	Q_OBJECT
public:
	explicit BaseTool(QObject* parent = nullptr);
	virtual ~BaseTool();

	BaseTool(const BaseTool&) = delete;
	BaseTool& operator=(const BaseTool&) = delete;

public:
	virtual void ResetUI() = 0;

private:
	virtual void CreateUI() = 0;
	virtual void Connections() = 0;
	virtual void SetToolTips() = 0;
};
