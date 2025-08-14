#include "measure_resource_mgr.h"

#include <QDebug>

#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/event_handle_measure.h>
#include <Engine/Common/Common/event_handler.h>
#include <Engine/Common/Common/will3d_id_parser.h>
#include <Engine/Resource/Resource/include/measure_data.h>
#include <Engine/Resource/Resource/measure_resource.h>
#ifndef WILL3D_VIEWER
#include <Engine/Core/W3ProjectIO/project_io_measure_resource.h>
#endif

MeasureResourceMgr::MeasureResourceMgr(QObject* parent)
	: QObject(parent), res_measure_(new MeasureResource)
{
	SetConnections();
}

MeasureResourceMgr::~MeasureResourceMgr() {}

void MeasureResourceMgr::Reset()
{
	res_measure_.reset(new MeasureResource);
	view_infos_.clear();
	counterpart_mpr_ = TabType::TAB_UNKNOWN;
	counterpart_pano_ = TabType::TAB_UNKNOWN;
	counterpart_mpr_for_export_project_ = TabType::TAB_UNKNOWN;
	counterpart_pano_for_export_project_ = TabType::TAB_UNKNOWN;
	curr_tab_ = TabType::TAB_UNKNOWN;

	Will3DIDParser::ResetMeasureIDs();
}

void MeasureResourceMgr::SetCurrentTab(const TabType& tab_type)
{
	if (
#ifndef WILL3D_VIEWER
		tab_type == TabType::TAB_FILE ||
#endif
		tab_type == TabType::TAB_REPORT)
	{
		return;
	}

	TabType prev_tab = curr_tab_;
	curr_tab_ = tab_type;

	// 占쌔댐옙 counterpart占쏙옙 占쏙옙占쏙옙tab 占쏙옙 占쏙옙占승몌옙 占쏙옙占쏙옙占싼댐옙(a).
	if (prev_tab == TabType::TAB_PANORAMA
#ifndef WILL3D_VIEWER
		|| prev_tab == TabType::TAB_IMPLANT
#endif
		)
	{
		counterpart_pano_ = prev_tab;
	}
	else if (prev_tab == TabType::TAB_MPR
#ifndef WILL3D_LIGHT
		|| prev_tab == TabType::TAB_SI
#endif
		)
	{
		counterpart_mpr_ = prev_tab;
	}

#if 1
	if (curr_tab_ == TabType::TAB_PANORAMA
#ifndef WILL3D_VIEWER
		|| curr_tab_ == TabType::TAB_IMPLANT
#endif
		)
	{
		counterpart_pano_for_export_project_ = curr_tab_;
	}
	else if (curr_tab_ == TabType::TAB_MPR
#ifndef WILL3D_LIGHT
		|| curr_tab_ == TabType::TAB_SI
#endif
		)
	{
		counterpart_mpr_for_export_project_ = curr_tab_;
	}
#endif
}

void MeasureResourceMgr::GetMeasureDatas(
	common::measure::MeasureDataContainer* measure_datas)
{
	res_measure_->GetMeasureDatas(measure_datas);
}

