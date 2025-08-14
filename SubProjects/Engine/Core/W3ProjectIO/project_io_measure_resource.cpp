#include "project_io_measure_resource.h"
#include <qpoint.h>

#include "../../Common/Common/will3d_id_parser.h"
#include "../../Resource/Resource/include/measure_data.h"
#include "datatypes.h"
#include "io_functions.h"
#include "project_path_info.h"

using namespace H5;
using namespace project;

ProjectIOMeasureResource::ProjectIOMeasureResource(const project::Purpose& purpose,
												   const std::shared_ptr<H5::H5File>& file) :
	file_(file) {
	if (purpose == project::Purpose::SAVE) {
		file_->createGroup(group::kResMeasure);
		file_->createGroup(group::kResMeasureViewInfo);
		file_->createGroup(group::kResMeasureDatas);
	}

	vec3_type_ = CompType(sizeof(Vec3Data));
	vec3_type_.insertMember(ds_member::kX, HOFFSET(Vec3Data, x), PredType::NATIVE_FLOAT);
	vec3_type_.insertMember(ds_member::kY, HOFFSET(Vec3Data, y), PredType::NATIVE_FLOAT);
	vec3_type_.insertMember(ds_member::kZ, HOFFSET(Vec3Data, z), PredType::NATIVE_FLOAT);

	measure_info_type_ = CompType(sizeof(MeasureInfo));
	measure_info_type_.insertMember(ds_member::kViewTypeID,
									HOFFSET(MeasureInfo, view_type),
									PredType::NATIVE_INT);
	measure_info_type_.insertMember(ds_member::kMeasureID,
									HOFFSET(MeasureInfo, measure_id),
									PredType::NATIVE_UINT32);
	measure_info_type_.insertMember(ds_member::kMeasureType,
									HOFFSET(MeasureInfo, measure_type),
									PredType::NATIVE_INT);
	measure_info_type_.insertMember(ds_member::kMeasurePointCount,
									HOFFSET(MeasureInfo, point_count),
									PredType::NATIVE_INT);
	measure_info_type_.insertMember(ds_member::kMeasureNoteID,
									HOFFSET(MeasureInfo, note_id),
									PredType::NATIVE_UINT32);
	measure_info_type_.insertMember(ds_member::kMeasureProfileID,
									HOFFSET(MeasureInfo, profile_id),
									PredType::NATIVE_UINT32);
	measure_info_type_.insertMember(ds_member::kVectorCenter,
									HOFFSET(MeasureInfo, vp_center),
									vec3_type_);
	measure_info_type_.insertMember(ds_member::kVectorUp,
									HOFFSET(MeasureInfo, vp_up),
									vec3_type_);
	measure_info_type_.insertMember(ds_member::kVectorBack,
									HOFFSET(MeasureInfo, vp_back),
									vec3_type_);

	view_info_type_ = CompType(sizeof(MeasureViewInfo));
	view_info_type_.insertMember(ds_member::kTabType,
								 HOFFSET(MeasureViewInfo, tab_type),
								 PredType::NATIVE_INT);
	view_info_type_.insertMember(ds_member::kViewTypeID,
								 HOFFSET(MeasureViewInfo, view_type),
								 PredType::NATIVE_INT);
	view_info_type_.insertMember(ds_member::kCenterX,
								 HOFFSET(MeasureViewInfo, center_x),
								 PredType::NATIVE_DOUBLE);
	view_info_type_.insertMember(ds_member::kCenterY,
								 HOFFSET(MeasureViewInfo, center_y),
								 PredType::NATIVE_DOUBLE);
	view_info_type_.insertMember(ds_member::kScale,
								 HOFFSET(MeasureViewInfo, scale),
								 PredType::NATIVE_FLOAT);
	view_info_type_.insertMember(ds_member::kX,
								 HOFFSET(MeasureViewInfo, trans_x),
								 PredType::NATIVE_FLOAT);
	view_info_type_.insertMember(ds_member::kY,
								 HOFFSET(MeasureViewInfo, trans_y),
								 PredType::NATIVE_FLOAT);
	view_info_type_.insertMember(ds_member::kPixelSpacing,
								 HOFFSET(MeasureViewInfo, pixel_spacing),
								 PredType::NATIVE_FLOAT);
}

ProjectIOMeasureResource::~ProjectIOMeasureResource() {}

void ProjectIOMeasureResource::SaveViewParamsCount(int total_count) {
	Group view_info_grp = file_->openGroup(group::kResMeasureViewInfo);
	IOFunctions::WriteInt(view_info_grp, ds::kViewParamsCount, total_count);
	view_info_grp.close();
}

