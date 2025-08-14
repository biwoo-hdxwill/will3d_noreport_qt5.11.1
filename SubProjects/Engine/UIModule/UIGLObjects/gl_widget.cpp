#include "gl_widget.h"

#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"

using namespace UIGLObjects;

GLWidget::GLWidget() {
}

GLWidget::~GLWidget() {
}

void GLWidget::EditTransformMat(const glm::mat4& mat4, TransformType type) {
	switch (type) {
		case TransformType::TRANSLATE: transform_.translate = mat4*transform_.translate; break;
		case TransformType::ROTATE: transform_.rotate = mat4*transform_.rotate; break;
		case TransformType::SCALE: transform_.scale = mat4*transform_.scale; break;
		case TransformType::ARCBALL: transform_.arcball = mat4*transform_.arcball; break;
		case TransformType::REORIENTATION: transform_.reorien = mat4*transform_.reorien; break;
		default:
			break;
	}
}

void GLWidget::SetTransformMat(const glm::mat4& mat4, TransformType type) {
	switch (type) {
		case TransformType::TRANSLATE: transform_.translate = mat4; break;
		case TransformType::ROTATE: transform_.rotate = mat4; break;
		case TransformType::SCALE: transform_.scale = mat4; break;
		case TransformType::ARCBALL: transform_.arcball = mat4; break;
		case TransformType::REORIENTATION: transform_.reorien = mat4; break;
		default:
			break;
	}
}

void GLWidget::SetAlpha(double alpha) {
	alpha_ = alpha;

	if (alpha_ == 1.0f)
		is_transparency_ = false;
	else
		is_transparency_ = true;
}

const glm::mat4& GLWidget::GetTransformMat(TransformType type) {
	switch (type) {
		case TransformType::TRANSLATE: return transform_.translate;
		case TransformType::ROTATE: return transform_.rotate;
		case TransformType::SCALE: return transform_.scale;
		case TransformType::ARCBALL: return transform_.arcball;
		case TransformType::REORIENTATION: return transform_.reorien;
		default:
			assert(false);
			return *(new glm::mat4());
	}
}

glm::mat4 GLWidget::GetVolTexTransformMat() {
	mat4 inv_scale = glm::inverse(transform_.scale);
	mat4 trans_tex = transform_.translate;
	trans_tex[3][0] *= inv_scale[0][0];
	trans_tex[3][1] *= inv_scale[1][1];
	trans_tex[3][2] *= inv_scale[2][2];

	mat4 rot_tex = glm::translate(mat3(transform_.scale)*centroid_)*transform_.rotate*glm::translate(mat3(transform_.scale)*-centroid_);
	rot_tex = inv_scale*rot_tex*transform_.scale;

	return trans_tex*rot_tex;
}

void GLWidget::SetUniformColor(uint program) {
	WGLSLprogram::setUniform((GLuint)program, "Material.Ka", material_.Ka);
	WGLSLprogram::setUniform((GLuint)program, "Material.Ks", material_.Ks);
	WGLSLprogram::setUniform((GLuint)program, "Material.Shininess", material_.Shininess);
	WGLSLprogram::setUniform((GLuint)program, "Material.Kd", material_.Kd);
	WGLSLprogram::setUniform((GLuint)program, "alpha", static_cast<float>(alpha_));
}

void GLWidget::SetUniformMatrix(uint program) {
	mat4 rotate_model = glm::translate(mat3(transform_.scale)*centroid_)*
		transform_.rotate*
		glm::translate(mat3(transform_.scale)*-centroid_);

	model_ = transform_.translate*rotate_model*transform_.scale;
	mv_ = view_*transform_.arcball*transform_.reorien*model_;
	WGLSLprogram::setUniform((GLuint)program, "ModelViewMatrix", mv_);
	WGLSLprogram::setUniform((GLuint)program, "NormalMatrix",
							 glm::mat3(glm::vec3(mv_[0]), glm::vec3(mv_[1]), glm::vec3(mv_[2])));

	mvp_ = projection_ * mv_;
	WGLSLprogram::setUniform((GLuint)program, "MVP", mvp_);
}

void GLWidget::SetUniformLight(uint program) {
	WGLSLprogram::setUniform(program, "Light.Position", light_.position);
	WGLSLprogram::setUniform(program, "Light.Intensity", light_.intensity);
}

void GLWidget::ReadPickInfo(int x, int y, unsigned char* pick_id, glm::vec3* pick_gl_coord) {
	vec4 pick_color = CW3GLFunctions::readPickColor(vec2(x, y), GL_RGBA, GL_UNSIGNED_BYTE);

	*pick_id = pick_color.w;
	*pick_gl_coord = (vec3(pick_color.x, pick_color.y, pick_color.z) / 255.0f) - 1.0f;
}

void GLWidget::SetLightPosition() {

	//light position은 view eye로 설정한다. 바꾸고 싶으면 override 하면 된다.
	glm::mat4 inv_view = glm::inverse(view_);
	glm::vec4 eye = vec4(inv_view[3]);

	light_.position = (eye);
}
