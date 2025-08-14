#pragma once
/**=================================================================================================

Project: 			UITools
File:				otf_tool.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-09-17
Last modify:		2018-09-17

 *===============================================================================================**/
#include <Engine/Common/Common/W3Types.h>
#include "base_tool.h"
#include "uitools_global.h"

class QToolButton;
class QSlider;
class QWidget;
class ToolBox;

class UITOOLS_EXPORT OTFTool : public BaseTool {
	Q_OBJECT
public:
	explicit OTFTool(QObject* parent = nullptr);
	virtual ~OTFTool();

	OTFTool(const OTFTool&) = delete;
	OTFTool& operator=(const OTFTool&) = delete;

public:
	enum OTFPresetID {
		PRESET1 = 0,
		PRESET2,
		PRESET3,
		PRESET4,
		PRESET5,
		PRESET6,
		PRESET7,
		PRESET8,
		PRESET_END
	};

	enum OTFSliderID {
		OPACITY = 0,
		BRIGHTNESS,
		CONTRAST,
		SL_END
	};

signals:
	void sigOTFAuto();
	void sigOTFManualOnOff();
	void sigOTFPreset(const QString& preset);
	void sigOTFAdjust(AdjustOTF& values);
	void sigOTFAdjustDone();

public:
	virtual void ResetUI() override;
	void InitOTFPreset(const QString& curr_preset);
	bool SetOTFButtonStatus(const QString& preset);

	QWidget* GetWidget();

private slots:
	void slotAuto();
	void slotPreset();
	void slotAdjust();

private:
	virtual void CreateUI() override;
	virtual void Connections() override;
	virtual void SetToolTips() override;

private:
	std::unique_ptr<QToolButton> preset_[OTFPresetID::PRESET_END];
	std::unique_ptr<QSlider> slider_[OTFSliderID::SL_END];
	std::unique_ptr<QToolButton> auto_ = nullptr;
	std::unique_ptr<QToolButton> manual_ = nullptr;

	std::unique_ptr<ToolBox> tool_box_ = nullptr;
};