TabType MeasureResourceMgr::GetTabType(const common::ViewTypeID& view_type)
{
	switch (view_type)
	{
	case common::ViewTypeID::MPR_AXIAL:
	case common::ViewTypeID::MPR_SAGITTAL:
	case common::ViewTypeID::MPR_CORONAL:
		if (curr_tab_ == TabType::TAB_MPR || counterpart_mpr_ == TabType::TAB_MPR)
		{
			return TabType::TAB_MPR;
		}
#ifndef WILL3D_LIGHT
		else
		{
			return TabType::TAB_SI;
		}
#endif
		return TabType::TAB_UNKNOWN;
	case common::ViewTypeID::MPR_3D:
	case common::ViewTypeID::MPR_ZOOM3D:
	case common::ViewTypeID::LIGHTBOX:
		return TabType::TAB_MPR;
	case common::ViewTypeID::PANO:
	case common::ViewTypeID::CROSS_SECTION:
	case common::ViewTypeID::PANO_ARCH:
#ifndef WILL3D_VIEWER
		if (curr_tab_ == TabType::TAB_PANORAMA ||
			counterpart_pano_ == TabType::TAB_PANORAMA)
			return TabType::TAB_PANORAMA;
		else
			return TabType::TAB_IMPLANT;
#else
		return TabType::TAB_PANORAMA;
#endif
	case common::ViewTypeID::PANO_ORIENTATION:
		return TabType::TAB_PANORAMA;
#ifndef WILL3D_VIEWER
	case common::ViewTypeID::IMPLANT_SAGITTAL:
	case common::ViewTypeID::IMPLANT_3D:
	case common::ViewTypeID::IMPLANT_PREVIEW:
	case common::ViewTypeID::IMPLANT_BONEDENSITY:
		return TabType::TAB_IMPLANT;
#endif
	case common::ViewTypeID::TMJ_FRONTAL_LEFT:
	case common::ViewTypeID::TMJ_FRONTAL_RIGHT:
	case common::ViewTypeID::TMJ_LATERAL_LEFT:
	case common::ViewTypeID::TMJ_LATERAL_RIGHT:
	case common::ViewTypeID::TMJ_ARCH:
		return TabType::TAB_TMJ;
#ifndef WILL3D_LIGHT
	case common::ViewTypeID::ENDO:
	case common::ViewTypeID::ENDO_MODIFY:
	case common::ViewTypeID::ENDO_SAGITTAL:
	case common::ViewTypeID::ENDO_SLICE:
		return TabType::TAB_ENDO;
	case common::ViewTypeID::SUPERIMPOSITION:
		return TabType::TAB_SI;
	case common::ViewTypeID::CEPH:
	case common::ViewTypeID::FACE_AFTER:
		return TabType::TAB_3DCEPH;
	case common::ViewTypeID::FACE_BEFORE:
	case common::ViewTypeID::FACE_PHOTO:
	case common::ViewTypeID::FACE_SURFACE:
		return TabType::TAB_FACESIM;
#endif
	}
	return TabType::TAB_UNKNOWN;
}
#ifndef WILL3D_VIEWER
void MeasureResourceMgr::ExportProject(ProjectIOMeasureResource& out)
{
	common::measure::MeasureDataContainer measure_datas;
	res_measure_->GetMeasureDatas(&measure_datas);
	int measure_data_cnt = 0;

	for (const auto& data_list : measure_datas)
	{
		for (const auto& data : data_list.second)
		{
			if (data_list.first == common::ViewTypeID::LIGHTBOX) continue;
			out.SaveMeasure(data_list.first, *data.lock().get());
			++measure_data_cnt;
		}
	}
	out.SaveMeasureCount(measure_data_cnt);

	int view_info_cnt = 0;
	for (const auto& info : view_infos_)
	{
		TabType tab_type = info.first.first;
		common::ViewTypeID view_type = info.first.second;
		if (view_type == common::ViewTypeID::LIGHTBOX) continue;
		out.SaveViewParams(tab_type, view_type, info.second);
		++view_info_cnt;
	}
	out.SaveViewParamsCount(view_info_cnt);

#if 0
	out.SaveCounterparts(counterpart_mpr_, counterpart_pano_);
#else
	out.SaveCounterparts(counterpart_mpr_for_export_project_, counterpart_pano_for_export_project_);
#endif
}

