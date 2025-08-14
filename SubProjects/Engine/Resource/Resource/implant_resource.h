#pragma once
/**=================================================================================================

Project:		Resource
File:			implant_resouce.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-03-29
Last modify: 	2018-03-29

Copyright (c) 2018 HDXWILL. All rights reserved.

*===============================================================================================**/

#include "resource_global.h"

#include <memory>
#include <map>

#include <qstring.h>

#include "../../Common/Common/W3Define.h"

#include "include\implant_data.h"
#include "W3Resource.h"


const int kImplantNumbers[MAX_IMPLANT] = {
	17, 16, 15, 14, 13, 12, 11, 21, 22, 23, 24, 25, 26, 27,
	47, 46, 45, 44, 43, 42, 41, 31, 32, 33, 34, 35, 36, 37
};

namespace implant_resource
{
	const QString kCustomImplantManufacturerNamePrefix("+");

	typedef struct _ImplantInfo
	{
		QString file_path;
		QString manufacturer;
		QString product;
		QString sub_category;
		float diameter;
		float length;
		float platform_diameter;
		float total_length;
		float custom_apical_diameter;
		bool is_custom;
		unsigned int id;

		_ImplantInfo(const QString& implant_file_path, const unsigned int implant_id,
			const QString& manufacturer_name, const QString& product_name,
			const float diameter, const float length, const float platform_diameter, const float total_length, const QString& sub_category) :
			file_path(implant_file_path), manufacturer(manufacturer_name),
			product(product_name), diameter(diameter), length(length), platform_diameter(platform_diameter), total_length(total_length), id(implant_id), sub_category(sub_category),
			custom_apical_diameter(0.0f)
		{
		}

		_ImplantInfo(
			const QString& implant_file_path,
			const unsigned int implant_id,
			const QString& manufacturer_name,
			const QString& product_name,
			const float coronal_diameter,
			const float apical_diameter,
			const float length) :
			file_path(implant_file_path),
			manufacturer(manufacturer_name),
			product(product_name),
			diameter(coronal_diameter),
			custom_apical_diameter(apical_diameter),
			platform_diameter(coronal_diameter),
			length(length),
			total_length(length),
			id(implant_id),
			sub_category("")
		{
		}
	} ImplantInfo;
} // end of namespace implant_resource

class RESOURCE_EXPORT ImplantResource : public CW3Resource {
public:
	ImplantResource();
	~ImplantResource();

	ImplantResource(const ImplantResource&) = delete;
	ImplantResource& operator=(const ImplantResource&) = delete;

public:
	void Initialize(float pixel_spacing, float slice_spacing);

	bool AddImplant(const implant_resource::ImplantInfo& implant_params);
	bool ChangeImplant(const implant_resource::ImplantInfo& implant_params);

	void SetImplantPositionInVol(int implant_id, const glm::vec3& pt_in_vol, const glm::vec3& pt_in_vol_gl);
	void SetImplantPositionInPano(int implant_id, const glm::vec3& pt_in_pano, const glm::vec3& pt_in_pano_gl);
	void SetImplantPositionInPanoPlane(int implant_id, const glm::vec3& pt_in_pano_plane, const glm::vec3& pt_in_pano_plane_gl);
	void SetImplantTranslateInVol(int implant_id, const glm::mat4& translate_matrix);
	void SetImplantRotateInVol(int implant_id, const glm::mat4 & rotate_matrix);
	void SetImplantRotateInPano(int implant_id, const glm::mat4& rotate_matrix);
	void SetImplantRotateInPanoPlane(int implant_id, const glm::mat4& rotate_matrix);
	void EditImplantRotateInVol(int implant_id, const glm::mat4& delta_rotate);
	void EditImplantRotateInPano(int implant_id, const glm::mat4& delta_rotate);
	void EditImplantRotateInPanoPlane(int implant_id, const glm::mat4& delta_rotate);
	void SetImplantAxisPointInVol(int implant_id, const glm::vec3 & point);
	void SetImplantAxisPointInPano(int implant_id, const glm::vec3 & point);
	void SetImplantAxisPointInPanoPlane(int implant_id, const glm::vec3 & point);
	void SetImplantPlaced();
	void SelectImplant(int implant_id, bool selected);
	void SetCollideIds(const std::vector<int>& collided_implant_ids);

	void RemoveAll();
	void RemoveAt(const int& implant_id);

	void SetVisibleAll(const bool& is_visible);

	void set_memo(const QString& memo) { memo_ = memo; }
	const QString& memo() const noexcept { return memo_; }

	bool IsSetImplant() const;
	inline const int& add_implant_id() const noexcept { return add_implant_id_; }
	inline const int& selected_implant_id() const noexcept { return selected_implant_id_; }
	inline const std::map<int, std::unique_ptr<ImplantData>>& data() const { return data_; }
	inline const float& pixel_spacing() const noexcept { return pixel_spacing_; }
	inline const float& slice_spacing() const noexcept { return slice_spacing_; }
	inline const bool& is_visible_all() const noexcept { return is_visible_all_; }

private:
	bool LoadImplantFileWIM(const QString & file_path,
							const glm::mat3& implant_rotate,
							const float& diameter,
							const float& length,
							std::vector<glm::vec3>* mesh_verts,
							std::vector<glm::vec3>* mesh_normals,
							std::vector<unsigned int>* mesh_indices,
							glm::vec3* bounding_box_max,
							glm::vec3* bounding_box_min,
							glm::vec3* major_axis);

	bool LoadCustomImplant(
		const implant_resource::ImplantInfo& info,
		std::vector<glm::vec3>* mesh_verts,
		std::vector<glm::vec3>* mesh_normals,
		std::vector<unsigned int>* mesh_indices,
		glm::vec3* bounding_box_max,
		glm::vec3* bounding_box_min,
		glm::vec3* major_axis
	);

	glm::mat3 GetImplantModelRotate(const QString& implant_file_path,
									const QString& manufacturer, const QString& product);

	bool IsValidImplant(int implant_id) const;
private:
	QString memo_; 
	int add_implant_id_ = -1;
	int selected_implant_id_ = -1; 
	std::map<int, std::unique_ptr<ImplantData>> data_;
	bool initialized_ = false;
	float pixel_spacing_;
	float slice_spacing_;
	bool is_visible_all_ = true;
};

