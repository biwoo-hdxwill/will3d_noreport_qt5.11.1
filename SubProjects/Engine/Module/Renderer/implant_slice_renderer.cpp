#include "implant_slice_renderer.h"

#include <qstring.h>

#include "../../Common/Common/global_preferences.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/GLfunctions/gl_helper.h"

#include "../../UIModule/UIGLObjects/gl_implant_widget.h"
#include "../../Common/Common/W3Logger.h"

using namespace UIGLObjects;
namespace {
	float kCameraDistance = 420.0f;
}
ImplantSliceRenderer::ImplantSliceRenderer()
{
	if (BaseOffscreenRenderer::MakeCurrent())
	{
		WGLSLprogram::createShaderProgram(
			QString(":/volume_slice/slice_implant.vert"),
			QString(":/volume_slice/slice_implant.frag"), 
			programs_.render_slice_implant
		);

		WGLSLprogram::createShaderProgram(
			QString(":/shader/Test/implant.vert"),
			QString(":/shader/Test/implant.frag"),
			programs_.render_slice_implant_wire
		);

		WGLSLprogram::createShaderProgram(
			QString(":/surface/pick_with_coord.vert"),
			QString(":/surface/pick_with_coord.frag"),
			programs_.pick_object
		);

		BaseOffscreenRenderer::DoneCurrent();
	}

	is_wire_ = (GlobalPreferences::GetInstance()->preferences_.objects.implant.rendering_type == GlobalPreferences::MeshRenderingType::Wire);
}

ImplantSliceRenderer::~ImplantSliceRenderer() {
	if(BaseOffscreenRenderer::MakeCurrent()){
		if (implant_in_vol)
			implant_in_vol->ClearVAOVBO();
		if (implant_in_pano)
			implant_in_pano->ClearVAOVBO();

		BaseOffscreenRenderer::DoneCurrent();
	}
}

void ImplantSliceRenderer::RenderInVol(const Arguments & args, resource::Image2D*& image) {
	if (!implant_in_vol)
		implant_in_vol.reset(new GLImplantWidget(ObjCoordSysType::TYPE_VOLUME));

	int width = (int)(((float)args.img_width)*args.scale);
	int height = (int)(((float)args.img_height)*args.scale);
	image = new resource::Image2D(width, height, image::ImageFormat::RGBA32);
	Render(implant_in_vol.get(), args, image);
}

void ImplantSliceRenderer::RenderInPano(const Arguments & args, resource::Image2D*& image) {
	if (!implant_in_pano)
		implant_in_pano.reset(new GLImplantWidget(ObjCoordSysType::TYPE_PANORAMA_SLICE));

	int width = (int)(((float)args.img_width)*args.scale);
	int height = (int)(((float)args.img_height)*args.scale);
	image = new resource::Image2D(width, height, image::ImageFormat::RGBA32);
	Render(implant_in_pano.get(), args, image);
}

void ImplantSliceRenderer::PickInVol(const Arguments& args, const QPointF& pt_in_image, int* pick_implant_id) {
	if (!implant_in_vol)
		implant_in_vol.reset(new GLImplantWidget(ObjCoordSysType::TYPE_VOLUME));

	*pick_implant_id = Pick(implant_in_vol.get(), args, pt_in_image);
}
void ImplantSliceRenderer::PickInPano(const Arguments & args, const QPointF& pt_in_image, int * pick_implant_id) {
	if (!implant_in_pano)
		implant_in_pano.reset(new GLImplantWidget(ObjCoordSysType::TYPE_PANORAMA_SLICE));

	*pick_implant_id = Pick(implant_in_pano.get(), args, pt_in_image);
}
void ImplantSliceRenderer::SetSize(int width, int height) {
	size_ = Size(width, height);
}
glm::mat4 ImplantSliceRenderer::GetProjectionMatrix(const Arguments& args) const {
	int viewport_width, viewport_height;
	GetViewportSize(args.img_width, args.img_height, args.scale, &viewport_width, &viewport_height);
	return GetProjectionMatrix(viewport_width, viewport_height);
}
glm::mat4 ImplantSliceRenderer::GetViewMatrix(const Arguments& args) const {
	glm::vec3 eye = (glm::normalize(args.slice_up_vector) * (is_wire_ ? 1.0f : kCameraDistance)) + args.center_position_in_vol_gl;
	glm::vec3 center = args.center_position_in_vol_gl;

	glm::vec3 cam_up_vector = -glm::normalize(args.slice_back_vector);
	return glm::lookAt(eye, center, cam_up_vector);
}

