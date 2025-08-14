#include "project_io_vtosto.h"

#include "project_path_info.h"
#include "io_functions.h"

using namespace H5;
using namespace project;

ProjectIOVTOSTO::ProjectIOVTOSTO(const project::Purpose& purpose, const std::shared_ptr<H5::H5File>& file) :
	file_(file) {
	if (purpose == project::Purpose::SAVE) {
		file_->createGroup(group::kVTOSTO);
		file_->createGroup(group::kResFace);
	}

	vtosto_flags_type_ = CompType(sizeof(VTOSTOFlags));
	vtosto_flags_type_.insertMember(ds_member::kIsSetIsoValue,
									HOFFSET(VTOSTOFlags, is_set_isovalue),
									PredType::NATIVE_INT);
	vtosto_flags_type_.insertMember(ds_member::kIsGenerateHead,
									HOFFSET(VTOSTOFlags, is_generate_head),
									PredType::NATIVE_INT);
	vtosto_flags_type_.insertMember(ds_member::kIsMakeTetra,
									HOFFSET(VTOSTOFlags, is_make_tetra),
									PredType::NATIVE_INT);
	vtosto_flags_type_.insertMember(ds_member::kIsFixedIsoValueInSurgery,
									HOFFSET(VTOSTOFlags, is_fixed_isovalue_in_surgery),
									PredType::NATIVE_INT);
	vtosto_flags_type_.insertMember(ds_member::kIsLandmark,
									HOFFSET(VTOSTOFlags, is_landmark),
									PredType::NATIVE_INT);
	vtosto_flags_type_.insertMember(ds_member::kIsCutFace,
									HOFFSET(VTOSTOFlags, is_cut_face),
									PredType::NATIVE_INT);
	vtosto_flags_type_.insertMember(ds_member::kIsDoMapping,
									HOFFSET(VTOSTOFlags, is_do_mapping),
									PredType::NATIVE_INT);
	vtosto_flags_type_.insertMember(ds_member::kIsCalcDisp,
									HOFFSET(VTOSTOFlags, is_calc_disp),
									PredType::NATIVE_INT);
	vtosto_flags_type_.insertMember(ds_member::kIsMakeSurf,
									HOFFSET(VTOSTOFlags, is_make_surf),
									PredType::NATIVE_INT);
	vtosto_flags_type_.insertMember(ds_member::kIsMakeField,
									HOFFSET(VTOSTOFlags, is_make_field),
									PredType::NATIVE_INT);
	vtosto_flags_type_.insertMember(ds_member::kIsLoadTRD,
									HOFFSET(VTOSTOFlags, is_load_TRD),
									PredType::NATIVE_INT);
}

ProjectIOVTOSTO::~ProjectIOVTOSTO() {}

void ProjectIOVTOSTO::InitVTOSTO() {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	IOFunctions::WriteBool(vtosto_grp, ds::kIsTabInit, true);
	vtosto_grp.close();
}

bool ProjectIOVTOSTO::IsInit() {
	bool exists = false;
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	if (vtosto_grp.exists(ds::kIsTabInit))
		exists = true;
	vtosto_grp.close();
	return exists;
}

void ProjectIOVTOSTO::SaveIsoValue(float iso_value) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	IOFunctions::WriteFloat(vtosto_grp, ds::kIsoValue, iso_value);
	vtosto_grp.close();
}

void ProjectIOVTOSTO::SaveHeadPoints(const std::vector<glm::vec3>& head_points) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	IOFunctions::WriteVec3List(vtosto_grp, ds::kHeadPoints, head_points);
	vtosto_grp.close();
}

void ProjectIOVTOSTO::SaveFacePoints(const std::vector<glm::vec3>& face_points) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	IOFunctions::WriteVec3List(vtosto_grp, ds::kFacePoints, face_points);
	vtosto_grp.close();
}

void ProjectIOVTOSTO::SaveFacePointsAfter(const std::vector<glm::vec3>& face_points_after) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	IOFunctions::WriteVec3List(vtosto_grp, ds::kFacePointsAfter, face_points_after);
	vtosto_grp.close();
}

