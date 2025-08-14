#include "nerve_resource.h"

#include "../../Common/Common/W3Logger.h"

#include "include\nerve_impl.h"

NerveResource::NerveResource() {
	mask_roi_.reset(new NerveMaskROI());
	impl_in_vol.reset(new NerveImpl());
	impl_in_pano.reset(new NerveImpl());
}

NerveResource::~NerveResource() {
}

void NerveResource::SetNerve(int nerve_id, const std::vector<glm::vec3>& points_in_vol,
							 const std::vector<glm::vec3>& points_in_pano) {
	if (!impl_in_vol->IsValidNerve(nerve_id)) {
		common::Logger::instance()->Print(common::LogType::ERR,
										  "NerveResource::SetNerve: please check nerve_tool_values.");
		assert(false);
		return;
	}
	impl_in_vol->SetNervePoints(nerve_id, points_in_vol);
	impl_in_pano->SetNervePoints(nerve_id, points_in_pano);
	curr_add_nerve_id_ = -1;
}
void NerveResource::SetNerveParams(const bool& nerve_visible, const int& nerve_id, const float& radius,
						const QColor& color) {

	impl_in_vol->SetNerveParams(nerve_id, nerve_visible, radius, color);
	impl_in_pano->SetNerveParams(nerve_id, nerve_visible, radius, color);
}
void NerveResource::SetNerveVisibleAll(const bool& nerve_visible) {
	impl_in_vol->SetNerveVisible(nerve_visible);
	impl_in_pano->SetNerveVisible(nerve_visible);
}

void NerveResource::GenerateNerveMeshInVol(int nerve_id, const std::vector<glm::vec3>& nerve_points_in_gl,
										   const glm::vec3& nerve_radius_scale_in_gl) {
	impl_in_vol->GenerateNerveMesh(nerve_id, nerve_points_in_gl, nerve_radius_scale_in_gl);
}

