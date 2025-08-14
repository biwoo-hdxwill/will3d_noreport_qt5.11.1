#include "base_view_controller_3d.h"

#include <GL/glew.h>
#include <qapplication.h>
#include <qmath.h>
#include <functional>

#include "../../Common/Common/W3Cursor.h"
#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/gl_helper.h"

#include "../../Module/Renderer/volume_renderer.h"
#include "../../Module/Will3DEngine/renderer_manager.h"
#include "../../UIModule/UIGLObjects/view_plane_obj_gl.h"

#include "surface_items.h"
#include "transform_basic_vr.h"
#include "view_render_param.h"

using namespace UIViewController;
using namespace Will3DEngine;

BaseViewController3D::BaseViewController3D() {
  surface_items_.reset(new SurfaceItems);
}

BaseViewController3D::~BaseViewController3D() { ClearGL(); }

/**=================================================================================================
public functions
*===============================================================================================**/

void BaseViewController3D::SetCliping(const std::vector<glm::vec4>& planes,
									  bool is_enable) {
  pack_clipping_.is_clipping = is_enable;
  pack_clipping_.planes = planes;
}
void BaseViewController3D::SetVisibleNerve(bool is_visible) {
  surface_items_->SetVisible(SurfaceItems::NERVE, is_visible);
}
void BaseViewController3D::SetVisibleImplant(bool is_visible) {
  surface_items_->SetVisible(SurfaceItems::IMPLANT, is_visible);
}
glm::vec3 BaseViewController3D::MapSceneToVol(const QPointF& pt_scene) const {
  return VolumeTracking(pt_scene, 0.0001f, false);
}
void BaseViewController3D::MapSceneToVol(const QPointF& src_scene_point,
										 glm::vec3* dst_vol_point) const {}
inline QPointF BaseViewController3D::MapVolToScene(
  const glm::vec3& src_vol_point) const {
  ViewRenderParam* view_param = BaseViewController::view_param();

  if (view_param == nullptr) return QPointF();

  QRectF proj_rect = transform().projection_rect();
  QPointF proj_center = proj_rect.center();
  proj_center.setY(-proj_center.y());

  vec4 pt_gl =
	vec4(GLhelper::MapVolToWorldGL(src_vol_point, Renderer().GetVolCenter(),
		 Renderer().GetSpacingZ()),
		 1.0f);
  pt_gl.x = -pt_gl.x;

  glm::mat4 pv = transform().projection() * transform().GetCameraMatrix();
  pt_gl = pv * pt_gl;
  QPointF norm_p = QPointF(pt_gl.x, -pt_gl.y);
  QPointF p = QPointF(norm_p.x() * (proj_rect.width() * 0.5f),
					  norm_p.y() * (proj_rect.height() * 0.5f));

  p = p + proj_center - view_param->gl_trans();

  QPointF pt_scene = view_param->MapGLToScene(p);
  return pt_scene + view_param->scene_center();
}
void BaseViewController3D::ClearGL() {
  BaseViewController::ClearGL();

  if (vao_vol_) {
	glDeleteVertexArrays(1, &vao_vol_);
	vao_vol_ = 0;
  }

  surface_items_->ClearGL();
}
void BaseViewController3D::ProcessViewEvent(bool* need_render) {
  auto view_param = BaseViewController::view_param();
  if (view_param == nullptr) {
	*need_render = false;
	return;
  }

  EVIEW_EVENT_TYPE event_type = view_param->event_type();
  switch (event_type) {
  case UIViewController::NO_EVENT:
	*need_render = false;
	break;
  case UIViewController::MEASUREMENT:
	*need_render = false;
	break;
  case UIViewController::PAN:
	BaseViewController::PanView();
	this->SetProjection();
	*need_render = true;
	break;
  case UIViewController::ZOOM:
	BaseViewController::ScaleView();
	this->SetProjection();
	*need_render = true;
	break;
  case UIViewController::ZOOM_WHEEL:
	BaseViewController::ScaleWheelView();
	this->SetProjection();
	*need_render = true;
	break;
  case UIViewController::FIT:
	BaseViewController::FitView();
	this->SetProjection();
	*need_render = true;
	break;
  case UIViewController::LIGHT:
	this->LightView();
	*need_render = false;
	break;
  case UIViewController::RESET:
	this->ResetView();
	*need_render = true;
	break;
  case UIViewController::ROTATE:
	RotateArcBall();
	*need_render = true;
	break;
  case UIViewController::UPDATE:
	this->SetProjection();
	*need_render = true;
	break;
  default:
	assert(false);
	*need_render = false;
  }
}

