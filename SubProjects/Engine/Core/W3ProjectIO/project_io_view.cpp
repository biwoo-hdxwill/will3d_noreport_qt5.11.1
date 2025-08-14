#include "project_io_view.h"

#include <QDebug>
#include <QPointF>

#include "../../Common/Common/W3Define.h"
#include "../../Resource/Resource/W3ViewPlane.h"

#include "project_path_info.h"
#include "io_functions.h"
#include "datatypes.h"

using namespace H5;
using namespace project;

ProjectIOView::ProjectIOView(const std::shared_ptr<H5::H5File>& file,
	const H5::Group& curr_view_group) :
	file_(file), view_group_(curr_view_group)
{
	const H5std_string view_group_name = view_group_.getObjName();
	bool exists = view_group_.exists(view_group_name + group::kMeasure3D);
	if (!exists)
	{
		Group measure_data_group = view_group_.createGroup(view_group_name + group::kMeasure3D);
		for (int i = 0; i < static_cast<int>(Measure3DType::END); ++i)
		{
			measure_data_group.createGroup(std::to_string(i));
		}
	}
}

ProjectIOView::~ProjectIOView() {}

void ProjectIOView::SaveRotateMatrix(const glm::mat4 & mat)
{
	IOFunctions::WriteMatrix(view_group_, ds::k3DRotMatrix, mat);
}

void ProjectIOView::SaveVolRange(const glm::vec3& vol_range)
{
	IOFunctions::WriteVec3(view_group_, ds::kVolRange, vol_range);
}

void ProjectIOView::SaveViewInfo(const float& scale, const float& scale_scene_to_gl,
	const float& gl_trans_x, const float& gl_trans_y)
{
	IOFunctions::WriteFloat(view_group_, ds::kViewScale, scale);
	IOFunctions::WriteFloat(view_group_, ds_member::kSceneToGL, scale_scene_to_gl);
	IOFunctions::WriteFloat(view_group_, ds_member::kX, gl_trans_x);
	IOFunctions::WriteFloat(view_group_, ds_member::kY, gl_trans_y);
}

void ProjectIOView::SaveViewInfo(const float& scale, const float& scale_scene_to_gl,
	const float& gl_trans_x, const float& gl_trans_y,
	const int window_center, const int window_width)
{
	SaveViewInfo(scale, scale_scene_to_gl, gl_trans_x, gl_trans_y);
	IOFunctions::WriteInt(view_group_, ds_member::kWindowCenter, window_center);
	IOFunctions::WriteInt(view_group_, ds_member::kWindowWidth, window_width);
}

void ProjectIOView::SaveMeasure3D(const Measure3DType type, const int index, const std::vector<glm::vec3>& points)
{
	const H5std_string view_group_name = view_group_.getObjName();
	Group measure_data_group = view_group_.openGroup(view_group_name + group::kMeasure3D);
	Group measure_type_group = measure_data_group.openGroup(std::to_string(static_cast<int>(type)));
	Group primitive_measure_grp = measure_type_group.createGroup(std::to_string(index));

	IOFunctions::WriteVec3List(primitive_measure_grp, ds::kMeasure3DPoint, points);

	primitive_measure_grp.close();
	measure_type_group.close();
	measure_data_group.close();
}

void ProjectIOView::SaveMeasure3DCount(const Measure3DType type, const int count)
{
	const H5std_string view_group_name = view_group_.getObjName();
	Group measure_data_group = view_group_.openGroup(view_group_name + group::kMeasure3D);
	Group measure_type_group = measure_data_group.openGroup(std::to_string(static_cast<int>(type)));

	IOFunctions::WriteInt(measure_type_group, ds::kMeasure3DCount, count);

	measure_type_group.close();
	measure_data_group.close();
}

void ProjectIOView::SaveViewPlane(CW3ViewPlane* view_plane)
{
	IOFunctions::WriteVec3(view_group_, ds::kViewPlaneUpVector, view_plane->getUpVec());
	IOFunctions::WriteVec3(view_group_, ds::kViewPlaneBackVector, view_plane->getBackVec());
	IOFunctions::WriteVec3(view_group_, ds::kViewPlaneCenter, view_plane->getPlaneCenterInVol());
	IOFunctions::WriteFloat(view_group_, ds::kViewPlaneDistanceFromVolumeCenter, view_plane->getDistFromVolCenter());
	IOFunctions::WriteFloat(view_group_, ds::kViewPlaneAvailableDepth, view_plane->getAvailableDetph());
}

void ProjectIOView::LoadRotateMatrix(glm::mat4 & mat)
{
	mat = IOFunctions::ReadMatrix(view_group_, ds::k3DRotMatrix);
}

void ProjectIOView::LoadVolRange(glm::vec3& vol_range)
{
	vol_range = IOFunctions::ReadVec3(view_group_, ds::kVolRange);
}