void ImplantSliceRenderer::Render(GLImplantWidget* implant, const Arguments& args, resource::Image2D* image_2d) {
	if (MakeCurrent()) {
		int viewport_width, viewport_height;
		GetViewportSize(args.img_width, args.img_height, args.scale, &viewport_width, &viewport_height);

		BaseOffscreenRenderer::BindFrameBufferObject();

		if(!buf_basic().initialized ||
		   buf_basic().width != viewport_width ||
		   buf_basic().height != viewport_height)
			BaseOffscreenRenderer::AttarchBasicBuffers(viewport_width, viewport_height);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			common::Logger::instance()->Print(common::LogType::ERR, "ImplantSliceRenderer::Render: invalid fbo.");
			assert(false);
		}
		glViewport(0, 0, viewport_width, viewport_height);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 view = GetViewMatrix(args);
		glm::mat4 projection = GetProjectionMatrix(GLhelper::ScaleVolToGL(args.img_width),
												   GLhelper::ScaleVolToGL(args.img_height));

		implant->set_projection(projection);
		implant->set_view(view);
		//implant->SetAlpha(0.3f);
		

		glm::vec3 plane_up_vector = glm::normalize(args.slice_up_vector);
		float plane_d = glm::dot(plane_up_vector, args.center_position_in_vol_gl);

		if (!is_wire_)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);

			glUseProgram(programs_.render_slice_implant);
			implant->RenderSlice(programs_.render_slice_implant, glm::vec4(plane_up_vector, -plane_d));
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);

			glUseProgram(programs_.render_slice_implant_wire);
			implant->RenderSliceWire(programs_.render_slice_implant_wire, glm::vec4(plane_up_vector, -plane_d), 2.0f);
		}

		glUseProgram(0);

		GrabFrameImage2D(viewport_width, viewport_height, image_2d);

		CW3GLFunctions::checkFramebufferStatus();
		BaseOffscreenRenderer::ReleaseFrameBufferObject();
		CW3GLFunctions::printError(__LINE__, "ImplantSliceRenderer::Render");

		DoneCurrent();
	}
}

int ImplantSliceRenderer::Pick(GLImplantWidget* implant, const Arguments& args, const QPointF& pt_in_image) {
	if (MakeCurrent()) {

		int viewport_width, viewport_height;
		GetViewportSize(args.img_width, args.img_height, args.scale, &viewport_width, &viewport_height);

		BaseOffscreenRenderer::BindFrameBufferObject();

		if (!buf_basic().initialized ||
			buf_basic().width != viewport_width ||
			buf_basic().height != viewport_height)
			BaseOffscreenRenderer::AttarchBasicBuffers(viewport_width, viewport_height);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			common::Logger::instance()->Print(common::LogType::ERR, "ImplantSliceRenderer::Render: invalid fbo.");
			assert(false);
		}
		glViewport(0, 0, viewport_width, viewport_height);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glm::mat4 view = GetViewMatrix(args);

		glm::mat4 projection = GetProjectionMatrix(GLhelper::ScaleVolToGL(args.img_width),
												   GLhelper::ScaleVolToGL(args.img_height));

		implant->set_projection(projection);
		implant->set_view(view);
		//implant->SetAlpha(0.3f);

		glUseProgram(programs_.pick_object);

		glm::vec3 plane_up_vector = glm::normalize(args.slice_up_vector);
		float plane_d = glm::dot(plane_up_vector, args.center_position_in_vol_gl);
		glm::vec2 pt_pick_in_gl = glm::vec2(pt_in_image.x(), pt_in_image.y())*2.0f;
		int result = implant->PickSlice(programs_.pick_object, glm::vec4(plane_up_vector, -plane_d), pt_pick_in_gl);

		CW3GLFunctions::checkFramebufferStatus();
		BaseOffscreenRenderer::ReleaseFrameBufferObject();
		CW3GLFunctions::printError(__LINE__, "ImplantSliceRenderer::Render");

		DoneCurrent();

		return result;
	}
	else return -1;
}
void ImplantSliceRenderer::GetViewportSize(const int& image_width, const int& image_height, const float& scale, 
										   int* viewport_width, int* viewport_height) const {
	
	*viewport_width = (int)GLhelper::ScaleVolToGL((float)image_width * scale);
	*viewport_height = (int)GLhelper::ScaleVolToGL((float)image_height * scale);
}

