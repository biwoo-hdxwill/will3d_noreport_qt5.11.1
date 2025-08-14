#include "base_view_controller.h"

#include <qapplication.h>
#include <GL/glew.h>

#include "../../Common/Common/W3Cursor.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/gl_helper.h"

#include "../UIGLObjects/view_plane_obj_gl.h"
#include "../../Module/Will3DEngine/texture_pack.h"

#include "view_render_param.h"

using namespace UIViewController;

BaseViewController::BaseViewController()
{
  plane_obj_ = new ViewPlaneObjGL();
 }

BaseViewController::~BaseViewController() {
  ClearGL();

  SAFE_DELETE_OBJECT(plane_obj_);
}


void BaseViewController::Initialize() 
{
  initialized_ = this->SetRenderer();

  if (!initialized_)
  {
	return;
  }

  this->ReadyBufferHandles();
  this->SetPackTexture();
}

void BaseViewController::ClearGL() {
  if (fb_handler_) {
	glDeleteFramebuffers(1, &fb_handler_);
	fb_handler_ = 0;
  }
  if (depth_handler_.size() > 0) {
	glDeleteRenderbuffers(depth_handler_.size(), &depth_handler_[0]);
	depth_handler_.assign(depth_handler_.size(), 0);
  }
  if (tex_handler_.size() > 0) {
	glDeleteTextures(tex_handler_.size(), &tex_handler_[0]);
	tex_handler_.assign(tex_handler_.size(), 0);
  }

  plane_obj_->ClearVAOVBO();
}

ViewRenderParam* BaseViewController::view_param() const {
  ViewRenderParam* ptr = view_param_.lock().get();

  if (ptr == nullptr) {
	common::Logger::instance()->Print(common::LogType::ERR, "BaseViewController::view_param:: nullptr.");
  }

  return ptr;
}

uint BaseViewController::fb_handler() const {
  return fb_handler_;
}

uint BaseViewController::depth_handler(int index) const {
  if (depth_handler_.size() >= index && index < 0)
	return -1;
  return depth_handler_[index];
}

uint BaseViewController::tex_buffer(int index) const {
  if (tex_buffer_.size() >= index && index < 0)
	return -1;
  return tex_buffer_[index];
}

uint BaseViewController::tex_handler(int index) const {
  if (tex_handler_.size() >= index && index < 0)
	return -1;
  return tex_handler_[index];
}

uint BaseViewController::tex_num(int index) const {
  if (tex_num_.size() >= index && index < 0)
	return -1;
  return tex_num_[index];
}

uint BaseViewController::_tex_num(int index) const {
  if (_tex_num_.size() >= index && index < 0)
	return -1;
  return _tex_num_[index];
}

void BaseViewController::ReadyBufferHandles(int depthCount, int texCount) {
  depth_handler_.resize(depthCount, 0);
  tex_handler_.resize(texCount, 0);

  for (int i = 0; i < texCount; ++i) {
	tex_buffer_.push_back(GL_COLOR_ATTACHMENT0 + i);
	tex_num_.push_back((unsigned int)TexturePack::GL_TEXTURE_ID::GL_TEXTURE_END + i);
	_tex_num_.push_back((unsigned int)TexturePack::GL_TEXTURE_ID::GL_TEXTURE_END_ + i);
  }
}

void BaseViewController::SetBuffers() {
  auto view_param = this->view_param();
  if (view_param == nullptr)
	return;

  const QSize& render_view_size = view_param->GetRenderViewSize();

  if (fb_handler_ != 0 && fb_size_ == render_view_size)
	return;

  CW3GLFunctions::initFrameBufferMultiTexture(fb_handler_, depth_handler_,
											  tex_handler_, render_view_size.width(), render_view_size.height(), tex_num_);

  SetPackTextureHandle();
  fb_size_ = render_view_size;
}

void BaseViewController::ReadyFrameBuffer() {
  auto view_param = this->view_param();
  if (view_param == nullptr)
	return;

  SetBuffers();

  glBindFramebuffer(GL_FRAMEBUFFER, fb_handler_);
  glViewport(0, 0, fb_size_.width(), fb_size_.height());

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
	common::Logger::instance()->PrintAndAssert(common::LogType::ERR, " BaseViewController::ReadyFrameBuffer: invalid fbo.");
  }
}