void MeasureResourceMgr::ImportProject(ProjectIOMeasureResource& in)
{
	int measure_data_cnt = in.LoadMeasureCount();
	for (int id = 0; id < measure_data_cnt; ++id)
	{
		common::ViewTypeID view_type;
		MeasureData* data;
		in.LoadMeasure(&view_type, data);
		std::shared_ptr<MeasureData> measure_data(data);
		res_measure_->AddMeasureData(view_type, measure_data);
	}

	int view_params_cnt = in.LoadViewParamsCount();
	for (int id = 0; id < view_params_cnt; ++id)
	{
		common::ViewTypeID view_type;
		TabType tab_type;
		common::measure::ViewInfo view_info;
		in.LoadViewParams(tab_type, view_type, view_info);
		KeyViewInfo key(tab_type, view_type);
		view_infos_.insert(
			std::pair<KeyViewInfo, common::measure::ViewInfo>(key, view_info));
		res_measure_->SetMeasurePixelPitch(view_type, view_info.spacing);
		res_measure_->SetMeasureScale(view_type, view_info.scale);
	}

	in.LoadCounterparts(counterpart_mpr_, counterpart_pano_);
}
#endif
void MeasureResourceMgr::slotMeasureCreate(
	const common::ViewTypeID& view_type,
	const common::measure::MeasureType& measure_type,
	const common::measure::VisibilityParams& vp, const float& pixel_pitch,
	const float& scale, std::weak_ptr<MeasureData>* w_measure_data)
{
	std::shared_ptr<MeasureData> measure_data(new MeasureData(
		Will3DIDParser::GetMeasureID(), measure_type, vp, pixel_pitch, scale));

	if (measure_type == common::measure::MeasureType::NOTE)
	{
		measure_data->set_note_id(Will3DIDParser::GetMeasureNoteID());
	}
	else if (measure_type == common::measure::MeasureType::PROFILE)
	{
		measure_data->set_profile_id(Will3DIDParser::GetMeasureProfileID());
	}

	*w_measure_data = measure_data;
	res_measure_->AddMeasureData(view_type, measure_data);
}

void MeasureResourceMgr::slotMeasureDelete(const common::ViewTypeID& view_type,
	const unsigned int& measure_id)
{
	res_measure_->DeleteMeasureData(view_type, measure_id);
}

void MeasureResourceMgr::slotMeasureDeleteAll(
	const common::ViewTypeID& view_type)
{
	res_measure_->DeleteMeasureDatas(view_type);
}

void MeasureResourceMgr::slotMeasureSetNote(const common::ViewTypeID& view_type,
	const unsigned int& measure_id,
	const QString& note)
{
	std::shared_ptr<MeasureData>& measure_data =
		res_measure_->GetMeasureData(view_type, measure_id);
	measure_data->set_note_txt(note);
}

void MeasureResourceMgr::slotMeasureModify(const common::ViewTypeID& view_type,
	const unsigned int& measure_id,
	const QString& value,
	const std::vector<QPointF>& points)
{
	std::shared_ptr<MeasureData>& measure_data =
		res_measure_->GetMeasureData(view_type, measure_id);
	measure_data->set_value(value);
	measure_data->set_points(points);
}

void MeasureResourceMgr::slotMeasureChangeMemo(
	const common::ViewTypeID& view_type, const unsigned int& measure_id,
	const QString& memo)
{
	std::shared_ptr<MeasureData>& measure_data =
		res_measure_->GetMeasureData(view_type, measure_id);
	measure_data->set_memo(memo);
}

void MeasureResourceMgr::slotGetMeasureList(
	const common::ViewTypeID& view_type,
	std::vector<std::weak_ptr<MeasureData>>* measure_list)
{
	res_measure_->GetMeasureList(view_type, measure_list);
}

void MeasureResourceMgr::slotGetMeasureData(
	const common::ViewTypeID& view_type, const unsigned int& measure_id,
	std::weak_ptr<MeasureData>* w_measure_data)
{
	std::shared_ptr<MeasureData>& measure_data =
		res_measure_->GetMeasureData(view_type, measure_id);
	*w_measure_data = measure_data;
}

void MeasureResourceMgr::slotMeasureSetScale(
	const common::ViewTypeID& view_type, const float& scale)
{
	KeyViewInfo key(curr_tab_, view_type);
	auto& view_info_at = view_infos_.find(key);
	if (view_info_at == view_infos_.end())
	{
		common::measure::ViewInfo view_info;
		view_info.scale = scale;
		view_infos_.insert(
			std::pair<KeyViewInfo, common::measure::ViewInfo>(key, view_info));
	}
	else
	{
		view_info_at->second.scale = scale;
	}

	res_measure_->SetMeasureScale(view_type, scale);
}

void MeasureResourceMgr::slotMeasureSetZoomFactor(const common::ViewTypeID& view_type, const float& zoom_factor)
{
	KeyViewInfo key(curr_tab_, view_type);
	auto& view_info_at = view_infos_.find(key);
	if (view_info_at == view_infos_.end())
	{
		common::measure::ViewInfo view_info;
		view_info.zoom_factor = zoom_factor;
		view_infos_.insert(std::pair<KeyViewInfo, common::measure::ViewInfo>(key, view_info));
	}
	else
	{
		view_info_at->second.zoom_factor = zoom_factor;
	}
}

