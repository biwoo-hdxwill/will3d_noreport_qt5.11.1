#include "pano_engine.h"

#include <qmath.h>
#include <QObject>
#include <QtConcurrent/QtConcurrent>
#include <QSettings>
#include <QDebug>

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3Math.h"
#include "../../Common/Common/W3MessageBox.h"
#include "../../Common/Common/W3ProgressDialog.h"
#include "../../Common/Common/common.h"
#include "../../Common/Common/event_handle_common.h"
#include "../../Common/Common/event_handler.h"

#include "../../Common/GLfunctions/gl_helper.h"
#include "../../Common/GLfunctions/gl_transform_functions.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/cross_section_resource.h"
#include "../../Resource/Resource/implant_resource.h"
#include "../../Resource/Resource/nerve_resource.h"
#include "../../Resource/Resource/pano_resource.h"
#include "../../Resource/Resource/sagittal_resource.h"
//20250123 LIN 주석처리
//#ifndef WILL3D_VIEWER
#include "../../Core/W3ProjectIO/project_io_pano_engine.h"
//#endif

#include "pano_module.h"

using namespace implant_resource;

namespace
{
	const glm::vec3 kInvXAxis(-1.0f, 1.0f, 1.0f);
	const glm::mat4 kToPanoCoordSys(glm::rotate(-(float)M_PI_2,
		glm::vec3(1.0f, 0.0f, 0.0f)));
}  // end of namespace

PanoEngine::PanoEngine()
{
	res_nerve_.reset(new NerveResource);
	ResourceContainer::GetInstance()->SetNerveResource(res_nerve_);

	res_implant_.reset(new ImplantResource);
	ResourceContainer::GetInstance()->SetImplantResource(res_implant_);

	for (int i = 0; i < ARCH_TYPE_END; i++) reorien_mat_[i] = glm::mat4();
}

PanoEngine::~PanoEngine() { this->ClearPano3D(); }

/**
Public fucntions
*/
void PanoEngine::Initialize(const CW3Image3D& volume)
{
	if (initialized_)
	{
		return common::Logger::instance()->Print(
			common::LogType::ERR, "PanoEngine::Initialize: already initialized.");
	}
	InitSliceLocation(volume);

	res_implant_->Initialize(volume.pixelSpacing(), volume.sliceSpacing());

	initialized_ = true;
}
void PanoEngine::InitSliceLocation(const CW3Image3D& volume)
{
#if 0
	QSettings settings(GlobalPreferences::GetInstance()->ini_path(), QSettings::IniFormat);
	bool auto_set_teeth_roi = settings.value("PANORAMA/auto_set_teeth_roi", true).toBool();

	const auto& slice_loc = volume.getSliceLoc();
	glm::vec3 slice = glm::vec3(slice_loc.nose, slice_loc.chin,
		(slice_loc.nose + slice_loc.chin) * 0.5f);
	if (auto_set_teeth_roi)
	{
		roi_vol_.top = slice.x;
		roi_vol_.bottom = slice.y;
		roi_vol_.slice = slice.z;
	}
	else
	{
		const auto& vol = ResourceContainer::GetInstance()->GetMainVolume();

		roi_vol_.top = 0.0f;
		roi_vol_.bottom = static_cast<float>(vol.depth());
		roi_vol_.slice = slice.z;
	}
#else
	const auto& slice_loc = volume.getSliceLoc();
	glm::vec3 slice = glm::vec3(slice_loc.nose, slice_loc.chin, (slice_loc.nose + slice_loc.chin) * 0.5f);

	roi_vol_.top = slice.x;
	roi_vol_.bottom = slice.y;
	roi_vol_.slice = slice.z;
#endif
}

void PanoEngine::InitCrossSectionResource()
{
	if (res_pano() == nullptr)
	{
		return common::Logger::instance()->Print(
			common::LogType::ERR,
			"PanoEngine::InitCrossSectionResource: res_pano_ is nullptr.");
	}

	if (res_pano()->GetCurrentCurveData().points().empty())
	{
		return common::Logger::instance()->Print(
			common::LogType::WRN,
			"PanoEngine::InitCrossSectionResource: pano_spline is empty.");
	}

	QPointF pt_pano_plane =
		MapVolToPanoPlane(res_pano()->pano_ctrl_points().front());

	res_cross_section_.reset(new CrossSectionResource(
		res_pano()->GetCurrentCurveData(), res_pano()->back_vector(),
		res_pano()->ruler_index().idx_arch_front_, res_pano()->pano_3d_height(),
		(float)pt_pano_plane.y()));

	res_cross_section_->EditShiftedValue(cross_shifted_value_);

	ResourceContainer::GetInstance()->SetCrossSectionResource(res_cross_section_);
}
void PanoEngine::InitSagittalResource()
{
	if (res_pano() == nullptr)
	{
		return common::Logger::instance()->Print(
			common::LogType::ERR,
			"PanoEngine::InitSagittalResource: res_pano_ is nullptr.");
	}
	if (res_pano()->GetCurrentCurveData().points().empty())
	{
		return common::Logger::instance()->Print(
			common::LogType::WRN,
			"PanoEngine::InitSagittalResource: pano_spline is empty.");
	}
	res_sagittal_.reset(new SagittalResource);

	ResourceContainer::GetInstance()->SetSagittalResource(res_sagittal_);
}

void PanoEngine::InitPanoramaResource(
	const std::vector<glm::vec3>& pano_points,
	const std::vector<glm::vec3>& pano_ctrl_points,
	const glm::vec3& pano_back_vector, float pano_depth, float pano_range_mm, float pano_shifted,
	float pano_thickness)
{
	this->ClearPano3D();

	res_pano_[curr_arch_type_].reset(new PanoResource(
		pano_points, pano_ctrl_points, pano_back_vector, pano_depth, pano_range_mm));

	res_pano()->SetShiftedValue(pano_shifted);
	res_pano()->set_thickness_value(pano_thickness);

	ResourceContainer::GetInstance()->SetPanoResource(res_pano_[curr_arch_type_]);
}

void PanoEngine::SetCurrentArchType(const ArchTypeID& type)
{
	curr_arch_type_ = type;
	ResourceContainer::GetInstance()->SetPanoResource(res_pano_[curr_arch_type_]);
}
void PanoEngine::EditCrossSectionShiftedValue(float delta)
{
	if (!res_cross_section_) return;

	res_cross_section_->EditShiftedValue(delta);

	cross_shifted_value_ = res_cross_section_->shifted_value();

	this->SetCrossCenterPositionsInPanoPlane();
	this->SetCrossSectionNervePoint();
}

void PanoEngine::ResetDeltaShiftedValue()
{
	if (!res_cross_section_)
	{
		return;
	}

	res_cross_section_->ResetDeltaShiftedValue();
}

bool PanoEngine::SetPanoThickness(const int thickness)
{
	if (res_pano() == nullptr || thickness < 0)
	{
		return false;
	}

	res_pano()->set_thickness_value(thickness);
	return true;
}

// pt_vol위치로 crosssection이 움직임.
void PanoEngine::EditCrossSectionShiftedValue(const glm::vec3& pt_vol)
{
	const auto& cross_data = res_cross_section_->data();
	if (&cross_data && cross_data.empty())
	{
		return common::Logger::instance()->Print(
			common::LogType::WRN, "PanoEngine::EditCrossSectionShiftedValue");
	}

	std::vector<int> cross_data_indices;
	for (const auto& elem : cross_data)
	{
		cross_data_indices.push_back(elem.first);
	}
	int select_idx =
		cross_data_indices[(int)((float)cross_data_indices.size() / 2.0f)];

	const glm::vec3& cross_point =
		cross_data.at(select_idx)->center_position_in_vol();

	QPointF changed_point_in_pano =
		PanoModule::MapVolToPanoPlane(*res_pano(), pt_vol);
	QPointF cross_point_in_pano =
		PanoModule::MapVolToPanoPlane(*res_pano(), cross_point);
	EditCrossSectionShiftedValue(
		(float)(changed_point_in_pano.x() - cross_point_in_pano.x()));
}

