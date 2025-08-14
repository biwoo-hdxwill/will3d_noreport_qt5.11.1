#pragma once
#pragma message("# TetrahedronGenerator/PolyhedronIOWrapper.h visited")
#include <string>
#include <vector>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif
#include "PolyhedronTypedef.h"
#include "TetrahedronGenerator_global.h"

/**********************************************************************************************//**
 * @class	TETRAHEDRONGENERATOR_EXPORT
 *
 * @brief	cgal의 polyhedron과 vertex of triangles, idxs of triangles 사이를 변환시켜주는 클레스.
 *
 * @author	Hosan
 * @date	2016-05-13
 **************************************************************************************************/

class TETRAHEDRONGENERATOR_EXPORT PolyhedronIOWrapper{
public:

	/**********************************************************************************************//**
	 * @fn	static void PolyhedronIOWrapper::toPolyhedron( Polyhedron& polyhedron, const std::vector<glm::vec3>& points, const std::vector<std::vector<int>>& triangles );
	 *
	 * @brief	vertexs of triangles, idxs of triangles를 cgal의 polyhedron객체로 변환한다.
	 * 			invalid한 points와 triangles는 내부적으로 버려진다.
	 *
	 * @author	Hosan
	 * @date	2016-05-13
	 *
	 * @param [out]	polyhedron	cgal의 polyhedron객체.
	 * @param [in]	points		vertexs of triangles.
	 * @param [in]	triangles	idxs of triangles.
	 **************************************************************************************************/
	static void toPolyhedron(
		Polyhedron&								polyhedron,		// out
		const std::vector<glm::vec3>&			points,			// in points
		const std::vector<std::vector<int>>&	triangles		// in triangles
		);

	/**********************************************************************************************//**
	 * @fn	static void PolyhedronIOWrapper::toPolyhedron( Polyhedron& polyhedron, std::vector<glm::vec3>& points_out, std::vector<std::vector<int>>& triangles_out, const std::vector<glm::vec3>& points, const std::vector<std::vector<int>>& triangles );
	 *
	 * @brief	vertexs of triangles, idxs of triangles를 cgal의 polyhedron객체로 변환한다.
	 *			invalid한 points와 triangles는 내부적으로 버려진다.
	 *			
	 * @author	Hosan
	 * @date	2016-05-13
	 *
	 * @param [out]	polyhedron   	cgal의 polyhedron객체.
	 * @param [out]	points_out   	polyhedron 객체로 변환되는데 사용된 point들.
	 * @param [out]	triangles_out	polyhedron 객체로 변환되는데 사용된 triangles들.
	 * @param [in]	points			vertexs of triangles.
	 * @param [in]	triangles		idxs of triangles.
	 **************************************************************************************************/
	static void toPolyhedron(
		Polyhedron&								polyhedron,		// out 
		std::vector<glm::vec3>&					points_out,		// out valid points
		std::vector<std::vector<int>>&			triangles_out,	// out valid triangles
		const std::vector<glm::vec3>&			points,			// in points
		const std::vector<std::vector<int>>&	triangles		// in triangles
		);

	/**********************************************************************************************//**
	 * @fn	static void PolyhedronIOWrapper::toPolyhedron( Polyhedron& polyhedron, std::vector<glm::vec3>& points_out, std::vector<std::vector<int>>& triangles_out, std::vector<std::vector<int>>& triangles_invalid, const std::vector<glm::vec3>& points, const std::vector<std::vector<int>>& triangles );
	 *
	 * @brief	vertexs of mesh, idxs of mesh를 cgal polyhedron 객체로 변환함.
	 *
	 * @author	Hosan
	 * @date	2016-05-13
	 *
	 * @param [out]	polyhedron		 	cgal의 polyhedron객체.
	 * @param [out]	points_out		 	polyhedron 객체로 변환되는데 사용된 point들.
	 * @param [out]	triangles_out	 	polyhedron 객체로 변환되는데 사용된 triangles들.
	 * @param [out]	triangles_invalid	polyhedron 객체로 변환될때 버려진 triangles들.
	 * @param [in]	points				vertexs of triangles.
	 * @param [in]	triangles			idxs of triangles.
	 **************************************************************************************************/
	static void toPolyhedron(
		Polyhedron&								polyhedron,
		std::vector<glm::vec3>&					points_out,		// out valid points
		std::vector<std::vector<int>>&			triangles_out,	// out valid triangles
		std::vector<std::vector<int>>&			triangles_invalid,	// out invalid triangles
		const std::vector<glm::vec3>&			points,			// in points
		const std::vector<std::vector<int>>&	triangles		// in triangles
		);

	/**********************************************************************************************//**
	 * @fn	static void PolyhedronIOWrapper::fromPolyhedron( std::vector<glm::vec3>& points, std::vector<std::vector<int>>& triangles, const Polyhedron& polyhedron );
	 *
	 * @brief	cgal polyhedron 객체를 vertexs of mesh, idxs of mesh 조합으로 변환함.
	 *
	 * @author	Hosan
	 * @date	2016-05-16
	 *
	 * @param [out]	points   	vertexs of mesh
	 * @param [out]	triangles	idxs of mesh
	 * @param [in]	polyhedron	cgal polyhedron
	 **************************************************************************************************/
	static void fromPolyhedron(
		std::vector<glm::vec3>&					points,			// out points
		std::vector<std::vector<int>>&			triangles,		// out triangles
		const Polyhedron&						polyhedron		// in
		);

	/**********************************************************************************************//**
	 * @fn	static bool PolyhedronIOWrapper::writeOffFile(const Polyhedron& polyhedron, const std::string& fpath);
	 *
	 * @brief	cgal polyhedron 객체를 off file로 씀.
	 *
	 * @author	Hosan
	 * @date	2016-05-16
	 *
	 * @param	polyhedron	cgal polyhedron
	 * @param	fpath	  	file path
	 *
	 * @return	true if it succeeds, false if it fails.
	 **************************************************************************************************/
	static bool writeOffFile(const Polyhedron& polyhedron, const std::string& fpath);

	/**********************************************************************************************//**
	 * @fn	static bool PolyhedronIOWrapper::writeOffFile(const std::vector<glm::vec3>& points, const std::vector<std::vector<int>>& triangles, const std::string& fpath);
	 *
	 * @brief	vertexs of mesh, idxs of mesh를 off file로 씀.
	 *
	 * @author	Hosan
	 * @date	2016-05-16
	 *
	 * @param	points   	vertexs of mesh
	 * @param	triangles	idxs of mesh
	 * @param	fpath	 	file path
	 *
	 * @return	true if it succeeds, false if it fails.
	 **************************************************************************************************/
	static bool writeOffFile(const std::vector<glm::vec3>& points, const std::vector<std::vector<int>>& triangles, const std::string& fpath);
};
