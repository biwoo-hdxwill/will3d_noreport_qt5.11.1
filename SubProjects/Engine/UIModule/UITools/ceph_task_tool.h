#pragma once
/**=================================================================================================

Project: 			UITools
File:				ceph_task_tool.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-09-27
Last modify:		2018-09-27

 *===============================================================================================**/
#include <Engine/Common/Common/W3Enum.h>

#include "datatypes.h"
#include "base_tool.h"
#include "uitools_global.h"

class QToolButton;
class ClippingTool;
class ToolBox;
class VisibilityToolBox;

class UITOOLS_EXPORT CephTaskTool : public BaseTool {
	Q_OBJECT

private:
	enum ClipID {
		AXIAL = 0,
		CORONAL,
		SAGITTAL,
		CLIP_END
	};
public:
	explicit CephTaskTool(QObject* parent = nullptr);
	virtual ~CephTaskTool();

	CephTaskTool(const CephTaskTool&) = delete;
	CephTaskTool& operator=(const CephTaskTool&) = delete;

signals:
	void sigCephTask(const CephTaskID& task_id);
	void sigCephClipParamsChanged();
	void sigCephClipSet();
	void sigCephVisible(const VisibleID&, int);
	void sigCephChangeFaceTransparency(int);

public:
	virtual void ResetUI() override;
	ClipStatus GetClipStatus();
	bool IsVisible(const VisibleID& id);

	void SyncTaskUI(const CephTaskID&, bool state);
	void SyncTaskEventLeave(const CephTaskID&);
	void SyncVisibilityEnable(const VisibleID& visible_id, const bool& enable);
  void SyncVisibilityResources();
	void SetClipUpper(int value);
	void SetClipLower(int value);
	void SetClipParams(const bool & isEnable, const bool & isFlip,
					   const MPRClipID & plane, const int & lower, const int & upper);

	QWidget* GetTaskWidget();
	QWidget* GetClipWidget();
	QWidget* GetVisibilityWidget();

private:
	virtual void CreateUI() override;
	virtual void SetToolTips() override;
	virtual void Connections() override;

private slots:
	void slotTaskCoordSys();
	void slotTaskTracing();
	void slotTaskSelectMethod();
	void slotTaskSurgery();
	void slotTaskShowSkin();

private:
	std::unique_ptr<ToolBox> task_tool_box_;
	std::unique_ptr<ClippingTool> clip_tool_;
	std::unique_ptr<VisibilityToolBox> visibility_tool_;

	std::unique_ptr<QToolButton> task_[CephTaskID::CEPH_TASK_END];
};
