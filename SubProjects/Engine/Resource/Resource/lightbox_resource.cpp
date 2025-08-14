#include "lightbox_resource.h"

void LightboxData::Initialize(const int & index, const glm::vec3 & plane_center) {
	index_ = index;
	plane_center_ = plane_center;
}

LightboxResource::LightboxResource(const glm::vec3& plane_center,
								   const lightbox_resource::LightboxParams& lightbox_params,
								   const lightbox_resource::PlaneParams& plane_params) :
	lightbox_params_(lightbox_params), plane_params_(plane_params) {
	CreateData(lightbox_params.count_row, lightbox_params.count_col, plane_center);
}

LightboxResource::~LightboxResource() {}

void LightboxResource::ChangeLightboxCount(const int& count_row, const int& count_col) {
	const glm::vec3 plane_center = data_[0]->plane_center();

	for (auto& data : data_)
		data.reset();
	data_.clear();

	CreateData(count_row, count_col, plane_center);
}

void LightboxResource::SaveMaximizeParams(const int & target_lightbox_id) {
	maximize_params_.target_lightbox_id = target_lightbox_id;
	maximize_params_.prev_row = lightbox_params_.count_row;
	maximize_params_.prev_col = lightbox_params_.count_col;
}

void LightboxResource::LoadMaximizeParams(int & prev_row, int & prev_col, int & target_lightbox_id) {
	prev_row = maximize_params_.prev_row;
	prev_col = maximize_params_.prev_col;
	target_lightbox_id = maximize_params_.target_lightbox_id;

	maximize_params_.prev_row = 0;
	maximize_params_.prev_col = 0;
	maximize_params_.target_lightbox_id = -1;
}

const glm::vec3& LightboxResource::GetPlaneCenter(const int & id) const {
	if (maximize_params_.target_lightbox_id >= 0 && !data_.empty())
		return data_[0]->plane_center();

	if (data_.size() <= id || data_.empty())
		return glm::vec3();

	return data_[id]->plane_center();
}

const glm::vec3 LightboxResource::GetLighboxDirection() const noexcept {
	if (lightbox_params_.view_type == LightboxViewType::CORONAL)
		return plane_params_.up;
	else
		return -plane_params_.up;
}

void LightboxResource::InitLightboxData(const int& id, const glm::vec3& plane_center) {
	data_[id]->Initialize(id, plane_center);
}

bool LightboxResource::IsDataExist(const int & id) const {
	if(maximize_params_.target_lightbox_id >= 0 && !data_.empty())
		return (data_[0]->index() > -1) ? true : false;

	if (data_.size() <= id || data_.empty())
		return false;

	return (data_[id]->index() > -1) ? true : false;
}

bool LightboxResource::IsMaximzeMode(int & target_lightbox_id) const {
	if (maximize_params_.target_lightbox_id < 0)
		return false;

	if (maximize_params_.prev_col == 0 || maximize_params_.prev_row == 0)
		return false;

	target_lightbox_id = maximize_params_.target_lightbox_id;
	return true;
}

void LightboxResource::CreateData(const int& count_row, const int& count_col,
								  const glm::vec3 & plane_center) {
	lightbox_params_.count_row = count_row;
	lightbox_params_.count_col = count_col;
	data_.resize(count_row*count_col);
	for (int id = 0; id < count_row*count_col; ++id)
		data_[id].reset(new LightboxData());
	data_[0]->Initialize(0, plane_center);
}
