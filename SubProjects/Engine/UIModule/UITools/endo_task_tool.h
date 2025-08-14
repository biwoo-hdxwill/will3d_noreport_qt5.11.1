#pragma once
/**=================================================================================================

Project: 			UITools
File:				endo_task_tool.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-10-03
Last modify:		2018-10-03

 *===============================================================================================**/
#include <Engine/Common/Common/W3Enum.h>

#include "base_tool.h"
#include "uitools_global.h"

class QCheckBox;
class QRadioButton;
class QSlider;
class QToolButton;

class ToolBox;
class VisibilityToolBox;

class UITOOLS_EXPORT EndoTaskTool : public BaseTool {
	Q_OBJECT

public:
	explicit EndoTaskTool(QObject* parent = nullptr);
	virtual ~EndoTaskTool();

	EndoTaskTool(const EndoTaskTool&) = delete;
	EndoTaskTool& operator=(const EndoTaskTool&) = delete;

signals:
	void sigEndoPlayerAction(const EndoPlayerID& id, QPrivateSignal);
	void sigEndoPlayerParam(const EndoPlayerParamID& param_id, int value, QPrivateSignal);
	void sigEndoFreeOnOff(bool, QPrivateSignal);
	void sigEndoCamDir(const EndoCameraDir& dir, QPrivateSignal);
	void sigEndoSelectPath(const EndoPathID& path_id, QPrivateSignal);
	void sigEndoRemovePath(const EndoPathID& path_id, QPrivateSignal);

	void sigEndoShowPath(int state, QPrivateSignal);
	void sigEndoVisible(int state, QPrivateSignal);

public:
	virtual void ResetUI() override;
	void SyncSetPath(const EndoPathID& path_id, bool checked);
	void SetAirwaySize(float size);
	void SetAirwayEnable(bool is_draw);

	QWidget* GetPlayerWidget();
	QWidget* GetPathWidget();
	QWidget* GetVisibilityWidget();

public slots:
	void slotEnableRemovePath(const int path_num, bool enable);
	void slotToggleFreeExplorer(bool state);

private:
	virtual void CreateUI() override;
	virtual void SetToolTips() override;
	virtual void Connections() override;

private:
	std::unique_ptr<ToolBox> player_tool_box_;
	std::unique_ptr<ToolBox> path_tool_box_;
	std::unique_ptr<VisibilityToolBox> visibility_tool_;

	std::unique_ptr<QToolButton> player_[EndoPlayerID::PLAYER_END];
	std::unique_ptr<QRadioButton> direction_[EndoCameraDir::ENDO_CAM_END];
	std::unique_ptr<QSlider> player_param_[EndoPlayerParamID::PLAYER_PARAM_END];
	std::unique_ptr<QToolButton> free_explorer_;

	std::unique_ptr<QRadioButton> path_[EndoPathID::ENDO_PATH_END];
	std::unique_ptr<QToolButton> remove_path_[EndoPathID::ENDO_PATH_END];
	std::unique_ptr<QCheckBox> show_path_;
};
