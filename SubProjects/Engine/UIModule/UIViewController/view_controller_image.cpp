#include "view_controller_image.h"

#include <QSettings>

#include <Engine/Common/Common/global_preferences.h>

#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/gl_helper.h"

#include "../../Module/Will3DEngine/renderer_manager.h"
#include "../../Module/Renderer/image_renderer.h"

#include "../UIGLObjects/view_plane_obj_gl.h"
#include "../UIGLObjects/gl_implant_widget.h"

#include "view_render_param.h"

using namespace UIViewController;
using namespace resource;
using namespace UIGLObjects;

ViewControllerImage::ViewControllerImage() {
	QSettings settings(GlobalPreferences::GetInstance()->ini_path(), QSettings::IniFormat);
	sharpen_level_ = static_cast<SharpenLevel>(settings.value("SLICE/default_filter", 0).toInt());

  transform_.reset(new TransformImage);
}

ViewControllerImage::~ViewControllerImage() {
}

/**======================================================================================
Public Functions
*====================================================================================**/
void ViewControllerImage::SetSharpenLevel(const SharpenLevel & level) {
  tex_info_img_.is_updated = true;
  sharpen_level_ = level;
}
void ViewControllerImage::ClearGL() {
  BaseViewController::ClearGL();

  DeleteTexture(&tex_info_img_);
  DeleteTexture(&mask_nerve_tex_info_);
  DeleteTexture(&mask_implant_tex_info_);

  tex_info_img_.is_updated = true;
  mask_nerve_tex_info_.is_updated = true;
  mask_implant_tex_info_.is_updated = true;
}

bool ViewControllerImage::SetRenderer() {
  if (Renderer().IsInitialize()) {
	auto view_param = BaseViewController::view_param();
	if (view_param == nullptr)
	  return false;

	view_param->set_base_pixel_spacing_mm(Renderer().GetBasePixelSpacingMM());

	SetProjection();
	return true;
  }
  else return false;
}

void ViewControllerImage::ProcessViewEvent(bool *need_render) {
  auto view_param = BaseViewController::view_param();
  if (view_param == nullptr) {
	*need_render = false;
	return;
  }

  switch (view_param->event_type()) {
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
  case UIViewController::UPDATE:
	this->SetProjection();
	*need_render = true;
	break;
  default:
	*need_render = false;
	break;
  }
}

void ViewControllerImage::SetProjection() {
  auto view_param = BaseViewController::view_param();
  if (view_param == nullptr)
	return;



  //TODO. image 준비가 안되어 있을 때 map_scene_to_gl을 Invalid값으로 초기화하고
  // map_scene_to_gl을 사용하는 입장에서는 Invalid값을 체크한 뒤 사용하도록 변경한다.

  //if (IsReady()) {
  //	PackViewProj arg = GetPackViewProj();
  //	float map_scene_to_gl;
  //	transform_->SetProjectionFitIn(arg, (float)tex_info_img_.width,
  //		(float)tex_info_img_.height, map_scene_to_gl);
  //	view_param->set_map_scene_to_gl(map_scene_to_gl);
  //}
  //else {
  //	const float kInvalid = -1.0f;
  //	view_param->set_map_scene_to_gl(kInvalid);
  //}

  PackViewProj arg = GetPackViewProj();
  float map_scene_to_gl;
  transform_->SetProjectionFitIn(arg, GLhelper::ScaleVolToGL((float)tex_info_img_.width),
								 GLhelper::ScaleVolToGL((float)tex_info_img_.height), map_scene_to_gl);
  view_param->set_map_scene_to_gl(map_scene_to_gl);

  // 현재 임시로 boolean flag를 두었음.
  view_param->set_is_valid_map_scene_to_gl(IsReady());
}

bool ViewControllerImage::IsReady() {
  if (img_.lock().get() != nullptr)
	return true;
  else
	return false;
}


void ViewControllerImage::SetImage(const std::weak_ptr<Image2D>& image) {
  Image2D* img = image.lock().get();
  if (!IsValidImage(*(img)))
	return;

  img_ = image;

  int width = img->Width();
  int height = img->Height();
  tex_info_img_.is_diff_size = IsDifferenceTextureSize(tex_info_img_, width, height);

  if (tex_info_img_.is_diff_size) {
	tex_info_img_.width = width;
	tex_info_img_.height = height;

	transform_->Initialize(width, height);
	SetProjection();
  }

  if (tex_info_img_.is_updated) {
	tex_info_img_.is_diff_size = true;
	SetProjection();
  }

  tex_info_img_.is_updated = true;
}

