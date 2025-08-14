#include "slice_renderer.h"

#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3Memory.h"

#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLHeaders.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/GLfunctions/gl_helper.h"

#include "../../Core/ImageProcessing/W3ImageProcessing.h"

#include "../../UIModule/UIGLObjects/gl_object.h"

using namespace NSliceParam;
using namespace Renderer;

namespace {
	const float kInvUshortMax = 1.0f / 65535.0f;
}  // end of namespace

SliceRenderer::SliceRenderer() { slice_params_.reset(new SliceParam); }

/**=================================================================================================
renderer settings
*===============================================================================================**/
void SliceRenderer::InvertWindow(bool is_invert) {
	slice_params_->set_invert_window(is_invert);
}
void SliceRenderer::ResetDeltaWindowWL() {
	WindowWL wwl = slice_params_->wwl();
	wwl.set_delta_width(0.0f);
	wwl.set_delta_level(0.0f);

	slice_params_->set_wwl(wwl);
}
void SliceRenderer::AdjustWindowWL(const float& delta_width,
	const float& delta_level) {
	WindowWL wwl = slice_params_->wwl();
	wwl.set_delta_width(wwl.delta_width() + delta_width);
	wwl.set_delta_level(wwl.delta_level() + delta_level);

	slice_params_->set_wwl(wwl);
}
void SliceRenderer::SetTexHandler(unsigned int handler,
	unsigned int texture_id) {
	slice_params_->SetVolTexHandler(handler, texture_id);
}

void SliceRenderer::InitSliceParamSet(const CW3Image3D& vol) {
	slice_params_.reset(new SliceParam);
	slice_params_->SetVolume(vol);
}

/**=================================================================================================
function calls
*===============================================================================================**/

void SliceRenderer::SetVolumeTexture() {
	glUseProgram(programs_.render_slice);
	glActiveTexture((GLuint)slice_params_->vol_tex_id());
	glBindTexture(GL_TEXTURE_3D, (GLuint)slice_params_->vol_tex_handler());
	WGLSLprogram::setUniform(programs_.render_slice, "VolumeTex",
		slice_params_->vol_tex_num());
}

void SliceRenderer::SetSliceWindow(float level, float width) {
	glUseProgram(programs_.render_slice);
	WGLSLprogram::setUniform(programs_.render_slice, "WindowLevel", level);
	WGLSLprogram::setUniform(programs_.render_slice, "WindowWidth", width);
}

void SliceRenderer::DrawSlice(GLObject* slice_obj, uint tex_buffer,
	const glm::mat4& mvp,
	const glm::mat4& vol_tex_transform_mat,
	const glm::vec3& up_vector, int thickness,
	bool cull_face /*= true*/, bool data /*= false*/)
{
	glDrawBuffer(tex_buffer);
	{
		if (slice_params_->invert_window())
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		else
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		glEnable(GL_DEPTH_TEST);
		glClearDepth(1.0f);
		glDepthFunc(GL_LESS);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(programs_.render_slice);

		float window_width, window_level;
		slice_params_->wwl().GetAdjustedWL(window_width, window_level);
		if (!slice_params_->invert_window())
		{
			WGLSLprogram::setUniform(programs_.render_slice, "WindowLevel", window_level);
			WGLSLprogram::setUniform(programs_.render_slice, "WindowWidth", window_width);
		}
		else
		{
			WGLSLprogram::setUniform(programs_.render_slice, "WindowLevel", window_level);
			WGLSLprogram::setUniform(programs_.render_slice, "WindowWidth", -window_width);
		}

		glm::mat4 m = vol_tex_transform_mat;
		glm::mat4 mpv_ = mvp;
		glm::mat4 bias = slice_params_->vol_tex_bias();
		glm::vec3 size = slice_params_->texel_size();

		WGLSLprogram::setUniform(programs_.render_slice, "MinIntensity", slice_params_->intensity_min());
		WGLSLprogram::setUniform(programs_.render_slice, "MaxIntensity", slice_params_->intensity_max());
		WGLSLprogram::setUniform(programs_.render_slice, "VolTexBias", slice_params_->vol_tex_bias());
		WGLSLprogram::setUniform(programs_.render_slice, "Thickness", std::max(thickness, 1));
		WGLSLprogram::setUniform(programs_.render_slice, "TexelSize", slice_params_->texel_size());
		WGLSLprogram::setUniform(programs_.render_slice, "VolTexTransformMat", vol_tex_transform_mat);
		WGLSLprogram::setUniform(programs_.render_slice, "UpVector", up_vector);
		WGLSLprogram::setUniform(programs_.render_slice, "MVP", mvp);
		WGLSLprogram::setUniform(programs_.render_slice, "get_data", data);

		if (cull_face)
		{
			slice_obj->Render(GL_BACK);
		}
		else
		{
			slice_obj->Render();
		}
	}
}