void BaseViewController3D::SetProjection() {
  if (!Renderer().IsInitialize()) return;

  auto view_param = BaseViewController::view_param();
  if (view_param == nullptr) return;

  PackViewProj arg = BaseViewController::GetPackViewProj();

  float map_scene_to_gl;
  transform().SetProjection(arg, map_scene_to_gl);

  view_param->set_map_scene_to_gl(map_scene_to_gl);
}

bool BaseViewController3D::IsReady() {
  return (Renderer().IsInitialize() && initialized());
}

void BaseViewController3D::RenderingVolume() {
  if (!initialized()) {
	Initialize();
  }

  if (!Renderer().IsInitialize()) {
	ClearGL();
	return;
  }

#if DEVELOP_MODE
  clock_t t_start, t_end;
  t_start = clock();
  common::Logger::instance()->PrintDebugMode(
	"BaseViewController3D::RenderingVolume");
#endif

  CW3GLFunctions::printError(__LINE__, "BaseViewController3D::RenderVolume Start");

  BaseViewController::ReadyFrameBuffer();

  this->InitVAOs();
  CW3GLFunctions::printError(__LINE__, "BaseViewController3D::InitVAOs");

  if (vao_vol_ == 0) return;

  this->SetSurfaceMVP();
  CW3GLFunctions::printError(__LINE__, "BaseViewController3D::SetSurfaceMVP");

  this->RayCasting();

  CW3GLFunctions::printError(__LINE__, "BaseViewController3D::RayCasting");

  this->BlendingGL();
  CW3GLFunctions::printError(__LINE__, "BaseViewController3D::BlendingGL");

#if DEVELOP_MODE
  t_end = clock();
  float elapsedTime = static_cast<float>(t_end - t_start);
  common::Logger::instance()->PrintDebugMode(
	"", QString("%1(ms)").arg(elapsedTime).toStdString());
#endif
}

void BaseViewController3D::RenderScreen(uint dfbo) {
  auto view_param = BaseViewController::view_param();
  if (view_param == nullptr) return;

  const QSize& view_size = view_param->view_size();

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dfbo);
  glViewport(0, 0, view_size.width(), view_size.height());

  float width, level;
  Renderer().GetWindowForClearColor(&width, &level);

  float window_width = 1.0f + width;
  if (Renderer().GetInvertWindow()) window_width *= -1.0f;

  float window_level = 0.5f + level;
  float window_min = (window_level - window_width * 0.5f);

  float color = (-window_min) / window_width;
  CW3GLFunctions::clearView(true, GL_BACK, color, color, color, 1.0f);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  Renderer().DrawTexture(plane_obj(), pack_screen_);
  glDisable(GL_BLEND);
  CW3GLFunctions::printError(__LINE__, "BaseViewController3D::RenderScreen");
}

void BaseViewController3D::RotateArcBall() {
  auto view_param = BaseViewController::view_param();
  if (view_param == nullptr) return;

  glm::vec3 v1 = GetMouseVector(view_param->scene_mouse_prev());
  glm::vec3 v2 = GetMouseVector(view_param->scene_mouse_curr());

  glm::vec3 rot_axis(1.f, 0.f, 0.f);
  float rot_angle = 0.0f;
  if (glm::length(v1 - v2) > 1e-5f) {
	rot_angle = std::acos(std::min(1.0f, glm::dot(v1, v2))) * 180.0f / M_PI;
	rot_angle *= common::kArcballSensitivity;

	rot_axis = glm::cross(v1, v2);
	rot_axis = glm::normalize(rot_axis);
  }

  transform().Rotate(rot_angle, rot_axis);

  QApplication::setOverrideCursor(CW3Cursor::ArrowCursor());
}

void BaseViewController3D::ForceRotateMatrix(const glm::mat4& mat) {
  transform().ForceRotate(mat);
}

void BaseViewController3D::SetReorienMatrix(const glm::mat4& mat) {
  transform().SetReorien(mat);
}