void PanoEngine::EditNerveCtrlPoint(int nerve_id, int nerve_selected_index,
	const glm::vec3& point_in_vol)
{
	res_nerve_->EditNerveCtrlPoint(nerve_id, nerve_selected_index, point_in_vol);
	UpdateNerveResource(nerve_id);

	const auto& modify_mode = res_nerve_->modify_mode();
	if (modify_mode.enable) this->SetCrossSectionNerveProjection();
}

void PanoEngine::EditNerveCtrlModifiedPoint(const glm::vec3& point_in_vol)
{
	const auto& modify_mode = res_nerve_->modify_mode();
	res_nerve_->EditNerveCtrlPoint(
		modify_mode.nerve_id, modify_mode.nerve_selected_index, point_in_vol);
	UpdateNerveResource(modify_mode.nerve_id);

	if (modify_mode.enable) this->SetCrossSectionNerveProjection();
}

void PanoEngine::SetNerveParams(bool nerve_visible, int nerve_id,
	float nerve_radius, const QColor& nerve_color,
	bool is_update_resource)
{
	res_nerve_->SetNerveParams(nerve_visible, nerve_id, nerve_radius,
		nerve_color);

	if (is_update_resource)
	{
		UpdateNerveResource(nerve_id);
		this->SetCrossCenterPositionsInPanoPlane();
		this->SetCrossSectionNervePoint();
	}
}

void PanoEngine::UpdateNerveResource()
{
	for (const auto& elem : res_nerve_->nerve_ctrl_points())
	{
		this->SetNervePoints(elem.first);
		this->SetNerveMesh(elem.first);
	}
	this->SetNerveMaskROI();
	EventHandler::GetInstance()->GetCommonEventHandle().EmitSigUpdateNerve();
}

void PanoEngine::SetModifyNervePoint(const int& nerve_id,
	const int& nerve_selected_index,
	const bool& is_modify)
{
	res_nerve_->SetModifyMode(is_modify, nerve_id, nerve_selected_index);

	if (!is_modify || !res_cross_section_) return;

	const auto& nerve_points = res_nerve_->GetNerveCtrlPoints(nerve_id);
	if (nerve_points.empty() || nerve_points.size() <= nerve_selected_index)
	{
		return common::Logger::instance()->Print(
			common::LogType::WRN, "PanoModule::SetModifyNervePoint: l0");
	}

	const glm::vec3& nerve_point = nerve_points[nerve_selected_index];
	this->EditCrossSectionShiftedValue(nerve_point);
}

void PanoEngine::SetCrossSectionParams(float interval, int width, int height,
	float thickness, float degree,
	int count)
{
	if (!res_cross_section_) return;

	CrossSectionResource::Params param(count, width, height, interval, thickness,
		degree);
	res_cross_section_->SetCrossSectionParams(param);

	this->SetCrossCenterPositionsInPanoPlane();
}

void PanoEngine::SetCrossSectionNervePoint()
{
	const auto& modify_mode = res_nerve_->modify_mode();

	if (modify_mode.enable)
		this->SetCrossSectionNerveProjection();
	else
		this->SetCrossSectionNerveCtrlPoint();
}

void PanoEngine::SetSagittalParams(int width, int height, float degree)
{
	SagittalResource::Params param(width, height, degree);
	res_sagittal_->SetSagittalParams(param);
}

void PanoEngine::UpdateSagittal(const glm::vec3& point_in_vol)
{
	QPointF pano_pos(MapVolToPanoPlane(point_in_vol).x(),
		static_cast<double>(res_pano()->pano_3d_height()) / 2.0);
	res_sagittal_->SetSagittal(point_in_vol, res_pano()->GetCurrentCurveData(),
		res_pano()->back_vector(), pano_pos);
}

void PanoEngine::SetPanoramaShiftedValue(int shifted_value)
{
	res_pano()->SetShiftedValue(shifted_value);

	UpdateImplantPositionInPanoPlane();

	if (res_cross_section_ == nullptr)
		InitCrossSectionResource();
	else
		res_cross_section_->SetPanoCurveData(
			res_pano()->GetCurrentCurveData(),
			res_pano()->ruler_index().idx_arch_front_);

	this->SetCrossCenterPositionsInPanoPlane();
	this->SetCrossSectionNervePoint();
}

void PanoEngine::SetPACSPanoShiftedValue(int shifted_value)
{
	res_pano()->SetShiftedValue(shifted_value);

	UpdateImplantPositionInPanoPlane();
}

void PanoEngine::AddNerveCtrlPoint(int nerve_Id,
	const glm::vec3& point_in_vol)
{
	res_nerve_->AddNerveCtrlPoint(nerve_Id, point_in_vol);
	this->SetCrossSectionNerveCtrlPoint();
}

void PanoEngine::InsertNerveCtrlPoint(int nerve_id, int nerve_insert_index,
	const glm::vec3& nerve_point_in_vol)
{
	res_nerve_->InsertNerveCtrlPoint(nerve_id, nerve_insert_index,
		nerve_point_in_vol);
	UpdateNerveResource(nerve_id);
}

void PanoEngine::RemoveNerveCtrlPoint(int nerve_id, int nerve_removed_index)
{
	res_nerve_->RemoveNerveCtrlPoint(nerve_id, nerve_removed_index);
	UpdateNerveResource(nerve_id);
}

void PanoEngine::CancleNerveCtrlPoint(int nerve_id, int nerve_removed_index)
{
	res_nerve_->RemoveNerveCtrlPoint(nerve_id, nerve_removed_index);
	this->SetCrossSectionNerveCtrlPoint();
}

void PanoEngine::SetNerveVisibleAll(bool is_visible)
{
	res_nerve_->SetNerveVisibleAll(is_visible);
	this->SetNerveMaskROI();
	this->SetCrossCenterPositionsInPanoPlane();
	this->SetCrossSectionNervePoint();
	EventHandler::GetInstance()->GetCommonEventHandle().EmitSigUpdateNerve();
}

void PanoEngine::DeleteNerve(int nerve_id)
{
	res_nerve_->ClearNervePointsAll(nerve_id);
	this->SetNerveMaskROI();
	this->SetCrossCenterPositionsInPanoPlane();
	this->SetCrossSectionNervePoint();
	EventHandler::GetInstance()->GetCommonEventHandle().EmitSigUpdateNerve();
}

void PanoEngine::SetImplantMemo(const QString& text)
{
	res_implant_->set_memo(text);
}

bool PanoEngine::AddImplant(const implant_resource::ImplantInfo& implant_params,
	int* prev_adding_id)
{
	*prev_adding_id = res_implant_->add_implant_id();
	if (!res_implant_->AddImplant(implant_params))
	{
		return false;
	}

	if (*prev_adding_id > 0 && *prev_adding_id != implant_params.id)
	{
		DeleteImplant(*prev_adding_id);
	}

	return true;
}

bool PanoEngine::ChangeImplant(
	const implant_resource::ImplantInfo& implant_params)
{
	if (res_implant_->ChangeImplant(implant_params))
	{
		auto& implant_data = res_implant_->data();
		for (const auto& data : implant_data)
		{
			if (data.first == implant_params.id)
			{
				this->SetImplantPositionVolAndPano3D(data.first,
					data.second->position_in_vol());
				this->SetImplantAxisPointVolAndPano3D(data.first);
				this->SetImplantPositionInPanoPlane(data.first, false);
				this->SetImplantAxisPointPanoPlane(data.first);
				break;
			}
		}
		return true;
	}
	return false;
}

void PanoEngine::SetImplantSelected(int implant_id, bool selected)
{
	res_implant_->SelectImplant(implant_id, selected);
	if (!selected)
	{
		ResetDeltaShiftedValue();
	}
}

void PanoEngine::SetImplantVisibleAll(bool is_visible)
{
	res_implant_->SetVisibleAll(is_visible);
	EventHandler::GetInstance()->GetCommonEventHandle().EmitSigUpdateImplant();
}

void PanoEngine::DeleteImplant(const int& implant_id)
{
	res_implant_->RemoveAt(implant_id);

	EventHandler::GetInstance()->GetCommonEventHandle().EmitSigUpdateImplant();
}

void PanoEngine::DeleteAllImplants()
{
	res_implant_->RemoveAll();
	EventHandler::GetInstance()->GetCommonEventHandle().EmitSigUpdateImplant();
}

