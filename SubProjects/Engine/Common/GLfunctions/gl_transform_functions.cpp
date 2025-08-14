#include "gl_transform_functions.h"

#if defined(__APPLE__)
#include <glm/gtx/transform2.hpp>
#else
#include <GL/glm/gtx/transform2.hpp>
#endif

glm::mat4 GLTransformFunctions::GetRightRotateMatrix(const glm::mat4& plane_rotate,
													 const glm::vec3& plane_right_vector,
													 const glm::vec3& init_right_vector) {
	glm::vec3 right_ori_rotate = glm::mat3(plane_rotate)*init_right_vector;
	return GetRotMatrixVecToVec(right_ori_rotate, glm::vec3(plane_right_vector));
}

glm::mat4 GLTransformFunctions::GetRotMatrixVecToVec(const glm::vec3& prev_vector, const glm::vec3& curr_vector) {
	glm::vec3 nv1 = glm::normalize(prev_vector);
	glm::vec3 nv2 = glm::normalize(curr_vector);

	if (abs(glm::dot(nv1, nv2)) >= 1.0f)
		return glm::mat4(1.0f);

	glm::vec3 h = glm::normalize(glm::cross(nv1, nv2));
	float th = acos(glm::clamp(glm::dot(nv1, nv2), -1.f, 1.f));
	return glm::rotate(th, h);
}
