#pragma once
/*=========================================================================

File:			class CW3ElementGenerator
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-01-29
Last modify:	2016-04-25

=========================================================================*/
#include <vector>
#if defined(__APPLE__)
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#else
#define GLM_SWIZZLE
#include <GL/glm/glm.hpp>
#include <GL/glm/gtx/transform.hpp>
#endif

#include "common_global.h"

class COMMON_EXPORT CW3ElementGenerator {
public:
	static void GenerateTMJCube(const std::vector<glm::vec3>& inVert, std::vector<glm::vec3>& outNorm,
								std::vector<unsigned int>& outIndices, bool isClockwise = false);
	static void GenerateCube(const glm::vec3& center, const glm::vec3& x_vector,
							 const glm::vec3& y_vector, const glm::vec3& z_vector,
							 float width, float height, float depth,
							 std::vector<glm::vec3>& outVert,
							 std::vector<glm::vec3>& outNorm,
							 std::vector<unsigned int>& outIndices, bool isClockwise = false);
	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// Generates smooth normals.
	///
	/// @param	vertices		  	The vertices.
	/// @param	indices			  	The indices.
	/// @param [in,out]	out_normals	output normals.
	/// @param	isClockwise		  	(Optional) the is clockwise.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	static void GenerateSmoothNormals(const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices,
									  std::vector<glm::vec3>& out_normals, bool isClockwise = false);

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// Generates a sector face.
	///
	/// @param	origin			  	The origin.
	/// @param	arch			  	The arch.
	/// @param [in,out]	outVert   	output vertex.
	/// @param [in,out]	outNorm   	output normal.
	/// @param [in,out]	outIndices	output indices.
	/// @param	isClockwise		  	(Optional) the is clockwise.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	static void generateSectorFace(glm::vec3 origin, const std::vector<glm::vec3>& arch,
								   std::vector<glm::vec3>& outVert, std::vector<glm::vec3>& outNorm, std::vector<unsigned int>& outIndices,
								   bool isClockwise = false);

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// Generates a rectangle face.
	///
	/// @param	line1			  	The upper line.
	/// @param	line2			  	The lower line.
	/// @param [in,out]	outVert   	output vertex.
	/// @param [in,out]	outNorm   	output normal.
	/// @param [in,out]	outIndices	output indices.
	/// @param	isClockwise		  	(Optional) the is clockwise.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	static void generateRectFace(const std::vector<glm::vec3>& line1, const std::vector<glm::vec3>& line2,
								 std::vector<glm::vec3>& outVert, std::vector<glm::vec3>& outNorm, std::vector<unsigned int>& outIndices,
								 bool isClockwise = false);

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// Generates a circle face.
	///
	/// @param	origin			  	The origin.
	/// @param	radius			  	The radius.
	/// @param	numOfsides		  	Number of sides.
	/// @param [in,out]	outVert   	output vertex.
	/// @param [in,out]	outNorm   	output normal.
	/// @param [in,out]	outIndices	output indices.
	/// @param	isClockwise		  	(Optional) the is clockwise.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	static void generateCircleFace(glm::vec3 origin, float radius, int numOfsides,
								   std::vector<glm::vec3>& outVert, std::vector<glm::vec3>& outNorm, std::vector<unsigned int>& outIndices, bool isClockwise = false);

private:
	static void pushTriIndex(int i1, int i2, int i3, std::vector<std::vector<unsigned int>>& table, bool isClockwise = false);
	static void pushTriIndex(int i1, int i2, int i3, std::vector<std::vector<unsigned int>>& table,
							 std::vector<unsigned int>& vec, bool isClockwise = false);
	static void CalculateSmoothNormal(const std::vector<std::vector<unsigned int>>& table,
									  const std::vector<glm::vec3> vertexs, std::vector<glm::vec3>& normals);
};
