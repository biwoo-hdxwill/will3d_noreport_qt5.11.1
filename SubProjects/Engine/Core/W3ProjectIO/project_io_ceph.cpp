#include "project_io_ceph.h"

#include "project_path_info.h"
#include "io_functions.h"
#include "project_io_view.h"

using namespace H5;
using namespace project;

ProjectIOCeph::ProjectIOCeph(const project::Purpose& purpose,
							 const std::shared_ptr<H5::H5File>& file) :
	file_(file) {
	Group view_grp;
	if (purpose == project::Purpose::SAVE) {
		file_->createGroup(group::kTabCeph);
		file_->createGroup(group::kSurgeryBar);
		file_->createGroup(group::kSurgeryBtnStatusText);

		view_grp = file_->createGroup(group::kViewCeph);
	} else {
		view_grp = file_->openGroup(group::kViewCeph);
	}
	view_io_.reset(new ProjectIOView(file_, view_grp));

	surgery_cut_flag_type_ = CompType(sizeof(SurgeryCutOn));
	surgery_cut_flag_type_.insertMember(ds_member::kSurgeryOnMaxilla,
										HOFFSET(SurgeryCutOn, maxilla_on),
										PredType::NATIVE_HBOOL);
	surgery_cut_flag_type_.insertMember(ds_member::kSurgeryOnMandible,
										HOFFSET(SurgeryCutOn, mandible_on),
										PredType::NATIVE_HBOOL);
	surgery_cut_flag_type_.insertMember(ds_member::kSurgeryOnChin,
										HOFFSET(SurgeryCutOn, chin_on),
										PredType::NATIVE_HBOOL);
}

ProjectIOCeph::~ProjectIOCeph() {}

void ProjectIOCeph::InitCephTab() {
	Group tab_grp = file_->openGroup(group::kTabCeph);
	IOFunctions::WriteBool(tab_grp, ds::kIsTabInit, true);
	tab_grp.close();
}

bool ProjectIOCeph::IsInit() {
	bool exists = false;
	Group tab_grp = file_->openGroup(group::kTabCeph);
	if (tab_grp.exists(ds::kIsTabInit))
		exists = true;
	tab_grp.close();
	return exists;
}

ProjectIOView & ProjectIOCeph::GetViewIO() {
	return *(view_io_.get());
}

void ProjectIOCeph::SaveIsSurgeryCut(const bool& maxilla, const bool& mandible,
									 const bool& chin) {
	SurgeryCutOn cut_on_flags;
	cut_on_flags.maxilla_on = maxilla;
	cut_on_flags.mandible_on = mandible;
	cut_on_flags.chin_on = chin;
	Group ceph_view_grp = file_->openGroup(group::kViewCeph);
	DataSet dataset = ceph_view_grp.createDataSet(ds::kSurgeryCutOn,
												  surgery_cut_flag_type_,
												  project::io::kDSScalar);
	dataset.write(&cut_on_flags, surgery_cut_flag_type_);
	dataset.close();
	ceph_view_grp.close();
}

void ProjectIOCeph::SaveIsSurgeryAdjust(const bool & maxilla, const bool & mandible,
										const bool & chin) {
	SurgeryCutOn cut_on_flags;
	cut_on_flags.maxilla_on = maxilla;
	cut_on_flags.mandible_on = mandible;
	cut_on_flags.chin_on = chin;
	Group ceph_view_grp = file_->openGroup(group::kViewCeph);
	DataSet dataset = ceph_view_grp.createDataSet(ds::kSurgeryAdjust,
												  surgery_cut_flag_type_,
												  project::io::kDSScalar);
	dataset.write(&cut_on_flags, surgery_cut_flag_type_);
	dataset.close();
	ceph_view_grp.close();
}

void ProjectIOCeph::SaveIsSurgeryMove(const bool & maxilla, const bool & mandible,
									  const bool & chin) {
	SurgeryCutOn cut_on_flags;
	cut_on_flags.maxilla_on = maxilla;
	cut_on_flags.mandible_on = mandible;
	cut_on_flags.chin_on = chin;
	Group ceph_view_grp = file_->openGroup(group::kViewCeph);
	DataSet dataset = ceph_view_grp.createDataSet(ds::kSurgeryMove,
												  surgery_cut_flag_type_,
												  project::io::kDSScalar);
	dataset.write(&cut_on_flags, surgery_cut_flag_type_);
	dataset.close();
	ceph_view_grp.close();
}

