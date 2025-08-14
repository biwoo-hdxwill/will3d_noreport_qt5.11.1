#pragma once

/**=================================================================================================

Project: 			Resource
File:				face_photo_resource.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-07-27
Last modify:		2017-07-27

 *===============================================================================================**/
#include <vector>

#if defined(__APPLE__)
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#else
#include <GL/glm/vec3.hpp>
#include <GL/glm/vec2.hpp>
#endif


#include "resource_global.h"
class RESOURCE_EXPORT FacePhotoResource {
public:
	FacePhotoResource();
	~FacePhotoResource();

	void clear();

	inline void AddPoint(const glm::vec3& point) { points_.push_back(point); }
	inline void AddPointAfter(const glm::vec3& point) { points_after_.push_back(point); }
	inline void AddTexCoord(const glm::vec2& point) { tex_coords_.push_back(point); }
	inline void AddIndex(const unsigned int& idx) { indices_.push_back(idx); }

	inline void set_points(const std::vector<glm::vec3>& points) { points_ = points; }
	inline void set_points_after(const std::vector<glm::vec3> points) { points_after_ = points; }
	inline void set_tex_coords(const std::vector<glm::vec2>& texCoords) { tex_coords_ = texCoords; }
	inline void set_indices(const std::vector<unsigned int>& indices) { indices_ = indices; }
	inline void set_tex_handler(unsigned int texHandler) { tex_handler_ = texHandler; }

	bool IsSetFace() const;
	const std::vector<glm::vec3>& points() const { return points_; }
	const std::vector<glm::vec3>& points_after() const { return points_after_; }
	const std::vector<glm::vec2>& tex_coords() const { return tex_coords_; }
	const std::vector<unsigned int>& indices() const { return indices_; }
	unsigned int tex_handler() const { return tex_handler_; }

private:
	std::vector<glm::vec3> points_;
	std::vector<glm::vec3> points_after_;

	std::vector<glm::vec2> tex_coords_;
	std::vector<unsigned int> indices_;
	unsigned int tex_handler_ = 0;
};