void ProjectIOVTOSTO::SaveFaceIndices(const std::vector<unsigned int>& face_indicies) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	hsize_t data_sz = face_indicies.size();
	unsigned int* data = new unsigned int[data_sz];
	unsigned int* data_ptr = data;
	for (const auto& index : face_indicies) {
		*data_ptr++ = index;
	}
	hsize_t dim[] = { data_sz };
	DataSpace space(1, dim);
	DataSet dataset = vtosto_grp.createDataSet(ds::kFaceIndicies,
											   PredType::NATIVE_UINT,
											   space);
	dataset.write(data, PredType::NATIVE_UINT);
	dataset.close();
	delete[] data;
	vtosto_grp.close();
}

void ProjectIOVTOSTO::SaveFaceTexCoords(const std::vector<glm::vec2>& tex_coords) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	IOFunctions::WriteVec2List(vtosto_grp, ds::kFaceTexCoords, tex_coords);
	vtosto_grp.close();
}

void ProjectIOVTOSTO::SaveModelPoints(const std::vector<glm::vec3>& model_points) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	IOFunctions::WriteVec3List(vtosto_grp, ds::kModelPoints, model_points);
	vtosto_grp.close();
}

void ProjectIOVTOSTO::SaveModelTetraIndices(const std::vector<std::vector<int>>& tetra_indices) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	hsize_t data_sz = tetra_indices.size() * tetra_indices[0].size();
	int* data = new int[data_sz];
	for (int idx = 0; idx < tetra_indices.size(); ++idx) {
		const auto& tetra = tetra_indices[idx];
		data[idx * 4] = tetra[0];
		data[idx * 4 + 1] = tetra[1];
		data[idx * 4 + 2] = tetra[2];
		data[idx * 4 + 3] = tetra[3];
	}

	hsize_t dim[] = { data_sz };
	DataSpace space(1, dim);
	DataSet dataset = vtosto_grp.createDataSet(ds::kHeadTriIndices,
											   PredType::NATIVE_INT,
											   space);
	dataset.write(data, PredType::NATIVE_INT);
	dataset.close();
	delete[] data;
	vtosto_grp.close();
}

void ProjectIOVTOSTO::SaveModelTriIndices(const std::vector<std::vector<int>>& model_tri_indices) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);

	hsize_t data_sz = model_tri_indices.size() * model_tri_indices[0].size();
	int* data = new int[data_sz];
	for (int idx = 0; idx < model_tri_indices.size(); ++idx) {
		const auto& tri = model_tri_indices[idx];
		data[idx * 3] = tri[0];
		data[idx * 3 + 1] = tri[1];
		data[idx * 3 + 2] = tri[2];
	}

	hsize_t dim[] = { data_sz };
	DataSpace space(1, dim);
	DataSet dataset = vtosto_grp.createDataSet(ds::kModelTriIndices,
											   PredType::NATIVE_INT,
											   space);
	dataset.write(data, PredType::NATIVE_INT);
	dataset.close();
	delete[] data;
	vtosto_grp.close();
}

void ProjectIOVTOSTO::SaveModelTetraMoveResult(const std::vector<glm::vec3>& move_results) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	IOFunctions::WriteVec3List(vtosto_grp, ds::kModelTetraMoveResult, move_results);
	vtosto_grp.close();
}

void ProjectIOVTOSTO::SaveModelPhotoToSurface(const glm::mat4 & mat) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	IOFunctions::WriteMatrix(vtosto_grp, ds::kModelPhotoToSurface, mat);
	vtosto_grp.close();
}

void ProjectIOVTOSTO::SaveVTOSTOFlags(const project::VTOSTOFlags & flags) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	DataSet dataset = vtosto_grp.createDataSet(ds::kVTOSTOFlags, vtosto_flags_type_,
											   project::io::kDSScalar);
	dataset.write(&flags, vtosto_flags_type_);
	dataset.close();
	vtosto_grp.close();
}