void ProjectIOCeph::SaveSurgeryCutItemCount(const int & cut_item_cnt) {
	Group ceph_view_grp = file_->openGroup(group::kViewCeph);
	IOFunctions::WriteInt(ceph_view_grp, ds::kSurgeryCutItemCount, cut_item_cnt);

	Group item_grp = ceph_view_grp.createGroup(group::kSurgeryCutItems);
	for (int cut_item_id = 0; cut_item_id < cut_item_cnt; ++cut_item_id) {
		const std::string kItem = std::to_string(cut_item_id);
		item_grp.createGroup(kItem);
	}
	item_grp.close();
	ceph_view_grp.close();
}

void ProjectIOCeph::SaveSurgeryCutItemPoints(const int & cut_item_id,
											 const std::vector<glm::vec3>& points) {
	Group ceph_view_grp = file_->openGroup(group::kViewCeph);
	Group item_grp = ceph_view_grp.openGroup(group::kSurgeryCutItems);
	const std::string kItem = std::to_string(cut_item_id);
	Group item = item_grp.openGroup(kItem);

	IOFunctions::WriteVec3List(item, ds::kSrugeryCutPoints, points);

	item.close();
	item_grp.close();
	ceph_view_grp.close();
}

void ProjectIOCeph::SaveSrugeryCutItemMatrix(const int& cut_item_id,
											 const glm::mat4& trans, const glm::mat4& rot,
											 const glm::mat4& scale, const glm::mat4& arcball,
											 const glm::mat4& reori) {
	Group ceph_view_grp = file_->openGroup(group::kViewCeph);
	Group item_grp = ceph_view_grp.openGroup(group::kSurgeryCutItems);
	const std::string kItem = std::to_string(cut_item_id);
	Group item = item_grp.openGroup(kItem);

	IOFunctions::WriteMatrix(item, ds::kSrugeryCutTransMat, trans);
	IOFunctions::WriteMatrix(item, ds::kSrugeryCutRotMat, rot);
	IOFunctions::WriteMatrix(item, ds::kSrugeryCutScaleMat, scale);
	IOFunctions::WriteMatrix(item, ds::kSrugeryCutArcballMat, arcball);
	IOFunctions::WriteMatrix(item, ds::kSrugeryCutReoriMat, reori);

	item.close();
	item_grp.close();
	ceph_view_grp.close();
}

void ProjectIOCeph::SaveLandmarks(const std::map<QString, glm::vec3>& landmarks) {
	Group ceph_view_grp = file_->openGroup(group::kViewCeph);
	Group landmark_grp = ceph_view_grp.createGroup(group::kSurgeryLandmarks);
	int landmark_id = 0;
	for (const auto& elem : landmarks) {
		const std::string kLandmarkID = std::to_string(landmark_id++);
		Group lm_grp = landmark_grp.createGroup(kLandmarkID);

		std::string landmark_name = elem.first.toLocal8Bit().toStdString();
		IOFunctions::WriteString(lm_grp, ds::kSurgeryLandmarkName, landmark_name);
		IOFunctions::WriteVec3(lm_grp, ds::kSurgeryLandmarkPoint, elem.second);
		lm_grp.close();
	}
	IOFunctions::WriteInt(landmark_grp, ds::kSurgeryLandmarkCount, landmarks.size());

	landmark_grp.close();
	ceph_view_grp.close();
}

void ProjectIOCeph::SaveSurgeryBtnStatusText(const int & button_id,
											 const std::string & status_text) {
	Group surgery_bar_grp = file_->openGroup(group::kSurgeryBar);
	Group status_text_grp = surgery_bar_grp.openGroup(group::kSurgeryBtnStatusText);

	const std::string kButtonID = std::to_string(button_id);
	IOFunctions::WriteString(status_text_grp, kButtonID, status_text);

	status_text_grp.close();
	surgery_bar_grp.close();
}

void ProjectIOCeph::SaveSrugeryParams(const std::vector<float>& params) {
	Group surgery_bar_grp = file_->openGroup(group::kSurgeryBar);
	hsize_t dim[] = { (hsize_t)params.size() };
	DataSpace space(1, dim);
	DataSet dataset = surgery_bar_grp.createDataSet(ds::kSurgeryParams,
													PredType::NATIVE_FLOAT,
													space);
	dataset.write(&params[0], PredType::NATIVE_FLOAT);
	dataset.close();

	surgery_bar_grp.close();
}

