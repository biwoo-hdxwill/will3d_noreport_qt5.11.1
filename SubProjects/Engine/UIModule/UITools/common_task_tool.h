#pragma once
/**=================================================================================================

Project: 			UITools
File:				common_task_tool.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-09-12
Last modify:		2018-09-12

 *===============================================================================================**/
#include <Engine/Common/Common/W3Enum.h>
#include <Engine/Common/Common/define_view.h>
#include "base_tool.h"
#include "uitools_global.h"

class QToolButton;
class QAction;
class QHBoxLayout;
class ToolBox;
class QLayout;

class UITOOLS_EXPORT CommonTaskTool : public BaseTool {
	Q_OBJECT

public:
	enum FileID
	{
		CAPTURE,
		SAVE,
		CD_EXPORT,
		PRINT,
		PACS,
		FID_END
	};

	enum ViewID
	{
		RESET = 0,
		FIT,
		PAN,
		ZOOM,
		LIGHT,
		INVERT,
		//HIDE_TEXT,
		HIDE_UI,
		GRID,
		VID_END
	};

	enum MeasureID
	{
		RULER = 0,
		TAPE,
		ANGLE,
		ANALYSIS,
		AREA,
		//ROI,
		DRAW,
		NOTE,
		HIDEMEASURE,
		DEL,
		IMPLANT_ANGLE,
		MID_END
	};

private:
	enum class ActionTypeTape { LINE, CURVE };
	enum class ActionTypeAnalysis { PROFILE, ROI };
	enum class ActionTypeAnno { ARROW, RECT, CIRCLE, LINE, FREEDRAW };
	enum class ActionTypeDelete { ONE, ALL };

	enum MeasureMenuID {
		M_TAPE = 0,
		M_ANALYSIS,
		M_DRAW,
		M_DELETE,
		MENUID_END
	};

	typedef struct _MeasureMenuActions {
		QAction* tape_line = nullptr;
		QAction* tape_curve = nullptr;

		QAction* analysis_profile = nullptr;
		QAction* analysis_roi = nullptr;
			   
		QAction* anno_arrow = nullptr;
		QAction* anno_rect = nullptr;
		QAction* anno_circle = nullptr;
		QAction* anno_line = nullptr;
		QAction* anno_freedraw = nullptr;
			   
		QAction* del_one = nullptr;
		QAction* del_all = nullptr;
	} MeasureMenuActions;

	typedef struct _Status {
		ActionTypeAnno type_anno = ActionTypeAnno::FREEDRAW;
		ActionTypeAnalysis type_analysis = ActionTypeAnalysis::PROFILE;
		ActionTypeTape type_tape = ActionTypeTape::LINE;
		common::CommonToolTypeOnOff curr_tool = common::CommonToolTypeOnOff::NONE;
		bool tools_enable_ = true;
	} Status;

public:
	explicit CommonTaskTool(QObject *parent = nullptr);
	virtual ~CommonTaskTool();

	CommonTaskTool(const CommonTaskTool&) = delete;
	CommonTaskTool& operator=(const CommonTaskTool&) = delete;

	QToolButton* GetFileButton(FileID id);
	QToolButton* GetViewButton(ViewID id);
	QToolButton* GetMeasureButton(MeasureID id);

signals:
	void sigShowImplantAngle();
	void sigFileTool(const CommonToolTypeFile& type);
	void sigCommonToolOnce(const common::CommonToolTypeOnce& type, bool enable);
	void sigCommonToolOnOff(const common::CommonToolTypeOnOff& type);
	void sigCommonMeasureListOn();

public:
	virtual void ResetUI() override;
	void SetTaskToolOnOff(const common::CommonToolTypeOnOff& type, bool on);
	common::CommonToolTypeOnOff CurrToolType();
	bool RemoveUnfinishiedMeasureTool();
	bool CancelSelectedTool();

	void SetOnlyTRDMode(bool enable);

	void ReleaseOtherButtons(QToolButton* exception = nullptr);
	void EnableAllButtons(bool enable);

	QLayout* GetLayout();

	void SetImplantAngleVisible(const bool visible);

public slots:
	void slotSyncImplantAngleButton();
	void slotTempSyncMenuEvent();

private:
	virtual void CreateUI() override;
	virtual void Connections() override;
	virtual void SetToolTips() override;

	common::CommonToolTypeOnOff GetToolTypeFromTapeType();
	common::CommonToolTypeOnOff GetToolTypeFromAnalysisType();
	common::CommonToolTypeOnOff GetToolTypeFromAnnoType();
	void SetTapeButton(const ActionTypeTape& tape_type);
	void SetAnalysisButton(const ActionTypeAnalysis& analysis_type);
	void SetAnnoButton(const ActionTypeAnno& anno_type);

	void EnableMeasrueButtons(bool enable);

	bool IsSetCurrTool();

private slots:
	void slotFileSaveProject();
	void slotFileCapture();
	void slotFilePrint();
	void slotFileCDExport();
	void slotPACS();

	void slotViewReset();
	void slotViewFit();
	void slotViewPan(bool);
	void slotViewZoom(bool);
	void slotViewLight(bool);
	void slotViewInvert(bool);
	void slotViewHideText(bool);
	void slotViewHideUI(bool);
	void slotViewGrid(bool);

	void slotMeasureRuler(bool);
	void slotMeasureTape(bool);
	void slotMeasureAngle(bool);
	void slotMeasureAnalysis(bool);
	void slotMeasureArea(bool);
	void slotMeasureHide(bool);
	void slotMeasureDraw(bool);
	void slotMeasureNote(bool);
	void slotMeasureImplantAngle();
	void slotMeasureDelete(bool);

	void slotActTapeLine();
	void slotActTapeCurve();
	void slotActAnalysisProfile();
	void slotActAnalysisROI();
	void slotActDrawArrow();
	void slotActDrawRect();
	void slotActDrawCircle();
	void slotActDrawLine();
	void slotActDrawFreedraw();
	void slotActDelOne();
	void slotActDelAll();

private:
	std::unique_ptr<QHBoxLayout> main_layout_;
	std::unique_ptr<QToolButton> file_[FileID::FID_END];
	std::unique_ptr<QToolButton> view_[ViewID::VID_END];
	std::unique_ptr<QToolButton> measure_list_;
	std::unique_ptr<QToolButton> measure_[MeasureID::MID_END];
	std::unique_ptr<QToolButton> measure_menu_[MeasureMenuID::MENUID_END];
	MeasureMenuActions measure_menu_actions_;
	Status status_;
	
	std::unique_ptr<ToolBox> tool_box_file_ = nullptr;
	std::unique_ptr<ToolBox> tool_box_view_ = nullptr;
	std::unique_ptr<ToolBox> tool_box_measure_ = nullptr;
};