// 내용 및 파라메터 수정해야 함
void ProjectIOMeasureResource::SaveViewParams(const TabType& tab_type, const common::ViewTypeID& view_type,
											  const common::measure::ViewInfo& view_info) {
	Group view_params_grp = file_->openGroup(group::kResMeasureViewInfo);
	const H5std_string kViewParamsGroup = std::to_string(curr_count_view_++);
	Group primitive_view_grp = view_params_grp.createGroup(kViewParamsGroup);

	// view params save
	project::MeasureViewInfo vp;
	vp.tab_type = static_cast<int>(tab_type);
	vp.view_type = static_cast<int>(view_type);
	vp.center_x = view_info.scene_center.x();
	vp.center_y = view_info.scene_center.y();
	vp.scale = view_info.scale;
	vp.trans_x = view_info.translate.x();
	vp.trans_y = view_info.translate.y();
	vp.pixel_spacing = view_info.spacing;
	DataSet dataset = primitive_view_grp.createDataSet(ds::kViewParams, view_info_type_,
													   project::io::kDSScalar);
	dataset.write(&vp, view_info_type_);
	dataset.close();

	primitive_view_grp.close();
	view_params_grp.close();
}

void ProjectIOMeasureResource::SaveCounterparts(const TabType & mpr_counterpart,
												const TabType & pano_conuterpart) {
	Group measure_grp = file_->openGroup(group::kResMeasure);
	IOFunctions::WriteInt(measure_grp, ds::kMeasureCounterpartMPR,
						  static_cast<int>(mpr_counterpart));
	IOFunctions::WriteInt(measure_grp, ds::kMeasureCounterpartPano,
						  static_cast<int>(pano_conuterpart));
	measure_grp.close();
}

void ProjectIOMeasureResource::SaveMeasureCount(int total_count) {
	Group measure_data_grp = file_->openGroup(group::kResMeasureDatas);
	IOFunctions::WriteInt(measure_data_grp, ds::kMeasureCount, total_count);
	measure_data_grp.close();
}

void ProjectIOMeasureResource::SaveMeasure(const common::ViewTypeID& view_type,
										   const MeasureData& measure_data) {
	Group measure_data_grp = file_->openGroup(group::kResMeasureDatas);
	const H5std_string kMeasureGroup = std::to_string(curr_count_measure_++);
	Group primitive_measure_grp = measure_data_grp.createGroup(kMeasureGroup);

	// measure info save
	MeasureInfo measure_info;
	measure_info.view_type = static_cast<int>(view_type);
	measure_info.measure_id = measure_data.id();
	measure_info.measure_type = static_cast<int>(measure_data.type());
	measure_info.point_count = measure_data.points().size();
	measure_info.note_id = measure_data.note_id();
	measure_info.profile_id = measure_data.profile_id();

	common::measure::VisibilityParams vp = measure_data.visibility_params();
	measure_info.vp_center.x = vp.center.x;
	measure_info.vp_center.y = vp.center.y;
	measure_info.vp_center.z = vp.center.z;
	measure_info.vp_up.x = vp.up.x;
	measure_info.vp_up.y = vp.up.y;
	measure_info.vp_up.z = vp.up.z;
	measure_info.vp_back.x = vp.back.x;
	measure_info.vp_back.y = vp.back.y;
	measure_info.vp_back.z = vp.back.z;

	DataSet dataset = primitive_measure_grp.createDataSet(ds::kMeasureInfo,
														  measure_info_type_,
														  project::io::kDSScalar);
	dataset.write(&measure_info, measure_info_type_);
	dataset.close();

	// measure points save
	const std::vector<QPointF>& points = measure_data.points();
	Group primitive_point_grp = primitive_measure_grp.createGroup(group::kMeasurePoints);
	for (int idx = 0; idx < points.size(); ++idx) {
		dataset = primitive_point_grp.createDataSet(std::to_string(idx), PredType::NATIVE_FLOAT,
													io::kDSPair);
		float data[2] = { static_cast<float>(points[idx].x()),
						  static_cast<float>(points[idx].y()) };
		dataset.write(data, PredType::NATIVE_FLOAT);
	}
	dataset.close();
	primitive_point_grp.close();

	// measure note save
	QByteArray buffer = measure_data.note_txt().toLocal8Bit();
	std::string note_string = QString::fromLocal8Bit(buffer).toStdString();
	IOFunctions::WriteString(primitive_measure_grp, ds::kMeasureNoteText, note_string);

	// measure memo save
	buffer = measure_data.memo().toLocal8Bit();
	std::string memo_string = QString::fromLocal8Bit(buffer).toStdString();
	IOFunctions::WriteString(primitive_measure_grp, ds::kMeasureMemoText, memo_string);

	primitive_measure_grp.close();
	measure_data_grp.close();
}

int ProjectIOMeasureResource::LoadViewParamsCount() {
	if (!IsValid())
	{
		return 0;
	}

	Group view_info_grp = file_->openGroup(group::kResMeasureViewInfo);
	int total_count = IOFunctions::ReadInt(view_info_grp, ds::kViewParamsCount);
	view_info_grp.close();
	return total_count;
}

