#pragma once

#include "glfunctions_global.h"

#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif

class GLFUNCTIONS_EXPORT GLTransformFunctions {
public:

	static glm::mat4 GetRightRotateMatrix(const glm::mat4 & plane_rotate, const glm::vec3& plane_right_vector,
										  const glm::vec3& init_right_vector);
	static glm::mat4 GetRotMatrixVecToVec(const glm::vec3 & prev_vector, const glm::vec3 & curr_vector);
};