void SliceRenderer::DrawScreen(GLObject* planeObj,
	const PackTexture& pack_screen) {
	glUseProgram(programs_.render_screen);
	glActiveTexture(pack_screen.tex_num);
	glBindTexture(GL_TEXTURE_2D, pack_screen.handler);
	WGLSLprogram::setUniform(programs_.render_screen, "image_texture",
		pack_screen._tex_num);
	WGLSLprogram::setUniform(programs_.render_screen, "MVP",
		slice_params_->mvp_for_final());
	planeObj->Render(GL_BACK);
}

void SliceRenderer::PostProcessingSharpen(const PackTexture& pack_texture,
	const SharpenLevel& level) {
	glActiveTexture(pack_texture.tex_num);
	glBindTexture(GL_TEXTURE_2D, pack_texture.handler);

	int w, h;
	int miplevel = 0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &h);

	const int size = w * h;
	const int w_4 = w * 4;
	float* tex_buffer = new float[size * 4];
	ushort* img_buffer = new ushort[size];

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, tex_buffer);

#pragma omp parallel for
	for (int i = 0; i < h; ++i) {
		ushort* pimg_buf = img_buffer + i * w;
		float* ptex_buf = tex_buffer + (h - i - 1) * w_4;
		for (int j = 0; j < w; ++j) {
			float fval = *ptex_buf;
			if (fval < 0.0f)
				fval = 0.0f;
			else if (fval > 1.0f)
				fval = 1.0f;
			*pimg_buf++ = static_cast<ushort>(fval * 65535.0f);
			ptex_buf += 4;
		}
	}

	CW3ImageProcessing::Sharpen(img_buffer, w, h, level);

#pragma omp parallel for
	for (int i = 0; i < h; ++i) {
		ushort* pimg_buf = img_buffer + i * w;
		float* ptex_buf = tex_buffer + (h - i - 1) * w_4;
		for (int j = 0; j < w; ++j) {
			float f_val = static_cast<float>(*pimg_buf++) * kInvUshortMax;
			if (f_val < 0.0f)
				f_val = 0.0f;
			else if (f_val > 1.0f)
				f_val = 1.0f;

			*ptex_buf++ = f_val;
			*ptex_buf++ = f_val;
			*ptex_buf++ = f_val;
			*ptex_buf++;
		}

	}

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_FLOAT, tex_buffer);

	delete[] img_buffer;
	delete[] tex_buffer;
}

bool SliceRenderer::IsInitialize() const {
	return slice_params_->IsInitialize();
}

void SliceRenderer::GetSliceRange(const glm::vec3& slice_up_vector,
	float& gl_min, float& gl_max) const {
	float length_vol = GetMaximumLengthVectorInVol(slice_up_vector);
	gl_max = GLhelper::ScaleVolToGL(length_vol - length_vol * 0.5f);
	gl_min = -gl_max;
}

float SliceRenderer::GetLocationChin() const {
	float vol_z = slice_params_->location_slice_in_vol().chin;
	glm::vec3 pt_gl = GLhelper::MapVolToWorldGL(glm::vec3(0.0f, 0.0f, vol_z),
		slice_params_->vol_center(),
		slice_params_->z_spacing());

	return pt_gl.z;
}

float SliceRenderer::GetLocationNose() const {
	float vol_z = slice_params_->location_slice_in_vol().nose;
	glm::vec3 pt_gl = GLhelper::MapVolToWorldGL(glm::vec3(0.0f, 0.0f, vol_z),
		slice_params_->vol_center(),
		slice_params_->z_spacing());

	return pt_gl.z;
}

const glm::vec3& SliceRenderer::GetModelScale() const {
	return slice_params_->model_scale();
}
const glm::vec3& SliceRenderer::GetVolCenter() const {
	return slice_params_->vol_center();
}
const glm::vec3& SliceRenderer::GetVolTexScale() const {
	return slice_params_->vol_tex_scale();
}

float SliceRenderer::GetBasePixelSpacingMM() const {
	return slice_params_->base_pixel_size_mm();
}

float SliceRenderer::GetIntercept() const { return slice_params_->intercept(); }

float SliceRenderer::GetSpacingZ() const { return slice_params_->z_spacing(); }

const std::vector<glm::vec3>& SliceRenderer::GetVolVertex() const
{
	return slice_params_->vol_vertex();
}

const bool& SliceRenderer::GetInvertWindow() const {
	return slice_params_->invert_window();
}

void SliceRenderer::GetWindowParams(float* window_level,
	float* window_width) const {
	return slice_params_->wwl().GetAdjustedWL(*window_level, *window_width);
}

float SliceRenderer::GetMaximumLengthVectorInVol(const glm::vec3& vec) const {
	std::vector<glm::vec3> vol_vertex = slice_params_->vol_vertex();
	glm::vec3 vol_center = slice_params_->model_scale() * 0.5f;

	float half_max_length = std::numeric_limits<float>::min();
	for (const auto vert : vol_vertex) {
		glm::vec3 center_to_vert = vert - vol_center;

		half_max_length =
			std::max(half_max_length, std::abs(glm::dot(center_to_vert, vec)));
	}

	return half_max_length * 2.0f;
}