void PanoEngine::SetPanoROISlice(float value)
{
	roi_vol_.slice = value;
}

void PanoEngine::SetPanoROITop(float value)
{
	roi_vol_.top = value;
}

void PanoEngine::SetPanoROIBottom(float value)
{
	roi_vol_.bottom = value;
}

void PanoEngine::SetImplantAxisPointVolAndPano3D(int implant_id)
{
	this->SetImplantAxisPointVol(implant_id);
	this->SetImplantAxisPointPano3D(implant_id);
}
void PanoEngine::SetImplantAxisPointVol(int implant_id)
{
	const CW3Image3D& vol = this->GetVolume();
	float z_spacing = vol.sliceSpacing() / vol.pixelSpacing();
	glm::vec3 pt_center_vol((float)vol.width() * 0.5f - 1.0f,
		(float)vol.height() * 0.5f - 1.0f,
		(float)vol.depth() * 0.5f - 1.0f);

	const auto& elem = res_implant_->data().find(implant_id);
	glm::vec4 major_axis(elem->second->major_axis()*0.5f, 1.0f);
	glm::vec3 major_axis_in_vol_gl(elem->second->translate_in_vol() *
		elem->second->rotate_in_vol() * major_axis);
	glm::vec3 major_axis_in_vol = GLhelper::MapWorldGLtoVol(
		kInvXAxis * major_axis_in_vol_gl, pt_center_vol, z_spacing);

	res_implant_->SetImplantAxisPointInVol(implant_id, major_axis_in_vol);
}

void PanoEngine::SetImplantAxisPointPano3D(int implant_id)
{
	const CW3Image3D& vol = this->GetVolume();
	float z_spacing = vol.sliceSpacing() / vol.pixelSpacing();
	glm::vec3 pt_center_pano((float)res_pano()->pano_3d_width() * 0.5f - 1.0f,
		(float)res_pano()->pano_3d_height() * 0.5f - 1.0f,
		0.0f);

	const auto& elem = res_implant_->data().find(implant_id);
	glm::vec4 major_axis(elem->second->major_axis()*0.5f, 1.0f);
	glm::vec3 major_axis_in_pano_gl(elem->second->translate_in_pano() *
		elem->second->rotate_in_pano() * major_axis);
	glm::vec3 major_axis_in_pano = GLhelper::MapWorldGLtoVol(
		kInvXAxis * major_axis_in_pano_gl, pt_center_pano, z_spacing);

	res_implant_->SetImplantAxisPointInPano(implant_id, major_axis_in_pano);
}

void PanoEngine::SetImplantAxisPointPanoPlane(int implant_id)
{
	const CW3Image3D& vol = this->GetVolume();
	float z_spacing = vol.sliceSpacing() / vol.pixelSpacing();
	glm::vec3 pt_center_pano_plane(
		(float)res_pano()->GetPanoPlaneWidth() * 0.5f - 1.0f,
		(float)res_pano()->GetPanoPlaneHeight() * 0.5f - 1.0f, 0.0f);

	const auto& elem = res_implant_->data().find(implant_id);
	glm::vec4 major_axis(elem->second->major_axis()*0.5f, 1.0f);

	glm::vec3 major_axis_in_pano_plane_gl(
		elem->second->translate_in_pano_plane() *
		elem->second->rotate_in_pano_plane() * major_axis);
	glm::vec3 major_axis_in_pano_plane = GLhelper::MapWorldGLtoVol(
		kInvXAxis * major_axis_in_pano_plane_gl, pt_center_pano_plane, z_spacing);

	res_implant_->SetImplantAxisPointInPanoPlane(implant_id,
		major_axis_in_pano_plane);
}

void PanoEngine::AppendImplantPosition(int implant_id, const glm::vec3& translate_vol)
{
	const CW3Image3D& vol = this->GetVolume();
	float z_spacing = vol.sliceSpacing() / vol.pixelSpacing();

	glm::vec3 pt_center_vol((float)vol.width() * 0.5f - 1.0f,
		(float)vol.height() * 0.5f - 1.0f,
		(float)vol.depth() * 0.5f - 1.0f);
	glm::vec3 result_pt_vol = translate_vol + res_implant_->data().at(implant_id)->position_in_vol();
	glm::vec3 pt_vol_gl =
		kInvXAxis * GLhelper::MapVolToWorldGL(result_pt_vol, pt_center_vol, z_spacing);
	res_implant_->SetImplantPositionInVol(implant_id, result_pt_vol, pt_vol_gl);

	glm::vec3 pt_pano = PanoModule::MapVolToPano3D(*res_pano(), result_pt_vol);
	glm::vec3 pt_center_pano((float)res_pano()->pano_3d_width() * 0.5f - 1.0f,
		(float)res_pano()->pano_3d_height() * 0.5f - 1.0f,
		0.0f);
	glm::vec3 pt_pano_gl =
		kInvXAxis * GLhelper::MapVolToWorldGL(pt_pano, pt_center_pano, z_spacing);
	res_implant_->SetImplantPositionInPano(implant_id, pt_pano, pt_pano_gl);

	auto iter = res_implant_->data().find(implant_id);
	this->SetImplantRotateInPanoFromVol(*(iter->second.get()));
}

void PanoEngine::SetImplantPositionVolAndPano3D(int implant_id,
	const glm::vec3& pt_vol)
{
	const CW3Image3D& vol = this->GetVolume();
	float z_spacing = vol.sliceSpacing() / vol.pixelSpacing();

	glm::vec3 pt_center_vol((float)vol.width() * 0.5f - 1.0f,
		(float)vol.height() * 0.5f - 1.0f,
		(float)vol.depth() * 0.5f - 1.0f);
	glm::vec3 pt_vol_gl =
		kInvXAxis * GLhelper::MapVolToWorldGL(pt_vol, pt_center_vol, z_spacing);
	res_implant_->SetImplantPositionInVol(implant_id, pt_vol, pt_vol_gl);

	glm::vec3 pt_pano = PanoModule::MapVolToPano3D(*res_pano(), pt_vol);
	glm::vec3 pt_center_pano((float)res_pano()->pano_3d_width() * 0.5f - 1.0f,
		(float)res_pano()->pano_3d_height() * 0.5f - 1.0f,
		0.0f);
	glm::vec3 pt_pano_gl =
		kInvXAxis * GLhelper::MapVolToWorldGL(pt_pano, pt_center_pano, z_spacing);
	res_implant_->SetImplantPositionInPano(implant_id, pt_pano, pt_pano_gl);

	auto iter = res_implant_->data().find(implant_id);
	this->SetImplantRotateInPanoFromVol(*(iter->second.get()));
}

void PanoEngine::SetImplantPositionInPanoPlane(int implant_id,
	bool is_update_z_position)
{
	glm::vec3 pt_center_pano_plane(
		(float)res_pano()->GetPanoPlaneWidth() * 0.5f - 1.0f,
		(float)res_pano()->GetPanoPlaneHeight() * 0.5f - 1.0f, 0.0f);

	const CW3Image3D& vol = this->GetVolume();
	const float z_spacing = vol.sliceSpacing() / vol.pixelSpacing();

	auto& elem = res_implant_->data().find(implant_id);
	const float z_pos = is_update_z_position ? res_pano()->shifted_value()
		: elem->second->position_in_pano().z;

	const glm::vec3& pt_vol = elem->second->position_in_vol();
	QPointF tmp_pano_plane = MapVolToPanoPlane(pt_vol);
	glm::vec3 pt_pano_plane(tmp_pano_plane.x(), tmp_pano_plane.y(), z_pos);
	glm::vec3 pt_pano_plane_gl =
		kInvXAxis *
		GLhelper::MapVolToWorldGL(pt_pano_plane, pt_center_pano_plane, z_spacing);

	res_implant_->SetImplantPositionInPanoPlane(elem->first, pt_pano_plane,
		pt_pano_plane_gl);

	this->SetImplantRotateInPanoPlaneFromVol(*(elem->second.get()));

	//SetCrossSectionAngle(implant_id);
}

