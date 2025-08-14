#pragma once
/**=================================================================================================

Project: 			UITools
File:				implant_task_tool.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-09-20
Last modify:		2018-09-20

 *===============================================================================================**/
#include <memory>

#include <Engine/Common/Common/W3Enum.h>
#include "base_pano_tool.h"
#include "uitools_global.h"

class QDialog;
class QRadioButton;
class QDoubleSpinBox;
class ToolBox;
class TextEdit;
class CW3Dialog;

class UITOOLS_EXPORT ImplantTaskTool : public BasePanoTaskTool {
	Q_OBJECT

public:
	explicit ImplantTaskTool(QObject* parent = nullptr);
	virtual ~ImplantTaskTool();

	ImplantTaskTool(const ImplantTaskTool&) = delete;
	ImplantTaskTool& operator=(const ImplantTaskTool&) = delete;

signals:
	void sigImplantSagittalRotate(double);
	void sigImplantMemoChanged(const QString& txt);
	void sigImplantBDSyncPopupStatus(bool popup);

public:
	void InitExternUIs(QWidget* bd_ui, QDoubleSpinBox* sagittal_ui,
					   const CrossSectionUI& cs_ui, const PanoUI& pano_ui);
	virtual void ResetUI() override;
	virtual void Connections() override;

	double RotateValue();
	void UpdateRotateAngle(double value);
	void SetMemoText(const QString& txt);

	QWidget* GetMemoWidget();
	QWidget* GetBoneDensityWidget();

private:
	virtual void CreateUI() override;
	virtual void SetToolTips() override;

public slots:
  void slotBoneDensityPopup(bool popup);

private slots:
	void slotImplantMemoChanged();
	void slotBDPopupClosed();

private:
	std::unique_ptr<ToolBox> memo_tool_box_ = nullptr;
	std::unique_ptr<ToolBox> bd_tool_box_ = nullptr;

	std::unique_ptr<TextEdit> implant_memo_;

	QDoubleSpinBox* sagittal_rotate_ = nullptr; // 외부에서 생성되어 포인터만 세팅됨

	std::unique_ptr<QDialog> bone_density_dialog_;
	bool is_bd_popup_ = false;
};
