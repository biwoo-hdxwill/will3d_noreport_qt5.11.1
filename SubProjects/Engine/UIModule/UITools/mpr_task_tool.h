#pragma once
/**=================================================================================================

Project: 			UITools
File:				mpr_task_tool.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-09-17
Last modify:		2018-09-17

 *===============================================================================================**/
#include <Engine/Common/Common/W3Enum.h>

#include "uitools_global.h"

#include "datatypes.h"
#include "base_tool.h"
#include "orientation_tool.h"

class QAction;
class QToolButton;
class QRadioButton;
class ClippingTool;
class VisibilityToolBox;
class ToolBox;

class UITOOLS_EXPORT MPRTaskTool : public BaseTool
{
	Q_OBJECT

private:
	enum ReorientID
	{
		REORIENT = 0,
		RESET,
		REORIENT_END
	};

	enum CutType
	{
		POLYGON = 0,
		FREEDRAW,
		CUT_END
	};

	enum ClipID
	{
		AXIAL = 0,
		CORONAL,
		SAGITTAL,
		MPROVERLAY,
		CLIP_END
	};

signals:
	void sigMPRAutoReorient();
	void sigMPRReorientReset();
	void sigMPRTask(const MPRTaskID&, bool);
	void sigMPRVisible(const VisibleID&, int);
	void sigMPRChangeFaceTransparency(int);
	void sigMPRClipEnable(int);
	void sigMPRClipRangeMove(int, int);
	void sigMPRClipRangeSet();
	void sigMPRClipPlaneChanged(const MPRClipID&);

	void sigAdjustOrientationClicked();
	void sigRotateOrientation(const ReorientViewID&, int);

	void sigFlipClipping(int);

	void sigAdjustImplantButtonToggled(bool checked);

public:
	explicit MPRTaskTool(QObject* parent = nullptr);
	virtual ~MPRTaskTool();

	MPRTaskTool(const MPRTaskTool&) = delete;
	MPRTaskTool& operator=(const MPRTaskTool&) = delete;

	void InitExternUIs(const OrientationTool::OrientUI& orient_ui);

	void ResetOrientationDegreesUI();
	int GetOrientationDegree(const ReorientViewID& view_type) const;
	void SetOrientationDegrees(const int& degree_a, const int& degree_r, const int& degree_i);
	void SyncOrientationDegreeUIOnly(const ReorientViewID& view_type, const int& degree_view);

	virtual void ResetUI() override;
	void SetLightboxOn(bool on);
	ClipStatus GetClipStatus();

	inline VRCutTool cut_tool() const noexcept { return cut_tool_; }
	void SetAirwaySize(float size);
	void SetAirwayEnable(bool is_enable);
	bool IsZoom3DOn() const;
	bool IsClipOn() const;
	void SyncTaskUI(const MPRTaskID&, bool state);
	void SyncVisibilityResources();
	void SyncVisibilityEnable(const VisibleID& visible_id, const bool& enable);
	void SyncVisibilityChecked(const VisibleID& visible_id, const bool& checked);

	QWidget* GetOrientationWidget();
	QWidget* GetTaskWidget();
	QWidget* GetClipWidget();
	QWidget* GetVisibilityWidget();
	const bool lightbox_on() const noexcept { return lightbox_on_; }

	virtual void Connections() override;

	const ArchTypeID GetArchType();

private:
	virtual void CreateUI() override;
	virtual void SetToolTips() override;
	void ResetOtherTaskButtons(const MPRTaskID&);
	void SetCheckCut3DButton();

private slots:
	void slotTaskZoom3D(bool);
	void slotTaskCut3D(bool);
	void slotTaskCut3DPolygon();
	void slotTaskCut3DFreedraw();
	void slotTaskOblique(bool);
	void slotTaskSTLExport();
	void slotTaskDrawArch();

	void slotClipPlaneChanged(int);

private:
	bool lightbox_on_ = false;

	bool use_auto_orientation_ = true;
	bool use_draw_arch_ = false;

	std::unique_ptr<QToolButton> reorient_[ReorientID::REORIENT_END];
	std::unique_ptr<OrientationTool> orientation_tool_;
	std::unique_ptr<QToolButton> task_[MPRTaskID::MPR_TASK_END];
	std::unique_ptr<QRadioButton> arch_type_[ArchTypeID::ARCH_TYPE_END];
	std::unique_ptr<QToolButton> cut_menu_;

	std::unique_ptr<ClippingTool> clip_tool_;
	std::unique_ptr<VisibilityToolBox> visibility_tool_;

	QAction* cut_actions_[CutType::CUT_END];
	VRCutTool cut_tool_ = VRCutTool::POLYGON;
	std::unique_ptr<ToolBox> orient_tool_box_;
	std::unique_ptr<ToolBox> task_tool_box_;
};