void PanoEngine::RotateImplantInPanoPlane(int implant_id, float delta_degree)
{
	const float radian_angle = glm::radians(delta_degree);
	const auto& iter = res_implant_->data().find(implant_id);

	const glm::vec3& pt_in_pano_plane = iter->second->position_in_pano_plane();
	const float memory_value = res_pano()->shifted_value();
	res_pano()->SetShiftedValue(pt_in_pano_plane.z);
	const glm::vec3& up_vector =
		res_pano()->GetCurrentCurveData().GetInterpolatedUpvector(
			pt_in_pano_plane.x);
	const glm::vec3& back_vector = res_pano()->back_vector();
	res_pano()->SetShiftedValue(memory_value);

	// volume 좌표계상의 upvector는 맞지만 GL좌표계의 X축이 뒤집혀 있기 때문에
	// kInvXAxis을 곱해준다.
	res_implant_->EditImplantRotateInVol(
		implant_id, glm::rotate(radian_angle, kInvXAxis * up_vector));
	res_implant_->EditImplantRotateInPano(implant_id,
		glm::rotate(radian_angle, back_vector));
	res_implant_->EditImplantRotateInPanoPlane(
		implant_id, glm::rotate(radian_angle, back_vector));

	//SetCrossSectionAngle(implant_id);
}

void PanoEngine::RotateImplantInSagittal(int implant_id, float delta_degree)
{
	const float radian_angle = glm::radians(delta_degree);
	const glm::vec3& up_vector = res_sagittal_->up_vector();
	const glm::vec3& back_vector = res_pano()->back_vector();
	res_implant_->EditImplantRotateInVol(
		implant_id, glm::rotate(radian_angle, kInvXAxis * up_vector));
	res_implant_->EditImplantRotateInPano(implant_id,
		glm::rotate(radian_angle, back_vector));
	res_implant_->EditImplantRotateInPanoPlane(
		implant_id, glm::rotate(radian_angle, back_vector));
}

void PanoEngine::RotateImplantInCross(int implant_id, int cross_id,
	float delta_degree)
{
	auto res_cross = res_cross_section_->data().find(cross_id);
	if (res_cross == res_cross_section_->data().end())
	{
		common::Logger::instance()->Print(
			common::LogType::ERR,
			"PanoEngine::RotateImplantInCross: invalid cross_id.");
		assert(false);
		return;
	}

	const float radian_angle = glm::radians(delta_degree);
	const glm::vec3& up_vector = res_cross->second->up_vector();
	const glm::vec3& back_vector = res_cross->second->back_vector();
	res_implant_->EditImplantRotateInVol(
		implant_id, glm::rotate(radian_angle, kInvXAxis * up_vector));
	res_implant_->EditImplantRotateInPano(implant_id,
		glm::rotate(radian_angle, back_vector));
	res_implant_->EditImplantRotateInPanoPlane(
		implant_id, glm::rotate(radian_angle, back_vector));
}

void PanoEngine::RotateImplantIn3D(int implant_id, const glm::vec3& rotate_axes,
	const float& delta_degree)
{
	const float radian_angle = glm::radians(delta_degree);
	glm::vec3 pano_rotate_axes =
		glm::vec3(kToPanoCoordSys * glm::vec4(rotate_axes, 1.0f));
	res_implant_->EditImplantRotateInVol(implant_id,
		glm::rotate(radian_angle, rotate_axes));
	res_implant_->EditImplantRotateInPano(
		implant_id, glm::rotate(radian_angle, pano_rotate_axes));
	res_implant_->EditImplantRotateInPanoPlane(
		implant_id, glm::rotate(radian_angle, pano_rotate_axes));
}

void PanoEngine::RotateImplantInPano3D(int implant_id,
	const glm::vec3& rotate_axes,
	const float& delta_degree)
{
	const float radian_angle = glm::radians(delta_degree);
	glm::mat4 rotate_axes_pano = glm::rotate(radian_angle, rotate_axes);

	res_implant_->EditImplantRotateInPano(implant_id, rotate_axes_pano);
	res_implant_->EditImplantRotateInPanoPlane(implant_id, rotate_axes_pano);

	auto iter = res_implant_->data().find(implant_id);
	this->SetImplantRotateInVolFromPano(*(iter->second.get()));
}

const float PanoEngine::GetImplantRotateDegreeInPanoPlane(const int implant_id)
{
	const auto& iter = res_implant_->data().find(implant_id);
	if (!&iter)
	{
		return 0.0f;
	}

	return iter->second->rotate_degree_in_pano_plane();
}

int PanoEngine::GetZValuePanorama(const glm::vec3& pt_vol)
{
	const glm::vec3& pt_pano_plane_3d = this->MapVolToPano3D(pt_vol);
	const int pano_z_pos = static_cast<int>(pt_pano_plane_3d.z + 0.05f);
	const int origin_z_pos = res_pano()->pano_3d_depth() / 2;
	return pano_z_pos + origin_z_pos;
}
void PanoEngine::MoveCrossSection(const glm::vec3& pt_vol)
{
	const auto& cross_data = res_cross_section_->data();
	const glm::vec3& cross_point = cross_data.at(0)->center_position_in_vol();

	const QPointF center_pos_in_pano_plane = this->MapVolToPanoPlane(pt_vol);
	QPointF cs_pos_in_pano = this->MapVolToPanoPlane(cross_point);

	this->EditCrossSectionShiftedValue(
		(float)(center_pos_in_pano_plane.x() - cs_pos_in_pano.x()));
}

void PanoEngine::SetImplantPlaced() { res_implant_->SetImplantPlaced(); }

void PanoEngine::UpdateImplantPositions()
{
	const auto& implant_data = res_implant_->data();

	for (const auto& data : implant_data)
	{
		this->SetImplantPositionVolAndPano3D(data.first,
			data.second->position_in_vol());
		this->SetImplantAxisPointVolAndPano3D(data.first);
		this->SetImplantPositionInPanoPlane(data.first, false);
		this->SetImplantAxisPointPanoPlane(data.first);
	}
}

void PanoEngine::TranslateImplantIn3D(int implant_id,
	const glm::vec3& delta_translate)
{
	const CW3Image3D& vol = this->GetVolume();
	float z_spacing = vol.sliceSpacing() / vol.pixelSpacing();

	glm::vec3 pt_trans_vol = GLhelper::MapWorldGLtoVol(
		delta_translate / kInvXAxis, glm::vec3(0.0f), z_spacing);

	const auto& iter = res_implant_->data().find(implant_id);
	const glm::vec3& translated_pt_vol =
		iter->second->position_in_vol() + pt_trans_vol;
	this->SetImplantPositionVolAndPano3D(implant_id, translated_pt_vol);
}
void PanoEngine::TranslateImplantInPano3D(int implant_id,
	const glm::vec3& delta_translate)
{
	const CW3Image3D& vol = this->GetVolume();
	float z_spacing = vol.sliceSpacing() / vol.pixelSpacing();
	auto iter = res_implant_->data().find(implant_id);
	glm::vec3 pt_center_vol((float)vol.width() * 0.5f - 1.0f,
		(float)vol.height() * 0.5f - 1.0f,
		(float)vol.depth() * 0.5f - 1.0f);
	glm::vec3 pt_center_pano((float)res_pano()->pano_3d_width() * 0.5f - 1.0f,
		(float)res_pano()->pano_3d_height() * 0.5f - 1.0f,
		0.0f);

	glm::vec3 pt_trans_pano = GLhelper::MapWorldGLtoVol(
		delta_translate / kInvXAxis, glm::vec3(0.0f), z_spacing);

	glm::vec3 translated_pt_pano =
		iter->second->position_in_pano() + pt_trans_pano;
	glm::vec3 translated_pt_pano_gl =
		GLhelper::MapVolToWorldGL(translated_pt_pano, pt_center_pano, z_spacing) *
		kInvXAxis;

	res_implant_->SetImplantPositionInPano(implant_id, translated_pt_pano,
		translated_pt_pano_gl);

	glm::vec3 pt_vol =
		PanoModule::MapPano3DToVol((*res_pano()), translated_pt_pano);
	glm::vec3 pt_vol_gl =
		kInvXAxis * GLhelper::MapVolToWorldGL(pt_vol, pt_center_vol, z_spacing);
	res_implant_->SetImplantPositionInVol(implant_id, pt_vol, pt_vol_gl);
}
void PanoEngine::ShiftCrossSectionByImplant(int implant_id, const int& cross_section_id)
{
	const auto& res_implant_data = ResourceContainer::GetInstance()->GetImplantResource().data();
	if (res_implant_data.find(implant_id) == res_implant_data.end())
	{
		return;
	}

	// implant position 으로 cross section resource들 이동
	const auto& cross_data = res_cross_section_->data();
	const glm::vec3& cross_point = cross_data.at(cross_section_id)->center_position_in_vol();

	glm::vec3 implant_pos_in_pano_plane = res_implant_data.at(implant_id)->position_in_pano_plane();
	QPointF cs_pos_in_pano = PanoModule::MapVolToPanoPlane(*res_pano(), cross_point);

	const float radian = glm::radians(90.0f - res_cross_section_->params().degree);
	const float offset_by_rotate = (implant_pos_in_pano_plane.y - (static_cast<float>(res_pano()->GetPanoPlaneHeight()) * 0.5f)) / tan(radian);

	qDebug() << res_pano()->GetPanoPlaneHeight() << roi_vol_.bottom << roi_vol_.top;
	qDebug() << implant_pos_in_pano_plane.y << res_cross_section_->params().degree << offset_by_rotate;

	EditCrossSectionShiftedValue(implant_pos_in_pano_plane.x - cs_pos_in_pano.x() + offset_by_rotate);
}

