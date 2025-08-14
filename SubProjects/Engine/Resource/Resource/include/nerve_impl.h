#pragma once

/**=================================================================================================

Project:		Resource
File:			nerve_impl.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-01-30
Last modify: 	2018-01-30

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include <memory>
#include <map>

#include "image_2d.h"
#include "include/nerve_data.h"

class NerveImpl {
public:
	NerveImpl();
	~NerveImpl();

	NerveImpl(const NerveImpl&) = delete;
	NerveImpl& operator=(const NerveImpl&) = delete;

public:
	void SetNervePoints(int nerve_id, const std::vector<glm::vec3>& nerve_points);
	void SetNerveParams(int nerve_id, bool nerve_visible, float nerve_radius, const QColor& nerve_color);

	void SetNerveVisible(bool visible);

	void GenerateNerveMesh(int nerve_id, const std::vector<glm::vec3>& nerve_points_in_gl, const glm::vec3& nerve_radius_scale_in_gl);
	void ClearNervePointsAll(int nerve_id);
	bool IsValidNerve(int nerve_id);

	inline const std::map<int, std::unique_ptr<NerveData>>& nerve_datas() const { return nerve_datas_; }


private:
	std::map<int, std::unique_ptr<NerveData>> nerve_datas_;
};
