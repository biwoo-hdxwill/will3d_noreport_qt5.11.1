#pragma once
/**=================================================================================================

Project: 			UITools
File:				si_task_tool.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-10-02
Last modify:		2018-10-02

 *===============================================================================================**/
#include <Engine/Common/Common/W3Enum.h>

#include "base_tool.h"
#include "uitools_global.h"

class QToolButton;
class QRadioButton;
class ToolBox;

class UITOOLS_EXPORT SITaskTool : public BaseTool {
	Q_OBJECT

public:
	explicit SITaskTool(QObject* parent = nullptr);
	virtual ~SITaskTool();

	SITaskTool(const SITaskTool&) = delete;
	SITaskTool& operator=(const SITaskTool&) = delete;

signals:
	void sigSITask(const SITaskID& task_id);
	void sigSIVisible(const SIVisibleID& visible_id, bool on);

public:
	virtual void ResetUI() override;

	void SetEnableSecondVolume(bool enable);

	QWidget* GetTaskWidget();
	QWidget* GetVisibilityWidget();

private:
	virtual void CreateUI() override;
	virtual void SetToolTips() override;
	virtual void Connections() override;

private slots:
	void slotTaskLoadNew();
	void slotTaskAutoReg();

	void slotVisibleMain(bool on);
	void slotVisibleSecond(bool on);
	void slotVisibleBoth(bool on);

private:
	std::unique_ptr<ToolBox> task_tool_box_ ;
	std::unique_ptr<ToolBox> visible_tool_box_;

	std::unique_ptr<QToolButton> task_[SITaskID::SI_TASK_END];
	std::unique_ptr<QRadioButton> visible_[SIVisibleID::SI_VISIBLE_END];
};
