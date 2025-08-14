#include "image_renderer.h"

#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"

#include "../../Core/ImageProcessing/W3ImageProcessing.h"

#include "../../UIModule/UIGLObjects/gl_object.h"

using namespace Renderer;

ImageRenderer::ImageRenderer() {
	image_params_.reset(new ImageParam);
}

/**=================================================================================================
renderer settings
*===============================================================================================**/

void ImageRenderer::InvertWindow(bool is_invert) {
	image_params_->set_invert_window(is_invert);
}

void ImageRenderer::ResetDeltaWindowWL() {
	WindowWL wwl = image_params_->wwl();
	wwl.set_delta_width(0.0f);
	wwl.set_delta_level(0.0f);

	image_params_->set_wwl(wwl);
}
void ImageRenderer::AdjustWindowWL(const float& delta_width, const float& delta_level) {
	WindowWL wwl = image_params_->wwl();
	wwl.set_delta_width(wwl.delta_width() + delta_width);
	wwl.set_delta_level(wwl.delta_level() + delta_level);

	image_params_->set_wwl(wwl);
}

void ImageRenderer::SettingsImageParamSet(const CW3Image3D& vol) {
	image_params_->SetVolume(vol);
}

/**=================================================================================================
function calls
*===============================================================================================**/


void ImageRenderer::DrawImage(GLObject* planeObj, const glm::mat4& mvp, const PackTexture& pack_image) {
	glUseProgram(programs_.render_image);
	glActiveTexture(pack_image.tex_num);
	glBindTexture(GL_TEXTURE_2D, pack_image.handler);

	float window_width, window_level;
	image_params_->wwl().GetAdjustedWL(window_width, window_level);

	if (!image_params_->invert_window()) {
		WGLSLprogram::setUniform(programs_.render_image, "WindowLevel", window_level);
		WGLSLprogram::setUniform(programs_.render_image, "WindowWidth", window_width);
	} else {
		WGLSLprogram::setUniform(programs_.render_image, "WindowLevel", window_level);
		WGLSLprogram::setUniform(programs_.render_image, "WindowWidth", -window_width);
	}

	WGLSLprogram::setUniform(programs_.render_image, "image_texture", pack_image._tex_num);
	WGLSLprogram::setUniform(programs_.render_image, "MVP", mvp);

	planeObj->Render();
}

void ImageRenderer::BlendImage(GLObject* planeObj, const glm::mat4& mvp, const PackTexture& pack_mask) {
	CW3GLFunctions::setBlend(true);

	glUseProgram(programs_.render_mask);
	glActiveTexture(pack_mask.tex_num);
	glBindTexture(GL_TEXTURE_2D, pack_mask.handler);

	WGLSLprogram::setUniform(programs_.render_mask, "image_texture", pack_mask._tex_num);
	WGLSLprogram::setUniform(programs_.render_mask, "MVP", mvp);

	planeObj->Render();

	CW3GLFunctions::setBlend(false);
}

void ImageRenderer::PostProcessingSharpen(const PackTexture& pack_texture, SharpenLevel level) {

	glActiveTexture(pack_texture.tex_num);
	glBindTexture(GL_TEXTURE_2D, pack_texture.handler);

	int w, h;
	int miplevel = 0;

	glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &h);

	int size = w*h;

	float* tex_buffer = new float[size * 4];
	ushort* img_buffer = new ushort[size];

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, tex_buffer);

#pragma omp parallel for
	for (int i = 0; i < h; i++) {
		ushort* pimg_buf = img_buffer + i*w;
		float* ptex_buf = tex_buffer + ((h - i - 1)*w * 4);
		for (int j = 0; j < w; j++) {
			float fval = *ptex_buf;
			fval = (fval < 0.0f) ? 0.0f : (fval > 1.0f) ? 1.0f : fval;
			ushort uval = static_cast<ushort>(fval*65535.0f);
			*pimg_buf++ = uval;
			ptex_buf += 4;
		}
	}

	CW3ImageProcessing::Sharpen(img_buffer, w, h, level);

#pragma omp parallel for
	for (int i = 0; i < h; i++) {
		ushort* pimg_buf = img_buffer + i*w;
		float* ptex_buf = tex_buffer + ((h - i - 1)*w * 4);
		for (int j = 0; j < w; j++) {

			ushort val = *pimg_buf++;
			float f_val = static_cast<float>(val) / 65535.0f;
			f_val = (f_val < 0.0f) ? 0.0f : (f_val > 1.0f) ? 1.0f : f_val;

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

bool ImageRenderer::IsInitialize() const {
	return image_params_->IsInitialize();
}

float ImageRenderer::GetBasePixelSpacingMM() const {
	return image_params_->base_pixel_size_mm();
}
float ImageRenderer::GetIntercept() const {
	return image_params_->intercept();
}

void ImageRenderer::GetWindowParams(float* window_width, float* window_level) const {
	image_params_->wwl().GetAdjustedWL(*window_width, *window_level);
}

const bool & ImageRenderer::GetInvertWindow() const {
	return image_params_->invert_window();
}
