#pragma once
#include <vector>
#include <functional>
#include <gl/glm/glm.hpp>
#include "../Core/util_global.h"
class UTIL_EXPORT MeshGenerator {
public:
	// execute(points, triangles, nx, ny, fx, fy, fz)
	// generate mesh
	// nx and ny follow right hand coordinate
	// i.e. left below coner is (0,0)
	static bool execute(
		std::vector<glm::vec3>& points,
		std::vector<std::vector<int>>& triangles,
		int nx,
		int ny,
		const std::function<glm::vec3(int, int)>& f
	);

	static bool execute2(
		std::vector<glm::vec3>& points,
		std::vector<std::vector<int>>& triangles,
		int nx,
		int ny,
		const std::function<void(glm::vec3&/*return*/, bool&/*valid*/, int/*nx*/, int/*ny*/)>& f
	);

	//static bool execute(
	//	std::vector<glm::vec3>& points,
	//	std::vector<std::vector<int>>& triangles,
	//	int nx,
	//	int ny,
	//	const std::function<glm::vec3 (int, int)>& f,
	//	const std::function<bool (int, int)>& exist
	//);
};
