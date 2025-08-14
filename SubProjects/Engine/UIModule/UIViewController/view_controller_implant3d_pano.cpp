#include "view_controler_implant3d_pano.h"

#include "../../Common/Common/W3Logger.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"

#include "../../Module/Will3DEngine/renderer_manager.h"

#include "../../Resource/Resource/implant_resource.h"

#include "../../UIGLObjects/gl_implant_widget.h"
#include "../../UIGLObjects/W3SurfaceAxesItem.h"
#include "../../Resource/ResContainer/resource_container.h"

#include "transform_basic_vr.h"
#include "view_render_param.h"
#include "surface_items.h"

using namespace UIViewController;
using namespace UIGLObjects;
using namespace Will3DEngine;

ViewControllerImplant3Dpano::ViewControllerImplant3Dpano() {
  axes_item_.reset(new CW3SurfaceAxesItem());
}

ViewControllerImplant3Dpano::~ViewControllerImplant3Dpano() {
}

void ViewControllerImplant3Dpano::MoveImplant(int* implant_id,
											  glm::vec3* delta_translate,
											  glm::vec3* rotate_axes,
											  float* delta_degree) {
  if (!axes_item_->isPicking()) {
	return;
  }

  ImplantData* implant_data = GetImplantData();

  if (implant_data == nullptr || !implant_data->is_visible())
	return;

  *implant_id = implant_data->id();

  auto view_param = BaseViewController::view_param();

  if (view_param == nullptr)
	return;

  QRectF proj_rect = transform().projection_rect();

  if (axes_item_->isSelectTranslate()) {
	QPointF delta_scene = view_param->scene_mouse_curr() - view_param->scene_mouse_prev();
	QPointF proj_center = proj_rect.center();
	proj_center.setY(-proj_center.y()); //화면 Y축 반전.. 하..

	QPointF delta_gl = view_param->MapSceneToGL(delta_scene) - proj_center;
	QPointF delta_norm_gl(delta_gl.x() / (proj_rect.width()*0.5f),
						  delta_gl.y() / (proj_rect.height()*0.5f));

	*delta_translate = axes_item_->translate(glm::vec3(delta_norm_gl.x(), -delta_norm_gl.y(), 0.0f));
  }
  else if (axes_item_->isSelectRotate()) {

	QPointF proj_center = proj_rect.center();
	proj_center.setY(-proj_center.y());

	QPointF pt_scene_curr = view_param->scene_mouse_curr() - view_param->scene_center();
	QPointF gl_curr = view_param->MapSceneToGL(pt_scene_curr) - proj_center + view_param->gl_trans();
	QPointF gl_norm_curr(gl_curr.x() / (proj_rect.width()*0.5f),
						 gl_curr.y() / (proj_rect.height()*0.5f));

	QPointF pt_scene_prev = view_param->scene_mouse_prev() - view_param->scene_center();
	QPointF gl_prev = view_param->MapSceneToGL(pt_scene_prev) - proj_center + view_param->gl_trans();
	QPointF gl_norm_prev(gl_prev.x() / (proj_rect.width()*0.5f),
						 gl_prev.y() / (proj_rect.height()*0.5f));
	auto rot = axes_item_->rotate(glm::vec3(gl_norm_curr.x(), -gl_norm_curr.y(), 0.0f),
								  glm::vec3(gl_norm_prev.x(), -gl_norm_prev.y(), 0.0f));

	*rotate_axes = rot.second;
	*delta_degree = glm::degrees(rot.first);
  }

#if DEVELOP_MODE
  common::Logger::instance()->PrintDebugMode("ViewControllerImplant3D::MoveImplant");
#endif 
}
void ViewControllerImplant3Dpano::PickAxesItem(bool* is_update_scene) {
  auto view_param = BaseViewController::view_param();
  if (view_param == nullptr)
	return;
  if (!axes_item_->isShown())
	return;


  CW3GLFunctions::printError(__LINE__, "ViewControllerImplant3D::PickAxesItem Start");
  QPointF pt_view = view_param->view_mouse_curr();
  BaseViewController::ReadyFrameBuffer();

  glActiveTexture(pack_pick_axes_.tex_num);
  glBindTexture(GL_TEXTURE_2D, pack_pick_axes_.handler);
  glReadBuffer(pack_pick_axes_.tex_buffer);
  axes_item_->pick(pt_view, is_update_scene, GL_RGBA, GL_FLOAT);
  CW3GLFunctions::printError(__LINE__, "ViewControllerImplant3D::PickAxesItem Start");


#if DEVELOP_MODE
  common::Logger::instance()->PrintDebugMode("ViewControllerImplant3D::PickAxesItem");
#endif 
}

void ViewControllerImplant3Dpano::RenderForPickAxes() {
  if (!IsReady()) {
	return;
  }

  this->SetSurfaceAxesMVP();

  if (!axes_item_->isShown())
	return;

  CW3GLFunctions::printError(__LINE__, "ViewControllerImplant3D::RenderForPickAxes Start");

  BaseViewController::ReadyFrameBuffer();

  glDrawBuffer(pack_pick_axes_.tex_buffer);
  CW3GLFunctions::clearView(true, GL_BACK);


  const auto& prog = Renderer().programs();
  glUseProgram(prog.pick_object);
  axes_item_->render_for_pick(prog.pick_object);

  CW3GLFunctions::printError(__LINE__, "ViewControllerImplant3D::RenderForPickAxes End");

#if DEVELOP_MODE
  common::Logger::instance()->PrintDebugMode("ViewControllerImplant3D::RenderForPickAxes");
#endif 
}
void ViewControllerImplant3Dpano::ClearGL() {
  BaseViewController3D::ClearGL();

  if (axes_item_) {
	axes_item_->clearVAOVBO();
	axes_item_->setTransformMat(glm::mat4(), TransformType::ROTATE);
  }
}