void PanoEngine::ReconPanoramaPlane()
{
	if (!res_pano()) return;

	if (res_pano()->thickness_value() == 0.0f)
	{
		PanoModule::ReconPanoPlane(this->GetVolume(), res_pano());
	}
	else
	{
		PanoModule::ReconPanoThickness(this->GetVolume(), res_pano());
	}
}

void PanoEngine::ReconPanoramaXrayEnhancement()
{
	PanoModule::ReconPanoXrayEnahancement(this->GetVolume(), res_pano());
}

void PanoEngine::ReconPanorama3D()
{
	auto res_container = ResourceContainer::GetInstance();
	PanoModule::ReconPano3D(res_container->GetMainVolume(), res_pano());
	EventHandler::GetInstance()->GetCommonEventHandle().EmitSigSetPanoVolume();
}

void PanoEngine::ReconPanoramaNerveMask()
{
	if (!res_pano()) return;

	if (res_pano()->thickness_value() == 0.0f)
	{
		PanoModule::ReconNerveMaskPano(this->GetVolume(), res_pano(),
			res_nerve_.get());
	}
	else
	{
		PanoModule::ReconNerveMaskPanoThickness(this->GetVolume(), res_pano(),
			res_nerve_.get());
	}
}

void PanoEngine::ReconPanoramaXrayNerveMask()
{
	if (!res_pano()) return;

	PanoModule::ReconNerveMaskPanoXray(this->GetVolume(), res_pano(),
		res_nerve_.get());
}

void PanoEngine::ReconPanoramaImplantMask(const float& view_scale)
{
	if (!res_pano()) return;

	PanoModule::ReconPanoImplantMask(this->GetVolume(), res_pano(),
		res_implant_.get(), view_scale);
}

void PanoEngine::ClearCrossSectionResource() { res_cross_section_.reset(); }

void PanoEngine::ClearPano3D()
{
	auto res_container = ResourceContainer::GetInstance();
	for (int i = 0; i < ArchTypeID::ARCH_TYPE_END; i++)
	{
		if (res_pano_[i] && (&res_pano_[i]->pano_vol()))
		{
			res_container->SetPanoResource(res_pano_[i]);
			EventHandler::GetInstance()
				->GetCommonEventHandle()
				.EmitSigClearPanoVolume();
		}
		res_pano_[i].reset();
	}
	res_container->SetPanoResource(res_pano_[curr_arch_type_]);
}

void PanoEngine::GetNerveCtrlPointsInPanoPlane(
	std::map<int, std::vector<QPointF>>* ctrl_nerve_in_pano_plane) const
{
	for (const auto& elem : res_nerve_->nerve_ctrl_points())
	{
		PanoModule::MapVolToPanoPlane((*res_pano()), elem.second,
			(*ctrl_nerve_in_pano_plane)[elem.first]);
	}
}
glm::vec3 PanoEngine::MapPano3DToVol(const glm::vec3& pt_pano3d) const
{
	return PanoModule::MapPano3DToVol((*res_pano()), pt_pano3d);
}
glm::vec3 PanoEngine::MapVolToPano3D(const glm::vec3& pt_vol) const
{
	return PanoModule::MapVolToPano3D((*res_pano()), pt_vol);
}
glm::vec3 PanoEngine::MapPanoPlaneToVol(const QPointF& pt_in_pano_plane,
	bool is_print_log) const
{
	if (!res_pano())
	{
		return glm::vec3(-1.0f);
	}
	else
	{
		return PanoModule::MapPanoPlaneToVol((*res_pano()), pt_in_pano_plane, is_print_log);
	}
}

QPointF PanoEngine::MapVolToPanoPlane(const glm::vec3& vol_pos) const
{
	return PanoModule::MapVolToPanoPlane((*res_pano()), vol_pos);
}

QPointF PanoEngine::MapVolToPanoPlane_Sagittal(const glm::vec3 vol_pos) const
{
	return PanoModule::MapVolToPanoPlane_Sagittal((*res_pano()), vol_pos);
}

glm::vec3 PanoEngine::MapCrossPlaneToVol(int cross_id,
	const QPointF& pt_in_cross_plane,
	bool is_print_log) const
{
	return PanoModule::MapCrossSectionPlaneToVol((*res_cross_section_), cross_id,
		pt_in_cross_plane, is_print_log);
}

glm::vec3 PanoEngine::MapSagittalPlaneToVol(const QPointF& pt_in_sagittal_plane,
	bool is_print_log) const
{
	return PanoModule::MapSagittalPlaneToVol((*res_sagittal_),
		pt_in_sagittal_plane, is_print_log);
}

QPointF PanoEngine::MapVolToSagittalPlane(const glm::vec3 vol_pos) const
{
	return PanoModule::MapVolToSagittalPlane((*res_sagittal_), vol_pos);
}

void PanoEngine::SetReorienMat(const glm::mat4& mat)
{
	reorien_mat_[curr_arch_type_] = mat;
}

void PanoEngine::SetReorienMat(const glm::mat4& mat, const ArchTypeID& arch_type)
{
	reorien_mat_[arch_type] = mat;
}

QPointF PanoEngine::MapVolToCrossPlane(int cross_id,
	const glm::vec3& pt_vol) const
{
	return PanoModule::MapVolToCrossSectionPlane((*res_cross_section_), cross_id,
		pt_vol);
}

void PanoEngine::GetProfileDataInPanoPlane(const QPointF& start_pt_plane,
	const QPointF& end_pt_plane,
	std::vector<short>& data)
{
	std::vector<QPointF> plane_points;
	W3::GenerateSequencialPlanePointsInLine(start_pt_plane, end_pt_plane,
		plane_points);

	std::vector<glm::vec3> vol_points;
	PanoModule::MapPanoPlaneToVol((*res_pano()), plane_points, vol_points, false);

	this->GetVolume().GetVolumeHU(vol_points, data);
}

void PanoEngine::GetROIDataInPanoPlane(const QPointF& start_pt_plane,
	const QPointF& end_pt_plane,
	std::vector<short>& data)
{
	std::vector<QPointF> plane_points;
	W3::GenerateSequencialPlanePointsInRect(start_pt_plane, end_pt_plane,
		plane_points);

	std::vector<glm::vec3> vol_points;
	PanoModule::MapPanoPlaneToVol((*res_pano()), plane_points, vol_points, false);

	this->GetVolume().GetVolumeHU(vol_points, data);
}

bool PanoEngine::IsValidPanorama() const
{
	return (res_pano() != nullptr && res_pano()->is_valid()) ? true : false;
}

bool PanoEngine::IsValidPanoramaImage() const
{
	return (res_pano() != nullptr && res_pano()->GetPanoImageWeakPtr().lock().get()) ? true : false;
}