glm::mat4 BaseViewController3D::GetRotateMatrix() {
  return transform().rotate() * transform().reorien();
}
glm::mat4 BaseViewController3D::GetCameraMatrix() {
  return transform().GetCameraMatrix();
}
const glm::mat4& BaseViewController3D::GetViewMatrix() {
  return transform().view();
}
const glm::mat4& BaseViewController3D::GetProjectionMatrix() {
  return transform().projection();
}
const bool& BaseViewController3D::GetInvertWindow() const {
  return Renderer().GetInvertWindow();
}
float BaseViewController3D::GetIntercept() const {
  return Renderer().GetIntercept();
}
void BaseViewController3D::GetWindowParams(float* window_width,
										   float* window_level) const {
  return Renderer().GetWindowParams(window_width, window_level);
}
glm::vec3 BaseViewController3D::VolumeTracking(const QPointF& pt_mouse,
											   const float& threshold_alpha,
											   bool is_print_log) const {
  try {
	if (vao_vol() == 0) {
	  throw std::exception(
		"CW3GLFunctions::readPickColor: vao is not ready.");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, fb_handler());
	GLint viewport[4];

	glGetIntegerv(GL_VIEWPORT, viewport);

	if (pt_mouse.x() < viewport[0] || pt_mouse.y() < viewport[1] ||
		pt_mouse.x() > viewport[2] || pt_mouse.y() > viewport[3]) {
	  throw std::exception(
		"CW3GLFunctions::readPickColor: viewport out of range");
	}

	Renderer().DrawFirstHitRay(plane_obj(), tex_buffer(TEX_RAYCASTING),
							   pack_front_, pack_back_, threshold_alpha);

	int pt_x = pt_mouse.x();
	int pt_y = viewport[3] - pt_mouse.y();

	vec4 position;

	glReadBuffer(tex_buffer(TEX_RAYCASTING));
	glReadPixels(pt_x, pt_y, 1, 1, GL_RGBA, GL_FLOAT, &position);

	if (position.w == 0.0f ||
		std::numeric_limits<float>::infinity() == position.w)
	  throw std::exception("position not found");

	mat4 invTexBias = glm::inverse(Renderer().GetVolTexBias());
	vec3 pt_gl =
	  vec3(transform().model() * invTexBias * vec4(vec3(position), 1.0f));
	pt_gl *= vec3(-1.0f, 1.0f, 1.0f);

	return GLhelper::MapWorldGLtoVol(pt_gl, Renderer().GetVolCenter(),
									 Renderer().GetSpacingZ());
  }
  catch (std::exception& e) {
	if (is_print_log)
	  common::Logger::instance()->Print(common::LogType::WRN, e.what());
	return vec3(-1.0f);
  }
}
/**=================================================================================================
protected functions
*===============================================================================================**/

void BaseViewController3D::InitVAOs() {
  uint renderer_vol_indices = Renderer().GetActiveIndices();
  if (vao_vol() == 0 || indices_vol_ != renderer_vol_indices) {
	Renderer().UpdateActiveBlockVAO(&vao_vol_);
	set_indices_vol(renderer_vol_indices);
  }
}

void BaseViewController3D::SetSurfaceMVP() {
  surface_items_->SetSurfaceMVP(transform());
}

void BaseViewController3D::DrawBackFaceSurface() {
  const auto& prog = Renderer().programs();
  surface_items_->RenderAll(prog.back_face_cube);
}

void BaseViewController3D::DrawSurface() {
  const auto& prog = Renderer().programs();
  surface_items_->RenderAll(prog.render_surface);
}

/**=================================================================================================
private functions: level 0
*===============================================================================================**/

bool BaseViewController3D::SetRenderer() {
  if (Renderer().IsInitialize()) {
	auto view_param = BaseViewController::view_param();
	if (view_param == nullptr) return false;

	transform().Initialize(Renderer().GetModelScale());
	view_param->set_base_pixel_spacing_mm(Renderer().GetBasePixelSpacingMM());

	this->SetProjection();

	return true;
  }
  else
	return false;
}

void BaseViewController3D::SetPackTexture() {
  pack_front_ = PackTexture(
	tex_buffer(TEX_ENTRY_POSITION), tex_num(TEX_ENTRY_POSITION),
	tex_handler(TEX_ENTRY_POSITION), _tex_num(TEX_ENTRY_POSITION));

  pack_back_ =
	PackTexture(tex_buffer(TEX_EXIT_POSITION), tex_num(TEX_EXIT_POSITION),
				tex_handler(TEX_EXIT_POSITION), _tex_num(TEX_EXIT_POSITION));

  pack_raycasting_ =
	PackTexture(tex_buffer(TEX_RAYCASTING), tex_num(TEX_RAYCASTING),
				tex_handler(TEX_RAYCASTING), _tex_num(TEX_RAYCASTING));

  pack_screen_ = PackTexture(tex_buffer(TEX_SCREEN), tex_num(TEX_SCREEN),
							 tex_handler(TEX_SCREEN), _tex_num(TEX_SCREEN));
}

void BaseViewController3D::SetPackTextureHandle() {
  pack_front_.handler = tex_handler(TEX_ENTRY_POSITION);
  pack_back_.handler = tex_handler(TEX_EXIT_POSITION);
  pack_raycasting_.handler = tex_handler(TEX_RAYCASTING);
  pack_screen_.handler = tex_handler(TEX_SCREEN);
}