PackViewProj BaseViewController::GetPackViewProj() const {
  PackViewProj arg;

  auto view_param = this->view_param();
  if (view_param == nullptr)
	return arg;

  arg.width = view_param->view_size_width();
  arg.height = view_param->view_size_height();
  arg.scale = view_param->scene_scale();
  arg.trans_x = view_param->gl_trans().x();
  arg.trans_y = view_param->gl_trans().y();

  return arg;
}

void BaseViewController::ScaleView() {
  auto view_param = this->view_param();
  if (view_param == nullptr)
	return;

  float scale = GetMouseScale();
  const float kMinimumScale = 0.3f;
  scale = std::max(kMinimumScale, scale);

  view_param->set_scene_scale(scale);
}

void BaseViewController::ScaleWheelView() {
  auto view_param = this->view_param();
  if (view_param == nullptr)
	return;

  float scale = GetWheelScale();

  const float kMinimumScale = 0.3f;
  scale = std::max(kMinimumScale, scale);

  view_param->set_scene_scale(scale);
}

void BaseViewController::PanView() {
  auto view_param = this->view_param();
  if (view_param == nullptr)
	return;

  QPointF trans = GetMouseTranslate();

  const QPointF& gl_trans = view_param->gl_trans();
  view_param->set_gl_trans(gl_trans + view_param->MapSceneToGL(trans));
}

void BaseViewController::FitView() {
  auto view_param = this->view_param();
  if (view_param == nullptr)
	return;

  view_param->FitView();
}

float BaseViewController::GetMouseScale() const {
  auto view_param = this->view_param();

  const QPointF& pos_curr = view_param->scene_mouse_curr();
  const QPointF& pos_prev = view_param->scene_mouse_prev();

  float pre_scale = view_param->scene_scale();

  float trans_normalized_y = float(pos_prev.y() - pos_curr.y()) / ((float)view_param->scene_size().height());
  trans_normalized_y *= pre_scale;

  float trans_gl_y = GLhelper::ScaleSceneToGL(trans_normalized_y);
  float scale = pre_scale + trans_gl_y;

  if (std::abs(scale - pre_scale) > std::numeric_limits<float>::epsilon()) {
	if (scale - pre_scale > 0.0f) {
	  QApplication::setOverrideCursor(CW3Cursor::ZoomInCursor());
	}
	else {
	  QApplication::setOverrideCursor(CW3Cursor::ZoomOutCursor());
	}
  }

  return scale;
}

float BaseViewController::GetWheelScale() const {
  auto view_param = this->view_param();

  float pre_scale = view_param->scene_scale();

  float kSensitivity = 10.0f;
  float scale = pre_scale + (view_param->mouse_wheel_delta() / kSensitivity);

  //if (scale - pre_scale > 0.0f) {
  //	QApplication::setOverrideCursor(CW3Cursor::ZoomInCursor());
  //} else {
  //	QApplication::setOverrideCursor(CW3Cursor::ZoomOutCursor());
  //}

  return scale;
}

QPointF BaseViewController::GetMouseTranslate() const {

  auto view_param = this->view_param();
  if (view_param == nullptr)
	return QPointF();

  const QPointF& pos_curr = view_param->scene_mouse_curr();
  const QPointF& pos_prev = view_param->scene_mouse_prev();

  if (pos_curr != pos_prev)
	QApplication::setOverrideCursor(CW3Cursor::ClosedHandCursor());

  return pos_prev - pos_curr;
}

QPointF BaseViewController::GetMouseDeltaWindowLevel() const {

  auto view_param = this->view_param();
  if (view_param == nullptr)
	return QPointF();

  const QPointF& pos_curr = view_param->scene_mouse_curr();
  const QPointF& pos_prev = view_param->scene_mouse_prev();

  float kSensitivity = 8.0f;
  QPointF delta = pos_curr - pos_prev;
  delta.setY(-delta.y());

  return delta * kSensitivity;

}