void MeasureResourceMgr::slotMeasureSetPixelPitch(
	const common::ViewTypeID& view_type, const float& pixel_pitch)
{
	KeyViewInfo key(curr_tab_, view_type);
	auto& view_info_at = view_infos_.find(key);
	if (view_info_at == view_infos_.end())
	{
		common::measure::ViewInfo view_info;
		view_info.spacing = pixel_pitch;
		view_infos_.insert(
			std::pair<KeyViewInfo, common::measure::ViewInfo>(key, view_info));
	}
	else
	{
		view_info_at->second.spacing = pixel_pitch;
	}

	res_measure_->SetMeasurePixelPitch(view_type, pixel_pitch);
}

void MeasureResourceMgr::slotMeasureSetSceneCenter(
	const common::ViewTypeID& view_type, const QPointF& center)
{
	KeyViewInfo key(curr_tab_, view_type);
	auto& view_info_at = view_infos_.find(key);
	if (view_info_at == view_infos_.end())
	{
		common::measure::ViewInfo view_info;
		view_info.scene_center = center;
		view_infos_.insert(
			std::pair<KeyViewInfo, common::measure::ViewInfo>(key, view_info));
	}
	else
	{
		view_info_at->second.scene_center = center;
	}
}

void MeasureResourceMgr::slotMeasureSetSceneTrans(
	const common::ViewTypeID& view_type, const QPointF& trans)
{
	KeyViewInfo key(curr_tab_, view_type);
	auto& view_info_at = view_infos_.find(key);
	if (view_info_at == view_infos_.end())
	{
		common::measure::ViewInfo view_info;
		view_info.translate = trans;
		view_infos_.insert(
			std::pair<KeyViewInfo, common::measure::ViewInfo>(key, view_info));
	}
	else
	{
		view_info_at->second.translate = trans;
	}
}

void MeasureResourceMgr::slotMeasureSetSceneTransform(
	const common::ViewTypeID& view_type, const QTransform& transform)
{
	KeyViewInfo key(curr_tab_, view_type);
	auto& view_info_at = view_infos_.find(key);
	if (view_info_at == view_infos_.end())
	{
		common::measure::ViewInfo view_info;
		view_info.transform = transform;
		view_infos_.insert(
			std::pair<KeyViewInfo, common::measure::ViewInfo>(key, view_info));
	}
	else
	{
		view_info_at->second.transform = transform;
	}
}

void MeasureResourceMgr::slotMeasureCounterpartViewInfo(
	const common::ViewTypeID& view_type, common::measure::ViewInfo* view_info)
{
	TabType counterpart_tab = GetCounterpartTab();
	if (counterpart_tab == TabType::TAB_UNKNOWN) return;

	KeyViewInfo key(counterpart_tab, view_type);
	auto& view_info_at = view_infos_.find(key);

	if (view_info_at != view_infos_.end())
	{
		*view_info = view_info_at->second;
	}
}

void MeasureResourceMgr::slotMeasureCurrentViewInfo(
	const common::ViewTypeID& view_type, common::measure::ViewInfo* view_info)
{
	KeyViewInfo key(curr_tab_, view_type);
	auto& view_info_at = view_infos_.find(key);

	if (view_info_at != view_infos_.end())
	{
		*view_info = view_info_at->second;
	}
}

