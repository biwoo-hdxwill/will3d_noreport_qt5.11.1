#include "transform_image.h"

#include "../../Common/GLfunctions/WGLHeaders.h"
#include "../../Common/GLfunctions/gl_helper.h"

void TransformImage::Initialize(int img_width, int img_height)
{
	img_width_ = img_width;
	img_height_ = img_height;

	glm::vec3 model_scale = glm::vec3((float)(img_width), -(float)(img_height), 1.0f); //데카르트 좌표계 y축 반전.
	BaseTransform::Initialize(model_scale);
}

void TransformImage::Initialize(const glm::vec3 & image_scale) {

	img_width_ = (int)image_scale.x;
	img_height_ = (int)image_scale.y;

	glm::vec3 model_scale = glm::vec3(image_scale.x, -(image_scale.y), 1.0f);
	BaseTransform::Initialize(model_scale);
}

void TransformImage::SetViewMatrix()
{
	set_view(glm::lookAt(glm::vec3(0.0f, 0.0f, cam_fov()), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
	UpdateMVP();
}
