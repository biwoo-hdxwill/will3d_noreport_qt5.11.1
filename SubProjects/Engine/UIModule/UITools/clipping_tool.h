#pragma once
/**=================================================================================================

Project: 			UITools
File:				clipping_tool.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-09-18
Last modify:		2018-09-18

 *===============================================================================================**/
#include <vector>
#include "base_tool.h"
#include "uitools_global.h"

class QButtonGroup;
class QCheckBox;
class QLayout;
class QRadioButton;
class QVBoxLayout;
class QWidget;

class ToolBox;
class CW3SpanSlider;

class UITOOLS_EXPORT ClippingTool : public BaseTool {
	Q_OBJECT

public:
	enum DirectionType {
		VERTICAL, GRID
	};

public:
	explicit ClippingTool(const std::vector<QString>& mode_names,
						  const DirectionType& direction,
						  bool using_toolbox = true,
						  QObject* parent = nullptr);
	virtual ~ClippingTool();

	ClippingTool(const ClippingTool&) = delete;
	ClippingTool& operator=(const ClippingTool&) = delete;

signals:
	void sigEnable(int);
	void sigRangeMove(int, int);
	void sigRangeSet();
	void sigPlaneChanged(int);
	void sigFlip(int);

public:
	virtual void ResetUI() override;
	void SetEnable(bool enable);
	int GetClipPlaneID();
	bool IsClipEnable();
	bool IsFlip();
	int GetLowerValue() const;
	int GetUpperValue() const;
	void SetLowerValue(const int& value);
	void SetUpperValue(const int& value);

	void SetClipParams(const bool& is_enable, const bool& is_flip,
					   const int& clip_id, const int& lower, const int& upper);

	QWidget* GetWidget();
	// Tool box를 사용하지 않고 그 안의 레이아웃만 가져오는 함수
	QLayout* GetLayoutOnly();

private:
	virtual void CreateUI() override;
	virtual void Connections() override;
	virtual void SetToolTips() override;
	void SetEnableUI(bool is_enable);

private slots:
	void slotFlip(int state);
	void slotPlaneChanged();
	void slotClipEnable(int state);

private:
	DirectionType direction_;
	std::unique_ptr<QButtonGroup> mode_group_;
	std::vector<std::unique_ptr<QRadioButton>> modes_;

	std::unique_ptr<QCheckBox> enable_;
	std::unique_ptr<QCheckBox> flip_;
	std::unique_ptr<CW3SpanSlider> slider_;

	std::unique_ptr<ToolBox> clip_tool_box_;
	std::unique_ptr<QVBoxLayout> clipping_layout_;
};