void ViewControllerImplant3Dpano::SelectImplant(int* implant_id) {
  if (GetImplantData() == nullptr)
	return;

  auto view_param = BaseViewController::view_param();
  if (view_param == nullptr)
	return;

  if (!IsReady()) {
	common::Logger::instance()->Print(common::LogType::ERR,
									  "ViewControllerImplant3D::SelectImplant: not ready.");
	assert(false);
	return;
  }

  QPointF pt_view = view_param->view_mouse_curr();
  const auto& prog = Renderer().programs();

  BaseViewController::ReadyFrameBuffer();

  glDrawBuffer(pack_pick_implant_.tex_buffer);

  CW3GLFunctions::clearView(true, GL_BACK);


  surface_items()->SetSurfaceMVP(transform());
  surface_items()->RenderForPick(SurfaceItems::IMPLANT, prog.pick_object);

  glActiveTexture(pack_pick_implant_.tex_num);
  glBindTexture(GL_TEXTURE_2D, pack_pick_implant_.handler);
  glReadBuffer(pack_pick_implant_.tex_buffer);
  glm::vec4 res = CW3GLFunctions::readPickColor(glm::vec2(pt_view.x(), pt_view.y()), GL_RGBA, GL_FLOAT);

  int id = (int)(res.w*255.0f);
  *implant_id = id > 0 ? id : -1;

#if DEVELOP_MODE
  common::Logger::instance()->PrintDebugMode("ViewControllerImplant3D::SelectImplant");
#endif 
}

bool ViewControllerImplant3Dpano::IsPickImplant() const {
  return axes_item_->isPicking();
}

int ViewControllerImplant3Dpano::GetPickImplantID() const {
  ImplantData* implant_data = GetImplantData();

  if (implant_data == nullptr)
	return -1;
  else
	return implant_data->id();
}


ImplantData* ViewControllerImplant3Dpano::GetImplantData() const {
  const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
  int selected_id = res_implant.selected_implant_id();
  if (selected_id < 0)
	return nullptr;

  const auto& implant_datas = res_implant.data();
  if (implant_datas.find(selected_id) == implant_datas.end())
	return nullptr;

  return implant_datas.at(selected_id).get();
}

void ViewControllerImplant3Dpano::SetSurfaceMVP() {
  BaseViewController3D::SetSurfaceMVP();

  this->SetSurfaceAxesMVP();
}

void ViewControllerImplant3Dpano::SetSurfaceAxesMVP() {
  is_selected_implant_ = false;
  axes_item_->setShown(false);

  ImplantData* implant_data = GetImplantData();
  if (implant_data == nullptr || !implant_data->is_visible())
	return;

  if (id_axes_implant_ != implant_data->id()) {
	id_axes_implant_ = implant_data->id();
	axes_item_->setTransformMat(glm::mat4(), TransformType::ROTATE);
  }

  glm::vec3 world_scale = Renderer().GetModelScale();
  glm::mat4 axes_model = glm::scale(glm::vec3(glm::length(world_scale)*0.3f));

  axes_item_->setProjViewMat(transform().projection(), transform().view());
  axes_item_->setTransformMat(axes_model, TransformType::SCALE);
  axes_item_->setTransformMat(implant_data->translate_in_pano(), TransformType::TRANSLATE);
  axes_item_->setTransformMat(transform().rotate(), TransformType::ARCBALL);
  axes_item_->setShown(true);

  is_selected_implant_ = true;
}
void ViewControllerImplant3Dpano::DrawOverwriteSurface() {
  BaseViewController3D::DrawOverwriteSurface();

  const auto& prog = Renderer().programs();
  if (axes_item_->isShown()) {
	glUseProgram(prog.render_surface);

	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);
	glClear(GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	axes_item_->draw(prog.render_surface);
	glDisable(GL_CULL_FACE);

	glDisable(GL_DEPTH_TEST);
  }
}
void ViewControllerImplant3Dpano::SetPackTexture() {
  BaseViewController3D::SetPackTexture();

  pack_pick_implant_ = PackTexture(
	tex_buffer(TEX_PICK_IMPLANT),
	tex_num(TEX_PICK_IMPLANT),
	tex_handler(TEX_PICK_IMPLANT),
	_tex_num(TEX_PICK_IMPLANT)
  );
  pack_pick_axes_ = PackTexture(
	tex_buffer(TEX_PICK_AXIS),
	tex_num(TEX_PICK_AXIS),
	tex_handler(TEX_PICK_AXIS),
	_tex_num(TEX_PICK_AXIS)
  );
}
void ViewControllerImplant3Dpano::SetPackTextureHandle() {
  BaseViewController3D::SetPackTextureHandle();

  pack_pick_implant_.handler = tex_handler(TEX_PICK_IMPLANT);
  pack_pick_axes_.handler = tex_handler(TEX_PICK_AXIS);
}
void ViewControllerImplant3Dpano::ReadyBufferHandles() {
  BaseViewController::ReadyBufferHandles(DEPTH_END, TEX_IMP_END);
}
