#pragma once
/**=================================================================================================

Project:		Resource
File:			implant_data.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-03-28
Last modify: 	2018-03-28

Copyright (c) 2018 HDXWILL. All rights reserved.

*===============================================================================================**/
#include <vector>
#if defined(__APPLE__)
#include <glm/detail/type_mat4x4.hpp>
#include <glm/vec3.hpp>
#else
#include <GL/glm/detail/type_mat4x4.hpp>
#include <GL/glm/vec3.hpp>
#endif

#include <qstring.h>

#include "../resource_global.h"

class RESOURCE_EXPORT ImplantData {
public:
	struct InitPack {
		uint implant_id = 0;
		float diameter = 0.0f;
		float length = 0.0f;
		float platform_diameter = 0.0f;
		float total_length = 0.0f;
		float custom_apical_diameter = 0.0f;
		QString file_path;
		QString manufacturer;
		QString product;
		QString sub_category;
		std::vector<glm::vec3> mesh_vertices;
		std::vector<glm::vec3> mesh_normals;
		std::vector<uint> mesh_indices;
		glm::vec3 bounding_box_max;
		glm::vec3 bounding_box_min;
		glm::vec3 major_axis;
		float spacing = 0.0f;
	};
	ImplantData(const InitPack& init_pack);
	~ImplantData() {}

	ImplantData(const ImplantData&) = delete;
	ImplantData& operator=(const ImplantData&) = delete;

public:
	inline void set_position_in_vol(const glm::vec3& position) { position_in_vol_ = position; }
	inline void set_position_in_pano(const glm::vec3& position) { position_in_pano_ = position; }
	inline void set_position_in_pano_plane(const glm::vec3& position) { position_in_pano_plane_ = position; }

	inline void set_translate_in_vol(const glm::mat4& translate) { translate_in_vol_ = translate; }
	inline void set_translate_in_pano(const glm::mat4& translate) { translate_in_pano_ = translate; }
	inline void set_translate_in_pano_plane(const glm::mat4& translate) { translate_in_pano_plane_ = translate; }

	inline void set_rotate_in_vol(const glm::mat4& rotate) { rotate_in_vol_ = rotate; }
	inline void set_rotate_in_pano(const glm::mat4& rotate) { rotate_in_pano_ = rotate; }
	inline void set_rotate_in_pano_plane(const glm::mat4& rotate) { rotate_in_pano_plane_ = rotate; }
	
	inline void set_axis_point_in_vol(const glm::vec3& point) { axis_point_in_vol_ = point; }
	inline void set_axis_point_in_pano(const glm::vec3& point) { axis_point_in_pano_ = point; }
	inline void set_axis_point_in_pano_plane(const glm::vec3& point) { axis_point_in_pano_plane_ = point; }

	inline void set_is_visible(const bool& is_visible) { is_visible_ = is_visible; }
	inline void set_is_collide(const bool& is_collide) { is_collide_ = is_collide; }

	inline void set_rotate_degree_in_pano_plane(const float degree) { rotate_degree_in_pano_plane_ = degree; }

	inline const bool& is_visible() const noexcept { return is_visible_; }
	inline const bool& is_collide() const noexcept { return is_collide_; }
	inline const unsigned int& id() const noexcept { return id_; }
	inline const float& diameter() const noexcept { return diameter_; }
	inline const float& length() const noexcept { return length_; }
	inline const float& platform_diameter() const noexcept { return platform_diameter_; }
	inline const float& custom_apical_diameter() const noexcept { return custom_apical_diameter_; }
	inline const float& total_length() const noexcept { return total_length_; }
	inline const QString& manufacturer() const noexcept { return manufacturer_; }
	inline const QString& product() const noexcept { return product_; }
	inline const QString& file_path() const noexcept { return file_path_; }
	inline const QString& sub_category() const noexcept { return sub_category_; }

	inline const std::vector<glm::vec3>& mesh_vertices() const { return mesh_vertices_; }
	inline const std::vector<glm::vec3>& mesh_normals() const { return mesh_normals_; }
	inline const std::vector<uint>& mesh_indices() const { return mesh_indices_; }

	inline const glm::vec3& bounding_box_max() const noexcept { return bounding_box_max_; }
	inline const glm::vec3& bounding_box_min() const noexcept { return bounding_box_min_; }
	inline const glm::vec3& major_axis() const noexcept { return major_axis_; }
	inline const glm::vec3 ImplantDirection() const { return position_in_vol_ - axis_point_in_vol_; }

	inline const glm::vec3& position_in_vol() const noexcept { return position_in_vol_; }
	inline const glm::vec3& position_in_pano() const noexcept { return position_in_pano_; }
	inline const glm::vec3& position_in_pano_plane() const noexcept { return position_in_pano_plane_; }

	inline const glm::vec3& axis_point_in_vol() const noexcept { return axis_point_in_vol_; }
	inline const glm::vec3& axis_point_in_pano() const noexcept { return axis_point_in_pano_; }
	inline const glm::vec3& axis_point_in_pano_plane() const noexcept { return axis_point_in_pano_plane_; }

	inline const glm::mat4& translate_in_vol() const noexcept { return translate_in_vol_; }
	inline const glm::mat4& translate_in_pano() const noexcept { return translate_in_pano_; }
	inline const glm::mat4& translate_in_pano_plane() const noexcept { return translate_in_pano_plane_; }

	inline const glm::mat4& rotate_in_vol() const noexcept { return rotate_in_vol_; }
	inline const glm::mat4& rotate_in_pano() const noexcept { return rotate_in_pano_; }
#if 1
	inline const glm::mat4& rotate_in_pano_plane() const noexcept { return rotate_in_pano_plane_; }
#else
	inline const glm::mat4& rotate_in_pano_plane() const noexcept { return glm::mat4(); }
#endif


	inline const float rotate_degree_in_pano_plane() { return rotate_degree_in_pano_plane_; }
private:
	bool is_visible_ = true;
	bool is_collide_ = false;

	unsigned int id_ = 0; 
	float diameter_ = 0.0f; 
	float length_ = 0.0f; 
	float platform_diameter_ = 0.0f;
	float custom_apical_diameter_ = 0.0f;
	float total_length_ = 0.0f;
	QString manufacturer_; 
	QString product_; 
	QString file_path_;
	QString sub_category_;

	glm::vec3 bounding_box_max_;
	glm::vec3 bounding_box_min_;
	glm::vec3 major_axis_;

	std::vector<glm::vec3> mesh_vertices_;
	std::vector<glm::vec3> mesh_normals_;
	std::vector<uint> mesh_indices_;

	glm::vec3 position_in_vol_;
	glm::vec3 axis_point_in_vol_;
	glm::mat4 rotate_in_vol_;
	glm::mat4 translate_in_vol_;

	glm::vec3 position_in_pano_;
	glm::vec3 axis_point_in_pano_;
	glm::mat4 rotate_in_pano_;
	glm::mat4 translate_in_pano_;
	
	glm::vec3 position_in_pano_plane_ = glm::vec3(-1.0f);
	glm::vec3 axis_point_in_pano_plane_;
	glm::mat4 rotate_in_pano_plane_;
	glm::mat4 translate_in_pano_plane_;

	float rotate_degree_in_pano_plane_ = 0.0f;
};
