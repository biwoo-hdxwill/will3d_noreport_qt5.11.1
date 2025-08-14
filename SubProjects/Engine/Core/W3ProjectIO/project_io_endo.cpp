#include "project_io_endo.h"

#include <H5Cpp.h>

#include "../../Common/Common/W3Logger.h"

#include "project_path_info.h"
#include "io_functions.h"
#include "project_io_view.h"

using namespace H5;
using namespace project;

namespace {
H5::Group GetEndoGroup(H5File* file, const ProjectIOEndo::ViewType& view_type) {
	switch (view_type) {
	case ProjectIOEndo::ViewType::ENDO:
		return file->openGroup(group::kViewEndo);
	case ProjectIOEndo::ViewType::SAGITTAL:
		return file->openGroup(group::kViewEndoSagittal);
	case ProjectIOEndo::ViewType::MODIFY:
		return file->openGroup(group::kViewEndoModify);
	case ProjectIOEndo::ViewType::SLICE:
		return file->openGroup(group::kViewEndoSlice);
	default:
		common::Logger::instance()->Print(common::LogType::ERR,
										  "Project IO SI : invalid view Type");
		return Group();
	}
}
} // end of namespace

ProjectIOEndo::ProjectIOEndo(const project::Purpose& purpose,
							 const std::shared_ptr<H5::H5File>& file) :
	file_(file) {
	if (purpose == project::Purpose::SAVE) {
		file_->createGroup(group::kTabEndo);
		file_->createGroup(group::kViewEndo);
		file_->createGroup(group::kViewEndoModify);
		file_->createGroup(group::kViewEndoSlice);
		file_->createGroup(group::kViewEndoSagittal);
		file_->createGroup(group::kEndoPath);
	}
}

ProjectIOEndo::~ProjectIOEndo() {}

void ProjectIOEndo::InitEndoTab() {
	Group tab_grp = file_->openGroup(group::kTabEndo);
	IOFunctions::WriteBool(tab_grp, ds::kIsTabInit, true);
	tab_grp.close();
}

bool ProjectIOEndo::IsInit() {
	bool exists = false;
	Group tab_grp = file_->openGroup(group::kTabEndo);
	if (tab_grp.exists(ds::kIsTabInit))
		exists = true;
	tab_grp.close();
	return exists;
}

void ProjectIOEndo::InitializeView(ProjectIOEndo::ViewType view_type) {
	curr_view_type_ = view_type;

	view_io_.reset(
		new ProjectIOView(file_, GetEndoGroup(file_.get(), curr_view_type_)));
}

ProjectIOView & ProjectIOEndo::GetViewIO() {
	return *(view_io_.get());
}

void ProjectIOEndo::SaveCurrPathNum(const int & number) {
	Group sagittal_grp = file_->openGroup(group::kViewEndoSagittal);
	IOFunctions::WriteInt(sagittal_grp, ds::kEndoCurrPathNum, number);
	sagittal_grp.close();
}

void ProjectIOEndo::SavePath(const int & id, const std::vector<glm::vec3>& path) {
	Group path_grp = file_->openGroup(group::kEndoPath);
	const std::string kEndoPathID = std::to_string(id);
	IOFunctions::WriteVec3List(path_grp, kEndoPathID, path);
	path_grp.close();
}

void ProjectIOEndo::SaveAirway(const std::vector<tri_STL>& airway,
							   const double& airway_size) {
	CompType vec3_type(sizeof(Vec3Data));
	vec3_type.insertMember(ds_member::kX, HOFFSET(Vec3Data, x), PredType::NATIVE_FLOAT);
	vec3_type.insertMember(ds_member::kY, HOFFSET(Vec3Data, y), PredType::NATIVE_FLOAT);
	vec3_type.insertMember(ds_member::kZ, HOFFSET(Vec3Data, z), PredType::NATIVE_FLOAT);

	CompType stl_info(sizeof(STLTri));
	stl_info.insertMember(ds_member::kNormal, HOFFSET(STLTri, normal), vec3_type);
	stl_info.insertMember(ds_member::kV1, HOFFSET(STLTri, v1), vec3_type);
	stl_info.insertMember(ds_member::kV2, HOFFSET(STLTri, v2), vec3_type);
	stl_info.insertMember(ds_member::kV3, HOFFSET(STLTri, v3), vec3_type);
	stl_info.insertMember(ds_member::kColor, HOFFSET(STLTri, color), vec3_type);
	stl_info.insertMember(ds_member::kCntAttributes, HOFFSET(STLTri, cnt_attributes),
						  PredType::NATIVE_USHORT);
	stl_info.insertMember(ds_member::kColorValue, HOFFSET(STLTri, color_value),
						  PredType::NATIVE_UINT);

	STLTri* tries = new STLTri[airway.size()];
	for (int idx = 0; idx < airway.size(); ++idx) {
		const auto& tri = airway[idx];
		tries[idx].normal = Vec3Data(tri.normal.x, tri.normal.y, tri.normal.z);
		tries[idx].v1 = Vec3Data(tri.v1.x, tri.v1.y, tri.v1.z);
		tries[idx].v2 = Vec3Data(tri.v2.x, tri.v2.y, tri.v2.z);
		tries[idx].v3 = Vec3Data(tri.v3.x, tri.v3.y, tri.v3.z);
		tries[idx].color = Vec3Data(tri.fColor.x, tri.fColor.y, tri.fColor.z);
		tries[idx].cnt_attributes = tri.cntAttributes;
		tries[idx].color_value = tri.nColorVal;
	}

	Group airway_grp = file_->createGroup(group::kResAirway);
	hsize_t dim[] = { airway.size() };
	DataSpace space(1, dim);
	DataSet dataset = airway_grp.createDataSet(ds::kAirwayData, stl_info, space);
	dataset.write(tries, stl_info);
	delete[] tries;

	IOFunctions::WriteDouble(airway_grp, ds::kAirwaySize, airway_size);
	dataset.close();
	airway_grp.close();
}