void ProjectIOCeph::SaveSurgeryParamsPrev(const std::vector<float>& params) {
	Group surgery_bar_grp = file_->openGroup(group::kSurgeryBar);
	hsize_t dim[] = { (hsize_t)params.size() };
	DataSpace space(1, dim);
	DataSet dataset = surgery_bar_grp.createDataSet(ds::kSurgeryParamsPrev,
													PredType::NATIVE_FLOAT,
													space);
	dataset.write(&params[0], PredType::NATIVE_FLOAT);
	dataset.close();

	surgery_bar_grp.close();
}

void ProjectIOCeph::SaveIsOutterEdit(const bool & edit) {
	Group surgery_bar_grp = file_->openGroup(group::kSurgeryBar);
	IOFunctions::WriteBool(surgery_bar_grp, ds::kSurgeryOutterEdit, edit);
	surgery_bar_grp.close();
}

void ProjectIOCeph::LoadIsSurgeryCut(bool& maxilla, bool& mandible, bool& chin) {
	SurgeryCutOn cut_on_flags;
	Group ceph_view_grp = file_->openGroup(group::kViewCeph);
	DataSet dataset = ceph_view_grp.openDataSet(ds::kSurgeryCutOn);
	dataset.read(&cut_on_flags, surgery_cut_flag_type_);
	dataset.close();
	ceph_view_grp.close();

	maxilla = cut_on_flags.maxilla_on;
	mandible = cut_on_flags.mandible_on;
	chin = cut_on_flags.chin_on;
}

void ProjectIOCeph::LoadIsSurgeryAdjust(bool & maxilla, bool & mandible, bool & chin) {
	SurgeryCutOn cut_on_flags;
	Group ceph_view_grp = file_->openGroup(group::kViewCeph);
	DataSet dataset = ceph_view_grp.openDataSet(ds::kSurgeryAdjust);
	dataset.read(&cut_on_flags, surgery_cut_flag_type_);
	dataset.close();
	ceph_view_grp.close();

	maxilla = cut_on_flags.maxilla_on;
	mandible = cut_on_flags.mandible_on;
	chin = cut_on_flags.chin_on;
}

void ProjectIOCeph::LoadIsSurgeryMove(bool & maxilla, bool & mandible, bool & chin) {
	SurgeryCutOn cut_on_flags;
	Group ceph_view_grp = file_->openGroup(group::kViewCeph);
	DataSet dataset = ceph_view_grp.openDataSet(ds::kSurgeryMove);
	dataset.read(&cut_on_flags, surgery_cut_flag_type_);
	dataset.close();
	ceph_view_grp.close();

	maxilla = cut_on_flags.maxilla_on;
	mandible = cut_on_flags.mandible_on;
	chin = cut_on_flags.chin_on;
}

void ProjectIOCeph::LoadSurgeryCutItemCount(int & cut_item_cnt) {
	Group ceph_view_grp = file_->openGroup(group::kViewCeph);
	cut_item_cnt = IOFunctions::ReadInt(ceph_view_grp, ds::kSurgeryCutItemCount);
	ceph_view_grp.close();
}

void ProjectIOCeph::LoadSurgeryCutItemPoints(const int & cut_item_id, std::vector<glm::vec3>& points) {
	Group ceph_view_grp = file_->openGroup(group::kViewCeph);
	Group item_grp = ceph_view_grp.openGroup(group::kSurgeryCutItems);
	const std::string kItem = std::to_string(cut_item_id);
	Group item = item_grp.openGroup(kItem);

	points = IOFunctions::ReadVec3List(item, ds::kSrugeryCutPoints);

	item.close();
	item_grp.close();
	ceph_view_grp.close();
}

