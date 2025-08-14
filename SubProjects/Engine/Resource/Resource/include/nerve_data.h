#pragma once

/**=================================================================================================

Project:		Resource
File:			nerve_data.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2017-11-01
Last modify: 	2017-11-01

	Copyright (c) 2017 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <vector>

#if defined(__APPLE__)
#include <glm/vec3.hpp>
#else
#include <gl/glm/vec3.hpp>
#endif

#include <QColor>
#include "resource_global.h"

class RESOURCE_EXPORT NerveData {
public:
	NerveData();
	~NerveData();

	NerveData(const NerveData&) = delete;
	NerveData& operator=(const NerveData&) = delete;

public:
	void GenerateMesh(const std::vector<glm::vec3>& nerve_points_in_gl, const glm::vec3& nerve_radius_scale_in_gl);

	inline void set_points(const std::vector<glm::vec3>& nerve_points) { points_ = nerve_points; }
	inline void set_radius(const float& radius) { radius_ = radius; }
	inline void set_color(const QColor& color) { color_ = color; }
	inline void set_visible(const bool& visible) { is_visible_ = visible; }

	inline bool IsInitialize() const { return (points_.size() == 0) ? false : true; }

	inline const std::vector<glm::vec3>& nerve_points() const { return points_; }
	inline float radius() const { return radius_; }
	inline const QColor& color() const { return color_; }
	inline const bool& is_visible() const { return is_visible_; }

	inline const std::vector<glm::vec3>& mesh_vertices() const { return mesh_vertices_; }
	inline const std::vector<uint>& mesh_indices() const { return mesh_indices_; }
	inline const std::vector<glm::vec3>& mesh_normals() const { return mesh_normals_; }

private:
	inline glm::vec3 GetOrthoVector(const glm::vec3 &dir, const glm::vec3& unit) const;

private:
	std::vector<glm::vec3> points_;
	float radius_ = 0.0f;
	bool is_visible_ = false;
	QColor color_;

	std::vector<glm::vec3> mesh_vertices_;
	std::vector<uint> mesh_indices_;
	std::vector<glm::vec3> mesh_normals_;
};