void ViewControllerImage::SetNerveMask(const std::weak_ptr<Image2D>& mask_image) {
  mask_nerve_tex_info_.is_updated = true;

  Image2D* mask = mask_image.lock().get();
  if (!IsValidMask(*(mask)))
	return;

  mask_nerve_ = mask_image;

  int width = mask->Width();
  int height = mask->Height();
  mask_nerve_tex_info_.is_diff_size = IsDifferenceTextureSize(mask_nerve_tex_info_,
															  width, height);

  if (mask_nerve_tex_info_.is_diff_size) {
	mask_nerve_tex_info_.width = width;
	mask_nerve_tex_info_.height = height;
  }

  if (mask_nerve_tex_info_.is_updated)
	mask_nerve_tex_info_.is_diff_size = true;

  mask_nerve_tex_info_.is_updated = true;
}

void ViewControllerImage::SetImplantMask(const std::weak_ptr<resource::Image2D>& mask_image) {
  mask_implant_tex_info_.is_updated = true;

  Image2D* mask = mask_image.lock().get();
  if (!IsValidMask(*(mask)))
	return;

  mask_implant_ = mask_image;

  int width = mask->Width();
  int height = mask->Height();
  mask_implant_tex_info_.is_diff_size = IsDifferenceTextureSize(mask_implant_tex_info_,
																width, height);

  if (mask_implant_tex_info_.is_diff_size) {
	mask_implant_tex_info_.width = width;
	mask_implant_tex_info_.height = height;
  }

  if (mask_implant_tex_info_.is_updated)
	mask_implant_tex_info_.is_diff_size = true;

  mask_implant_tex_info_.is_updated = true;
}

void ViewControllerImage::RenderScreen(uint dfbo) {
  CW3GLFunctions::printError(__LINE__, "ViewControllerImage::RenderingImage Start");

  auto view_param = BaseViewController::view_param();

  if (view_param == nullptr)
	return;

  if (tex_info_img_.is_updated) {
	SetImageTexture();
	tex_info_img_.is_updated = false;

	CW3GLFunctions::printError(__LINE__, "ViewControllerImage::SetImageTexture");
  }
  if (mask_nerve_tex_info_.is_updated) {
	SetMaskTexture(mask_nerve_.lock().get(), &mask_nerve_tex_info_);
	mask_nerve_tex_info_.is_updated = false;

	CW3GLFunctions::printError(__LINE__, "ViewControllerImage::SetMaskTexture");
  }
  if (mask_implant_tex_info_.is_updated) {
	SetMaskTexture(mask_implant_.lock().get(), &mask_implant_tex_info_);
	mask_implant_tex_info_.is_updated = false;

	CW3GLFunctions::printError(__LINE__, "ViewControllerImage::SetMaskTexture");
  }
  const QSize& view_size = view_param->view_size();

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dfbo);
  glViewport(0, 0, view_size.width(), view_size.height());

  if (Renderer().GetInvertWindow())
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  else
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);


  if (tex_info_img_.pack_.handler) {
	Renderer().DrawImage(plane_obj(), transform_->mvp(), tex_info_img_.pack_);
	CW3GLFunctions::printError(__LINE__, "ViewControllerImage::DrawImage");
  }

  if (mask_nerve_tex_info_.pack_.handler) {
	Renderer().BlendImage(plane_obj(), transform_->mvp(), mask_nerve_tex_info_.pack_);
	CW3GLFunctions::printError(__LINE__, "ViewControllerImage::BlendImage");
  }
  if (mask_implant_tex_info_.pack_.handler) {
	Renderer().BlendImage(plane_obj(), transform_->mvp(), mask_implant_tex_info_.pack_);
	CW3GLFunctions::printError(__LINE__, "ViewControllerImage::BlendImage");
  }
}

void ViewControllerImage::ResetView() {
  RendererManager::GetInstance().ResetDeltaWindowWL();
}
void ViewControllerImage::LightView() {
  const QPointF& delta_pos = BaseViewController::GetMouseDeltaWindowLevel();

  RendererManager::GetInstance().AdjustWindowWL(delta_pos.x(), delta_pos.y());
}

void ViewControllerImage::MapSceneToImage(const std::vector<QPointF>& src_scene_points, std::vector<QPointF>& dst_image_points) const {
  dst_image_points.clear();
  dst_image_points.reserve(src_scene_points.size());
  for (const auto& elem : src_scene_points) {
	dst_image_points.push_back(this->MapSceneToImage(elem));
  }
}

