#include "view_controller_face3d.h"

#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/face_photo_resource.h"

#include "../../Module/Will3DEngine/renderer_manager.h"
#include "../../Module/Will3DEngine/texture_pack.h"

#include "../../UIModule/UIGLObjects/W3SurfaceItem.h"

#include "view_render_param.h"

using namespace UIViewController;
using namespace Will3DEngine;
using namespace UIGLObjects;

ViewControllerFace3D::ViewControllerFace3D() {
  transform_.reset(new TransformBasicVR());
  surface_face_.reset(new CW3SurfaceItem());
  surface_face_->setShown(false);
}

ViewControllerFace3D::~ViewControllerFace3D() {
}

/**=================================================================================================
public functions
*===============================================================================================**/

void ViewControllerFace3D::ClearGL() {
  BaseViewController3D::ClearGL();

  surface_face_->clearVAOVBO();
}

void ViewControllerFace3D::SetTFupdated(bool is_min_max_changed) {
  if (!is_min_max_changed)
	return;

  set_indices_vol(Renderer().GetActiveIndices());

  uint vao = vao_vol();
  Renderer().UpdateActiveBlockVAO(&vao);
  set_vao_vol(vao);
}
void ViewControllerFace3D::SetTransparencySurfaceFace(float alpha) {
  surface_face_->setAlpha(alpha);
}

void ViewControllerFace3D::SetVisibleSurfaceFace(bool is_visible) {
  surface_face_->setShown(is_visible);
}

void ViewControllerFace3D::LoadFace3D()
{
	surface_face_->clearVAOVBO();
}

/**=================================================================================================
private functions: level 1
*===============================================================================================**/

void ViewControllerFace3D::InitVAOs() {
  BaseViewController3D::InitVAOs();

  if (surface_face_->isShown() && !surface_face_->isReadyVAO()) {
	const FacePhotoResource& res_face = ResourceContainer::GetInstance()->GetFacePhotoResource();
	surface_face_->initSurfaceFillTexture(res_face.points(), res_face.tex_coords(), res_face.indices());
  }
}

VolumeRenderer& ViewControllerFace3D::Renderer() const {
  return RendererManager::GetInstance().renderer_vol(VOL_MAIN);
}

void ViewControllerFace3D::SetSurfaceMVP() {
  this->SetSurfaceFaceMVP();
}

void ViewControllerFace3D::DrawBackFaceSurface() {
  auto prog = Renderer().programs();

  if (surface_face_->isShown() && !surface_face_->isTransparency())
	this->DrawSurfaceFaceBackFace(prog.back_face_cube);
}

void ViewControllerFace3D::DrawSurface() {
  auto prog = Renderer().programs();

  if (surface_face_->isShown() && !surface_face_->isTransparency())
	this->DrawSurfaceFace(prog.texturing_surface);
}
void ViewControllerFace3D::DrawTransparencySurface() {
  auto prog = Renderer().programs();

  if (surface_face_->isShown() && surface_face_->isTransparency())
	this->DrawSurfaceFace(prog.texturing_surface);
}

/**=================================================================================================
private functions: level 2
*===============================================================================================**/

BaseTransform& ViewControllerFace3D::transform() const {
  return *(dynamic_cast<BaseTransform*>(transform_.get()));
}

void ViewControllerFace3D::SetSurfaceFaceMVP() {
  surface_face_->setProjViewMat(transform().projection(), transform().view());
  surface_face_->setTransformMat(transform().rotate(), ARCBALL);
  surface_face_->setTransformMat(transform().reorien(), REORIENTATION);
  surface_face_->setTransformMat(transform().model(), SCALE);
}

void ViewControllerFace3D::DrawSurfaceFaceBackFace(uint prog) {
  glUseProgram(prog);
  WGLSLprogram::setUniform(prog, "VolTexTransformMat", glm::mat4(1.0f));
  surface_face_->draw(prog);
}

void ViewControllerFace3D::DrawSurfaceFace(uint prog) {
  glUseProgram(prog);
  glActiveTexture((GLenum)TexturePack::GL_TEXTURE_ID::GL_TEXTURE_DEFAULT);
  const FacePhotoResource& pResFace = ResourceContainer::GetInstance()->GetFacePhotoResource();
  glBindTexture(GL_TEXTURE_2D, pResFace.tex_handler());
  WGLSLprogram::setUniform(prog, "tex1", (int)TexturePack::GL_TEXTURE_ID::GL_TEXTURE_DEFAULT_);
  surface_face_->draw(prog, GL_BACK);
}

void ViewControllerFace3D::ApplyPreferences()
{
	BaseViewController3D::ApplyPreferences();
}

void ViewControllerFace3D::SetFitMode(BaseTransform::FitMode mode)
{
	transform_->set_fit_mode(mode);
}
