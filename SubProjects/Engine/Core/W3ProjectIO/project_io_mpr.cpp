#include "project_io_mpr.h"
#include <H5Cpp.h>

#include "../../Common/Common/W3Logger.h"
#include "io_functions.h"
#include "project_io_view.h"
#include "project_path_info.h"

using namespace H5;
using namespace project;

namespace
{
	H5::Group GetMPRGroup(H5File* file, const ProjectIOMPR::ViewType& view_type)
	{
		switch (view_type)
		{
		case ProjectIOMPR::ViewType::AXIAL:
			return file->openGroup(group::kViewMPRAxial);
		case ProjectIOMPR::ViewType::SAGITTAL:
			return file->openGroup(group::kViewMPRSagittal);
		case ProjectIOMPR::ViewType::CORONAL:
			return file->openGroup(group::kViewMPRCoronal);
		case ProjectIOMPR::ViewType::VR:
			return file->openGroup(group::kViewMPR3D);
		case ProjectIOMPR::ViewType::ZOOM3D:
			return file->openGroup(group::kViewMPRZoom3D);
		default:
			common::Logger::instance()->Print(common::LogType::ERR, "Project IO MPR : invalid view Type");
			return Group();
		}
	}
}  // end of namespace

ProjectIOMPR::ProjectIOMPR(const project::Purpose& purpose,
	const std::shared_ptr<H5::H5File>& file)
	: file_(file)
{
	if (purpose == project::Purpose::SAVE)
	{
		file_->createGroup(group::kTabMPR);
		file_->createGroup(group::kViewMPRAxial);
		file_->createGroup(group::kViewMPRSagittal);
		file_->createGroup(group::kViewMPRCoronal);
		file_->createGroup(group::kViewMPR3D);
		file_->createGroup(group::kViewMPRZoom3D);
	}
}
ProjectIOMPR::~ProjectIOMPR() {}

void ProjectIOMPR::InitializeView(const ProjectIOMPR::ViewType& view_type)
{
	curr_view_type_ = view_type;
	view_io_.reset(new ProjectIOView(file_, GetMPRGroup(file_.get(), curr_view_type_)));
}

void ProjectIOMPR::InitMPRTab()
{
	Group tab_grp = file_->openGroup(group::kTabMPR);
	IOFunctions::WriteBool(tab_grp, ds::kIsTabInit, true);
	tab_grp.close();
}

bool ProjectIOMPR::IsInit()
{
	bool exists = false;
	Group tab_grp = file_->openGroup(group::kTabMPR);
	if (tab_grp.exists(ds::kIsTabInit))
	{
		exists = true;
	}
	tab_grp.close();
	return exists;
}

void ProjectIOMPR::SaveRegistrationParams(float* params, int count)
{
	Group tab_mpr = file_->openGroup(group::kTabMPR);
	hsize_t dim[] = { static_cast<hsize_t>(count) };
	DataSpace space(1, dim);
	DataSet dataset = tab_mpr.createDataSet(ds::kMPRRegistrationParams, PredType::NATIVE_FLOAT, space);
	dataset.write(params, PredType::NATIVE_FLOAT);
	dataset.close();
	tab_mpr.close();
}

void ProjectIOMPR::LoadRegistrationParams(float* params)
{
	Group tab_mpr = file_->openGroup(group::kTabMPR);
	if (!tab_mpr.exists(ds::kMPRRegistrationParams))
	{
		return;
	}

	DataSet dataset = tab_mpr.openDataSet(ds::kMPRRegistrationParams);
	dataset.read(params, PredType::NATIVE_FLOAT);
	dataset.close();
	tab_mpr.close();
}

void ProjectIOMPR::SaveSecondTransformMatrix(const glm::mat4& mat)
{
	Group curr_view_group = GetMPRGroup(file_.get(), curr_view_type_);
	IOFunctions::WriteMatrix(curr_view_group, ds::kMPRSecondTransform, mat);
}

