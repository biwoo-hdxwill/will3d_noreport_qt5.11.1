#include "volume_renderer.h"

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/GLfunctions/WGLHeaders.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"

#include "../../Core/ImageProcessing/W3ImageProcessing.h"

#include "../../UIModule/UIGLObjects/gl_object.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/active_block_resource.h"

namespace {
  const int kActiveBlockSize = 16;
}
using namespace Renderer;

VolumeRenderer::VolumeRenderer() {
  vr_params_.reset(new VRParam());
}
VolumeRenderer::~VolumeRenderer() {
  Clear();

}
/**=================================================================================================
renderer settings
*===============================================================================================**/

void VolumeRenderer::InvertWindow(bool is_invert) {
  vr_params_->set_invert_window(is_invert);
}

void VolumeRenderer::ResetDeltaWindowWL() {
  WindowWL wwl = vr_params_->wwl();
  wwl.set_delta_width(0.0f);
  wwl.set_delta_level(0.0f);

  vr_params_->set_wwl(wwl);
}
void VolumeRenderer::AdjustWindowWL(const float& delta_width, const float& delta_level) {
  WindowWL wwl = vr_params_->wwl();
  wwl.set_delta_width(wwl.delta_width() + delta_width);
  wwl.set_delta_level(wwl.delta_level() + delta_level);

  vr_params_->set_wwl(wwl);
}

void VolumeRenderer::SetActiveBlock(const int& min_intensity, const int& max_intensity) {
  if (active_block_resource_)
	active_block_resource_->setActiveBlock(min_intensity, max_intensity);

}

void VolumeRenderer::SetTexHandler(unsigned int handler, unsigned int texture_id) {
  vr_params_->SetVolTexHandler(handler, texture_id);
}

void VolumeRenderer::SetRenderDownFactor(int down_factor) {
  vr_params_->SetRenderDownFactor(down_factor);
}

void VolumeRenderer::InitVRparamSet(const CW3Image3D& vol) {
  vr_params_.reset(new VRParam());
  vr_params_->SetVolume(vol);

  active_block_resource_.reset(new ActiveBlockResource(vol, kActiveBlockSize));

  ClearActiveVBO();
  CW3GLFunctions::initVBO(&active_vbo_[0],
						  active_block_resource_->getVertexC(),
						  active_block_resource_->getTexC(),
						  active_block_resource_->getNvertices() * 3);
}

void VolumeRenderer::SetVolCenterZtoZero() {
  glm::vec3 vol_center = vr_params_->vol_center();
  //vol_center.z = 0.0f; // ???

  vr_params_->set_vol_center(vol_center);
}

void VolumeRenderer::SettingsTFhandler(unsigned int handler, unsigned int texture_id) {
  vr_params_->SetTFhandler(handler, texture_id);
}

void VolumeRenderer::SettingsTFMaxAxisTexLength(int max_tf_axis_tex_length) {
  vr_params_->SetTFMaxAxisTexLength(max_tf_axis_tex_length);
}

void VolumeRenderer::SetEnableShade(bool is_shade) {
  vr_params_->set_is_shade(is_shade);
}

void VolumeRenderer::SetEnableMIP(bool is_mip) {
  vr_params_->SetIsMIP(is_mip);
}

void VolumeRenderer::SetEnableXray(bool is_xray) {
  vr_params_->SetIsXRAY(is_xray);
}

void VolumeRenderer::Clear() {
  ClearActiveVBO();

  active_block_resource_.reset();
  vr_params_.reset(new VRParam());
}