bool PanoEngine::IsValidNerve() const
{
	return (res_nerve_ && !res_nerve_->nerve_ctrl_points().empty()) ? true
		: false;
}

bool PanoEngine::IsValidImplant() const
{
	return (res_implant_ && !res_implant_->data().empty()) ? true : false;
}

bool PanoEngine::IsValidCrossSection() const
{
	return (res_cross_section_.get() != nullptr) ? true : false;
}

bool PanoEngine::IsValidSagittal() const
{
	return (res_sagittal_.get() != nullptr) ? true : false;
}

void PanoEngine::GetPanoDirection(const QPointF& pt_pano_plane,
	glm::vec3& pt_prev, glm::vec3& pt_next)
{
	PanoModule::GetPanoDirection((*res_pano()), pt_pano_plane, pt_prev, pt_next);
}

void PanoEngine::HoveredImplantInPanoPlane(const QPointF& pt_in_pano_plane,
	int* implant_id) const
{
	PanoModule::HoveredImplantInPanoPlane((*res_pano()), (*res_implant_),
		pt_in_pano_plane, implant_id);
}

void PanoEngine::CheckCollideNerve()
{
	if (projection_view_implant_3d_ == glm::mat4() ||
		!res_implant_.get()->IsSetImplant())
		return;

	PanoModule::CheckCollideNerve(res_implant_.get(), projection_view_implant_3d_);
}
void PanoEngine::CheckCollideImplant(const glm::mat4& projection_view_mat)
{
	projection_view_implant_3d_ = projection_view_mat;
	PanoModule::CheckCollideImplant(res_implant_.get(), projection_view_mat);
}

void PanoEngine::HoveredImplantInSagittalPlane(const QPointF& pt_sagittal_plane,
	int* implant_id) const
{
	PanoModule::HoveredImplantInSagittalPlane(GetVolume(), (*res_sagittal_),
		(*res_implant_), pt_sagittal_plane,
		implant_id);
}

void PanoEngine::HoveredImplantInCSPlane(const int& cross_id,
	const QPointF& pt_cs_plane,
	int& implant_id,
	QPointF& implant_pos) const
{
	PanoModule::HoveredImplantInCSPlane(GetVolume(), (*res_cross_section_),
		(*res_implant_), cross_id, pt_cs_plane,
		implant_id, implant_pos);
}

/**
Private fucntions
*/

// volume 좌표 변화에 따른 panorama plane rotate 세팅.
void PanoEngine::SetImplantRotateInPanoPlaneFromVol(
	const ImplantData& implant_data)
{
	glm::vec3 curr_pt = implant_data.position_in_pano_plane();

	const auto& curve_data = res_pano()->GetCurrentCurveData();
	glm::vec3 curr_up_vector =
		kInvXAxis * curve_data.GetInterpolatedUpvector(curr_pt.x);
	curr_up_vector = glm::vec3(kToPanoCoordSys * glm::vec4(curr_up_vector, 1.0f));
	const glm::vec3& back_vector = res_pano()->back_vector();

	glm::mat4 rotate_delta =
		GLTransformFunctions::GetRotMatrixVecToVec(curr_up_vector, back_vector);
	res_implant_->SetImplantRotateInPanoPlane(
		implant_data.id(),
		rotate_delta * kToPanoCoordSys * implant_data.rotate_in_vol());
}

// volume 좌표 변화에 따른 panorama rotate 세팅.
void PanoEngine::SetImplantRotateInPanoFromVol(
	const ImplantData& implant_data)
{
	glm::vec3 curr_pt = implant_data.position_in_pano();

	const auto& curve_data = res_pano()->curve_center_data();
	if (curve_data.GetCurveLength() < 2)
	{
		return;
	}
	glm::vec3 curr_up_vector =
		kInvXAxis * curve_data.GetInterpolatedUpvector(curr_pt.x);
	curr_up_vector = glm::vec3(kToPanoCoordSys * glm::vec4(curr_up_vector, 1.0f));

	glm::mat4 rotate_delta = GLTransformFunctions::GetRotMatrixVecToVec(
		curr_up_vector, res_pano()->back_vector());
	res_implant_->SetImplantRotateInPano(
		implant_data.id(),
		rotate_delta * kToPanoCoordSys * implant_data.rotate_in_vol());
}

// panorama 좌표 변화에 따른 volume rotate 세팅.
void PanoEngine::SetImplantRotateInVolFromPano(
	const ImplantData& implant_data)
{
	const glm::vec3& pt_in_pano = implant_data.position_in_pano();
	glm::vec3 back_vector = res_pano()->back_vector();
	glm::vec3 up_vector =
		kInvXAxis *
		res_pano()->curve_center_data().GetInterpolatedUpvector(pt_in_pano.x);
	up_vector = glm::vec3(kToPanoCoordSys * glm::vec4(up_vector, 1.0f));

	glm::mat4 rotate_delta =
		GLTransformFunctions::GetRotMatrixVecToVec(up_vector, back_vector);
	glm::mat4 rotate_axes_vol = glm::inverse(rotate_delta * kToPanoCoordSys) *
		implant_data.rotate_in_pano();
	res_implant_->SetImplantRotateInVol(implant_data.id(), rotate_axes_vol);
}

void PanoEngine::UpdateNerveResource(int nerve_id)
{
	this->SetNervePoints(nerve_id);
	this->SetNerveMaskROI();
	this->SetNerveMesh(nerve_id);

	// 3D Nerve Mesh를 그리는 오브젝트에게 nerve resource의 nerve mesh가 업데이트
	// 되었다고 알린다.
	EventHandler::GetInstance()->GetCommonEventHandle().EmitSigUpdateNerve();
}

/** Updates the implant position in pano plane.
implant의 volume 위치를 가져와서 현재 panorama plane 위치로 변환한다.
panorama의 z위치(panorama resource의 shifted_value)가 바뀔 때 마다 Z위치를
제외하고 X, Y좌표를 갱신해주어야 함.
*/
void PanoEngine::UpdateImplantPositionInPanoPlane()
{
	for (const auto& elem : res_implant_->data())
	{
		SetImplantPositionInPanoPlane(elem.first, false);
		SetImplantAxisPointPanoPlane(elem.first);
	}
}

void PanoEngine::SetCrossCenterPositionsInPanoPlane()
{
	if (res_cross_section_ == nullptr)
	{
		return;
	}
	std::map<int, QPointF> cross_center_positions_in_pano_plane;
	std::map<int, QPointF> cross_arch_positions_in_pano_plane;
	for (const auto& elem : res_cross_section_->data())
	{
		QPointF center_position_in_pano = PanoModule::MapVolToPanoPlane(
			(*res_pano()), elem.second->center_position_in_vol());
		QPointF arch_position_in_pano = PanoModule::MapVolToPanoPlane(
			(*res_pano()), elem.second->arch_position_in_vol());
		cross_center_positions_in_pano_plane[elem.first] = center_position_in_pano;
		cross_arch_positions_in_pano_plane[elem.first] = arch_position_in_pano;
	}

	res_cross_section_->SetCenterPositionsInPanoPlane(
		cross_center_positions_in_pano_plane);
	res_cross_section_->SetArchPositionsInPanoPlane(
		cross_arch_positions_in_pano_plane);
}

