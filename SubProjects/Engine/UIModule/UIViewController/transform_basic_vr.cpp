#include "transform_basic_vr.h"

#include "../../Common/GLfunctions/WGLHeaders.h"

void TransformBasicVR::SetViewMatrix()
{
	set_view(glm::lookAt(glm::vec3(0.0f, -cam_fov(), 0.0f),
			 glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	UpdateMVP();
}