void BaseViewController3D::ReadyBufferHandles() {
  BaseViewController::ReadyBufferHandles(DEPTH_END, TEX_END);
}

void BaseViewController3D::SetProjectionFitIn(float world_fit_width,
											  float world_fit_height) {
  if (!Renderer().IsInitialize()) return;

  auto view_param = BaseViewController::view_param();
  if (view_param == nullptr) return;

  PackViewProj arg = BaseViewController::GetPackViewProj();

  float map_scene_to_gl;
  transform().SetProjectionFitIn(arg, world_fit_width, world_fit_height,
								 map_scene_to_gl);

  view_param->set_map_scene_to_gl(map_scene_to_gl);
}

void BaseViewController3D::RayCasting() {
  this->SetRayStepSize();

  Renderer().SetVolumeTexture();
  Renderer().SetTFtexture();

  Renderer().DrawFrontFace(vao_vol_, indices_vol_, pack_front_.tex_buffer, transform().mvp());

  Renderer().DrawBackFace(vao_vol_, indices_vol_, pack_back_.tex_buffer, transform().mvp());

  glDepthFunc(GL_LESS);
  this->DrawBackFaceSurface();

  Renderer().DrawFinalFrontFace(plane_obj(), pack_front_, pack_back_, pack_clipping_);

  CW3GLFunctions::setDepthStencilAttarch(depth_handler(DEPTH_RAYCASTING));

  glDrawBuffer(pack_raycasting_.tex_buffer);
  {
	CW3GLFunctions::clearView(true, GL_BACK);

	this->DrawSurface();

	Renderer().DrawRaycasting(plane_obj(), pack_raycasting_.tex_buffer, transform().mvp(), pack_front_, pack_back_);

	if (Renderer().IsXRAY())
	  Renderer().PostProcessingEnhanced(pack_raycasting_);

	this->DrawOverwriteSurface();
  }

  CW3GLFunctions::setDepthStencilAttarch(depth_handler(DEPTH_DEFAULT));

  Renderer().DrawTextureToTexture(plane_obj(), pack_screen_.tex_buffer, pack_raycasting_);
}

void BaseViewController3D::BlendingGL() {
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
							GL_RENDERBUFFER, depth_handler(DEPTH_RAYCASTING));
  {
	const std::function<void()>& callback =
	  std::bind(&BaseViewController3D::DrawTransparencySurface, this);
	Renderer().DrawTransparencySurface(callback);
  }
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
							GL_RENDERBUFFER, depth_handler(DEPTH_DEFAULT));
}

void BaseViewController3D::ResetView() {
  RendererManager::GetInstance().ResetDeltaWindowWL();
}
void BaseViewController3D::LightView() {
  const QPointF& delta_pos = BaseViewController::GetMouseDeltaWindowLevel();

  RendererManager::GetInstance().AdjustWindowWL(delta_pos.x(), delta_pos.y());
}

/**=================================================================================================
private functions: level 1
*===============================================================================================**/

void BaseViewController3D::SetRayStepSize() {
  auto view_param = BaseViewController::view_param();
  if (view_param == nullptr) return;

  if (view_param->is_render_fast())
	Renderer().SetStepSizeFast();
  else
	Renderer().SetStepSize();
}

glm::vec3 BaseViewController3D::GetMouseVector(const QPointF& pt_scene) {
  auto view_param = BaseViewController::view_param();
  if (view_param == nullptr) return glm::vec3();

  const QSize& scene_size = view_param->scene_size();

  QPointF pt_norm_scene = QPointF(pt_scene.x() / scene_size.width(),
								  pt_scene.y() / scene_size.height()) -
	QPointF(0.5f, 0.5f);
  QPointF pt_gl = GLhelper::ScaleSceneToGL(pt_norm_scene);

  return GetArcBallVector(pt_gl);
}

glm::vec3 BaseViewController3D::GetArcBallVector(const QPointF& pt_gl) {
  vec3 arcball_vector = vec3(pt_gl.x(), 0.0f, -pt_gl.y());
  float sqrt_xz =
	arcball_vector.x * arcball_vector.x + arcball_vector.z * arcball_vector.z;

  if (sqrt_xz < 1.0f) {
	arcball_vector.y = sqrt(1.0f - sqrt_xz);
  }
  else {
	arcball_vector = glm::normalize(arcball_vector);
  }

  return arcball_vector;
}

void BaseViewController3D::ApplyPreferences()
{
	surface_items_->ApplyPreferences();
}
