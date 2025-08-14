#pragma once

/**=================================================================================================

Project: 			Resource
File:				pano_resource.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-23
Last modify:		2017-08-23

Copyright (c) 2017 All rights reserved by HDXWILL.

 *===============================================================================================**/

#include "resource_global.h"
#include "W3Resource.h"

#include "image_2d.h"
#include "W3Image3D.h"
#include "include/curve_data.h"
class RESOURCE_EXPORT PanoResource : public CW3Resource {
public:
	PanoResource(
		const std::vector<glm::vec3>& pano_points,
		const std::vector<glm::vec3>& pano_ctrl_points,
		const glm::vec3& back_vector,
		float pano_depth,
    float range_mm);
	PanoResource(const PanoResource&) = delete;
	PanoResource& operator=(const PanoResource&) = delete;
public:
	class RulerIndex {
	public:
		RulerIndex() { };
		~RulerIndex() { };

		std::vector<int> medium_gradation_;
		std::vector<int> small_gradation_;

		int idx_min_ = 0;
		int idx_max_ = 0;
		int idx_arch_front_ = 0;
		bool is_set_ = false;
	};
public:

	const CurveData& GetCurrentCurveData() const;

	int GetPanoPlaneWidth() const;
	int GetPanoPlaneHeight() const;
	inline std::weak_ptr<resource::Image2D> GetPanoImageWeakPtr() const { return pano_image_; }
	inline const CW3Image3D& pano_vol() const { return *(pano_vol_.get()); }
	inline CW3Image3D* pano_vol_ptr() const { return pano_vol_.get(); } //TODO VREngine의 setVolume함수 인자가 const로 바뀌면 삭제할 함수.
	inline const CurveData& curve_center_data() const { return curve_center_data_; }
	inline const CurveData& curve_shifted_data() const { return *curve_shifted_data_; }
	inline const glm::vec3& back_vector() const { return back_vector_; }
	inline float shifted_value() const { return shifted_value_; }
	inline float thickness_value() const { return thickness_value_; }
	inline float range_value_mm() const { return range_value_mm_; }

	inline int pano_3d_width() const { return pano_3d_width_; }
	inline int pano_3d_height() const { return pano_3d_height_; }
	inline int pano_3d_depth() const { return pano_3d_depth_; }

	inline const RulerIndex& ruler_index() const { return *(ruler_index_.get()); }
	inline const resource::Image2D& mask_nerve_image() const { return *mask_nerve_image_; }
	inline std::weak_ptr<resource::Image2D> GetMaskNerveImageWeakPtr() const { return mask_nerve_image_; }
	inline const resource::Image2D& mask_implant_image() const { return *mask_implant_image_; }
	inline std::weak_ptr<resource::Image2D> GetMaskimplantImageWeakPtr() const { return mask_implant_image_; }

	inline void set_pano_image(resource::Image2D* pano_image) { pano_image_.reset(pano_image); }
	inline void set_pano_vol(CW3Image3D* pano_vol) { pano_vol_.reset(pano_vol); }

	void SetShiftedValue(float value);
	inline void set_thickness_value(float value) { thickness_value_ = value; }
	inline void set_mask_nerve_image(resource::Image2D* mask_image) { mask_nerve_image_.reset(mask_image); }
	inline void set_mask_implant_image(resource::Image2D* mask_image) { mask_implant_image_.reset(mask_image); }

	inline const std::vector<glm::vec3>& pano_ctrl_points() const { return pano_ctrl_points_; }
	inline const bool is_valid() const { return is_valid_; }
	
private:
	void InitCurveData(
		const std::vector<glm::vec3>& pano_points,
		const glm::vec3& back_vector,
		CurveData& dst_curve_data
	);

	void GetPointsIntervalOnePixel(
		const std::vector<glm::vec3>& points,
		std::vector<glm::vec3>& dst_one_points);

	void SetCurrentRulerIndex();

private:
	std::shared_ptr<resource::Image2D> pano_image_;
	std::shared_ptr<resource::Image2D> mask_nerve_image_;
	std::shared_ptr<resource::Image2D> mask_implant_image_;

	std::unique_ptr<CW3Image3D> pano_vol_;
	// shift가 안된 center arch line이고 볼륨상의 Z위치는
	// 파노라마 뷰에서 상단(Top) line에 해당한다.
	CurveData curve_center_data_;
	std::unique_ptr<CurveData> curve_shifted_data_;

	int pano_3d_width_ = 0;
	int pano_3d_height_ = 0;
	int pano_3d_depth_ = 0;
	float shifted_value_ = 0.0f;
	float thickness_value_ = 0.0f;
	float range_value_mm_ = 0.0f;
	float cross_section_value_ = 0.0f;
	bool is_valid_ = false;
	std::vector<glm::vec3> pano_ctrl_points_;
	glm::vec3 back_vector_ = glm::vec3(0.0f);
	std::unique_ptr<RulerIndex> ruler_index_;
	bool is_line_ = false;
};