void ProjectIOEndo::LoadCurrPathNum(int & number) {
	Group sagittal_grp = file_->openGroup(group::kViewEndoSagittal);
	number = IOFunctions::ReadInt(sagittal_grp, ds::kEndoCurrPathNum);
	sagittal_grp.close();
}

void ProjectIOEndo::LoadPath(const int & id, std::vector<glm::vec3>& path) {
	path.clear();
	Group path_grp = file_->openGroup(group::kEndoPath);
	const std::string kEndoPathID = std::to_string(id);
	if (!path_grp.exists(kEndoPathID))
		return;
	path = IOFunctions::ReadVec3List(path_grp, kEndoPathID);
	path_grp.close();
}

void ProjectIOEndo::LoadAirway(std::vector<tri_STL>& airway,
							   double& airway_size) {
	airway.clear();
	airway_size = 0.0;
	if (!file_->exists(group::kResAirway))
		return;

	CompType vec3_type(sizeof(Vec3Data));
	vec3_type.insertMember(ds_member::kX, HOFFSET(Vec3Data, x), PredType::NATIVE_FLOAT);
	vec3_type.insertMember(ds_member::kY, HOFFSET(Vec3Data, y), PredType::NATIVE_FLOAT);
	vec3_type.insertMember(ds_member::kZ, HOFFSET(Vec3Data, z), PredType::NATIVE_FLOAT);

	CompType stl_info(sizeof(STLTri));
	stl_info.insertMember(ds_member::kNormal, HOFFSET(STLTri, normal), vec3_type);
	stl_info.insertMember(ds_member::kV1, HOFFSET(STLTri, v1), vec3_type);
	stl_info.insertMember(ds_member::kV2, HOFFSET(STLTri, v2), vec3_type);
	stl_info.insertMember(ds_member::kV3, HOFFSET(STLTri, v3), vec3_type);
	stl_info.insertMember(ds_member::kColor, HOFFSET(STLTri, color), vec3_type);
	stl_info.insertMember(ds_member::kCntAttributes, HOFFSET(STLTri, cnt_attributes),
						  PredType::NATIVE_USHORT);
	stl_info.insertMember(ds_member::kColorValue, HOFFSET(STLTri, color_value),
						  PredType::NATIVE_UINT);

	Group airway_grp = file_->openGroup(group::kResAirway);
	DataSet dataset = airway_grp.openDataSet(ds::kAirwayData);
	hsize_t dims[1];
	DataSpace space = dataset.getSpace();
	space.getSimpleExtentDims(dims); // 되는지 확인해봐야 함
	int sz = dims[0];

	STLTri* tries = new STLTri[sz];
	dataset.read(tries, stl_info);
	for (int tri_idx = 0; tri_idx < sz; ++tri_idx) {
		tri_STL tri;
		const STLTri& tri_at = tries[tri_idx];
		tri.normal = glm::vec3(tri_at.normal.x, tri_at.normal.y, tri_at.normal.z);
		tri.v1 = glm::vec3(tri_at.v1.x, tri_at.v1.y, tri_at.v1.z);
		tri.v2 = glm::vec3(tri_at.v2.x, tri_at.v2.y, tri_at.v2.z);
		tri.v3 = glm::vec3(tri_at.v3.x, tri_at.v3.y, tri_at.v3.z);
		tri.fColor = glm::vec3(tri_at.color.x, tri_at.color.y, tri_at.color.z);
		tri.nColorVal = tri_at.color_value;
		tri.cntAttributes = tri_at.cnt_attributes;
		airway.push_back(tri);
	}

	airway_size = IOFunctions::ReadDouble(airway_grp, ds::kAirwaySize);
	delete[] tries;

	airway_grp.close();
}
