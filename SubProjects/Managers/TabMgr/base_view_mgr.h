#pragma once

/**=================================================================================================

Project:		TabMgr
File:			base_view_mgr.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-11-20
Last modify: 	2018-11-20

		Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <memory>
#include <vector>

#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif

#include <QObject>
#include "../../Engine/Common/Common/W3Enum.h"
#include "../../Engine/Common/Common/W3Define.h"
#include "../../Engine/Common/Common/define_view.h"
#include "tabmgr_global.h"

namespace common 
{
	namespace measure 
	{
		struct VisibilityParams;
	}  // end of namespace measure
}  // end of namespace common

class View;
class TABMGR_EXPORT BaseViewMgr : public QObject 
{
	Q_OBJECT
public:
	explicit BaseViewMgr(QObject* parent = nullptr);
	virtual ~BaseViewMgr();

	BaseViewMgr(const BaseViewMgr&) = delete;
	BaseViewMgr& operator=(const BaseViewMgr&) = delete;

signals:
#ifdef WILL3D_EUROPE
	void sigShowButtonListDialog(const QPoint& global_pos, const int mpr_type = -1);
#endif // WILL3D_EUROPE

public:
	virtual void SetRenderModeQuality(bool is_high_quality);
	virtual void SetVisibleViews(bool visible);
	// common menu bar와 연결되는 동작들
	void SetCommonToolOnce(const common::CommonToolTypeOnce& type, bool on);
	void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type);

	virtual void ApplyPreferences();

	virtual void SyncMeasureResource() {};
	virtual void MoveViewsToSelectedMeasure(const common::ViewTypeID& view_type, const unsigned int& measure_id) {}
	void DeleteMeasureUI(const common::ViewTypeID& view_type, const unsigned int& measure_id);
	void GetMeasureParamsInView(const common::ViewTypeID& view_type, const unsigned int& measure_id, common::measure::VisibilityParams* visibility_params);

protected:
	virtual void InitializeViews() = 0;
	void SetCastedView(unsigned int view_id, const std::shared_ptr<View>& view);

private:
	/** The caseted views. key: view_id*/
	std::map<unsigned int, std::weak_ptr<View>> casted_views_;
};
