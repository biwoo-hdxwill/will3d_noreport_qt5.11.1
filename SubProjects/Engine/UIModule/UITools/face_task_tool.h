#pragma once
/**=================================================================================================

Project: 			UITools
File:				face_task_tool.h
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
class ToolBox;
class VisibilityToolBox;

class UITOOLS_EXPORT FaceTaskTool : public BaseTool {
	Q_OBJECT

public:
	explicit FaceTaskTool(QObject* parent = nullptr);
	virtual ~FaceTaskTool();

	FaceTaskTool(const FaceTaskTool&) = delete;
	FaceTaskTool& operator=(const FaceTaskTool&) = delete;

signals:
	void sigFaceTask(const FaceTaskID& task_id);
	void sigFaceVisible(const VisibleID&, int);
	void sigFaceChangeFaceTransparency(float);

public:
	virtual void ResetUI() override;

	bool IsVisibleFace() const;
	void SyncTaskUI(const FaceTaskID&, bool state);
	void SyncVisibilityEnable(const bool& enable);

	void EnableFaceUI();

	QWidget* GetTaskWidget();
	QWidget* GetVisibilityWidget();

private:
	virtual void CreateUI() override;
	virtual void SetToolTips() override;
	virtual void Connections() override;

private slots:
	void slotTaskCompare();
	void slotTaskLoad();
	void slotTaskGenerate();
	void slotTaskClear();
	void slotTaskMapping();

	void slotTransparencyChanged(int value);
	
private:
	std::unique_ptr<ToolBox> task_tool_box_;
	std::unique_ptr<VisibilityToolBox> visibility_tool_;

	std::unique_ptr<QToolButton> task_[FaceTaskID::FACE_TASK_END];
};
