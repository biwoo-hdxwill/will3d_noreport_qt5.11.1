#include "navigator_offscreen_renderer.h"

#include <QImage>
#include <QPixmap>
#include <qstring.h>
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/color_will3d.h"

#include "../../UIModule/UIGLObjects/W3VBOs.h"
#include "../../UIModule/UIGLObjects/W3SurfaceLineItem.h"
#include "../../UIModule/UIGLObjects/uiglobjects_defines.h"

namespace {
	float kCamFOV = 2.5f;
	float kCamDistance = kCamFOV*3.0f;
}
NavigatorOffscreenRenderer::NavigatorOffscreenRenderer() {
	view_ = glm::lookAt(glm::vec3(0.0f, -kCamDistance, 0.0f),
						glm::vec3(0.0f, 0.0f, 0.0f),
						glm::vec3(0.0f, 0.0f, -1.0f));
	projection_ = glm::frustum(-kCamFOV*0.5f, kCamFOV*0.5f, -kCamFOV*0.5f, kCamFOV*0.5f, kCamFOV*2.0f, kCamFOV*4.0f);

	BaseOffscreenRenderer::MakeCurrent();

	WGLSLprogram::createShaderProgram(QString(":/surface/surface.vert"),
									  QString(":/surface/surface.frag"), shader_prog_);

	stl_head_.reset(new CW3VBOSTL(":/stl/Navigator/head_volume_coordinate_system.stl"));

	for (int i = 0; i < AXIS_END; i++) {
		major_axes_[i].reset(new CW3SurfaceLineItem(CW3SurfaceLineItem::CURVE, false));
		major_axes_[i]->set_line_width(0.1f);
		major_axes_[i]->addPoint(glm::vec3(0.0f));
		major_axes_[i]->setProjViewMat(projection_, view_);
	}
	major_axes_[AXIS_X]->addPoint(glm::vec3(-2.0f, 0.0f, 0.0f));
	major_axes_[AXIS_Y]->addPoint(glm::vec3(0.0f, -2.0f, 0.0f));
	major_axes_[AXIS_Z]->addPoint(glm::vec3(0.0f, 0.0f, 2.0f));

	BaseOffscreenRenderer::DoneCurrent();
}

NavigatorOffscreenRenderer::~NavigatorOffscreenRenderer() {
	BaseOffscreenRenderer::MakeCurrent();
	stl_head_->clearVAOVBO();

	for (int i = 0; i < AXIS_END; i++)
		major_axes_[i]->clearVAOVBO();

	glDeleteProgram(shader_prog_);
	BaseOffscreenRenderer::DoneCurrent();
}
void NavigatorOffscreenRenderer::Render() {
	if(MakeCurrent()) {
		BaseOffscreenRenderer::BindFrameBufferObject();
		int viewport_width = size_.w;
		int viewport_height = size_.h;

		if (viewport_width < 1 ||
			viewport_height < 1)
		{
			DoneCurrent();
			return;
		}
		if (!buf_basic().initialized ||
			buf_basic().width != viewport_width ||
			buf_basic().height != viewport_height)
			BaseOffscreenRenderer::AttarchBasicBuffers(viewport_width, viewport_height);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			common::Logger::instance()->Print(common::LogType::ERR, "ImplantSliceRenderer::Render: invalid fbo.");
			DoneCurrent();
			return;
		}
		glViewport(0, 0, viewport_width, viewport_height);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glUseProgram(shader_prog_);
		this->SetUniformMatrix(shader_prog_);
		this->SetUniformLight(shader_prog_);
		this->SetUniformColor(shader_prog_);
		stl_head_->render();
		for (int i = 0; i < AXIS_END; i++) {
			SetAxisColor(shader_prog_, (Axis)i);
			major_axes_[i]->draw(shader_prog_);
		}
		glDisable(GL_DEPTH_TEST);

		BaseOffscreenRenderer::ReleaseFrameBufferObject();
		DoneCurrent();
	}
}
void NavigatorOffscreenRenderer::SetSize(int width, int height) {
	size_ = Size(width, height);
}
void NavigatorOffscreenRenderer::SetWorldAxisDirection(const glm::mat4& rot_mat, const glm::mat4& view_mat) 
{
	rotate_ = rot_mat;

	glm::vec3 forward = -glm::vec3(view_mat[0][2], view_mat[1][2], view_mat[2][2]);
	glm::vec3 up = glm::vec3(view_mat[0][1], view_mat[1][1], view_mat[2][1]);

	glm::mat4 inv_view = glm::inverse(view_mat);
	glm::vec3 cam_position = glm::normalize(vec3(inv_view[3])) * kCamDistance;
	glm::vec3 center = cam_position + forward * kCamDistance;

	view_ = glm::lookAt(cam_position, center, up);

	for (int i = 0; i < AXIS_END; i++) 
	{
		major_axes_[i]->setTransformMat(rotate_, UIGLObjects::ROTATE);
		major_axes_[i]->setProjViewMat(projection_, view_);
	}

}
void NavigatorOffscreenRenderer::SetUniformMatrix(const uint& prog) {
	mat4 mv = view_ * rotate_;

	GLuint program = (GLuint)prog;
	WGLSLprogram::setUniform(program, "ModelViewMatrix", mv);
	WGLSLprogram::setUniform(program, "NormalMatrix",
							 glm::mat3(glm::vec3(mv[0]), glm::vec3(mv[1]), glm::vec3(mv[2])));
	WGLSLprogram::setUniform(program, "MVP", projection_ * mv);

}
void NavigatorOffscreenRenderer::SetUniformLight(const uint& prog) {
	glm::mat4 inv_view = glm::inverse(view_);
	glm::vec4 eye = vec4(inv_view[3]);

	GLuint program = (GLuint)prog;
	WGLSLprogram::setUniform(program, "Light.Position", eye);
	WGLSLprogram::setUniform(program, "Light.Intensity", glm::vec3(1.0f));
}
void NavigatorOffscreenRenderer::SetUniformColor(const uint& prog) {
	QColor color = ColorNavigator::kColor;
	glm::vec3 colr = glm::vec3((float)color.red() / 255.0f,
		(float)color.green() / 255.0f,
							   (float)color.blue() / 255.0f);
	UIGLObjects::Material material(colr*0.65f, colr*0.35f, glm::vec3(0.0f), 1.0f);

	GLuint program = (GLuint)prog;
	WGLSLprogram::setUniform(program, "Material.Ka", material.Ka);
	WGLSLprogram::setUniform(program, "Material.Ks", material.Ks);
	WGLSLprogram::setUniform(program, "Material.Shininess", material.Shininess);
	WGLSLprogram::setUniform(program, "Material.Kd", material.Kd);
	WGLSLprogram::setUniform(program, "alpha", 1.0f);

}
void NavigatorOffscreenRenderer::SetAxisColor(const uint& prog, const Axis& type) {
	QColor color;
	switch (type) {
		case AXIS_X:			
			color = AxisColor::kX;
			break;
		case AXIS_Y:
			color = AxisColor::kY;
			break;
		case AXIS_Z:
			color = AxisColor::kZ;
			break;
		default:
			assert(false);
			break;
	}

	major_axes_[type]->setLineColor(color);
}