void ProjectIOMeasureResource::LoadViewParams(TabType& tab_type, common::ViewTypeID& view_type,
											  common::measure::ViewInfo& view_info) {
	if (!IsValid())
	{
		return;
	}

	Group view_params_grp = file_->openGroup(group::kResMeasureViewInfo);
	const H5std_string kViewParamsGroup = std::to_string(curr_count_view_++);
	Group primitive_view_grp = view_params_grp.openGroup(kViewParamsGroup);
	DataSet dataset = primitive_view_grp.openDataSet(ds::kViewParams);
	project::MeasureViewInfo vp;
	dataset.read(&vp, view_info_type_);
	dataset.close();
	primitive_view_grp.close();
	view_params_grp.close();

	tab_type = static_cast<TabType>(vp.tab_type);
	view_type = static_cast<common::ViewTypeID>(vp.view_type);
	view_info.scene_center = QPointF(vp.center_x, vp.center_y);
	view_info.scale = vp.scale;
	view_info.translate = QPointF(vp.trans_x, vp.trans_y);
	view_info.spacing = vp.pixel_spacing;
}

void ProjectIOMeasureResource::LoadCounterparts(TabType & mpr_counterpart,
												TabType & pano_counterpart) {
	if (!IsValid())
	{
		return;
	}

	Group measure_grp = file_->openGroup(group::kResMeasure);
	mpr_counterpart = static_cast<TabType>(IOFunctions::ReadInt(measure_grp, ds::kMeasureCounterpartMPR));
	pano_counterpart = static_cast<TabType>(IOFunctions::ReadInt(measure_grp, ds::kMeasureCounterpartPano));
	measure_grp.close();
}

int ProjectIOMeasureResource::LoadMeasureCount() {
	if (!IsValid())
	{
		return 0;
	}

	Group measure_data_grp = file_->openGroup(group::kResMeasureDatas);
	int total_count = IOFunctions::ReadInt(measure_data_grp, ds::kMeasureCount);
	measure_data_grp.close();
	return total_count;
}

void ProjectIOMeasureResource::LoadMeasure(common::ViewTypeID* view_type, MeasureData*& measure_data) {
	if (!IsValid())
	{
		return;
	}

	Group measure_data_grp = file_->openGroup(group::kResMeasureDatas);
	const H5std_string kMeasureGroup = std::to_string(curr_count_measure_++);
	Group primitive_measure_grp = measure_data_grp.openGroup(kMeasureGroup);
	DataSet dataset = primitive_measure_grp.openDataSet(ds::kMeasureInfo);
	MeasureInfo measure_info;
	dataset.read(&measure_info, measure_info_type_);
	dataset.close();

	*view_type = static_cast<common::ViewTypeID>(measure_info.view_type);
	common::measure::VisibilityParams vp;
	vp.center = glm::vec3(measure_info.vp_center.x,
						  measure_info.vp_center.y,
						  measure_info.vp_center.z);
	vp.up = glm::vec3(measure_info.vp_up.x,
					  measure_info.vp_up.y,
					  measure_info.vp_up.z);
	vp.back = glm::vec3(measure_info.vp_back.x,
						measure_info.vp_back.y,
						measure_info.vp_back.z);
	measure_data = new MeasureData(Will3DIDParser::GetMeasureID(),
								   static_cast<common::measure::MeasureType>(measure_info.measure_type),
								   vp, 0.0f, 0.0f);

	// measure points load
	Group primitive_point_grp = primitive_measure_grp.openGroup(group::kMeasurePoints);
	std::vector<QPointF> points;
	points.reserve(measure_info.point_count);
	for (int idx = 0; idx < measure_info.point_count; ++idx) {
		dataset = primitive_point_grp.openDataSet(std::to_string(idx));
		float data[2] = { 0.0f };
		dataset.read(data, PredType::NATIVE_FLOAT);
		points.push_back(QPointF(data[0], data[1]));
	}
	dataset.close();
	primitive_point_grp.close();
	measure_data->set_points(points);

	// measure note load
	std::string memo_string = IOFunctions::ReadString(primitive_measure_grp, ds::kMeasureMemoText);
	QString memo_txt = QString::fromLocal8Bit(memo_string.c_str());
	measure_data->set_memo(memo_txt);

	// measure memo load
	std::string note_string = IOFunctions::ReadString(primitive_measure_grp, ds::kMeasureNoteText);
	QString note_txt = QString::fromLocal8Bit(note_string.c_str());
	measure_data->set_note_txt(note_txt);

	primitive_measure_grp.close();
	measure_data_grp.close();
}

bool ProjectIOMeasureResource::IsValid()
{
	return 
		file_->exists(group::kResMeasure) && 
		file_->exists(group::kResMeasureViewInfo) && 
		file_->exists(group::kResMeasureDatas);
}