glm::mat4 ImplantSliceRenderer::GetProjectionMatrix(int viewport_width,
													int viewport_height) const {
	float left = (-(float)viewport_width * 0.5f);
	float right = ((float)viewport_width * 0.5f);
	float top = ((float)viewport_height * 0.5f);
	float bottom = (-(float)viewport_height * 0.5f);

	float far = is_wire_ ? 0.001f : kCameraDistance * 2.0f;
	return glm::ortho(left, right, bottom, top, 0.0f, far);
}

void ImplantSliceRenderer::GrabFrameImage2D(int viewport_width, int viewport_height, resource::Image2D* image_2d) {
	uchar *buf_img = reinterpret_cast<uchar*>(image_2d->Data());

	int width = image_2d->Width() * 2;
	int height = image_2d->Height() * 2;

	int size = width*height;
	float* buf_frame = new float[size * 4];
	uchar* img_buffer = new uchar[size * 4];

	GLint left = static_cast<GLint>((float)viewport_width*0.5f - (float)width*0.5f);
	GLint top = static_cast<GLint>((float)viewport_height*0.5f - (float)height*0.5f);
	glReadPixels(left, top, width, height, GL_RGBA, GL_FLOAT, buf_frame);

	const float normalize_value = 0.25f * 255.0f;
#pragma omp parallel for
	for (int j = 0; j < image_2d->Height() - 1; j++) {
		int buf_j = (height - 1) - j * 2; //Y축 반전.

		float* _tmp_buf_img_j0 = &buf_frame[buf_j * width * 4];
		float* _tmp_buf_img_j1 = &buf_frame[(buf_j - 1) * width * 4];

		uchar* _buf_img = &buf_img[j * image_2d->Width() * 4];
		for (int i = 0; i < image_2d->Width() - 1; i++) {
			for (int channel = 0; channel < 4; channel++) {
				float a00 = *(_tmp_buf_img_j0 + channel);
				float a01 = *(_tmp_buf_img_j0 + 4 + channel);
				float a10 = *(_tmp_buf_img_j1 + channel);
				float a11 = *(_tmp_buf_img_j1 + 4 + channel);

				uchar interpol = (uchar)((a00 + a01 + a10 + a11) * normalize_value);
				
				*_buf_img++ = interpol;
			}

			_tmp_buf_img_j0 += 8;
			_tmp_buf_img_j1 += 8;
		}
	}

	delete[] img_buffer;
	delete[] buf_frame;
}

void ImplantSliceRenderer::ApplyPreferences()
{
	is_wire_ = (GlobalPreferences::GetInstance()->preferences_.objects.implant.rendering_type == GlobalPreferences::MeshRenderingType::Wire);

	if (implant_in_pano)
	{
		implant_in_pano->ApplyPreferences();
	}
	if (implant_in_vol)
	{
		implant_in_vol->ApplyPreferences();
	}
}
