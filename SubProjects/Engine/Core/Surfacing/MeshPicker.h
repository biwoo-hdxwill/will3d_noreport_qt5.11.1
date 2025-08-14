#pragma once
#include <vector>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif

#include "surfacing_global.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class	MeshPicker
///
/// @brief	A mesh picker.
///////////////////////////////////////////////////////////////////////////////////////////////////
class SURFACING_EXPORT MeshPicker {
public:
	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	static bool MeshPicker::pick( glm::vec3& point, int& triangleId,
	//  const std::vector<glm::vec3>& points, const std::vector<std::vector<int>>& triangles,
	//   const glm::vec2& normPickingPoint, const glm::mat4& MVP );
	///
	/// @brief	Picks.
	///
	/// @param [in,out]	point	  	picked point
	/// @param [in,out]	triangleId	picked triangle id
	/// @param	points			  	vertex of mesh
	/// @param	triangles		  	idx of mesh
	/// @param	normPickingPoint  	screen point normalized to [-1, 1]^2
	/// @param	MVP				  	MVP
	///
	/// @return	true if it succeeds, false if it fails.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	static bool pick(
		glm::vec3& point,
		int& triangleId,
		const std::vector<glm::vec3>& points,
		const std::vector<std::vector<int>>& triangles,
		const glm::vec2& normPickingPoint,
		const glm::mat4& MVP
	);
};
