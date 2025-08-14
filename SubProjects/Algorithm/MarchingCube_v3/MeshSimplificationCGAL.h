#pragma once
#pragma message("# MeshSimplificationCGAL.h visited")
#include "marchingcube_v3_global.h"
#include <vector>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif
class MARCHINGCUBE_V3_EXPORT MeshSimplificationCGAL {
public:
	static bool execute(
		std::vector<glm::vec3>& points,
		std::vector<std::vector<int>>& triangles,
		float fraction
	);
};
