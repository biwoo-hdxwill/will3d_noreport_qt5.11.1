#pragma once
/**=================================================================================================

Project: 			Resource
File:				lightbox_resource.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-10-10
Last modify:		2018-10-10

Copyright (c) 2018 All rights reserved by HDXWILL.

 *===============================================================================================**/
#include <memory>
#include <vector>

#include "../../Common/Common/w3_struct.h"

#include "W3Resource.h"
#include "resource_global.h"

class RESOURCE_EXPORT LightboxData 
{
public:
	LightboxData() {}
	~LightboxData() {}

	LightboxData(const LightboxData&) = delete;
	LightboxData& operator=(const LightboxData&) = delete;

public:
	void Initialize(const int& index, const glm::vec3& plane_center);

	inline void set_index(const int& index) { index_ = index; }
	inline void set_plane_center(const glm::vec3& pos) noexcept { plane_center_ = pos; }
	inline const int& index() const { return index_; }
	inline const glm::vec3& plane_center() const noexcept { return plane_center_; }

private:
	int index_ = -1;
	glm::vec3 plane_center_ = glm::vec3(0.0f);
};

class RESOURCE_EXPORT LightboxResource : public CW3Resource {
public:
	LightboxResource(const glm::vec3& plane_center,
					 const lightbox_resource::LightboxParams& lightbox_params,
					 const lightbox_resource::PlaneParams& plane_params);
	virtual ~LightboxResource();

	LightboxResource(const LightboxResource&) = delete;
	LightboxResource& operator=(const LightboxResource&) = delete;

public:
	void ChangeLightboxCount(const int& count_row, const int& count_col);
	void SaveMaximizeParams(const int& target_lightbox_id);
	void LoadMaximizeParams(int& prev_row, int& prev_col, int& target_lightbox_id);

	inline const int GetLightboxCount() const noexcept { return lightbox_params_.count_row*lightbox_params_.count_col; }
	inline const int& GetLightboxCountRow() const noexcept { return lightbox_params_.count_row; }
	inline const int& GetLightboxCountCol() const noexcept { return lightbox_params_.count_col; }
	inline const float& GetLightboxInterval() const noexcept { return lightbox_params_.interval; }
	inline const float& GetLightboxThickness() const noexcept { return lightbox_params_.thickness; }
	inline const LightboxViewType& GetLightboxViewType() const noexcept { return lightbox_params_.view_type; }

	inline void SetLightboxInterval(const float& interval) noexcept { lightbox_params_.interval = interval; }
	inline void SetLightboxThickness(const float& thickness) noexcept { lightbox_params_.thickness = thickness; }

	const glm::vec3& GetPlaneCenter(const int& id) const;
	const glm::vec3 GetLighboxDirection() const noexcept;
	inline const glm::vec3& GetPlaneUp() const noexcept { return plane_params_.up; }
	inline const glm::vec3& GetPlaneBottom() const noexcept { return plane_params_.back; }
	inline const glm::vec3& GetPlaneRight() const noexcept { return plane_params_.right; }
	inline const int& GetPlaneWidth() const noexcept { return plane_params_.width; }
	inline const int& GetPlaneHeight() const noexcept { return plane_params_.height; }
	inline const int& GetPlaneAvailableDepth() const noexcept { return plane_params_.available_depth; }

	inline glm::vec3& first_cross_pt() { return mpr_params_.first_cross_pt; }
	inline glm::vec3& last_cross_pt() { return mpr_params_.last_cross_pt; }
	inline float& dist_from_vol_center() { return plane_params_.dist_from_vol_center; }

	void InitLightboxData(const int& id, const glm::vec3& plane_center);
	bool IsDataExist(const int& id) const;
	bool IsMaximzeMode(int & target_lightbox_id) const;

private:
	void CreateData(const int& count_row, const int& count_col,
					const glm::vec3 & plane_center);

private:
	lightbox_resource::LightboxParams lightbox_params_;
	lightbox_resource::PlaneParams plane_params_;
	lightbox_resource::MPRParams mpr_params_;
	lightbox_resource::MaximizeParams maximize_params_;

	std::vector<std::unique_ptr<LightboxData>> data_;
};
