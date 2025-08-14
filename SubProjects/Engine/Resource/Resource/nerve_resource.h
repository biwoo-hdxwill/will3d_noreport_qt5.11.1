#pragma once

/**=================================================================================================

Project: 			Resource
File:				nerve_resource.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-09-15
Last modify:		2017-09-15

Copyright (c) 2017 All rights reserved by HDXWILL.

 *===============================================================================================**/
#include <memory>
#include <vector>
#include <map>

#if defined(__APPLE__)
#include <glm/vec3.hpp>
#else
#include <GL/glm/vec3.hpp>
#endif

#include "W3Resource.h"
#include "include/nerve_mask_roi.h"
#include "include/nerve_data.h"

#include "resource_global.h"

class NerveImpl;

class RESOURCE_EXPORT NerveResource : public CW3Resource {
public:

	NerveResource();
	~NerveResource();

	NerveResource(const NerveResource&) = delete;
	NerveResource& operator=(const NerveResource&) = delete;

	struct ModifyMode {
		bool enable = false;
		int nerve_id = -1;
		int nerve_selected_index = -1;
	};

public:
	void SetNerve(int nerve_id, const std::vector<glm::vec3>& points_in_vol,
				  const std::vector<glm::vec3>& points_in_pano);
	void SetNerveParams(const bool& nerve_visible, const int& nerve_id, const float& radius,
						const QColor& color);
	void SetNerveVisibleAll(const bool& nerve_visible);

	void GenerateNerveMeshInVol(int nerve_id, const std::vector<glm::vec3>& nerve_points_in_gl,
								const glm::vec3& nerve_radius_scale_in_gl);
	void GenerateNerveMeshInPano(int nerve_id, const std::vector<glm::vec3>& nerve_points_in_gl,
								 const glm::vec3& nerve_radius_scale_in_gl);

	void AddNerveCtrlPoint(int nerve_id, const glm::vec3& nerve_point_in_vol);
	void EditNerveCtrlPoint(int nerve_id, int nerve_selected_index, const glm::vec3& nerve_point_in_vol);

	void InsertNerveCtrlPoint(int nerve_id, int nerve_insert_index, const glm::vec3& nerve_point_in_vol);
	void RemoveNerveCtrlPoint(int nerve_id, int nerve_remove_index);

	void ClearNervePointsAll(int nerve_id);

	void SetNerveMaskROI(int vol_width, int vol_height, int vol_depth, float z_spacing);
	bool IsRangeInNerveCtrlPoints(int nerve_id, int index);

	void SetModifyMode(const bool& is_modify, int nerve_id, int nerve_selected_index);

	const std::vector<glm::vec3>& GetNerveCtrlPoints(const int& nerve_id) const;
	const std::vector<glm::vec3>& GetNervePointsInVol(const int& nerve_id) const;
	const std::vector<glm::vec3>& GetNervePointsInPano(const int& nerve_id) const;

	const std::map<int, std::unique_ptr<NerveData>>& GetNerveDataInVol() const;
	const std::map<int, std::unique_ptr<NerveData>>& GetNerveDataInPano() const;

	float GetNerveRadius(int nerve_id) const;
	const QColor& GetNerveColor(int nerve_id) const;
	bool IsSetNervePoints() const;

	inline bool IsSetNerveMask() const { return mask_roi_->is_mask_filled(); }
	inline const std::map<int, std::vector<glm::vec3>>& nerve_ctrl_points() const { return nerve_ctrl_points_; } 
	inline const NerveMaskROI& mask_roi() const { return *mask_roi_; } 
	inline const ModifyMode& modify_mode() const { return modify_mode_; }
	inline const int& curr_add_nerve_id() const { return curr_add_nerve_id_; }

private:
	void ClearNerveCtrlPoints(int nerve_id);

private:
	std::unique_ptr<NerveImpl> impl_in_vol;
	std::unique_ptr<NerveImpl> impl_in_pano;

	std::unique_ptr<NerveMaskROI> mask_roi_;
	std::map<int, std::vector<glm::vec3>> nerve_ctrl_points_;

	ModifyMode modify_mode_;
	int curr_add_nerve_id_ = -1;
};