void ProjectIOVTOSTO::SaveHeadTriIndices(const std::vector<std::vector<int>>& tri_indices) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);

	hsize_t data_sz = tri_indices.size() * tri_indices[0].size();
	int* data = new int[data_sz];
	for (int idx = 0; idx < tri_indices.size(); ++idx) {
		const auto& tri = tri_indices[idx];
		data[idx * 3] = tri[0];
		data[idx * 3 + 1] = tri[1];
		data[idx * 3 + 2] = tri[2];
	}

	hsize_t dim[] = { data_sz };
	DataSpace space(1, dim);
	DataSet dataset = vtosto_grp.createDataSet(ds::kModelTetraIndices,
											   PredType::NATIVE_INT,
											   space);
	dataset.write(data, PredType::NATIVE_INT);
	dataset.close();
	delete[] data;
	vtosto_grp.close();
}

void ProjectIOVTOSTO::LoadIsoValue(float & iso_value) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	iso_value = IOFunctions::ReadFloat(vtosto_grp, ds::kIsoValue);
	vtosto_grp.close();
}

void ProjectIOVTOSTO::LoadHeadPoints(std::vector<glm::vec3>& head_points) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	head_points = IOFunctions::ReadVec3List(vtosto_grp, ds::kHeadPoints);
	vtosto_grp.close();
}

void ProjectIOVTOSTO::LoadFacePoints(std::vector<glm::vec3>& face_points) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	face_points = IOFunctions::ReadVec3List(vtosto_grp, ds::kFacePoints);
	vtosto_grp.close();
}

void ProjectIOVTOSTO::LoadFacePointsAfter(std::vector<glm::vec3>& face_points_after) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	face_points_after = IOFunctions::ReadVec3List(vtosto_grp, ds::kFacePointsAfter);
	vtosto_grp.close();
}

void ProjectIOVTOSTO::LoadFaceIndices(std::vector<unsigned int>& face_indicies) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	if (!vtosto_grp.exists(ds::kFaceIndicies))
		return;

	DataSet dataset = vtosto_grp.openDataSet(ds::kFaceIndicies);
	DataSpace space = dataset.getSpace();

	hsize_t dims[1];
	space.getSimpleExtentDims(dims);
	int sz = dims[0];

	unsigned int* data = new unsigned int[sz];

	dataset.read(data, PredType::NATIVE_UINT);
	face_indicies.clear();
	face_indicies.reserve(sz);
	for (int idx = 0; idx < sz; ++idx) {
		face_indicies.push_back(data[idx]);
	}
	dataset.close();
	delete[] data;
	vtosto_grp.close();
}

void ProjectIOVTOSTO::LoadFaceTexCoords(std::vector<glm::vec2>& tex_coords) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	tex_coords = IOFunctions::ReadVec2List(vtosto_grp, ds::kFaceTexCoords);
	vtosto_grp.close();
}

void ProjectIOVTOSTO::LoadModelPoints(std::vector<glm::vec3>& model_points) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	model_points = IOFunctions::ReadVec3List(vtosto_grp, ds::kModelPoints);
	vtosto_grp.close();
}

void ProjectIOVTOSTO::LoadModelTetraIndices(std::vector<std::vector<int>>& tetra_indices) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	if (!vtosto_grp.exists(ds::kModelTetraIndices))
		return;

	DataSet dataset = vtosto_grp.openDataSet(ds::kModelTetraIndices);
	DataSpace space = dataset.getSpace();
	hsize_t dims[1];
	space.getSimpleExtentDims(dims);
	int sz = dims[0];

	int* data = new int[sz];
	dataset.read(data, PredType::NATIVE_INT);
	tetra_indices.clear();
	tetra_indices.reserve(sz / 4);
	for (int idx = 0; idx < sz / 4; ++idx) {
		tetra_indices.push_back({
			data[idx * 4], data[idx * 4 + 1],
			data[idx * 4 + 2], data[idx * 4 + 3]
		});
	}
	dataset.close();
	delete[] data;
	vtosto_grp.close();
}