void PanoEngine::SetCrossSectionNerveCtrlPoint()
{
	if (res_cross_section_ == nullptr)
	{
		return;
	}

	std::vector<glm::vec3> nerve_points, nerve_ctrl_points;

	bool is_curr_add_nerve_mode =
		(res_nerve_->curr_add_nerve_id() < 0) ? false : true;

	// recalculate nerve spline points
	if (is_curr_add_nerve_mode)
	{
		nerve_ctrl_points =
			res_nerve_->GetNerveCtrlPoints(res_nerve_->curr_add_nerve_id());

		if (nerve_ctrl_points.size() == 0)
		{
			common::Logger::instance()->Print(
				common::LogType::WRN, "PanoEngine::SetCrossSectionNerveCtrlPoint");
			assert(false);
			return;
		}
		else if (nerve_ctrl_points.size() == 1)
		{
			nerve_points.assign(nerve_ctrl_points.begin(), nerve_ctrl_points.end());
		}
		else
		{
			std::vector<glm::vec3> temp;
			Common::generateCubicSpline(nerve_ctrl_points, temp);
			Common::equidistanceSpline(nerve_points, temp);
		}
	}

	// set nerve control points in cs resource
	for (const auto& cross_data : res_cross_section_->data())
	{
		cross_data.second->set_ctrl_nerve(QPointF(-1.0, -1.0));

		for (const auto& pt_nerve : nerve_points)
		{
			QPointF pt_nerve_in_pano_plane =
				PanoModule::MapVolToPanoPlane(*res_pano(), pt_nerve);
			glm::vec2 vec_nerve_in_pano_plane =
				glm::vec2(pt_nerve_in_pano_plane.x(), pt_nerve_in_pano_plane.y());
			glm::vec2 vec_center_cross_in_pano_plane =
				glm::vec2(cross_data.second->center_position_in_pano_plane().x(),
					cross_data.second->center_position_in_pano_plane().y());

			float radian = glm::radians(res_cross_section_->params().degree);
			glm::vec2 ortho_vector(-sin(radian + M_PI_2), cos(radian + M_PI_2));

			float dist =
				abs(glm::dot(ortho_vector, vec_nerve_in_pano_plane -
					vec_center_cross_in_pano_plane));

			if (dist < 1.0f)
			{
				QPointF pt_cross_plane = PanoModule::MapVolToCrossSectionPlane(
					*res_cross_section_, cross_data.first, pt_nerve);
				cross_data.second->set_ctrl_nerve(pt_cross_plane);
				break;
			}
		}
	}
}
void PanoEngine::SetCrossSectionNerveProjection()
{
	if (res_cross_section_ == nullptr)
	{
		assert(false);
		return;
	}

	const auto& modify_mode = res_nerve_->modify_mode();
	const auto& nerve_points =
		res_nerve_->GetNerveCtrlPoints(modify_mode.nerve_id);

	if (nerve_points.size() == 0 || nerve_points.size() <= modify_mode.nerve_id)
	{
		return common::Logger::instance()->Print(
			common::LogType::WRN,
			"PanoModule::SetNerveProjectionCrossSectionPlane: l0");
	}

	glm::vec3 nerve_point = nerve_points[modify_mode.nerve_selected_index];
	for (const auto& cross_data : res_cross_section_->data())
	{
		QPointF pt_cross_plane = PanoModule::MapVolToCrossSectionPlane(
			*res_cross_section_, cross_data.first, nerve_point);
		cross_data.second->set_proj_nerve(pt_cross_plane);
	}
}

void PanoEngine::SetNerveMaskROI()
{
	const CW3Image3D& vol = this->GetVolume();
	const float z_spacing = vol.sliceSpacing() / vol.pixelSpacing();
	res_nerve_->SetNerveMaskROI(vol.width(), vol.height(), vol.depth(),
		z_spacing);
}

void PanoEngine::SetNervePoints(int nerve_id)
{
	std::vector<glm::vec3> nerve_ctrl_points_in_pano3d;
	PanoModule::MapVolToPano3D(*res_pano(),
		res_nerve_->GetNerveCtrlPoints(nerve_id),
		nerve_ctrl_points_in_pano3d);

	std::vector<glm::vec3> nerve_points_in_pano3d;
	Common::generateCubicSpline(nerve_ctrl_points_in_pano3d,
		nerve_points_in_pano3d);

	std::vector<glm::vec3> nerve_points_in_vol;
	PanoModule::MapPano3DToVol(*res_pano(), nerve_points_in_pano3d,
		nerve_points_in_vol);

	res_nerve_->SetNerve(nerve_id, nerve_points_in_vol, nerve_points_in_pano3d);
}

void PanoEngine::SetNerveMesh(int nerve_id)
{
	const std::vector<glm::vec3>& pts_in_vol =
		res_nerve_->GetNervePointsInVol(nerve_id);
	const std::vector<glm::vec3>& pts_in_pano =
		res_nerve_->GetNervePointsInPano(nerve_id);

	std::vector<glm::vec3> pts_in_vol_gl, pts_in_pano_gl;
	pts_in_vol_gl.reserve(pts_in_vol.size());
	pts_in_pano_gl.reserve(pts_in_pano.size());

	const CW3Image3D& vol = this->GetVolume();
	const float z_spacing = vol.sliceSpacing() / vol.pixelSpacing();
	const glm::vec3 pt_center_vol((float)vol.width() * 0.5f - 1.0f,
		(float)vol.height() * 0.5f - 1.0f,
		(float)vol.depth() * 0.5f - 1.0f);
	for (const auto& elem : pts_in_vol)
	{
		pts_in_vol_gl.push_back(
			kInvXAxis * GLhelper::MapVolToWorldGL(elem, pt_center_vol, z_spacing));
	}

	const glm::vec3 pt_center_pano(
		(float)res_pano()->pano_3d_width() * 0.5f - 1.0f,
		(float)res_pano()->pano_3d_height() * 0.5f - 1.0f, 0.0f);
	for (const auto& elem : pts_in_pano)
	{
		pts_in_pano_gl.push_back(
			kInvXAxis * GLhelper::MapVolToWorldGL(elem, pt_center_pano, z_spacing));
	}

	glm::vec3 nerve_radius_scale_gl =
		GLhelper::ScaleVolToGL(glm::vec3(res_nerve_->GetNerveRadius(nerve_id)));
	res_nerve_->GenerateNerveMeshInVol(nerve_id, pts_in_vol_gl,
		nerve_radius_scale_gl);
	res_nerve_->GenerateNerveMeshInPano(nerve_id, pts_in_pano_gl,
		nerve_radius_scale_gl);
}

const CW3Image3D& PanoEngine::GetVolume() const
{
	return ResourceContainer::GetInstance()->GetMainVolume();
}

const glm::vec3 PanoEngine::GetVolumePos(float z_pos_vol) const
{
	const CW3Image3D& vol = this->GetVolume();
	return glm::vec3((float)vol.width() * 0.5f, (float)vol.height() * 0.5f,
		z_pos_vol);
}

const bool PanoEngine::IsValidVolumePos(const glm::vec3 vol_pos) const
{
	return (vol_pos != panorama::kInvalidVolPt) ? true : false;
}

