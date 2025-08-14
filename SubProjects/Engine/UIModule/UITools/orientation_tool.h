#pragma once
/**=================================================================================================

Project: 			UITools
File:				orientation_tool.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-11-23
Last modify:		2018-11-23

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include "base_tool.h"
#include <Engine/Common/Common/W3Enum.h>
#include "uitools_global.h"

class QToolButton;
class QSpinBox;
class ToolBox;

class UITOOLS_EXPORT OrientationTool : public BaseTool {
	Q_OBJECT

public:
	explicit OrientationTool(QObject* parent = nullptr);
	virtual ~OrientationTool();

	OrientationTool(const OrientationTool&) = delete;
	OrientationTool& operator=(const OrientationTool&) = delete;

public:
	struct OrientUI {
		QSpinBox *a = nullptr;
		QSpinBox *r = nullptr;
		QSpinBox *i = nullptr;
	};

public:
	virtual void ResetUI() override;
	void InitExternUIs(const OrientationTool::OrientUI& orient_ui);
	void SetOrientDegrees(const int& degree_a, const int& degree_r, const int& degree_i);
	int GetOrientDegree(const ReorientViewID& view_type) const;
	void ResetOrientDegreesUI();
	void SyncOrientDegreeUIOnly(const ReorientViewID& view_type, const int& degree_view);

	QWidget* GetWidget();
	virtual void Connections() override;

signals:
	void sigReorient();
	void sigReorientReset();
	void sigOrientRotate(const ReorientViewID& orient_id, const int& angle);

private:
	enum ReorientID {
		REORIENT = 0,
		RESET,
		REORIENT_END
	};

private:
	virtual void CreateUI() override;
	virtual void SetToolTips() override;

private:
	std::unique_ptr<ToolBox> tool_box_;

	std::unique_ptr<QToolButton> reorient_[ReorientID::REORIENT_END];
	QSpinBox* orient_[ReorientViewID::REORI_VIEW_END]; // 외부에서 생성되어 포인터만 세팅됨
};