/**=================================================================================================
function calls
*===============================================================================================**/
void VolumeRenderer::UpdateActiveBlockVAO(uint* vao) {
  if (*vao == 0) {
	glGenVertexArrays(1, vao);
  }
  glBindVertexArray(*vao);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, active_vbo_[0]);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);
  glBindBuffer(GL_ARRAY_BUFFER, active_vbo_[1]);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);

  if (active_vbo_[2]) {
	glDeleteBuffers(1, &active_vbo_[2]);
	glGenBuffers(1, &active_vbo_[2]);
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, active_vbo_[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, active_block_resource_->GetNumActiveBlockFace() * sizeof(unsigned int),
			   active_block_resource_->getActiveIndex(), GL_DYNAMIC_DRAW);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void VolumeRenderer::SetVolumeTexture() {
  SetVolumeTexture(programs_.ray_firsthit);
  SetVolumeTexture(programs_.ray_casting);
}

void VolumeRenderer::SetVolumeTexture(const uint& program) {
  glActiveTexture((GLuint)vr_params_->vol_tex_id());
  glBindTexture(GL_TEXTURE_3D, (GLuint)vr_params_->vol_tex_handler());

  glUseProgram(program);
  WGLSLprogram::setUniform(program, "VolumeTex", vr_params_->vol_tex_num());
}

void VolumeRenderer::SetTFtexture() {
  glActiveTexture((GLuint)vr_params_->tf_tex_id());
  glBindTexture(GL_TEXTURE_2D, (GLuint)vr_params_->tf_tex_handler());
  glUseProgram(programs_.ray_firsthit);
  WGLSLprogram::setUniform(programs_.ray_firsthit, "MaxTexSize", vr_params_->tf_max_axis_tex_length());
  WGLSLprogram::setUniform(programs_.ray_firsthit, "TransferFunc", vr_params_->tf_tex_num());
  glUseProgram(programs_.ray_casting);
  WGLSLprogram::setUniform(programs_.ray_casting, "MaxTexSize", vr_params_->tf_max_axis_tex_length());
  WGLSLprogram::setUniform(programs_.ray_casting, "TransferFunc", vr_params_->tf_tex_num());
}

void VolumeRenderer::SetStepSizeFast() {
  glUseProgram(programs_.ray_firsthit);
  WGLSLprogram::setUniform(programs_.ray_firsthit, "StepSize", vr_params_->step_size_fast());
  glUseProgram(programs_.ray_casting);
  WGLSLprogram::setUniform(programs_.ray_casting, "StepSize", vr_params_->step_size_fast());
}
void VolumeRenderer::SetStepSize() {
  glUseProgram(programs_.ray_firsthit);
  WGLSLprogram::setUniform(programs_.ray_firsthit, "StepSize", vr_params_->step_size());
  glUseProgram(programs_.ray_casting);
  WGLSLprogram::setUniform(programs_.ray_casting, "StepSize", vr_params_->step_size());
}

void VolumeRenderer::DrawFrontFace(uint vao_cube, int indices, uint front_tex_buffer, const glm::mat4 & mvp, const glm::mat4 & vol_tex_transform) {
  glDrawBuffer(front_tex_buffer);
  {
	glUseProgram(programs_.front_face_cube);

	CW3GLFunctions::clearView(true, GL_BACK);
	WGLSLprogram::setUniform(programs_.front_face_cube, "MVP", mvp);
	WGLSLprogram::setUniform(programs_.front_face_cube, "VolTexTransformMat", vol_tex_transform);
	WGLSLprogram::setUniform(programs_.front_face_cube, "VolTexBias", vr_params_->vol_tex_bias());


	CW3GLFunctions::drawView(vao_cube, indices, GL_BACK);
  }
}

void VolumeRenderer::DrawBackFace(uint vao_cube, int indices, uint back_tex_buffer, const glm::mat4 & mvp, const glm::mat4 & vol_tex_transform) {
  glDrawBuffer(back_tex_buffer);
  {
	glUseProgram(programs_.back_face_cube);

	CW3GLFunctions::clearView(true, GL_FRONT);
	WGLSLprogram::setUniform(programs_.back_face_cube, "MVP", mvp);
	WGLSLprogram::setUniform(programs_.back_face_cube, "VolTexTransformMat", vol_tex_transform);
	WGLSLprogram::setUniform(programs_.back_face_cube, "VolTexBias", vr_params_->vol_tex_bias());

	CW3GLFunctions::drawView(vao_cube, indices, GL_FRONT);

	glDepthFunc(GL_LESS);
  }
}


void VolumeRenderer::DrawFinalFrontFace(GLObject* plane_obj,
										const PackTexture & pack_front,
										const PackTexture & pack_back,
										const PACK_CLIPPING & clipping_pack) {
  CW3GLFunctions::clearDepth(GL_LESS);

  GLenum textures[2] = { pack_front.tex_buffer, pack_back.tex_buffer };
  glDrawBuffers(2, textures);
  {
	glUseProgram(programs_.front_face_final);

	WGLSLprogram::setUniform(programs_.front_face_final, "MVP", vr_params_->mvp_for_final());

	glActiveTexture(pack_back.tex_num);
	glBindTexture(GL_TEXTURE_2D, pack_back.handler);
	WGLSLprogram::setUniform(programs_.front_face_final, "ExitPositions", pack_back._tex_num);

	glActiveTexture(pack_front.tex_num);
	glBindTexture(GL_TEXTURE_2D, pack_front.handler);
	WGLSLprogram::setUniform(programs_.front_face_final, "EntryPositions", pack_front._tex_num);

	WGLSLprogram::setUniform(programs_.front_face_final, "isPlaneClipped", clipping_pack.is_clipping);
	if (clipping_pack.is_clipping) {
	  for (int i = 0; i < clipping_pack.planes.size(); i++) {
		WGLSLprogram::setUniform(programs_.front_face_final, QString("clipPlanes[%1]").arg(i).toStdString().c_str(), clipping_pack.planes[i]);
	  }
	  WGLSLprogram::setUniform(programs_.front_face_final, "numClipPlanes", (int)clipping_pack.planes.size());
	}
	else {
	  WGLSLprogram::setUniform(programs_.front_face_final, "numClipPlanes", 0);
	}

	plane_obj->Render(GL_BACK);

  }
}
void VolumeRenderer::DrawFirstHitRay(GLObject* plane_obj,
									 uint tex_buffer,
									 const PackTexture & pack_front,
									 const PackTexture & pack_back,
									 float threshold_alpha) {

  glDrawBuffer(tex_buffer);
  {
	glUseProgram(programs_.ray_firsthit);
	CW3GLFunctions::clearView(true, GL_BACK);

	WGLSLprogram::setUniform(programs_.ray_firsthit, "MVP", vr_params_->mvp_for_final());

	glActiveTexture(pack_back.tex_num);
	glBindTexture(GL_TEXTURE_2D, pack_back.handler);
	WGLSLprogram::setUniform(programs_.ray_firsthit, "ExitPositions", pack_back._tex_num);

	glActiveTexture(pack_front.tex_num);
	glBindTexture(GL_TEXTURE_2D, pack_front.handler);
	WGLSLprogram::setUniform(programs_.ray_firsthit, "EntryPositions", pack_front._tex_num);

	WGLSLprogram::setUniform(programs_.ray_firsthit, "ThresholdAlpha", threshold_alpha);

	plane_obj->Render(GL_BACK);
  }
}
void VolumeRenderer::DrawRaycastingCut(GLObject * plane_obj,
									   uint ray_tex_buffer,
									   int cut_step,
									   const glm::mat4 & mvp,
									   const glm::mat4 & map_vol_to_mask,
									   const PackTexture & pack_front,
									   const PackTexture & pack_back,
									   const PackTexture & pack_mask) {
  glDrawBuffer(ray_tex_buffer);
  {
	CW3GLFunctions::clearDepth(GL_LESS);
	CW3GLFunctions::setBlend(true);
	glUseProgram(programs_.ray_casting);

	unsigned int pass = glGetSubroutineIndex(programs_.ray_casting, GL_FRAGMENT_SHADER, "cuttingRayCasting");
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass);


	glActiveTexture(pack_mask.tex_num);
	glBindTexture(GL_TEXTURE_3D, pack_mask.handler);
	WGLSLprogram::setUniform(programs_.ray_casting, "CutMaskTex", pack_mask._tex_num);
	WGLSLprogram::setUniform(programs_.ray_casting, "CutStep", cut_step);
	WGLSLprogram::setUniform(programs_.ray_casting, "MapVolToMask", map_vol_to_mask);

	WGLSLprogram::setUniform(programs_.ray_casting, "isShade", vr_params_->is_shade());
	WGLSLprogram::setUniform(programs_.ray_casting, "isMIP", vr_params_->is_mip());
	WGLSLprogram::setUniform(programs_.ray_casting, "isXRAY", vr_params_->is_xray());

	WGLSLprogram::setUniform(programs_.ray_casting, "MVP", vr_params_->mvp_for_final());
	WGLSLprogram::setUniform(programs_.ray_casting, "BMVP", mvp*glm::inverse(vr_params_->vol_tex_bias()));
	WGLSLprogram::setUniform(programs_.ray_casting, "VolTexelSize", vr_params_->texel_size());
	WGLSLprogram::setUniform(programs_.ray_casting, "InvVolTexScale", vr_params_->vol_tex_scale_inv());

	this->SetRaycastingWindow();

	glActiveTexture(pack_front.tex_num);
	glBindTexture(GL_TEXTURE_2D, pack_front.handler);
	WGLSLprogram::setUniform(programs_.ray_casting, "EntryPositions", pack_front._tex_num);
	glActiveTexture(pack_back.tex_num);
	glBindTexture(GL_TEXTURE_2D, pack_back.handler);
	WGLSLprogram::setUniform(programs_.ray_casting, "ExitPositions", pack_back._tex_num);

	plane_obj->Render(GL_BACK);

	CW3GLFunctions::setBlend(false);
  }
}
void VolumeRenderer::DrawRaycasting(GLObject * plane_obj,
									uint ray_tex_buffer,
									const glm::mat4 & mvp,
									const PackTexture & pack_front,
									const PackTexture & pack_back) {
  glDrawBuffer(ray_tex_buffer);
  {
	CW3GLFunctions::clearDepth(GL_LESS);
	CW3GLFunctions::setBlend(true);
	glUseProgram(programs_.ray_casting);

	unsigned int pass = glGetSubroutineIndex(programs_.ray_casting, GL_FRAGMENT_SHADER, "basicRayCasting");
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass);

	WGLSLprogram::setUniform(programs_.ray_casting, "isShade", vr_params_->is_shade());
	WGLSLprogram::setUniform(programs_.ray_casting, "isMIP", vr_params_->is_mip());
	WGLSLprogram::setUniform(programs_.ray_casting, "isXRAY", vr_params_->is_xray());

	WGLSLprogram::setUniform(programs_.ray_casting, "MVP", vr_params_->mvp_for_final());
	WGLSLprogram::setUniform(programs_.ray_casting, "BMVP", mvp*glm::inverse(vr_params_->vol_tex_bias()));
	WGLSLprogram::setUniform(programs_.ray_casting, "VolTexelSize", vr_params_->texel_size());
	WGLSLprogram::setUniform(programs_.ray_casting, "InvVolTexScale", vr_params_->vol_tex_scale_inv());

	this->SetRaycastingWindow();

	glActiveTexture(pack_front.tex_num);
	glBindTexture(GL_TEXTURE_2D, pack_front.handler);
	WGLSLprogram::setUniform(programs_.ray_casting, "EntryPositions", pack_front._tex_num);
	glActiveTexture(pack_back.tex_num);
	glBindTexture(GL_TEXTURE_2D, pack_back.handler);
	WGLSLprogram::setUniform(programs_.ray_casting, "ExitPositions", pack_back._tex_num);

	plane_obj->Render(GL_BACK);

	CW3GLFunctions::setBlend(false);

  }
}
void VolumeRenderer::PostProcessingEnhanced(const PackTexture& pack_texture) {
  glActiveTexture(pack_texture.tex_num);
  glBindTexture(GL_TEXTURE_2D, pack_texture.handler);

  int w, h;
  int miplevel = 0;

  glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &w);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &h);

  int size = w * h;

  float* tex_buffer = new float[size * 4];
  ushort* img_buffer = new ushort[size];

  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, tex_buffer);

  const float kGammma = 2.0f;
