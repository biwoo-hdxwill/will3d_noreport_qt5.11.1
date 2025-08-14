#pragma once
/**=================================================================================================

Project: 			Resource
File:				plane_resource.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-02
Last modify:		2017-08-02

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

class RESOURCE_EXPORT PlaneResource {
public:
	PlaneResource();

	PlaneResource(const PlaneResource&) = delete;
	PlaneResource& operator=(const PlaneResource&) = delete;

public:
	const std::vector<glm::vec3>& vertices() const { return vertices_; }
	const std::vector<glm::vec2>& tex_coord() const { return tex_coord_; }
	const std::vector<glm::vec2>& tex_coord_inverse_y() const { return tex_coord_inverse_y_; }
	const std::vector<unsigned int> indices() const { return indices_; }

private:
	std::vector<glm::vec3> vertices_;
	std::vector<glm::vec2> tex_coord_;
	std::vector<glm::vec2> tex_coord_inverse_y_;
	std::vector<unsigned int>  indices_;
};