void NerveResource::GenerateNerveMeshInPano(int nerve_id, const std::vector<glm::vec3>& nerve_points_in_gl,
											const glm::vec3& nerve_radius_scale_in_gl) {
	impl_in_pano->GenerateNerveMesh(nerve_id, nerve_points_in_gl, nerve_radius_scale_in_gl);
}
void NerveResource::AddNerveCtrlPoint(int nerve_id, const glm::vec3 & nerve_point_in_vol) {
	nerve_ctrl_points_[nerve_id].push_back(nerve_point_in_vol);
	curr_add_nerve_id_ = nerve_id;
}
void NerveResource::EditNerveCtrlPoint(int nerve_id, int nerve_selected_index, const glm::vec3& nerve_point_in_vol) {
	if (!IsRangeInNerveCtrlPoints(nerve_id, nerve_selected_index)) {
		auto logger = common::Logger::instance();
		logger->Print(common::LogType::ERR, "NerveResource::EditNerveCtrlPoint: Index out of range.");
		assert(false);
		return;
	}

	nerve_ctrl_points_[nerve_id][nerve_selected_index] = nerve_point_in_vol;
}
void NerveResource::InsertNerveCtrlPoint(int nerve_id, int nerve_insert_index, const glm::vec3& nerve_point_in_vol) {
	if (!IsRangeInNerveCtrlPoints(nerve_id, nerve_insert_index)) {
		auto logger = common::Logger::instance();
		logger->Print(common::LogType::ERR, "NerveResource::InsertNerveCtrlPoint: Index out of range.");
		assert(false);
		return;
	}

	auto insert_iter = nerve_ctrl_points_[nerve_id].begin() + nerve_insert_index;
	nerve_ctrl_points_[nerve_id].insert(insert_iter, nerve_point_in_vol);
}
void NerveResource::RemoveNerveCtrlPoint(int nerve_id, int nerve_remove_index) {
	if (!IsRangeInNerveCtrlPoints(nerve_id, nerve_remove_index)) {
		auto logger = common::Logger::instance();
		logger->Print(common::LogType::ERR, "NerveResource::RemoveNerveCtrlPoint: Index out of range.");
		assert(false);
		return;
	}

	auto erase_iter = nerve_ctrl_points_[nerve_id].begin() + nerve_remove_index;
	nerve_ctrl_points_[nerve_id].erase(erase_iter);

	if (nerve_ctrl_points_[nerve_id].size() == 0)
		nerve_ctrl_points_.erase(nerve_id);

	if (modify_mode_.enable && modify_mode_.nerve_id == nerve_id &&
		modify_mode_.nerve_selected_index == nerve_remove_index) {
		modify_mode_.enable = false;
	}
}
void NerveResource::ClearNervePointsAll(int nerve_id) {
	ClearNerveCtrlPoints(nerve_id);
	impl_in_vol->ClearNervePointsAll(nerve_id);
	impl_in_pano->ClearNervePointsAll(nerve_id);

	if (curr_add_nerve_id_ == nerve_id)
		curr_add_nerve_id_ = -1;
	if (modify_mode_.enable && modify_mode_.nerve_id == nerve_id) {
		modify_mode_.enable = false;
	}
}
void NerveResource::SetNerveMaskROI(int vol_width, int vol_height, int vol_depth, float z_spacing) {
	if (!IsSetNervePoints()) {
		mask_roi_.reset(new NerveMaskROI());
		return;
	}

	mask_roi_->ReadyFillNerveMaskROI(vol_width, vol_height, vol_depth);

	const auto& nerve_datas = impl_in_vol->nerve_datas();

	for (const auto& elem : nerve_datas) {
		if(elem.second->is_visible())
			mask_roi_->FillNerveMaskROI(*(elem.second.get()), z_spacing);
	}
}
bool NerveResource::IsRangeInNerveCtrlPoints(int nerve_id, int index) {
	if (nerve_ctrl_points_[nerve_id].size() > index && index >= 0)
		return true;
	else
		return false;
}
void NerveResource::SetModifyMode(const bool& is_modify, int nerve_id, int nerve_selected_index) {
	modify_mode_.enable = is_modify;
	modify_mode_.nerve_id = nerve_id;
	modify_mode_.nerve_selected_index = nerve_selected_index;
}
const std::vector<glm::vec3>& NerveResource::GetNerveCtrlPoints(const int & nerve_id) const {
	if (nerve_ctrl_points_.find(nerve_id) != nerve_ctrl_points_.end())
		return nerve_ctrl_points_.at(nerve_id);
	else {
		auto logger = common::Logger::instance();
		logger->Print(common::LogType::ERR, "NerveResource::GetNerveCtrlPoints: empty.");
		assert(false);
		return nerve_ctrl_points_.at(nerve_id);
	}
}
const std::vector<glm::vec3>& NerveResource::GetNervePointsInVol(const int & nerve_id) const {
	const auto& nerve_datas = impl_in_vol->nerve_datas();

	if (nerve_datas.find(nerve_id) != nerve_datas.end())
		return nerve_datas.at(nerve_id)->nerve_points();
	else {
		auto logger = common::Logger::instance();
		logger->Print(common::LogType::ERR, "NerveResource::GetNervePointsInVol: empty.");
		assert(false);
		return nerve_datas.at(nerve_id)->nerve_points();
	}
}
const std::vector<glm::vec3>& NerveResource::GetNervePointsInPano(const int & nerve_id) const {
	const auto& nerve_datas = impl_in_pano->nerve_datas();

	if (nerve_datas.find(nerve_id) != nerve_datas.end())
		return nerve_datas.at(nerve_id)->nerve_points();
	else {
		auto logger = common::Logger::instance();
		logger->Print(common::LogType::ERR, "NerveResource::GetNervePointsInPano: empty.");
		assert(false);
		return nerve_datas.at(nerve_id)->nerve_points();
	}
}
const std::map<int, std::unique_ptr<NerveData>>& NerveResource::GetNerveDataInVol() const {
	return impl_in_vol->nerve_datas();
}
const std::map<int, std::unique_ptr<NerveData>>& NerveResource::GetNerveDataInPano() const {
	return impl_in_pano->nerve_datas();
}
float NerveResource::GetNerveRadius(int nerve_id) const {
	if (!impl_in_vol->IsValidNerve(nerve_id)) {
		auto logger = common::Logger::instance();
		logger->Print(common::LogType::ERR, "NerveResource::GetNerveRadius: invalid id.");
		assert(false);
	}
	return impl_in_vol->nerve_datas().at(nerve_id)->radius();
}
const QColor& NerveResource::GetNerveColor(int nerve_id) const {
	if (!impl_in_vol->IsValidNerve(nerve_id)) {
		auto logger = common::Logger::instance();
		logger->Print(common::LogType::ERR, "NerveResource::GetNerveRadius: invalid id.");
		assert(false);
	}
	return impl_in_vol->nerve_datas().at(nerve_id)->color();
}
bool NerveResource::IsSetNervePoints() const {
	const auto& nerve_datas = impl_in_vol->nerve_datas();

	for (const auto& elem : nerve_datas) {
		if (elem.second->IsInitialize() != 0)
			return true;
	}

	return false;
}
void NerveResource::ClearNerveCtrlPoints(int nerve_id) {
	auto iter_nctp = nerve_ctrl_points_.find(nerve_id);
	if (iter_nctp != nerve_ctrl_points_.end())
		nerve_ctrl_points_.erase(iter_nctp);
}
