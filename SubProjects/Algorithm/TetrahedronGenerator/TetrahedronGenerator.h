#pragma once
#pragma message("# TetrahedronGenerator/TetrahedronGenerator.h visited")
#include <vector>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif

#include "TetrahedronGenerator_global.h"
#define DEPRECATED 0


/**********************************************************************************************//**
 * @class	TETRAHEDRONGENERATOR_EXPORT
 *
 * @brief	폐곡면이 주어지면 내부를 채우는 tetrahedron을 만들어내는 cgal 알고리즘을 wrapping한 객체.
 * 			http://doc.cgal.org/latest/Mesh_3/index.html 참조.
 *
 * @author	Hosan
 * @date	2016-05-16
 **************************************************************************************************/

class TETRAHEDRONGENERATOR_EXPORT TetrahedronGenerator {
protected:
	void _init();
public:

	/** @brief	This parameter controls the shape of surface facets. Actually, it is a lower bound for the angle (in degree) of surface facets.
	When boundary surfaces are smooth, the termination of the meshing process is guaranteed if the angular bound is at most 30 degrees. */
	float facetAngle;
	/** @brief	This parameter controls the size of surface facets. Actually, each surface facet has a surface Delaunay ball which is a ball circumscribing the surface facet and centered on the surface patch.
	The parameter facet_size is either a constant or a spatially variable scalar field, providing an upper bound for the radii of surface Delaunay balls. */
	float facetSize;
	/** @brief	This parameter controls the approximation error of boundary and subdivision surfaces.
	Actually, it is either a constant or a spatially variable scalar field.
	It provides an upper bound for the distance between the circumcenter of a surface facet and the center of a surface Delaunay ball of this facet. */
	float facetDistance;
	/** @brief	This parameter controls the shape of mesh cells (but can't filter slivers, as we discussed earlier).
	Actually, it is an upper bound for the ratio between the circumradius of a mesh tetrahedron and its shortest edge.
	There is a theoretical bound for this parameter: the Delaunay refinement process is guaranteed to terminate for values of cell_radius_edge_ratio bigger than 2. */
	float cellRadiusEdgeRatio;
	/** @brief	This parameter controls the size of mesh tetrahedra. 
	It is either a scalar or a spatially variable scalar field. 
	It provides an upper bound on the circumradii of the mesh tetrahedra. */
	float cellSize;

	/**********************************************************************************************//**
	 * @fn	TetrahedronGenerator::TetrahedronGenerator();
	 *
	 * @brief	생성자
	 *
	 * @author	Hosan
	 * @date	2016-05-16
	 **************************************************************************************************/
	TetrahedronGenerator();

	/**********************************************************************************************//**
	 * @fn	bool TetrahedronGenerator::generate_c3t3( std::vector<glm::vec3>& points, std::vector<std::vector<int>>& tetrahedrons, std::vector<std::vector<int>>& triangles, const std::vector<glm::vec3>& surfacePoints, const std::vector<std::vector<int>>& surfaceTriangles ) const;
	 *
	 * @brief	페곡면인 surface mesh 내부에 tetrahedron을 만들어냄. 폐곡면이 아니면 만들어지지 않음.
	 *
	 * @author	Hosan
	 * @date	2016-05-16
	 *
	 * @param [out]	points				vertexs of tetrahedron
	 * @param [out]	tetrahedrons		idxs of tetrahedron
	 * @param [out]	triangles   		idxs of surface triangles
	 * @param [in]	surfacePoints		폐곡면의 vertexs of surface mesh
	 * @param [in]	surfaceTriangles	폐곡면의 idxs of surface mesh
	 *
	 * @return	true if it succeeds, false if it fails.
	 **************************************************************************************************/
	bool generate_c3t3(
		std::vector<glm::vec3>& points,
		std::vector<std::vector<int>>& tetrahedrons,
		std::vector<std::vector<int>>& triangles,
		const std::vector<glm::vec3>& surfacePoints,
		const std::vector<std::vector<int>>& surfaceTriangles
		) const;

};