void ProjectIOMPR::LoadSecondTransformMatrix(glm::mat4& mat)
{
	Group curr_view_group = GetMPRGroup(file_.get(), curr_view_type_);
	mat = IOFunctions::ReadMatrix(curr_view_group, ds::kMPRSecondTransform);
}

void ProjectIOMPR::SaveRotateMatrix(const glm::mat4& mat)
{
	Group curr_view_group = GetMPRGroup(file_.get(), curr_view_type_);
	IOFunctions::WriteMatrix(curr_view_group, ds::k3DRotMatrix, mat);
}

void ProjectIOMPR::LoadRotateMatrix(glm::mat4& mat)
{
	Group curr_view_group = GetMPRGroup(file_.get(), curr_view_type_);
	mat = IOFunctions::ReadMatrix(curr_view_group, ds::k3DRotMatrix);
}

void ProjectIOMPR::SaveTransformStatusCount(const int& cnt)
{
	Group tab_mpr = file_->openGroup(group::kTabMPR);
	IOFunctions::WriteInt(tab_mpr, ds::kMPRTransformStatusCnt, cnt);
	file_->createGroup(group::kMPRTransformStatus);
	tab_mpr.close();
}

void ProjectIOMPR::LoadTransformStatusCount(int& cnt)
{
	if (!file_->exists(group::kMPRTransformStatus))
	{
		cnt = 0;
		return;
	}

	Group tab_mpr = file_->openGroup(group::kTabMPR);
	cnt = IOFunctions::ReadInt(tab_mpr, ds::kMPRTransformStatusCnt);
	tab_mpr.close();
}

void ProjectIOMPR::SaveThicknessInterval(const float& thickness, const float& interval)
{
	Group curr_view_group = GetMPRGroup(file_.get(), curr_view_type_);
	IOFunctions::WriteFloat(curr_view_group, ds::kMPRThickness, thickness);
	IOFunctions::WriteFloat(curr_view_group, ds::kMPRInterval, interval);
}

void ProjectIOMPR::LoadThicknessInterval(float& thickenss, float& interval)
{
	Group curr_view_group = GetMPRGroup(file_.get(), curr_view_type_);
	thickenss = IOFunctions::ReadFloat(curr_view_group, ds::kMPRThickness);
	interval = IOFunctions::ReadFloat(curr_view_group, ds::kMPRInterval);
}

void ProjectIOMPR::SaveTransformStatusForMeasure(
	const unsigned int& measure_id, const glm::mat4& axial,
	const glm::mat4& sagittal, const glm::mat4& coronal,
	const glm::vec3& rot_center, const float& axial_angle,
	const float& sagittal_angle, const float& coronal_angle)
{
	Group tab_mpr = file_->openGroup(group::kTabMPR);
	const H5std_string kTransformGrp = std::to_string(curr_cnt_transform_status_++);
	Group primitive_transform_grp = tab_mpr.createGroup(kTransformGrp);

	IOFunctions::WriteUInt(primitive_transform_grp, ds::kMPRTransformMeasureID, measure_id);
	IOFunctions::WriteMatrix(primitive_transform_grp, ds::kMPRTransformAxial, axial);
	IOFunctions::WriteMatrix(primitive_transform_grp, ds::kMPRTransformSagittal, sagittal);
	IOFunctions::WriteMatrix(primitive_transform_grp, ds::kMPRTransformCoronal, coronal);
	IOFunctions::WriteVec3(primitive_transform_grp, ds::kMPRTransformRotCenter, rot_center);
	IOFunctions::WriteFloat(primitive_transform_grp, ds::kMPRTransformAxialAngle, axial_angle);
	IOFunctions::WriteFloat(primitive_transform_grp, ds::kMPRTransformSagittalAngle, sagittal_angle);
	IOFunctions::WriteFloat(primitive_transform_grp, ds::kMPRTransformCoronalAngle, coronal_angle);

	primitive_transform_grp.close();
	tab_mpr.close();
}

