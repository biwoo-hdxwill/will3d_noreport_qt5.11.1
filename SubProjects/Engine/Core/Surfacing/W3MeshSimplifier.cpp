#include "W3MeshSimplifier.h"

#include "../../../Algorithm/Util/Core/Logger.hpp"
#include "../../../Algorithm/MarchingCube_v3/MeshSimplification.h"
#include "../../../Algorithm/MarchingCube_v3/MeshSimplificationCGAL.h"

#include "../../Common/Common/W3Logger.h"

bool CW3MeshSimplifier::execute_CGAL(
	std::vector<glm::vec3>& points,
	std::vector<std::vector<int>>& triangles,
	float fraction)
{
	lg.tic("mesh simplification cgal");
	bool ret = MeshSimplificationCGAL::execute(points, triangles, fraction);
	lg.toc();
	return ret;
}

bool CW3MeshSimplifier::execute_IsolatedComponentSelectMax(
	std::vector<glm::vec3>& points,
	std::vector<std::vector<int>>& triangles)
{
	common::Logger::instance()->Print(common::LogType::DBG,
		"generateHead before ccl : points " + std::to_string(points.size()) +
		", indices " + std::to_string(triangles.size()));

	bool ret = MeshSimplification::executeIsolatedComponentSelectMax(points, triangles);

	common::Logger::instance()->Print(common::LogType::DBG,
		"generateHead after ccl : points " + std::to_string(points.size()) +
		", indices " + std::to_string(triangles.size()));

	return ret;
}
