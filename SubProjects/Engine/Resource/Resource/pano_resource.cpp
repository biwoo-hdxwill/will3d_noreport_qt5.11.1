#include "pano_resource.h"

#include "../../Common/Common/common.h"
#include "nerve_resource.h"

using glm::vec3;

PanoResource::PanoResource(
	const std::vector<glm::vec3>& pano_points,
	const std::vector<glm::vec3>& pano_ctrl_points,
	const glm::vec3& back_vector,
	float pano_depth,
  float range_mm) {

	pano_ctrl_points_ = pano_ctrl_points;
	pano_3d_depth_ = (int)pano_depth;
	range_value_mm_ = range_mm;
	pano_3d_height_ = glm::length(back_vector);
	back_vector_ = glm::normalize(back_vector);

	if (pano_ctrl_points_.size() > 1) {
		is_valid_ = true;

		if (pano_ctrl_points_.size() == 2)
			is_line_ = true;
		else {
			float diff = 0.0f;
			glm::vec3 curr_uv = glm::normalize(pano_ctrl_points_[1] - pano_ctrl_points_[0]);
			for (int i = 1; i < pano_ctrl_points_.size() - 1; i++) {
				glm::vec3 next_uv = glm::normalize(pano_ctrl_points_[i + 1] - pano_ctrl_points_[i]);
				diff += glm::length(next_uv - curr_uv);
				curr_uv = next_uv;
			}

			if (diff <= 0.000001f) {
				is_line_ = true;
			}
		}
	} else
		is_valid_ = false;


	InitCurveData(pano_points, back_vector, curve_center_data_);
	SetCurrentRulerIndex();

	pano_3d_width_ = curve_center_data_.points().size();
}
const CurveData& PanoResource::GetCurrentCurveData() const {
	if (shifted_value_ == 0.0f)
		return curve_center_data_;
	else
		return *(curve_shifted_data_.get());
}
int PanoResource::GetPanoPlaneWidth() const {
	return GetCurrentCurveData().GetCurveLength();
}
int PanoResource::GetPanoPlaneHeight() const {
	return pano_3d_height_;
}
void PanoResource::SetShiftedValue(float value) {
	if (shifted_value_ == value)
		return;

	shifted_value_ = value;

	if (shifted_value_ == 0.0f) {
		curve_shifted_data_.reset();
		SetCurrentRulerIndex();
		return;
	}

	curve_shifted_data_.reset(new CurveData);

	const std::vector<glm::vec3>& center_points = curve_center_data_.points();
	const std::vector<glm::vec3>& up_vectors = curve_center_data_.up_vectors();

	std::vector<glm::vec3> shfited_arch_points;
	shfited_arch_points.reserve(center_points.size());
	for (int i = 0; i < center_points.size(); i++)
		shfited_arch_points.push_back(center_points[i] + up_vectors[i] * shifted_value_);

	if (shfited_arch_points.empty())
		return;

	InitCurveData(shfited_arch_points, back_vector_, *curve_shifted_data_);
	SetCurrentRulerIndex();
}

void PanoResource::InitCurveData(
	const std::vector<glm::vec3>& pano_points,
	const glm::vec3 & back_vector,
	CurveData& dst_curve_data
) {
	if (pano_points.size() == 0)
		return;

	std::vector<vec3> norm_points;
	GetPointsIntervalOnePixel(pano_points, norm_points);

	vec3 add_tail = norm_points.back() * 2.0f - norm_points.at(norm_points.size() - 2);
	norm_points.push_back(add_tail);

	for (int i = 0; i < norm_points.size() - 1; i++) {
		const vec3& p0 = norm_points[i];
		const vec3& p1 = norm_points[i + 1];

		vec3 vec(p1 - p0);
		vec3 upVector = glm::normalize(glm::cross(vec, back_vector));

		dst_curve_data.AddPoint(p0);
		dst_curve_data.AddUpVector(upVector);
	}

}

void PanoResource::GetPointsIntervalOnePixel(
	const std::vector<glm::vec3>& points,
	std::vector<glm::vec3>& dst_one_points) {
	Common::equidistanceSpline(dst_one_points, points);
}

void PanoResource::SetCurrentRulerIndex() {
	const CurveData& curr_curve_data = GetCurrentCurveData();
	const auto& points = curr_curve_data.points();

	ruler_index_.reset(new RulerIndex);

	if (points.size() == 0) {
		return;
	}

	int point_size = (int)points.size();
	int idx_min_y;
	if (is_line_) {
		idx_min_y = point_size / 2;
	} else {
		float min_y = std::numeric_limits<float>::max();
		for (int i = 0; i < point_size; i++) {
			if (min_y > points[i].y) {
				min_y = points[i].y;
				idx_min_y = i;
			}
		}
	}
	ruler_index_->idx_arch_front_ = idx_min_y;
	ruler_index_->idx_min_ = 0;
	ruler_index_->idx_max_ = point_size - 1;

	int begin_idx = idx_min_y;
	if (begin_idx + 10 < point_size) {
		for (int i = begin_idx + 10; i < point_size; i += 10) {
			if ((i-begin_idx) % 50 == 0) {
				ruler_index_->medium_gradation_.push_back(i);
			}
			else {
				ruler_index_->small_gradation_.push_back(i);
			}
		}
	}

	begin_idx = idx_min_y;

	if (begin_idx  - 10 > 0) {
		for (int i = begin_idx - 10; i > 0; i -= 10) {
			if ((i - begin_idx) % 50 == 0) {
				ruler_index_->medium_gradation_.push_back(i);
			} else {
				ruler_index_->small_gradation_.push_back(i);
			}
		}
	}

	ruler_index_->is_set_ = true;
}
