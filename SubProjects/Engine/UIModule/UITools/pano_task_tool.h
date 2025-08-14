#pragma once
/**=================================================================================================

Project: 			UITools
File:				pano_task_tool.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-09-20
Last modify:		2018-09-20

 *===============================================================================================**/
#include <memory>
#include <Engine/Common/Common/W3Enum.h>
#include "base_pano_tool.h"
#include "orientation_tool.h"
#include "uitools_global.h"

class QToolButton;
class QRadioButton;
class QAction;
class NerveToolBox;
class ToolBox;
//20250123 LIN 주석처리
//#ifndef WILL3D_VIEWER
class ProjectIOPanoEngine;
//#endif

class UITOOLS_EXPORT PanoTaskTool : public BasePanoTaskTool
{
	Q_OBJECT

private:
	enum class AutoArchMenu
	{
		RESET,
		END
	};

public:
	explicit PanoTaskTool(QObject* parent = nullptr);
	virtual ~PanoTaskTool();

	PanoTaskTool(const PanoTaskTool&) = delete;
	PanoTaskTool& operator=(const PanoTaskTool&) = delete;

signals:
	void sigPanoReorient();
	void sigPanoReorientReset();
	void sigPanoOrientRotate(const ReorientViewID&, int);
	void sigPanoTask(const PanoTaskID&);

	void sigPanoNerveParamsChanged(const bool& nerve_visible, const int& nerve_id,
		const float& diameter, const QColor& color);
	void sigPanoNerveRecordHovered(int nerve_id, bool is_hovered);
	void sigPanoNerveToggledDraw(bool is_activated);
	void sigPanoNerveDelete(int nerve_id);

	void sigPanoArchTypeChanged(const ArchTypeID&);
	void sigTaskResetAutoArchSliceNumber();

public:
	void InitExternUIs(const OrientationTool::OrientUI& orient_ui,
		const CrossSectionUI& cs_ui, const PanoUI& pano_ui);

	void SetPanoArchTypeRadioUI(const ArchTypeID& arch_type);
//20250123 LIN importProject viewer에 적용
//#ifndef WILL3D_VIEWER
	void ImportProject(ProjectIOPanoEngine& in);
//#endif

	virtual void ResetUI() override;
	virtual void Connections() override;

	bool IsTaskManualOn() const;
	void TaskManualDeactivate();
	void ResetOrientDegreesUI();
	int GetOrientDegree(const ReorientViewID& view_type) const;
	void SetOrientDegrees(const int & degree_a, const int & degree_r,
		const int & degree_i);
	void SyncOrientDegreeUIOnly(const ReorientViewID& view_type, const int& degree_view);

	void NerveCreated(int nerve_id);
	void NerveDeleted(int nerve_id);
	void NerveToolDeactivate();
	void NerveToolVisibilityChange(bool visible);

	QWidget* GetOrientationWidget();
	QWidget* GetTaskWidget();
	QWidget* GetNerveToolWidget();

private:
	virtual void CreateUI() override;
	virtual void SetToolTips() override;

private slots:
	void slotTaskAutoArch();
	void slotTaskManualArch();
	void slotArchTypeChanged();

private:
	std::unique_ptr<ToolBox> task_tool_box_;
	std::unique_ptr<ToolBox> nerve_tool_box_;

	std::unique_ptr<QRadioButton> arch_type_[ArchTypeID::ARCH_TYPE_END];
	std::unique_ptr<QToolButton> task_[PanoTaskID::PANO_TASK_END];
	std::unique_ptr<OrientationTool> orient_tool_;
	std::unique_ptr<NerveToolBox> nerve_tool_;

	std::unique_ptr<QToolButton> auto_arch_menu_;
	QAction* auto_arch_actions_[static_cast<int>(AutoArchMenu::END)];
};