void ProjectIOView::LoadViewInfo(float& scale, float& scale_scene_to_gl,
	float& gl_trans_x, float& gl_trans_y)
{
	scale = IOFunctions::ReadFloat(view_group_, ds::kViewScale);
	scale_scene_to_gl = IOFunctions::ReadFloat(view_group_, ds_member::kSceneToGL);
	gl_trans_x = IOFunctions::ReadFloat(view_group_, ds_member::kX);
	gl_trans_y = IOFunctions::ReadFloat(view_group_, ds_member::kY);
}

void ProjectIOView::LoadViewInfo(float& scale, float& scale_scene_to_gl,
	float& gl_trans_x, float& gl_trans_y,
	int& window_center, int& window_width)
{
	LoadViewInfo(scale, scale_scene_to_gl, gl_trans_x, gl_trans_y);
	IOFunctions::ReadInt(view_group_, ds_member::kWindowCenter, window_center);
	IOFunctions::ReadInt(view_group_, ds_member::kWindowWidth, window_width);
}

void ProjectIOView::LoadMeasure3D(const Measure3DType type, const int index, std::vector<glm::vec3>& points)
{
	const H5std_string view_group_name = view_group_.getObjName();
	Group measure_data_group = view_group_.openGroup(view_group_name + group::kMeasure3D);
	Group measure_type_group = measure_data_group.openGroup(std::to_string(static_cast<int>(type)));
	Group primitive_measure_grp = measure_type_group.openGroup(std::to_string(index));

	points = IOFunctions::ReadVec3List(primitive_measure_grp, ds::kMeasure3DPoint);

	primitive_measure_grp.close();
	measure_type_group.close();
	measure_data_group.close();
}

void ProjectIOView::LoadMeasure3DCount(const Measure3DType type, int& count)
{
	const H5std_string view_group_name = view_group_.getObjName();
	H5std_string measure_data_group_name = view_group_name + group::kMeasure3D;
	if (!view_group_.exists(measure_data_group_name))
	{
		return;
	}
	Group measure_data_group = view_group_.openGroup(measure_data_group_name);
	H5std_string measure_type_group_name = std::to_string(static_cast<int>(type));
	if (!measure_data_group.exists(measure_type_group_name))
	{
		return;
	}
	Group measure_type_group = measure_data_group.openGroup(measure_type_group_name);

	count = IOFunctions::ReadInt(measure_type_group, ds::kMeasure3DCount);

	measure_type_group.close();
	measure_data_group.close();
}

void ProjectIOView::LoadViewPlane(CW3ViewPlane* view_plane)
{
#if 0
	glm::vec3 up_vector = IOFunctions::ReadVec3(view_group_, ds::kViewPlaneUpVector);
	glm::vec3 back_vector = IOFunctions::ReadVec3(view_group_, ds::kViewPlaneBackVector);
	glm::vec3 center = IOFunctions::ReadVec3(view_group_, ds::kViewPlaneCenter);
	float distance_from_volume_center = IOFunctions::ReadFloat(view_group_, ds::kViewPlaneDistanceFromVolumeCenter);
	float available_depth = IOFunctions::ReadFloat(view_group_, ds::kViewPlaneAvailableDepth);

	view_plane->set_up(up_vector);
	view_plane->setBackVec(back_vector);
	view_plane->setRightVec();
	view_plane->set_plane_center(center);
	view_plane->set_dist_from_vol_center(distance_from_volume_center);
	view_plane->setAvailableDepth(available_depth);
#else
	glm::vec3 up_vector = IOFunctions::ReadVec3(view_group_, ds::kViewPlaneUpVector);
	glm::vec3 back_vector = IOFunctions::ReadVec3(view_group_, ds::kViewPlaneBackVector);
	glm::vec3 center = IOFunctions::ReadVec3(view_group_, ds::kViewPlaneCenter);
	float distance_from_volume_center = IOFunctions::ReadFloat(view_group_, ds::kViewPlaneDistanceFromVolumeCenter);
	float available_depth = IOFunctions::ReadFloat(view_group_, ds::kViewPlaneAvailableDepth);

	if (IOFunctions::ReadVec3(view_group_, ds::kViewPlaneUpVector, up_vector))
	{
		view_plane->set_up(up_vector);
	}
	if (IOFunctions::ReadVec3(view_group_, ds::kViewPlaneBackVector, back_vector))
	{
		view_plane->setBackVec(back_vector);
	}
	view_plane->setRightVec();
	if (IOFunctions::ReadVec3(view_group_, ds::kViewPlaneCenter, center))
	{
		view_plane->set_plane_center(center);
	}
	if (IOFunctions::ReadFloat(view_group_, ds::kViewPlaneDistanceFromVolumeCenter, distance_from_volume_center))
	{
		view_plane->set_dist_from_vol_center(distance_from_volume_center);
	}
	if (IOFunctions::ReadFloat(view_group_, ds::kViewPlaneAvailableDepth, available_depth))
	{
		view_plane->setAvailableDepth(available_depth);
	}
#endif
}

void ProjectIOView::GetVersionInfo(int& version)
{
	DataSet dataset = file_->openDataSet(ds::kVersionInfo);
	int project_version = 0;
	dataset.read(&project_version, PredType::NATIVE_INT);
	dataset.close();

	version = project_version;
}
