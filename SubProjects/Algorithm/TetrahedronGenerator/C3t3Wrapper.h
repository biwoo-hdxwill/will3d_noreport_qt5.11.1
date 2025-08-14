#pragma once
#pragma message("# TetrahedronGenerator/C3t3Wrapper.h visited")
#include <vector>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif
#include "C3t3Typedef.h"
#include "TetrahedronGenerator_global.h"

/**********************************************************************************************//**
 * @class	C3t3Wrapper
 *
 * @brief	cgal의 c3t3객체를 간단한 tetrahedron으로 해석해주는 Wrapper 클레스이다.
 *
 * @author	Hosan
 * @date	2016-05-13
 **************************************************************************************************/

class TETRAHEDRONGENERATOR_EXPORT C3t3Wrapper{
public:

	/**********************************************************************************************//**
	 * @fn	template<class C3t3> static bool C3t3Wrapper::convert( std::vector<glm::vec3>& points, std::vector<std::vector<int>>& cells, std::vector<std::vector<int>>& facets, const C3t3& c3t3 );
	 *
	 * @brief	Converts.
	 *
	 * @author	Hosan
	 * @date	2016-05-13
	 *
	 * @tparam		C3t3	C3t3_polyhedron을 사용할것임.
	 * @param [out]	points	vertex of tetrahedrons.
	 * @param [out]	cells 	idxs of tetrahedrons.
	 * @param [out]	facets	idxs of surface triangles of tetrahedrons.
	 * @param [in]	c3t3	파싱할 c3t3 객체
	 *
	 * @return	true if it succeeds, false if it fails.
	 **************************************************************************************************/

	template<class C3t3>
	static bool convert(
		std::vector<glm::vec3>& points,
		std::vector<std::vector<int>>& cells,
		std::vector<std::vector<int>>& facets,
		const C3t3& c3t3
	);
};
