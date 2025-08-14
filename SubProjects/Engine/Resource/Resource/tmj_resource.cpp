#include "tmj_resource.h"

#include "../../Resource/Resource/W3Image3D.h"

namespace {
	const glm::vec3 kInvAxisX(-1.0f, 1.0f, 1.0f);
}
TMJfrontalResource::TMJfrontalResource(){
}

TMJfrontalResource::~TMJfrontalResource() {
}

TMJlateralResource::TMJlateralResource(){
}

TMJlateralResource::~TMJlateralResource() {
}
bool TMJlateralResource::IsValidCenterPosition(int lateral_id) const {
	if (center_positions_.size() <= lateral_id)
		return false;
	else if (lateral_id < -1)
		return false;
	else return true;
}

bool TMJlateralResource::GetCenterPosition(int lateral_id, glm::vec3* center_position) const {
	bool is_valid = this->IsValidCenterPosition(lateral_id);
	if (is_valid) {
		*center_position = center_positions_.at(lateral_id);
		return true;
	} else return false;
}
bool TMJlateralResource::GetNumber(int lateral_id, int* number) const {
	bool is_valid = this->IsValidCenterPosition(lateral_id);
	if (is_valid) {
		*number = number_.at(lateral_id);
		return true;
	} else return false;
}
TMJresource::TMJresource(){
	for (int type = 0; type < TMJDirectionType::TMJ_TYPE_END; type++) {
		CreateFrontalResource((TMJDirectionType)type);
		CreateLateralResource((TMJDirectionType)type);
	}
}

TMJresource::~TMJresource() {
}
void TMJresource::CreateFrontalResource(const TMJDirectionType & type) {
	frontal_[type].reset(new TMJfrontalResource);
}
void TMJresource::CreateLateralResource(const TMJDirectionType & type) {
	lateral_[type].reset(new TMJlateralResource);
}
bool TMJresource::DeleteFrontalResource(const TMJDirectionType & type) {
	bool is_delete = (frontal_[type].get()) ? true : false;
	frontal_[type].reset();
	return is_delete;
}

bool TMJresource::DeleteLateralResource(const TMJDirectionType & type) {
	bool is_delete = (lateral_[type].get()) ? true : false;
	lateral_[type].reset();
	return is_delete;
}
void TMJresource::ImportProject(const TMJDirectionType& type,
                              const glm::vec3& rect_center,
                   const glm::vec3& lateral_up_vector) {
  project_info_[type].is_imported = true;
  project_info_[type].rect_center = rect_center;
  project_info_[type].lateral_up_vector = lateral_up_vector;
}
void TMJresource::SetLateralParam(const TMJLateralID & id, const float & value) {
	switch (id) {
		case LEFT_INTERVAL:
			lateral_[TMJ_LEFT]->SetInterval(value);
			break;
		case LEFT_THICKNESS:
			lateral_[TMJ_LEFT]->SetThickness(value);
			break;
		case RIGHT_INTERVAL:
			lateral_[TMJ_RIGHT]->SetInterval(value);
			break;
		case RIGHT_THICKNESS:
			lateral_[TMJ_RIGHT]->SetThickness(value);
			break;
		default:
			assert(false);
			break;
	}
}

void TMJresource::SetTMJRectParam(const TMJRectID & id, const float & value) {
	switch (id) {
		case LEFT_W:
			frontal_[TMJ_LEFT]->SetWidth(value);
			break;
		case LEFT_H:
			lateral_[TMJ_LEFT]->SetWidth(value);
			break;
		case RIGHT_W:
			frontal_[TMJ_RIGHT]->SetWidth(value);
			break;
		case RIGHT_H:
			lateral_[TMJ_RIGHT]->SetWidth(value);
			break;
		default:
			assert(false);
			break;
	}
}

void TMJresource::SetLateralCount(const TMJDirectionType& type, const int& count) {
	lateral_[type]->SetCount(count);
}

void TMJresource::SetLateralPositionInfo(const TMJDirectionType& type,
										 const std::map<int, glm::vec3>& positions,
										 const glm::vec3& up_vector) {
	std::vector<glm::vec3> pts;
	std::vector<int> number;
	pts.reserve(positions.size());
	number.reserve(positions.size());
	for (const auto& elem : positions) {
		pts.push_back(elem.second);
		number.push_back(elem.first);
	}

	lateral_[type]->set_center_positions(pts);
	lateral_[type]->set_number(number);
	lateral_[type]->set_up_vector(up_vector);
}

void TMJresource::SetFrontalPositionInfo(const TMJDirectionType& type,
										 const glm::vec3& position,
										 const glm::vec3& up_vector) {
	frontal_[type]->set_center_position(position);
	frontal_[type]->set_up_vector(up_vector);
}

void TMJresource::SetSelectedLateralID(const TMJDirectionType& type,
									   const int& lateral_id) {
	lateral_[type]->set_selected_id(lateral_id);
}
void TMJresource::SetTMJRectCenter(const TMJDirectionType& type, const glm::vec3& pt_center) {
	rect_center_[type] = pt_center;
}
glm::mat4 TMJresource::GetTMJRoateMatrix(const TMJDirectionType& type) const {
	const glm::vec3& up_vector = frontal_[type]->up_vector();
	const glm::vec3& back_vector = back_vector_;
	const glm::vec3 right_vector = glm::normalize(glm::cross(up_vector, back_vector));
	return glm::mat4(glm::vec4(right_vector, 0.0f),
					 glm::vec4(up_vector, 0.0f),
					 glm::vec4(back_vector, 0.0f),
					 glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
}
TMJDirectionType TMJresource::GetDirectionType(const TMJLateralID& id) const {
  switch (id) {
    case TMJLateralID::LEFT_INTERVAL:
    case TMJLateralID::LEFT_THICKNESS:
      return TMJDirectionType::TMJ_LEFT;
    case TMJLateralID::RIGHT_INTERVAL:
    case TMJLateralID::RIGHT_THICKNESS:
      return TMJDirectionType::TMJ_RIGHT;
    default:
      assert(false);
      return TMJDirectionType::TMJ_TYPE_UNKNOWN;
  }
}

bool TMJresource::IsValidResource(const TMJDirectionType& type) const {
  return (frontal_[type] && lateral_[type]) ? true : false;
}
const int& TMJresource::GetSelectedLateralID(const TMJDirectionType& type) const {
	return lateral_[type]->selected_id();
}
