#pragma once
/**=================================================================================================

Project: 			UITools
File:				visibility_tool_box.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-09-18
Last modify:		2018-09-18

 *===============================================================================================**/
#include <Engine/Common/Common/W3Enum.h>
#include "base_tool.h"
#include "uitools_global.h"

class QToolButton;
class QCheckBox;
class QLineEdit;
class QSlider;
class QLabel;
class QWidget;
class ToolBox;

class UITOOLS_EXPORT VisibilityToolBox : public BaseTool
{
	Q_OBJECT

public:
	explicit VisibilityToolBox(bool nerve_on, bool implant_on,
		bool second_on, bool airway_on,
		bool face_on, QObject* parent = nullptr);
	virtual ~VisibilityToolBox();

	VisibilityToolBox(const VisibilityToolBox&) = delete;
	VisibilityToolBox& operator=(const VisibilityToolBox&) = delete;

signals:
	void sigVisible(const VisibleID&, int state);
	void sigChangeFaceTransparency(int);
	void sigAdjustImplantButtonToggled(bool checked);

public:
	virtual void ResetUI() override;
	void SetAirwaySize(float size);
	void EnableFaceUI(bool enable);
	void EnableAirwayUI(bool enable);
	void SetAdjustImplantButtonVisible(const bool visible);

	void SyncVisibilityResources();

	void SyncVisibilityEnable(const VisibleID& visible_id, const bool& enable);
	void SyncVisibilityChecked(const VisibleID& visible_id, const bool& checked);
	void SetVisibleResource(const VisibleID& visible_id, const bool& visible);

	bool IsEnable(const VisibleID& visible_id) const;
	bool IsChecked(const VisibleID& visible_id) const;

	QWidget* GetWidget();

private:
	virtual void CreateUI() override;
	virtual void Connections() override;
	virtual void SetToolTips() override;

	bool IsResourceEnable(const VisibleID& id);

private slots:
	void slotStateChangeAirway(int state);
	void slotStateChangeFace(int state);
	void slotStateChangeSecondVol(int state);
	void slotStateChangeNerve(int state);
	void slotStateChangeImplant(int state);

private:
	bool active_option_[VisibleID::VISIBLE_END];
	std::unique_ptr<QCheckBox> options_[VisibleID::VISIBLE_END];
	std::unique_ptr<QLineEdit> airway_size_;
	std::unique_ptr<QSlider> face_transparency_;
	std::unique_ptr<QLabel> lbl_face_;

	std::unique_ptr<ToolBox> tool_box_;

	QToolButton* adjust_implant_button_ = nullptr;
};