void ProjectIOVTOSTO::LoadModelTriIndices(std::vector<std::vector<int>>& model_tri_indices) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	if (!vtosto_grp.exists(ds::kModelTriIndices))
		return;

	DataSet dataset = vtosto_grp.openDataSet(ds::kModelTriIndices);
	DataSpace space = dataset.getSpace();
	hsize_t dims[1];
	space.getSimpleExtentDims(dims);
	int sz = dims[0];

	int* data = new int[sz];
	dataset.read(data, PredType::NATIVE_INT);
	model_tri_indices.clear();
	model_tri_indices.reserve(sz / 3);
	for (int idx = 0; idx < sz / 3; ++idx) {
		model_tri_indices.push_back({
			data[idx * 3], data[idx * 3 + 1], data[idx * 3 + 2]
		});
	}
	dataset.close();
	delete[] data;
	vtosto_grp.close();
}

void ProjectIOVTOSTO::LoadModelTetraMoveResult(std::vector<glm::vec3>& move_result) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	move_result = IOFunctions::ReadVec3List(vtosto_grp, ds::kModelTetraMoveResult);
	vtosto_grp.close();
}

void ProjectIOVTOSTO::LoadModelPhotoToSurface(glm::mat4 & mat) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	mat = IOFunctions::ReadMatrix(vtosto_grp, ds::kModelPhotoToSurface);
	vtosto_grp.close();
}

void ProjectIOVTOSTO::LoadVTOSTOFlags(project::VTOSTOFlags & flags) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	DataSet dataset = vtosto_grp.openDataSet(ds::kVTOSTOFlags);
	dataset.read(&flags, vtosto_flags_type_);
	dataset.close();
	vtosto_grp.close();
}

void ProjectIOVTOSTO::LoadHeadTriIndices(std::vector<std::vector<int>>& tri_indicies) {
	Group vtosto_grp = file_->openGroup(group::kVTOSTO);
	if (!vtosto_grp.exists(ds::kHeadTriIndices))
		return;

	DataSet dataset = vtosto_grp.openDataSet(ds::kHeadTriIndices);
	DataSpace space = dataset.getSpace();
	hsize_t dims[1];
	space.getSimpleExtentDims(dims);
	int sz = dims[0];

	int* data = new int[sz];
	dataset.read(data, PredType::NATIVE_INT);
	tri_indicies.clear();
	tri_indicies.reserve(sz / 3);
	for (int idx = 0; idx < sz / 3; ++idx) {
		tri_indicies.push_back({
			data[idx * 3],
			data[idx * 3 + 1],
			data[idx * 3 + 2]
		});
	}
	dataset.close();
	delete[] data;
	vtosto_grp.close();
}


// face trd load & save functions
void ProjectIOVTOSTO::SaveTRDPath(const std::string & path) {
	Group face_grp = file_->openGroup(group::kResFace);
	IOFunctions::WriteString(face_grp, ds::kTRDPath, path);
	face_grp.close();
}

void ProjectIOVTOSTO::SaveTRDPoints(const glm::vec3 * points, size_t count) {
	std::vector<glm::vec3> point_list;
	point_list.reserve(count);
	for (int idx = 0; idx < count; ++idx)
		point_list.push_back(points[idx]);

	Group face_grp = file_->openGroup(group::kResFace);
	IOFunctions::WriteVec3List(face_grp, ds::kTRDPoints, point_list);
	face_grp.close();
}

void ProjectIOVTOSTO::SaveTRDNormals(const glm::vec3 * normals, size_t count) {
	std::vector<glm::vec3> normal_list;
	normal_list.reserve(count);
	for (int idx = 0; idx < count; ++idx)
		normal_list.push_back(normals[idx]);

	Group face_grp = file_->openGroup(group::kResFace);
	IOFunctions::WriteVec3List(face_grp, ds::kTRDNormals, normal_list);
	face_grp.close();
}

void ProjectIOVTOSTO::SaveTRDTexCoords(const glm::vec2 * tex_coords, size_t count) {
	std::vector<glm::vec2> tex_coord_list;
	tex_coord_list.reserve(count);
	for (int idx = 0; idx < count; ++idx)
		tex_coord_list.push_back(tex_coords[idx]);

	Group face_grp = file_->openGroup(group::kResFace);
	IOFunctions::WriteVec2List(face_grp, ds::kTRDTexCoords, tex_coord_list);
	face_grp.close();
}