#pragma omp parallel for
  for (int i = 0; i < h; i++) {
	ushort* pimg_buf = img_buffer + i * w;
	float* ptex_buf = tex_buffer + ((h - i - 1)*w * 4);
	for (int j = 0; j < w; j++) {
	  float fval = *ptex_buf;
	  fval = (fval < 0.0f) ? 0.0f : (fval > 1.0f) ? 1.0f : fval;
	  ushort uval = static_cast<ushort>(pow(fval, kGammma)*65535.0f);
	  *pimg_buf++ = uval;
	  ptex_buf += 4;
	}
  }

  CW3ImageProcessing::Sharpen(img_buffer, w, h, SHARPEN_LEVEL_3);

#pragma omp parallel for
  for (int i = 0; i < h; i++) {
	ushort* pimg_buf = img_buffer + i * w;
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
void VolumeRenderer::DrawTextureToTexture(GLObject* plane_obj,
										  uint dst_tex_buffer,
										  const PackTexture& src_pack_texture) {
  glDrawBuffer(dst_tex_buffer);
  {
	CW3GLFunctions::clearView(true, GL_BACK);

	glUseProgram(programs_.render_screen);

	glActiveTexture(src_pack_texture.tex_num);
	glBindTexture(GL_TEXTURE_2D, src_pack_texture.handler);

	WGLSLprogram::setUniform(programs_.render_screen, "image_texture", src_pack_texture._tex_num);
	WGLSLprogram::setUniform(programs_.render_screen, "MVP", vr_params_->mvp_for_final());

	plane_obj->Render(GL_BACK);
  }
}

void VolumeRenderer::DrawTexture(GLObject* plane_obj, const PackTexture& pack_textrue) {
  glUseProgram(programs_.render_screen);
  glActiveTexture(pack_textrue.tex_num);
  glBindTexture(GL_TEXTURE_2D, pack_textrue.handler);
  WGLSLprogram::setUniform(programs_.render_screen, "image_texture", pack_textrue._tex_num);
  WGLSLprogram::setUniform(programs_.render_screen, "MVP", vr_params_->mvp_for_final());
  plane_obj->Render(GL_BACK);
}

void VolumeRenderer::DrawTransparencySurface(const std::function<void()>& func_draw_surface) {
  CW3GLFunctions::setBlend(true);

  glPushAttrib(GL_ENABLE_BIT);
  glColorMask(false, false, false, false);
  glDepthMask(false);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_STENCIL_TEST);
  glStencilMask(0xff);

  glDepthFunc(GL_LESS);

  glClearStencil(0.0);
  glClear(GL_STENCIL_BUFFER_BIT);
  glStencilFunc(GL_ALWAYS, 0, 0xff);
  glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

  func_draw_surface();

  glColorMask(true, true, true, true);
  glDepthMask(true);
  glStencilFunc(GL_NOTEQUAL, 0, 0xff);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  func_draw_surface();

  glDisable(GL_STENCIL_TEST);
  glPopAttrib();
  CW3GLFunctions::setBlend(false);
}
void VolumeRenderer::DrawBoneDensity(GLObject* surface_obj,
									 const glm::mat4& model,
									 const glm::mat4& view,
									 const glm::mat4& projection,
									 const glm::mat4 & vol_tex_transform) {
  glUseProgram(programs_.render_bone_density);
  SetVolumeTexture(programs_.render_bone_density);

  glm::mat4 mv = view * model;
  WGLSLprogram::setUniform(programs_.render_bone_density, "VolTexTransformMat", vol_tex_transform);
  WGLSLprogram::setUniform(programs_.render_bone_density, "ModelViewMatrix", mv);
  WGLSLprogram::setUniform(programs_.render_bone_density, "NormalMatrix",
						   glm::mat3(glm::vec3(mv[0]), glm::vec3(mv[1]), glm::vec3(mv[2])));
  WGLSLprogram::setUniform(programs_.render_bone_density, "MVP", projection*view*model);

  WGLSLprogram::setUniform(programs_.render_bone_density, "LightPosition", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

  WGLSLprogram::setUniform(programs_.render_bone_density, "tfOffset", vr_params_->intercept());
  WGLSLprogram::setUniform(programs_.render_bone_density, "is_collide", false);
  WGLSLprogram::setUniform(programs_.render_bone_density, "texelSize", vr_params_->texel_size());

  surface_obj->Render(GL_BACK);

}

bool VolumeRenderer::IsInitialize() const {
  return vr_params_->IsInitialize();
}


const unsigned int VolumeRenderer::GetActiveIndices() const {
  return active_block_resource_->GetNumActiveBlockFace();
}

const glm::vec3& VolumeRenderer::GetModelScale() const {
  return vr_params_->model_scale();
}
const glm::vec3& VolumeRenderer::GetVolTexScale() const {
  return vr_params_->vol_tex_scale();
}
const glm::vec3& VolumeRenderer::GetVolCenter() const {
  return vr_params_->vol_center();
}
float VolumeRenderer::GetSpacingZ() const {
  return vr_params_->z_spacing();
}
const glm::mat4& VolumeRenderer::GetVolTexBias() const {
  return vr_params_->vol_tex_bias();
}
float VolumeRenderer::GetBasePixelSpacingMM() const {
  return vr_params_->base_pixel_size_mm();
}
float VolumeRenderer::GetIntercept() const {
  return vr_params_->intercept();
}
const glm::vec3& VolumeRenderer::GetTexelSize() const {
  return vr_params_->texel_size();
}

const bool & VolumeRenderer::GetInvertWindow() const {
  return vr_params_->invert_window();
}

bool VolumeRenderer::IsXRAY() const {
  return vr_params_->is_xray();
}

void VolumeRenderer::GetWindowForClearColor(float * width, float * level) const {
  *width = vr_params_->wwl().delta_width() / vr_params_->wwl().width();
  *level = vr_params_->wwl().delta_level() / vr_params_->wwl().level();
}
void VolumeRenderer::GetWindowParams(float* window_width, float* window_level) const {
  vr_params_->wwl().GetAdjustedWL(*window_width, *window_level);
}
int VolumeRenderer::GetRenderDownFactor() const {
  return vr_params_->down_factor();
}
void VolumeRenderer::SetRaycastingWindow() {
  float window_width, window_level;

  if (vr_params_->is_xray()) {
	float bone_threshold = vr_params_->bone_threshold();
	window_width = vr_params_->wwl().GetAdjustedWidth();
	window_level = bone_threshold + vr_params_->wwl().delta_level();
  }
  else if (vr_params_->is_mip()) {
	vr_params_->wwl().GetAdjustedWL(window_width, window_level);
  }
  else {
	window_width = 65535.0f +
	  (vr_params_->wwl().delta_width() / vr_params_->wwl().width())*65535.0f;

	window_level = 32767.5f +
	  (vr_params_->wwl().delta_level() / vr_params_->wwl().level())*65535.0f;
  }

  if (!vr_params_->invert_window()) {
	WGLSLprogram::setUniform(programs_.ray_casting, "WindowLevel", window_level);
	WGLSLprogram::setUniform(programs_.ray_casting, "WindowWidth", window_width);
  }
  else {
	WGLSLprogram::setUniform(programs_.ray_casting, "WindowLevel", window_level);
	WGLSLprogram::setUniform(programs_.ray_casting, "WindowWidth", -window_width);
  }
}

void VolumeRenderer::ClearActiveVBO() {
  if (active_vbo_[0] != 0) {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(3, &active_vbo_[0]);
  }
  active_vbo_[0] = active_vbo_[1] = active_vbo_[2] = 0;
}

void VolumeRenderer::ApplyPreferences()
{
	vr_params_->ApplyPreferences();
}
