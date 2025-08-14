#pragma once

/**********************************************************************************************//**
 * @class	CW3MeshSimplifier
 *
 * @brief	A wrapping class for mesh simplification
 * 			using MeshSimplification in Algorithm
 * 			and MeshSimplificationCGAL in Algorithm
 *
 * @author	Seo Seok Man
 * @date	2017-07-03
 **************************************************************************************************/
#include <vector>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif

#include "surfacing_global.h"

class SURFACING_EXPORT CW3MeshSimplifier
{

public:
	/* CGAL MeshSimplification area */

	/**********************************************************************************************//**
	 * @fn	static bool CW3MeshSimplifier::executeCGAL( std::vector<glm::vec3>& points, std::vector<std::vector<int>>& triangles, float fraction);
	 *
	 * @brief	Executes the CGAL mesh simplification.
	 *
	 * @author	Seo Seok Man
	 * @date	2017-07-03
	 *
	 * @param [in,out]	points   	The points.
	 * @param [in,out]	triangles	The triangles.
	 * @param 		  	fraction 	The fraction.
	 *
	 * @return	True if it succeeds, false if it fails.
	 **************************************************************************************************/
	static bool execute_CGAL(
		std::vector<glm::vec3>& points,
		std::vector<std::vector<int>>& triangles,
		float fraction);

public: 
	/* MeshSimplification area */

	/**********************************************************************************************//**
	 * @fn	static bool CW3MeshSimplifier::executeIsolatedComponentSelectMax( std::vector<glm::vec3>& points, std::vector<std::vector<int>>& triangles);
	 *
	 * @brief	Executes the isolated component select maximum operation.
	 *
	 * @author	Seo Seok Man
	 * @date	2017-07-03
	 *
	 * @param [in,out]	points   	The points.
	 * @param [in,out]	triangles	The triangles.
	 *
	 * @return	True if it succeeds, false if it fails.
	 **************************************************************************************************/
	static bool execute_IsolatedComponentSelectMax(
		std::vector<glm::vec3>& points,
		std::vector<std::vector<int>>& triangles);
};

