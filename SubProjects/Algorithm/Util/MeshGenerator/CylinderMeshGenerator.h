#pragma once
#include "../Core/util_global.h"
#include "../geometry/CylinderSystem.h"
//#include "../../NormalEstimationWrapper/KnnWrapper.h"
#include "../KnnWrapper/KnnWrapper.h"
#include <vector>

class UTIL_EXPORT CylinderMeshGenerator{
protected:
	static bool _executeKnn(
		std::vector<glm::vec3>& pointsOut, 
		std::vector<std::vector<int>>& trianglesOut, 
		const std::vector<glm::vec3>& points, 
		const glm::vec3& center, 
		const glm::vec3& zAxis,
		const int nx = 100, // th
		const int ny = 100 // z
		);
	static bool _executeGrid(
		std::vector<glm::vec3>& pointsOut, 
		std::vector<std::vector<int>>& trianglesOut, 
		const std::vector<glm::vec3>& points, 
		const glm::vec3& center, 
		const glm::vec3& zAxis,
		const int nx = 100, // th
		const int ny = 100 // z
		);
	static bool _executePointIteration(
		std::vector<glm::vec3>& pointsOut, 
		std::vector<std::vector<int>>& trianglesOut, 
		const std::vector<glm::vec3>& points, 
		const glm::vec3& center, 
		const glm::vec3& zAxis,
		const int nx = 100, // th
		const int ny = 100 // z
		);
public:
	//CylinderMeshGenerator();
	//CylinderMeshGenerator(const std::vector<glm::vec3>& points, const glm::vec3& center, const glm::vec3& zAxis);

	// execute(pointsOut, trianglesOut, points, center, zAxis, nx, ny)
	// generate cylinder based mesh {pointsOut, trianglesOut} from points
	// where geometry is {center, zAxis}
	// nx: th direction
	// ny: z direction
	static bool execute(
		std::vector<glm::vec3>& pointsOut, 
		std::vector<std::vector<int>>& trianglesOut, 
		const std::vector<glm::vec3>& points, 
		const glm::vec3& center, 
		const glm::vec3& zAxis,
		const int nx = 100, // th
		const int ny = 100 // z
		);
};