void ProjectIOCeph::LoadSrugeryCutItemMatrix(const int& cut_item_id,
											 glm::mat4& trans, glm::mat4& rot,
											 glm::mat4& scale, glm::mat4& arcball,
											 glm::mat4& reori) {
	Group ceph_view_grp = file_->openGroup(group::kViewCeph);

	if (!ceph_view_grp.exists(group::kSurgeryCutItems))
		return;

	Group item_grp = ceph_view_grp.openGroup(group::kSurgeryCutItems);
	const std::string kItem = std::to_string(cut_item_id);
	Group item = item_grp.openGroup(kItem);

	trans = IOFunctions::ReadMatrix(item, ds::kSrugeryCutTransMat);
	rot = IOFunctions::ReadMatrix(item, ds::kSrugeryCutRotMat);
	scale = IOFunctions::ReadMatrix(item, ds::kSrugeryCutScaleMat);
	arcball = IOFunctions::ReadMatrix(item, ds::kSrugeryCutArcballMat);
	reori = IOFunctions::ReadMatrix(item, ds::kSrugeryCutReoriMat);

	item.close();
	item_grp.close();
	ceph_view_grp.close();
}

void ProjectIOCeph::LoadLandmarks(std::map<QString, glm::vec3>& landmarks) {
	Group ceph_view_grp = file_->openGroup(group::kViewCeph);
	if (!ceph_view_grp.exists(group::kSurgeryLandmarks))
		return;

	Group landmark_grp = ceph_view_grp.openGroup(group::kSurgeryLandmarks);

	const int kLandmarkCount = IOFunctions::ReadInt(landmark_grp, ds::kSurgeryLandmarkCount);
	landmarks.clear();
	int landmark_id = 0;
	for (int landmark_id = 0; landmark_id < kLandmarkCount; ++landmark_id) {
		const std::string kLandmarkID = std::to_string(landmark_id);
		Group lm_grp = landmark_grp.openGroup(kLandmarkID);

		std::string result = IOFunctions::ReadString(lm_grp, ds::kSurgeryLandmarkName);
		QString landmark_name = result.c_str();
		glm::vec3 point =  IOFunctions::ReadVec3(lm_grp, ds::kSurgeryLandmarkPoint);
		landmarks.insert(std::pair<QString, glm::vec3>(landmark_name, point));

		lm_grp.close();
	}

	landmark_grp.close();
	ceph_view_grp.close();
}

void ProjectIOCeph::LoadSurgeryBtnStatusText(const int & button_id,
											 std::string & status_text) {
	Group surgery_bar_grp = file_->openGroup(group::kSurgeryBar);

	if (!surgery_bar_grp.exists(group::kSurgeryBtnStatusText))
		return;

	Group status_text_grp = surgery_bar_grp.openGroup(group::kSurgeryBtnStatusText);

	const std::string kButtonID = std::to_string(button_id);
	status_text = IOFunctions::ReadString(status_text_grp, kButtonID);

	status_text_grp.close();
	surgery_bar_grp.close();
}

void ProjectIOCeph::LoadSurgeryParams(std::vector<float>& params) {
	Group surgery_bar_grp = file_->openGroup(group::kSurgeryBar);
	
	if (!surgery_bar_grp.exists(ds::kSurgeryParams))
		return;
	DataSet dataset = surgery_bar_grp.openDataSet(ds::kSurgeryParams);
	DataSpace space = dataset.getSpace();
	hsize_t dims[1];
	space.getSimpleExtentDims(dims);
	int sz = dims[0];

	params.reserve(sz);
	float* data = new float[sz];
	dataset.read(data, PredType::NATIVE_FLOAT);
	for (int i = 0; i < sz; ++i) {
		params.push_back(data[i]);
	}

	delete[] data;
	dataset.close();

	surgery_bar_grp.close();
}

void ProjectIOCeph::LoadSurgeryParamsPrev(std::vector<float>& params) {
	Group surgery_bar_grp = file_->openGroup(group::kSurgeryBar);
	if (!surgery_bar_grp.exists(ds::kSurgeryParamsPrev))
		return;

	DataSet dataset = surgery_bar_grp.openDataSet(ds::kSurgeryParamsPrev);
	DataSpace space = dataset.getSpace();
	hsize_t dims[1];
	space.getSimpleExtentDims(dims);
	int sz = dims[0];

	params.reserve(sz);
	float* data = new float[sz];
	dataset.read(data, PredType::NATIVE_FLOAT);
	for (int i = 0; i < sz; ++i) {
		params.push_back(data[i]);
	}

	delete[] data;
	dataset.close();

	surgery_bar_grp.close();
}

void ProjectIOCeph::LoadIsOutterEdit(bool & edit) {
	Group surgery_bar_grp = file_->openGroup(group::kSurgeryBar);
	edit = IOFunctions::ReadBool(surgery_bar_grp, ds::kSurgeryOutterEdit);
	surgery_bar_grp.close();
}
