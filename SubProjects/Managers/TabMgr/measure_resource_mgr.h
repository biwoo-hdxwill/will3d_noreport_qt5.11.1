#pragma once
/*=========================================================================

Project:		TabMgr
File:			measure_resource_mgr.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-10-31
Last date:		2018-10-31

Copyright (c) 2018 HDXWILL. All rights reserved.

=========================================================================*/
#include <vector>
#include <map>
#include <memory>
#include <qpoint.h>
#include <QTransform>
#include <QObject>
#include <Engine/Common/Common/W3Enum.h>
#include <Engine/Common/Common/define_measure.h>

class MeasureResource;
class MeasureData;
#ifndef WILL3D_VIEWER
class ProjectIOMeasureResource;
#endif

class MeasureResourceMgr : public QObject
{
	Q_OBJECT

public:
	MeasureResourceMgr(QObject *parent = nullptr);
	~MeasureResourceMgr();

public:
	void Reset();
	void SetCurrentTab(const TabType& tab_type);
	void GetMeasureDatas(common::measure::MeasureDataContainer* measure_datas);
	TabType GetTabType(const common::ViewTypeID & view_type);

#ifndef WILL3D_VIEWER
	void ExportProject(ProjectIOMeasureResource& out);
	void ImportProject(ProjectIOMeasureResource& in);
#endif

public slots:
	void slotMeasureCreate(const common::ViewTypeID& view_type,
		const common::measure::MeasureType& measure_type,
		const common::measure::VisibilityParams& vp,
		const float& pixel_pitch, const float& scale,
		std::weak_ptr<MeasureData>* w_measure_data);
	void slotMeasureDelete(const common::ViewTypeID& view_type,
		const unsigned int& measure_id);
	void slotMeasureDeleteAll(const common::ViewTypeID& view_type);
	void slotMeasureSetNote(const common::ViewTypeID& view_type,
		const unsigned int& measure_id,
		const QString & note);
	void slotMeasureModify(const common::ViewTypeID & view_type,
		const unsigned int & measure_id,
		const QString & value,
		const std::vector<QPointF>& points);
	void slotMeasureChangeMemo(const common::ViewTypeID & view_type,
		const unsigned int & measure_id,
		const QString & memo);
	void slotGetMeasureList(const common::ViewTypeID& view_type,
		std::vector<std::weak_ptr<MeasureData>>* measure_list);
	void slotGetMeasureData(const common::ViewTypeID& view_type,
		const unsigned int& measure_id,
		std::weak_ptr<MeasureData>* w_measure_data);
	void slotMeasureSetScale(const common::ViewTypeID& view_type, const float& scale);
	void slotMeasureSetZoomFactor(const common::ViewTypeID& view_type, const float& zoom_factor);
	void slotMeasureSetPixelPitch(const common::ViewTypeID& view_type, const float& pixel_pitch);
	void slotMeasureSetSceneCenter(const common::ViewTypeID& view_type, const QPointF& center);
	void slotMeasureSetSceneTrans(const common::ViewTypeID& view_type, const QPointF& trans);
	void slotMeasureSetSceneTransform(const common::ViewTypeID& view_type, const QTransform& trans);

	void slotMeasureCounterpartViewInfo(const common::ViewTypeID& view_type,
		common::measure::ViewInfo* view_info);
	void slotMeasureCurrentViewInfo(const common::ViewTypeID& view_type,
		common::measure::ViewInfo* view_info);
	void slotSetCounterpartTab(const TabType tab_type);
	void slotSetCounterpartAsCurrentTab();

private:
	void SetConnections();
	TabType GetCounterpartTab();

private:
	typedef std::pair<TabType, common::ViewTypeID> KeyViewInfo;

private:
	TabType counterpart_mpr_ = TabType::TAB_UNKNOWN;
	TabType counterpart_pano_ = TabType::TAB_UNKNOWN;
	TabType counterpart_mpr_for_export_project_ = TabType::TAB_UNKNOWN;
	TabType counterpart_pano_for_export_project_ = TabType::TAB_UNKNOWN;
	TabType curr_tab_ = TabType::TAB_UNKNOWN;
	std::map<KeyViewInfo, common::measure::ViewInfo> view_infos_;
	std::unique_ptr<MeasureResource> res_measure_;
};