void MeasureResourceMgr::SetConnections()
{
	const EventHandleMeasure& eh =
		EventHandler::GetInstance()->GetMeasureEventHandle();
	eh.ConnectSigMeasureCreate(
		this,
		SLOT(slotMeasureCreate(common::ViewTypeID, common::measure::MeasureType,
			common::measure::VisibilityParams, float, float,
			std::weak_ptr<MeasureData>*)));
	eh.ConnectSigMeasureDelete(
		this, SLOT(slotMeasureDelete(common::ViewTypeID, unsigned int)));
	eh.ConnectSigMeasureDeleteAll(this,
		SLOT(slotMeasureDeleteAll(common::ViewTypeID)));
	eh.ConnectSigMeasureSetNote(
		this,
		SLOT(slotMeasureSetNote(common::ViewTypeID, unsigned int, QString)));
	eh.ConnectSigMeasureModify(
		this, SLOT(slotMeasureModify(common::ViewTypeID, unsigned int, QString,
			std::vector<QPointF>)));
	eh.ConnectSigMeasureGetMeasureList(
		this, SLOT(slotGetMeasureList(common::ViewTypeID,
			std::vector<std::weak_ptr<MeasureData>>*)));
	eh.ConnectSigMeasureGetMeasureData(
		this, SLOT(slotGetMeasureData(common::ViewTypeID, unsigned int,
			std::weak_ptr<MeasureData>*)));

	eh.ConnectSigMeasureSetPixelPitch(
		this, SLOT(slotMeasureSetPixelPitch(common::ViewTypeID, float)));
	eh.ConnectSigMeasureSetScale(
		this, SLOT(slotMeasureSetScale(common::ViewTypeID, float)));
	eh.ConnectSigMeasureSetZoomFactor(this, SLOT(slotMeasureSetZoomFactor(common::ViewTypeID, float)));
	eh.ConnectSigMeasureSetSceneCenter(
		this, SLOT(slotMeasureSetSceneCenter(common::ViewTypeID, QPointF)));
	eh.ConnectSigMeasureSetSceneTrans(
		this, SLOT(slotMeasureSetSceneTrans(common::ViewTypeID, QPointF)));
	eh.ConnectSigMeasureSetSceneTransform(
		this, SLOT(slotMeasureSetSceneTransform(common::ViewTypeID, QTransform)));
	eh.ConnectSigMeasureGetCounterpartViewInfo(
		this, SLOT(slotMeasureCounterpartViewInfo(common::ViewTypeID,
			common::measure::ViewInfo*)));
	eh.ConnectSigMeasureGetCurrentViewInfo(
		this, SLOT(slotMeasureCurrentViewInfo(common::ViewTypeID,
			common::measure::ViewInfo*)));
	eh.ConnectSigMeasureSetCounterpartTab(this, SLOT(slotSetCounterpartTab(TabType)));
	eh.ConnectSigMeasureSetCounterpartAsCurrentTab(this, SLOT(slotSetCounterpartAsCurrentTab()));
}

TabType MeasureResourceMgr::GetCounterpartTab()
{
	if (curr_tab_ == TabType::TAB_MPR
#ifndef WILL3D_LIGHT
		|| curr_tab_ == TabType::TAB_SI
#endif
		)
	{
		if (counterpart_mpr_ != curr_tab_) return counterpart_mpr_;
	}
	else if (curr_tab_ == TabType::TAB_PANORAMA 
#ifndef WILL3D_VIEWER
		|| curr_tab_ == TabType::TAB_IMPLANT
#endif
		)
	{
		if (counterpart_pano_ != curr_tab_) return counterpart_pano_;
	}
	return TabType::TAB_UNKNOWN;
}

void MeasureResourceMgr::slotSetCounterpartTab(const TabType tab_type)
{
	return;

	if (curr_tab_ == TabType::TAB_MPR
#ifndef WILL3D_LIGHT
		|| curr_tab_ == TabType::TAB_SI
#endif
		)
	{
		counterpart_mpr_ = tab_type;
	}
	else if (curr_tab_ == TabType::TAB_PANORAMA 
#ifndef WILL3D_VIEWER
		|| curr_tab_ == TabType::TAB_IMPLANT
#endif
		)
	{
		counterpart_pano_ = tab_type;
	}
}

void MeasureResourceMgr::slotSetCounterpartAsCurrentTab()
{
	return;

	if (curr_tab_ == TabType::TAB_MPR
#ifndef WILL3D_LIGHT
		|| curr_tab_ == TabType::TAB_SI
#endif
		)
	{
		counterpart_mpr_ = curr_tab_;
	}
	else if (curr_tab_ == TabType::TAB_PANORAMA 
#ifndef WILL3D_VIEWER
		|| curr_tab_ == TabType::TAB_IMPLANT
#endif
		)
	{
		counterpart_pano_ = curr_tab_;
	}
}