void ProjectIOVTOSTO::SaveTRDIndices(const unsigned int * indices, size_t count) {
	Group face_grp = file_->openGroup(group::kResFace);
	hsize_t dim[] = { count };
	DataSpace space(1, dim);
	DataSet dataset = face_grp.createDataSet(ds::kTRDIndices,
											 PredType::NATIVE_UINT,
											 space);
	dataset.write(indices, PredType::NATIVE_UINT);
	dataset.close();
	face_grp.close();
}

void ProjectIOVTOSTO::SaveTRDTexImage(const unsigned char * image, unsigned int tex_w,
									unsigned int tex_h) {
	Group face_grp = file_->openGroup(group::kResFace);
	hsize_t dim_histogram[] = { 3 * tex_w*tex_h };
	DataSpace space_histogram(1, dim_histogram);
	DataSet dataset = face_grp.createDataSet(ds::kTRDTexImage, PredType::NATIVE_UCHAR,
											 space_histogram);
	dataset.write(image, PredType::NATIVE_UCHAR);
	dataset.close();
	IOFunctions::WriteUInt(face_grp, ds::kTRDTexWidth, tex_w);
	IOFunctions::WriteUInt(face_grp, ds::kTRDTexHeight, tex_h);
	face_grp.close();
}

void ProjectIOVTOSTO::LoadTRDPath(std::string & path) {
	Group face_grp = file_->openGroup(group::kResFace);
	path = IOFunctions::ReadString(face_grp, ds::kTRDPath);
	face_grp.close();
}

void ProjectIOVTOSTO::LoadTRDPoints(std::vector<glm::vec3>& points) {
	Group face_grp = file_->openGroup(group::kResFace);
	points = IOFunctions::ReadVec3List(face_grp, ds::kTRDPoints);
	face_grp.close();
}

void ProjectIOVTOSTO::LoadTRDNormals(std::vector<glm::vec3>& normals) {
	Group face_grp = file_->openGroup(group::kResFace);
	normals = IOFunctions::ReadVec3List(face_grp, ds::kTRDNormals);
	face_grp.close();
}

void ProjectIOVTOSTO::LoadTRDTexCoords(std::vector<glm::vec2>& tex_coords) {
	Group face_grp = file_->openGroup(group::kResFace);
	tex_coords = IOFunctions::ReadVec2List(face_grp, ds::kTRDTexCoords);
	face_grp.close();
}

void ProjectIOVTOSTO::LoadTRDIndices(std::vector<unsigned int>& indices) {
	Group face_grp = file_->openGroup(group::kResFace);
	if (!face_grp.exists(ds::kTRDIndices))
		return;

	DataSet dataset = face_grp.openDataSet(ds::kTRDIndices);
	DataSpace space = dataset.getSpace();
	hsize_t dims[1];
	space.getSimpleExtentDims(dims);
	int sz = dims[0];

	unsigned int* data = new unsigned int[sz];

	dataset.read(data, PredType::NATIVE_UINT);
	indices.clear();
	indices.reserve(sz);
	for (int idx = 0; idx < sz; ++idx) {
		indices.push_back(data[idx]);
	}
	dataset.close();
	delete[] data;
	face_grp.close();
}

void ProjectIOVTOSTO::LoadTRDTexImage(std::vector<unsigned char>& image,
									unsigned int& tex_w, unsigned int& tex_h) {
	Group face_grp = file_->openGroup(group::kResFace);
	if (!face_grp.exists(ds::kTRDTexImage))
		return;

	DataSet dataset = face_grp.openDataSet(ds::kTRDTexImage);
	DataSpace space = dataset.getSpace();
	hsize_t dims[1];
	space.getSimpleExtentDims(dims);
	int sz = dims[0];
	unsigned char* data = new unsigned char[sz * sizeof(unsigned char)];

	dataset.read(data, PredType::NATIVE_UCHAR);
	image.clear();
	image.reserve(sz);
	for (int idx = 0; idx < sz; ++idx) {
		image.push_back(data[idx]);
	}
	dataset.close();
	delete[] data;

	tex_w = IOFunctions::ReadUInt(face_grp, ds::kTRDTexWidth);
	tex_h = IOFunctions::ReadUInt(face_grp, ds::kTRDTexHeight);
	face_grp.close();
}