void ProjectIOMPR::LoadTransformStatusForMeasure(
	unsigned int& measure_id, glm::mat4& axial, glm::mat4& sagittal,
	glm::mat4& coronal, glm::vec3& rot_center, float& axial_angle,
	float& sagittal_angle, float& coronal_angle)
{
	Group tab_mpr = file_->openGroup(group::kTabMPR);
	const H5std_string kTransformGrp = std::to_string(curr_cnt_transform_status_++);
	Group primitive_transform_grp = tab_mpr.openGroup(kTransformGrp);

	measure_id = IOFunctions::ReadUInt(primitive_transform_grp, ds::kMPRTransformMeasureID);
	axial = IOFunctions::ReadMatrix(primitive_transform_grp, ds::kMPRTransformAxial);
	sagittal = IOFunctions::ReadMatrix(primitive_transform_grp, ds::kMPRTransformSagittal);
	coronal = IOFunctions::ReadMatrix(primitive_transform_grp, ds::kMPRTransformCoronal);
	rot_center = IOFunctions::ReadVec3(primitive_transform_grp, ds::kMPRTransformRotCenter);
	axial_angle = IOFunctions::ReadFloat(primitive_transform_grp, ds::kMPRTransformAxialAngle);
	sagittal_angle = IOFunctions::ReadFloat(primitive_transform_grp, ds::kMPRTransformSagittalAngle);
	coronal_angle = IOFunctions::ReadFloat(primitive_transform_grp, ds::kMPRTransformCoronalAngle);

	primitive_transform_grp.close();
	tab_mpr.close();
}

void ProjectIOMPR::SaveCrossController(const QPointF& center_pos, const float rotate_angle)
{
	Group curr_view_group = GetMPRGroup(file_.get(), curr_view_type_);
	glm::vec2 center_pos_vec2(center_pos.x(), center_pos.y());
	IOFunctions::WriteVec2(curr_view_group, ds::kMPRCrossControllerCenterPos, center_pos_vec2);
	IOFunctions::WriteFloat(curr_view_group, ds::kMPRCrossControllerRotateAngle, rotate_angle);
}

void ProjectIOMPR::SaveCrossControllerDelta(const QPointF& delta_pos, const float rotate_angle)
{
	Group curr_view_group = GetMPRGroup(file_.get(), curr_view_type_);
	glm::vec2 delta_pos_vec2(delta_pos.x(), delta_pos.y());
	IOFunctions::WriteVec2(curr_view_group, ds::kMPRCrossControllerCenterPosDelta, delta_pos_vec2);
	IOFunctions::WriteFloat(curr_view_group, ds::kMPRCrossControllerRotateAngle, rotate_angle);
}

bool ProjectIOMPR::LoadCrossController(QPointF& center_pos, float& rotate_angle)
{
	Group curr_view_group = GetMPRGroup(file_.get(), curr_view_type_);

	glm::vec2 center_pos_vec2;
	float angle = 0.0f;

	bool result = false;
	if (IOFunctions::ReadVec2(curr_view_group, ds::kMPRCrossControllerCenterPos, center_pos_vec2))
	{
		center_pos = QPointF(center_pos_vec2.x, center_pos_vec2.y);
		result |= true;
	}
	if (IOFunctions::ReadFloat(curr_view_group, ds::kMPRCrossControllerRotateAngle, angle))
	{
		rotate_angle = angle;
		result |= true;
	}

	return result;
}

void ProjectIOMPR::LoadCrossControllerDelta(QPointF& delta_pos, float& rotate_angle)
{
	Group curr_view_group = GetMPRGroup(file_.get(), curr_view_type_);

	glm::vec2 delta_pos_vec2;
	float angle = 0.0f;

	if (IOFunctions::ReadVec2(curr_view_group, ds::kMPRCrossControllerCenterPosDelta, delta_pos_vec2))
	{
		delta_pos = QPointF(delta_pos_vec2.x, delta_pos_vec2.y);
	}
	if (IOFunctions::ReadFloat(curr_view_group, ds::kMPRCrossControllerRotateAngle, angle))
	{
		rotate_angle = angle;
	}
}

ProjectIOView& ProjectIOMPR::GetViewIO()
{
	return *(view_io_.get());
}
