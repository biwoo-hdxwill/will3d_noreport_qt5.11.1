#pragma once

/**=================================================================================================

Project:		Resource
File:			nerve_mask_roi.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2017-10-31
Last modify: 	2017-10-31

	Copyright (c) 2017 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <vector>

#if defined(__APPLE__)
#include <glm/vec3.hpp>
#else
#include <gl/glm/vec3.hpp>
#endif

#include <QColor>

#include "../resource_global.h"
class NerveData;

class RESOURCE_EXPORT NerveMaskROI {
public:
	NerveMaskROI();
	NerveMaskROI(const NerveMaskROI&) = delete;
	NerveMaskROI& operator=(const NerveMaskROI&) = delete;

	struct ROIVolSize {
		int width = 0;
		int height = 0;
		int depth = 0;

		ROIVolSize() {}
		ROIVolSize(int w, int h, int d) : width(w), height(h), depth(d) {}
	};

public:
	void ReadyFillNerveMaskROI(int width, int height, int depth);
	void FillNerveMaskROI(const NerveData& nerve_data, float z_spacing);

	inline bool IsTrueBitMaskROI(int idxy, int idz) const;
	inline void SetTrueBitMaskROI(int idxy, int idz);

	inline const std::vector<std::vector<unsigned char>>& roi_mask_vol() const { return roi_mask_vol_; }
	inline const ROIVolSize& roi_vol_size() const { return roi_vol_size_; }

	inline const std::vector<glm::vec3>& roi_direction() const { return roi_direction_; }
	inline const std::vector<glm::vec3>& roi_start_pos() const { return roi_start_pos_; }
	inline const std::vector<glm::vec3>& roi_end_pos() const { return roi_end_pos_; }
	inline const std::vector<float>& roi_radius() const { return roi_radius_; }
	inline const std::vector<QColor>& roi_color() const { return roi_color_; }

	inline bool is_ready_mask() const { return is_ready_mask_; }
	inline bool is_mask_filled() const noexcept { return is_mask_filled_; }
	inline int roi_list_size() const { return roi_list_size_; }

private:
	void ClearAndResizeBuffer(int width, int height, int depth);
	void GetPointsIntervalOnePixel(const std::vector<glm::vec3>& points, std::vector<glm::vec3>& dst_one_points);

	inline void FillROIMaskBetweenTwoPoint(const glm::vec3& p1, const glm::vec3& p2,
										   const float& nerve_radius, const QColor& nerve_color);
	inline bool IsRangeInBuffer(const int& x, const int& y, const int& z);

private:
	std::vector<std::vector<unsigned char>> roi_mask_vol_;

	ROIVolSize roi_vol_size_;
	std::vector<glm::vec3> roi_direction_;
	std::vector<glm::vec3> roi_start_pos_;
	std::vector<glm::vec3> roi_end_pos_;
	std::vector<float> roi_radius_;
	std::vector<QColor> roi_color_;
	int roi_list_size_ = 0;
	bool is_ready_mask_ = false;
	bool is_mask_filled_ = false;
};