//20250214 exportPorject함수 Viewer에서도 써야돼서 #if 해제
//#ifndef WILL3D_VIEWER
void PanoEngine::exportProject(ProjectIOPanoEngine& out, QString path)
{
	out.SavePanoROI(roi_vol_.top, roi_vol_.bottom, roi_vol_.slice);
	out.SaveReoriMatrix(ArchTypeID::ARCH_MANDLBLE,
		reorien_mat_[ArchTypeID::ARCH_MANDLBLE]);
	out.SaveReoriMatrix(ArchTypeID::ARCH_MAXILLA,
		reorien_mat_[ArchTypeID::ARCH_MAXILLA]);
	out.SaveImplant3DMVP(projection_view_implant_3d_);
	out.SaveCSShiftedValue(cross_shifted_value_);

	// Q : 어떤 리소스를 가지고 있어야 뷰 상태를 만들 수 있는지?
	// Q : 해당 리소스가 유효한지 확인 어떻게 하는지?
	out.SaveCurrArchType(curr_arch_type_);
	auto func_save_pano = [&](ArchTypeID arch_type)
	{
		if (res_pano_[arch_type])
		{
			out.SavePanoCtrlPoints(arch_type,
				res_pano_[arch_type]->pano_ctrl_points());
			out.SavePanoShiftValue(arch_type, res_pano_[arch_type]->shifted_value());
		}
	};

	for (int i = 0; i < ARCH_TYPE_END; i++) func_save_pano((ArchTypeID)i);

	const CW3Image3D& vol = GetVolume();
	if (res_nerve_)
	{
		const auto& nerve_data = res_nerve_->GetNerveDataInVol();
		std::map<int, std::vector<glm::vec3>> nerve_spline_points;
		for (const auto& elem : nerve_data)
		{
			const auto& points = elem.second->nerve_points();
			nerve_spline_points[elem.first].assign(points.begin(), points.end());
		}

		out.SaveNervePoints(res_nerve_->nerve_ctrl_points(), nerve_spline_points);
		int id = 0;
		out.SaveNerveCount((int)nerve_data.size());
		for (const auto& nd : nerve_data)
		{
			const QColor& color = nd.second->color();

			float base_pixel_spacing_ =
				std::min(vol.pixelSpacing(), vol.sliceSpacing());
			double nerve_diameter_mm =
				(double)((nd.second->radius() * base_pixel_spacing_) * 2.0f);

			out.SaveNerveParams(id++, nd.first, color.red(), color.green(),
				color.blue(), nd.second->is_visible(),
				nd.second->radius(), nerve_diameter_mm);
		}
	}
	if (path != "")
	{
		if (res_implant_) out.SaveImplantResource(*res_implant_.get(), path);
	}
	else
	{
		if (res_implant_) out.SaveImplantResource(*res_implant_.get());
	}

}
//#endif
//20250123 LIN importProject기능 viewer에 적용
void PanoEngine::importProject(ProjectIOPanoEngine& in)
{
	in.LoadPanoROI(roi_vol_.top, roi_vol_.bottom, roi_vol_.slice);
	in.LoadCurrArchType(curr_arch_type_);
	in.LoadReoriMatrix(ArchTypeID::ARCH_MANDLBLE,
		reorien_mat_[ArchTypeID::ARCH_MANDLBLE]);
	in.LoadReoriMatrix(ArchTypeID::ARCH_MAXILLA,
		reorien_mat_[ArchTypeID::ARCH_MAXILLA]);
	in.LoadImplant3DMVP(projection_view_implant_3d_);
	in.LoadCSShiftedValue(cross_shifted_value_);

	auto func_load_pano = [&](ArchTypeID arch_type)
	{
		std::vector<glm::vec3> ctrl_points;
		float shifted_value;
		in.LoadPanoCtrlPoints(arch_type, ctrl_points);
		in.LoadPanoShiftValue(arch_type, shifted_value);

#if 0
		std::vector<glm::vec3> spline_points;
		Common::generateCubicSpline(ctrl_points, spline_points);
		res_pano_[arch_type].reset(new PanoResource(spline_points, ctrl_points, glm::vec3(), 0.0f, 0.0f));
#else
		res_pano_[arch_type].reset(new PanoResource(std::vector<glm::vec3>(), ctrl_points, glm::vec3(), 0.0f, 0.0f));
#endif
		res_pano_[arch_type]->SetShiftedValue(shifted_value);
	};

	for (int i = 0; i < ARCH_TYPE_END; i++) func_load_pano((ArchTypeID)i);

	ResourceContainer::GetInstance()->SetPanoResource(res_pano_[curr_arch_type_]);

	std::map<int, std::vector<glm::vec3>> nerve_ctrl_points;
	std::map<int, std::vector<glm::vec3>> nerve_spline_points;
	in.LoadNervePoints(nerve_ctrl_points, nerve_spline_points);

	int nerve_cnt;
	in.LoadNerveCount(nerve_cnt);

	for (int index = 0; index < nerve_cnt; ++index)
	{
		int color_r, color_g, color_b;
		bool visible;
		float radius;
		double diameter;

		int id;
		in.LoadNerveParams(index, id, color_r, color_g, color_b, visible, radius,
			diameter);

		this->SetNerveParams(visible, id, radius, QColor(color_r, color_g, color_b),
			false);
	}

	for (const auto& ctrl_points : nerve_ctrl_points)
	{
		for (const auto& pt : ctrl_points.second)
		{
			res_nerve_->AddNerveCtrlPoint(ctrl_points.first, pt);
		}
	}

	for (const auto& spline_points : nerve_spline_points)
	{
		res_nerve_->SetNerve(spline_points.first, spline_points.second,
			std::vector<glm::vec3>());
		this->SetNerveMesh(spline_points.first);
	}

	if (nerve_cnt > 0)
	{
		SetNerveMaskROI();
	}

	project::ImplantResParams proj_implant_res;
	in.LoadImplantResource(proj_implant_res);

	std::vector<int> failed_loading_imp_ids;

	res_implant_->set_memo(proj_implant_res.memo);

	CW3ProgressDialog* progress =
		CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);
	QFutureWatcher<void> watcher;
	QObject::connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));
	for (const auto& elem : proj_implant_res.datas)
	{
		bool is_custom_implant = elem.manufacturer[0] == kCustomImplantManufacturerNamePrefix;

		QFuture<bool> future;
		if (is_custom_implant)
		{
			implant_resource::ImplantInfo imp_info(
				elem.path,
				elem.id,
				elem.manufacturer,
				elem.product,
				elem.platform_diameter,
				elem.custom_apical_diameter,
				elem.length
			);
			future = QtConcurrent::run(res_implant_.get(), &ImplantResource::AddImplant, imp_info);
		}
		else
		{
			implant_resource::ImplantInfo imp_info(
				elem.path, elem.id,
				elem.manufacturer, elem.product,
				elem.diameter, elem.length,
				elem.platform_diameter, elem.total_length,
				elem.sub_category
			);
			future = QtConcurrent::run(res_implant_.get(), &ImplantResource::AddImplant, imp_info);
		}

		watcher.setFuture(future);
		progress->exec();
		watcher.waitForFinished();

		if (!future.result())
		{
			failed_loading_imp_ids.push_back(elem.id);
		}
		else
		{
			SetImplantPlaced();

			res_implant_->SetImplantPositionInVol(elem.id, elem.pos_in_vol,
				glm::vec3());
			res_implant_->SetImplantRotateInVol(elem.id, elem.rot_in_vol);
			res_implant_->SetImplantTranslateInVol(elem.id, elem.trans_in_vol);

			res_implant_->SelectImplant(elem.id, false);
		}
	}

#if 0 // selected implant id 는 복원하지 않도록 변경
	if (proj_implant_res.selected_id > 0)
	{
		res_implant_->SelectImplant(proj_implant_res.selected_id, true);
	}
#endif

	if (IsValidImplant())
	{
		res_implant_->SetVisibleAll(false);
	}

	if (!failed_loading_imp_ids.empty())
	{
		CW3MessageBox msg("Will3D", QString("Invalid Implant Model"));
		QString detail_text;
		detail_text += QString("( ");
		for (int i = 0; i < failed_loading_imp_ids.size(); i++)
		{
			detail_text += QString("%1").arg(failed_loading_imp_ids[i]);

			if (i != (int)failed_loading_imp_ids.size() - 1)
				detail_text += QString(", ");
		}
		detail_text += QString(" )");
		msg.setDetailedText(detail_text);
		msg.exec();
	}
}

void PanoEngine::SetCrossSectionAngle(const int implant_id)
{
	auto iter = res_implant_->data().find(implant_id);
	if (iter == res_implant_->data().end())
	{
		return;
	}

	glm::vec3 plane_normal_vector = res_pano()->back_vector();
	glm::mat4 rotate_matrix = iter->second->rotate_in_pano_plane();

	glm::vec3 target_vector = glm::vec3(rotate_matrix * glm::vec4(plane_normal_vector, 1.0f));
	if (target_vector == glm::vec3(0.0f))
	{
		iter->second->set_rotate_degree_in_pano_plane(0.0f);
		return;
	}

	glm::vec3 projection_vector = target_vector - (glm::dot(target_vector, plane_normal_vector) * plane_normal_vector);
	if (projection_vector == glm::vec3(0.0f))
	{
		iter->second->set_rotate_degree_in_pano_plane(0.0f);
		return;
	}

	glm::vec3 nv1 = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 nv2 = glm::normalize(projection_vector);

	float radian = acos(glm::clamp(glm::dot(nv1, nv2), -1.f, 1.f));
	int degree = qRound(glm::degrees(radian));
	degree = degree % 180;

	glm::vec3 cross = glm::cross(nv1, nv2);
	int direction = (cross.z < 0.0f) ? 1 : -1;

	int quotient = degree / 90;
	int remainder = degree % 90;

	degree = remainder - (90 * quotient);

	iter->second->set_rotate_degree_in_pano_plane(degree * direction);
}

void PanoEngine::SetIsImplantWire(const bool wire)
{
	PanoModule::SetIsImplantWire(wire);
}

const bool PanoEngine::GetIsImplantWire()
{
	return PanoModule::GetIsImplantWire();
}