QPointF ViewControllerImage::MapSceneToImage(const QPointF& pt_scene) const {
  auto view_param = BaseViewController::view_param();
  if (view_param == nullptr)
	return QPointF();

  QPointF p = pt_scene - view_param->scene_center();
  p = view_param->MapSceneToGL(p) + view_param->gl_trans();

  QPointF pt_img_center = QPointF(transform_->img_width(),
								  transform_->img_height())*0.5f - QPointF(1.f, 1.f);

  return GLhelper::ScaleGLtoVol(p) + pt_img_center;
}

void ViewControllerImage::MapImageToScene(const std::vector<QPointF>& src_image_points, std::vector<QPointF>& dst_scene_points) const {
  dst_scene_points.clear();
  dst_scene_points.reserve(src_image_points.size());

  for (const auto& elem : src_image_points) {
	dst_scene_points.push_back(this->MapImageToScene(elem));
  }
}

QPointF ViewControllerImage::MapImageToScene(const QPointF& pt_image) const {
  //TODO. 이벤트 정리 이후에 풀고 동작을 확인해야 함
  //auto view_param = BaseViewController::view_param();
  //if (view_param == nullptr)
  //	return QPointF(-1.0, -1.0);

  //int img_width = this->GetImageWidth();
  //int img_height = this->GetImageHeight();

  //if (pt_image.x() >= 0 &&
  //	pt_image.x() < img_width &&
  //	pt_image.y() >= 0 &&
  //	pt_image.y() < img_height) {

  //	QPointF pt_img_center = QPointF(transform_->img_width(), transform_->img_height())*0.5f - QPointF(1.0, 1.0);
  //	QPointF p = GLhelper::ScaledVolToGL(pt_image - pt_img_center) - view_param->gl_trans();

  //	p = view_param->MapGLToScene(p);
  //	return p + view_param->scene_center();
  //} else {
  //	return QPointF(-1.0, -1.0);
  //}
  auto view_param = BaseViewController::view_param();
  if (view_param == nullptr)
	return QPointF();

  QPointF pt_img_center = QPointF(transform_->img_width(), transform_->img_height())*0.5f - QPointF(1.0, 1.0);
  QPointF p = GLhelper::ScaleVolToGL(pt_image - pt_img_center) - view_param->gl_trans();

  p = view_param->MapGLToScene(p);
  return p + view_param->scene_center();
}

void ViewControllerImage::GetWindowParams(float* window_width, float* window_level) const {
  Renderer().GetWindowParams(window_width, window_level);
}

float ViewControllerImage::GetIntercept() const {
  return Renderer().GetIntercept();
}
bool ViewControllerImage::IsCursorInImage(const QPointF& pt_scene) const {
  QPointF pt_image = this->MapSceneToImage(pt_scene);
  int img_width = this->GetImageWidth();
  int img_height = this->GetImageHeight();

  if (pt_image.x() >= 0 &&
	  pt_image.x() < img_width &&
	  pt_image.y() >= 0 &&
	  pt_image.y() < img_height)
	return true;
  else
	return false;
}

const bool & ViewControllerImage::GetInvertWindow() const {
  return Renderer().GetInvertWindow();
}

/**=================================================================================================
Protected Functions
*===============================================================================================**/

void ViewControllerImage::SetPackTexture() {
  tex_info_img_.pack_ = PackTexture(tex_buffer(TEX_IMAGE),
									tex_num(TEX_IMAGE),
									tex_handler(TEX_IMAGE),
									_tex_num(TEX_IMAGE));
  mask_nerve_tex_info_.pack_ = PackTexture(tex_buffer(TEX_NERVE_MASK),
										   tex_num(TEX_NERVE_MASK),
										   tex_handler(TEX_NERVE_MASK),
										   _tex_num(TEX_NERVE_MASK));
  mask_implant_tex_info_.pack_ = PackTexture(tex_buffer(TEX_IMPLANT_MASK),
											 tex_num(TEX_IMPLANT_MASK),
											 tex_handler(TEX_IMPLANT_MASK),
											 _tex_num(TEX_IMPLANT_MASK));
}
void ViewControllerImage::SetPackTextureHandle() {
  tex_info_img_.pack_.handler = tex_handler(TEX_IMAGE);
  mask_nerve_tex_info_.pack_.handler = tex_handler(TEX_NERVE_MASK);
  mask_implant_tex_info_.pack_.handler = tex_handler(TEX_IMPLANT_MASK);
}

void ViewControllerImage::ReadyBufferHandles() {
  BaseViewController::ReadyBufferHandles(0, TEX_END);
}

/**=================================================================================================
Private Functions
 *===============================================================================================**/

ImageRenderer& ViewControllerImage::Renderer() const {
  return RendererManager::GetInstance().renderer_image();
}

