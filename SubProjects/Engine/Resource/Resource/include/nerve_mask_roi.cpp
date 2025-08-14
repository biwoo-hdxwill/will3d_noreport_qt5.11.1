#include "nerve_mask_roi.h"

#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/common.h"
#include "../../Common/Common/W3Math.h"

#include "nerve_data.h"

namespace {
const int kByteBit = 8;
}

NerveMaskROI::NerveMaskROI() {}

void NerveMaskROI::ReadyFillNerveMaskROI(int width, int height, int depth) {
	ClearAndResizeBuffer(width, height, depth);

	roi_direction_.clear();
	roi_start_pos_.clear();
	roi_end_pos_.clear();
	roi_radius_.clear();
	roi_color_.clear();

	is_ready_mask_ = true;
	is_mask_filled_ = false;
	roi_list_size_ = 0;
}

void NerveMaskROI::FillNerveMaskROI(const NerveData& nerve_data, float z_spacing) {
	if (!is_ready_mask_) {
		common::Logger::instance()->Print(common::LogType::ERR,
										  "NerveMaskROI::FillNerveMaskROI: this not ready fill mask.");
	}

	const auto& nerve_points = nerve_data.nerve_points();

	if (nerve_points.size() < 2)
		return;

	float nerve_radius = nerve_data.radius();
	const QColor& nerve_color = nerve_data.color();

	std::vector<glm::vec3> points_interval_one_pixel;
	GetPointsIntervalOnePixel(nerve_points, points_interval_one_pixel);

	if (points_interval_one_pixel.size() < 2)
		return;

	glm::vec3 tail = points_interval_one_pixel.back() * 2.0f
		- points_interval_one_pixel[points_interval_one_pixel.size() - 2];
	points_interval_one_pixel.push_back(tail);

	for (int i = 0; i < points_interval_one_pixel.size() - 1; i++) {
		glm::vec3 p1 = W3::NormailzeCoordWithZspacing(points_interval_one_pixel[i], z_spacing);
		glm::vec3 p2 = W3::NormailzeCoordWithZspacing(points_interval_one_pixel[i + 1], z_spacing);
		FillROIMaskBetweenTwoPoint(p1, p2, nerve_radius, nerve_color);
	}

	roi_list_size_ = roi_direction_.size();
	is_mask_filled_ = true;
}

inline bool NerveMaskROI::IsTrueBitMaskROI(int idxy, int idz) const {
	if (roi_mask_vol_.size() == 0)
		return false;

	int Quotient = idxy / kByteBit;
	int Remainder = idxy % kByteBit;

	return bool(roi_mask_vol_[idz][Quotient] & 0x01 << Remainder);
}

inline void NerveMaskROI::SetTrueBitMaskROI(int idxy, int idz) {
	int Quotient = idxy / kByteBit;
	int Remainder = idxy % kByteBit;

	roi_mask_vol_[idz][Quotient] |= 0x01 << Remainder;
}

void NerveMaskROI::ClearAndResizeBuffer(int width, int height, int depth) {
	roi_vol_size_ = ROIVolSize(width, height, depth);

	roi_mask_vol_.clear();
	roi_mask_vol_.resize(roi_vol_size_.depth);
	int slice_size = (roi_vol_size_.width*roi_vol_size_.height) / kByteBit;

	for (auto& elem : roi_mask_vol_)
		elem.resize(slice_size, 0);
}

void NerveMaskROI::GetPointsIntervalOnePixel(const std::vector<glm::vec3>& points,
											 std::vector<glm::vec3>& dst_one_points) {
	Common::equidistanceSpline(dst_one_points, points);
}

inline void NerveMaskROI::FillROIMaskBetweenTwoPoint(const glm::vec3& p1, const glm::vec3& p2,
													 const float& nerve_radius, const QColor& nerve_color) {
	glm::vec3 roi_space = glm::vec3(nerve_radius);
	glm::vec3 roi_start = glm::vec3(std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z))
		- roi_space;
	glm::vec3 roi_end = glm::vec3(std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z))
		+ roi_space;

	const int roi_start_x = (int)roi_start.x;
	const int roi_start_y = (int)roi_start.y;
	const int roi_start_z = (int)roi_start.z;
	const int roi_end_x = (int)roi_end.x;
	const int roi_end_y = (int)roi_end.y;
	const int roi_end_z = (int)roi_end.z;

	for (int z = roi_start_z; z <= roi_end_z; z++) {
		for (int y = roi_start_y; y <= roi_end_y; y++) {
			const int y_index = y * roi_vol_size_.width;
			for (int x = roi_start_x; x <= roi_end_x; x++) {
				if (IsRangeInBuffer(x, y, z))
					SetTrueBitMaskROI(y_index + x, z);
			}
		}
	}

	glm::vec3 dir = glm::normalize(p2 - p1);
	roi_direction_.push_back(dir);

	roi_start_pos_.push_back(roi_start);
	roi_end_pos_.push_back(roi_end);
	roi_radius_.push_back(nerve_radius);
	roi_color_.push_back(nerve_color);
}
inline bool NerveMaskROI::IsRangeInBuffer(const int& x, const int& y, const int& z) {
	if ((x >= 0 && x < roi_vol_size_.width) &&
		(y >= 0 && y < roi_vol_size_.height) &&
		(z >= 0 && z < roi_vol_size_.depth))
		return true;
	else
		return false;
}
