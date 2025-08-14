#pragma once
#pragma message("# RenderingCode_v2/MeshPicker.h visited")
#include "renderingcode_v2_global.h"
#include <vector>
#include <gl/glm/glm.hpp>
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class	MeshPicker
///
/// @brief	A mesh picker.
///////////////////////////////////////////////////////////////////////////////////////////////////
class RENDERINGCODE_V2_EXPORT MeshPicker {
public:
	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	static bool MeshPicker::pick( glm::vec3& point, int& triangleId, const std::vector<glm::vec3>& points, const std::vector<std::vector<int>>& triangles, const glm::vec2& normPickingPoint, const glm::mat4& MVP );
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
