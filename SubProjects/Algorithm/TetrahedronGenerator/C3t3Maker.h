#pragma once
#pragma message("# TetrahedronGenerator/C3t3Maker.h visited")
#include "TetrahedronGenerator_global.h"
#include "C3t3Typedef.h"
#include "PolyhedronTypedef.h"

/**********************************************************************************************//**
 * @class	C3t3Maker
 *
 * @brief	cgal의 Polyhedron객체에서 C3t3객체를 만들어주도록 wrapping한 클레스. (C3t3은 cgal의 tetrahedron mesh를 나타낸다.)
 * 			facetAngle, facetSize, facetDistance, cellRadiusEdgeRatio, cellSize 옵션을 지원한다.
 * 			http://doc.cgal.org/latest/Mesh_3/index.html 참조.
 *
 * @author	Hosan
 * @date	2016-05-13
 **************************************************************************************************/

class TETRAHEDRONGENERATOR_EXPORT C3t3Maker{
protected:
	void _init();
public:
	float facetAngle;
	float facetSize;
	float facetDistance;
	float cellRadiusEdgeRatio;
	float cellSize;
	C3t3Maker();

	/**********************************************************************************************//**
	 * @fn	bool C3t3Maker::makeFromPolyhedralDomain(C3t3_polyhedral& c3t3, const Polyhedron& polyhedron);
	 *
	 * @brief	Polyhedron 객체로 C3t3 객체를 만듬. (C3t3은 cgal의 tetrahedron mesh를 나타냄.)
	 * 			폐곡면내부를 채우는 tetrahedron mesh를 만들어 낸다.
	 *
	 * @author	Hosan
	 * @date	2016-05-13
	 *
	 * @param [out]	c3t3		cgal C3t3 객체. 폐곡면 내부를 채우도록 만들어진 tetrahedron mesh.
	 * @param [in]	polyhedron  cgal Polyhedron 객체. 폐곡면을 의미함.
	 *
	 * @return	true if it succeeds, false if it fails.
	 **************************************************************************************************/
	bool makeFromPolyhedralDomain(C3t3_polyhedral& c3t3, const Polyhedron& polyhedron);

	/* just for debug */
	static bool makeFromImplicit(C3t3_implicit& c3t3);
};
