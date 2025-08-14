#pragma once

/**=================================================================================================

Project:		TabMgr
File:			base_tab.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-11-20
Last modify: 	2018-11-20

		Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <QObject>
#include <memory>

#include <Engine/Common/Common/define_view.h>

#include "tabmgr_global.h"

class QLayout;
class BaseViewMgr;

class TABMGR_EXPORT BaseTab : public QObject
{
	Q_OBJECT
public:
	explicit BaseTab(QObject* parent = 0);
	virtual ~BaseTab(void);
	BaseTab(const BaseTab&) = delete;
	BaseTab& operator=(const BaseTab&) = delete;

signals:
	void sigCommonToolCancelSelected();

#ifdef WILL3D_EUROPE
	void sigShowButtonListDialog(const QPoint& global_pos, const int mpr_type = -1);
#endif // WILL3D_EUROPE

public:
	virtual void SetVisibleWindows(bool isVisible) = 0;
	virtual void SetApplicationUIMode(const bool& is_maximize) {}

	/*
	  SetCommonToolOnce(), SetCommonToolOnOff(), DeleteMeasureUI()
	  는 구형 뷰 및 뷰 매니저가 구현되면 virtual 이 없어져야 함
	*/
	virtual void SetCommonToolOnce(const common::CommonToolTypeOnce& type, bool on);
	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type);

	virtual void DeleteMeasureUI(const common::ViewTypeID& view_type,
		const unsigned int& measure_id);

	virtual void ApplyPreferences();

	virtual QLayout* GetTabLayout();
	void SetRenderModeQuality(bool is_high_quality);

	virtual QStringList GetViewList();
	virtual QImage GetScreenshot(int view_type);
	virtual QWidget* GetScreenshotSource(int view_type);
	virtual void MoveViewsToSelectedMeasure(const common::ViewTypeID& view_type,
		const unsigned int& measure_id);
	inline bool initialized() const noexcept { return initialized_; }

	inline void set_tab_changed(const bool changed) { tab_changed_ = changed; }
	inline const bool tab_changed() { return tab_changed_; }

#ifdef WILL3D_EUROPE
	virtual void SyncControlButtonOut() {};
	virtual void SyncToggleOut() {};
#endif // WILL3D_EUROPE

protected:
	virtual void SetLayout() = 0;
	virtual void Initialize() = 0;

	inline void set_initialized(bool is_init) { initialized_ = is_init; }
	void SetCastedViewMgr(const std::shared_ptr<BaseViewMgr>& view_mgr);

	QImage GetScreenshot(QWidget* source = nullptr);  // if (source == nullptr)
													  // return full screenshot
	QLayout* tab_layout() const { return tab_layout_; }

private:
	QImage GetScreenshot(int x, int y, int w, int h);

protected:
	QLayout* tab_layout_ = nullptr;
	bool initialized_ = false;

	static int kLayoutSpacing;

	bool tab_changed_ = false;

private:
	std::weak_ptr<BaseViewMgr> view_mgr_;
};
