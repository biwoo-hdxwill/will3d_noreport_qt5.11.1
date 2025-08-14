#pragma once
#pragma message("TetrahedronGenerator/SimpleTetrahedronGenerator.h visited")
#include <vector>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif
#include "TetrahedronGenerator_global.h"

/**********************************************************************************************//**
 * @class	SimpleTetrahedronGenerator
 *
 * @brief	MeshMove3d가 잘 동작하는지 확인하기위해 간단한 tetrahedron grid를 생성하는 클레스.
 *
 * @author	Hosan
 * @date	2016-05-13
 **************************************************************************************************/
class TETRAHEDRONGENERATOR_EXPORT SimpleTetrahedronGenerator{
public:

	/**********************************************************************************************//**
	 * @fn	static void SimpleTetrahedronGenerator::makeTetrahedronGrid( std::vector<glm::vec3>& points, std::vector<std::vector<int>>& tetrahedrons, float size, int nGrids );
	 *
	 * @brief	Makes tetrahedron grid.
	 *
	 * @author	Hosan
	 * @date	2016-05-13
	 *
	 * @param [out]	points			vertex of tetrahedrons
	 * @param [out]	tetrahedrons	idxs of tetrahedrons
	 * @param [in]	size			tetrahedron가 이루는 전체 큐브의 크기
	 * @param [in]	nGrids			전체 큐브를 나눌 그리드 수
	 **************************************************************************************************/
	static void makeTetrahedronGrid(
		std::vector<glm::vec3>& points,
		std::vector<std::vector<int>>& tetrahedrons,
		float size,
		int nGrids
		);
};