void ViewControllerImage::SetImageTexture() {

  CW3GLFunctions::printError(__LINE__, "ViewControllerImage::SetImage Start");

  Image2D* img = img_.lock().get();
  if (img == nullptr)
	return;

  if (!IsValidImage(*img)) {
	DeleteTexture(&tex_info_img_);
	return;
  }

  image::ImageFormat img_format = img->Format();

  glActiveTexture(tex_info_img_.pack_.tex_num);

  switch (img_format) {
  case image::ImageFormat::RGBA32:
	CW3GLFunctions::Update2DTexRGBA32UI(tex_info_img_.pack_.handler,
										tex_info_img_.width,
										tex_info_img_.height,
										img->Data(),
										tex_info_img_.is_diff_size);

	break;
  case image::ImageFormat::GRAY8:
	CW3GLFunctions::Update2DTex8(tex_info_img_.pack_.handler,
								 tex_info_img_.width,
								 tex_info_img_.height,
								 img->Data(),
								 tex_info_img_.is_diff_size);
	break;
  case image::ImageFormat::GRAY16UI:
	CW3GLFunctions::Update2DTex16UI(tex_info_img_.pack_.handler,
									tex_info_img_.width,
									tex_info_img_.height,
									reinterpret_cast<ushort*>(img->Data()),
									tex_info_img_.is_diff_size);
	break;
  case image::ImageFormat::GRAY16:
	CW3GLFunctions::Update2DTex16(tex_info_img_.pack_.handler,
								  tex_info_img_.width,
								  tex_info_img_.height,
								  reinterpret_cast<short*>(img->Data()),
								  tex_info_img_.is_diff_size);
	break;
  case image::ImageFormat::UNKNOWN:
	break;
  default:
	break;
  }

  if (sharpen_level_ != SHARPEN_OFF) {
	Renderer().PostProcessingSharpen(tex_info_img_.pack_, sharpen_level_);
  }

  CW3GLFunctions::printError(__LINE__, "ViewControllerImage::SetImage End");
}

void ViewControllerImage::SetMaskTexture(Image2D* mask, TextureInfo* texture_info) {
  CW3GLFunctions::printError(__LINE__, "ViewControllerImage::SetMask Start");

  if (!IsValidMask(*(mask))) {
	DeleteTexture(texture_info);
	return;
  }

  image::ImageFormat mask_format = mask->Format();
  glActiveTexture(texture_info->pack_.tex_num);

  switch (mask_format) {
  case image::ImageFormat::RGBA32:
	CW3GLFunctions::Update2DTexRGBA32UI(texture_info->pack_.handler,
										texture_info->width,
										texture_info->height,
										mask->Data(),
										texture_info->is_diff_size);
	break;
  case image::ImageFormat::GRAY8:
	CW3GLFunctions::Update2DTex8(texture_info->pack_.handler,
								 texture_info->width,
								 texture_info->height,
								 mask->Data(),
								 texture_info->is_diff_size);
	break;
  case image::ImageFormat::GRAY16UI:
	CW3GLFunctions::Update2DTex16UI(texture_info->pack_.handler,
									texture_info->width,
									texture_info->height,
									reinterpret_cast<ushort*>(mask->Data()),
									texture_info->is_diff_size);
	break;
  case image::ImageFormat::GRAY16:
	CW3GLFunctions::Update2DTex16(texture_info->pack_.handler,
								  texture_info->width,
								  texture_info->height,
								  reinterpret_cast<short*>(mask->Data()),
								  texture_info->is_diff_size);
	break;
  case image::ImageFormat::UNKNOWN:
	break;
  default:
	break;
  }
  CW3GLFunctions::printError(__LINE__, "ViewControllerImage::SetMask End");

}
bool ViewControllerImage::IsValidImage(const Image2D& img) const {
  if (&img == nullptr || img.Width() == 0 || img.Height() == 0)
	return false;
  else
	return true;
}
bool ViewControllerImage::IsValidMask(const Image2D& mask) const {
  if (&mask == nullptr || mask.Width() == 0 || mask.Height() == 0)
	return false;
  else
	return true;
}
bool ViewControllerImage::IsDifferenceTextureSize(const TextureInfo& texture_info,
												  int width, int height) const {
  if (texture_info.width != width ||
	  texture_info.height != height)
	return true;
  else
	return false;
}
void ViewControllerImage::DeleteTexture(TextureInfo* texture_info) {
  if (texture_info->pack_.handler) {
	glDeleteTextures(1, &texture_info->pack_.handler);
	texture_info->pack_.handler = 0;
  }
}

void ViewControllerImage::ApplyPreferences()
{
}

void ViewControllerImage::SetFitMode(BaseTransform::FitMode mode)
{
	transform_->set_fit_mode(mode);
}
