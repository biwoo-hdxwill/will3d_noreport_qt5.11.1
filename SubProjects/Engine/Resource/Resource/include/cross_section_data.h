#pragma once

/**=================================================================================================

Project:		Resource
File:			include\cross_section_data.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2017-12-29
Last modify: 	2017-12-29

	Copyright (c) 2017 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <memory>
#include <QPointF>
#include "curve_data.h"

#include "../resource_global.h"

class RESOURCE_EXPORT CrossSectionData {
public:
	CrossSectionData();
	~CrossSectionData();

	CrossSectionData(const CrossSectionData&) = delete;
	CrossSectionData& operator=(const CrossSectionData&) = delete;

public:
	void Initialize(const glm::vec3& right_vector, const glm::vec3& up_vector,
					const glm::vec3& center_pos, const glm::vec3& arch_pos,
					const int& index, const bool& is_flip);
	void Clear();

	inline void set_center_position_in_pano_plane(const QPointF& center_pos) { center_position_in_pano_plane_ = center_pos; }
	inline void set_arch_position_in_pano_plane(const QPointF& center_pos) { arch_position_in_pano_plane_ = center_pos; }
	inline void set_proj_nerve(const QPointF& proj_nerve) { proj_nerve_ = proj_nerve; }
	inline void set_ctrl_nerve(const QPointF& proj_nerve) { ctrl_nerve_ = proj_nerve; }

	inline const glm::vec3& center_position_in_vol() const { return center_position_in_vol_; }
	inline const glm::vec3& arch_position_in_vol() const { return arch_position_in_vol_; }
	inline const glm::vec3& right_vector() const { return right_vector_; }
	inline const glm::vec3& up_vector() const { return up_vector_; }
	inline const glm::vec3& back_vector() const { return back_vector_; }
	inline const QPointF& center_position_in_pano_plane() const { return center_position_in_pano_plane_; }
	inline const QPointF& arch_position_in_pano_plane() const { return arch_position_in_pano_plane_; }
	inline const QPointF& proj_nerve() const { return proj_nerve_; }
	inline const QPointF& ctrl_nerve() const { return ctrl_nerve_; }
	inline const bool& is_init() const { return is_init_; }
	inline const int& index() const { return index_; }
	inline const bool& flip() const { return flip_; }

private:
	glm::vec3 center_position_in_vol_;
	glm::vec3 arch_position_in_vol_;
	glm::vec3 right_vector_;
	glm::vec3 up_vector_;
	glm::vec3 back_vector_;
	QPointF center_position_in_pano_plane_ = QPointF(-1.0, -1.0);
	QPointF arch_position_in_pano_plane_ = QPointF(-1.0, -1.0);
	QPointF proj_nerve_ = QPointF(-1.0, -1.0);
	QPointF ctrl_nerve_ = QPointF(-1.0, -1.0);
	bool is_init_ = false;
	bool flip_ = false;
	int index_ = -1;
};
