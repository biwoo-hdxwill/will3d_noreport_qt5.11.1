#include "view_controller_slice.h"

#if defined(__APPLE__)
#include </usr/local/Cellar/llvm/5.0.0/lib/clang/5.0.0/include/omp.h>
#else
#include <omp.h>
#endif

#include <QDebug>

#include <Engine/Common/Common/global_preferences.h>
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3Math.h"
#include "../../Common/Common/color_will3d.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/gl_helper.h"

#include "../../Core/ImageProcessing/W3ImageProcessing.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/implant_resource.h"

#include "../../UIGLObjects/gl_implant_widget.h"
#include "../../UIGLObjects/gl_nerve_widget.h"
#include "../../UIGLObjects/view_plane_obj_gl.h"

#include "../../Module/Renderer/slice_renderer.h"
#include "../../Module/Will3DEngine/renderer_manager.h"

#include "surface_items.h"
#include "view_render_param.h"

using namespace UIViewController;
using namespace Will3DEngine;
using namespace UIGLObjects;
namespace
{
	const glm::vec3 kInvAxisX(-1.0f, 1.0f, 1.0f);
}

using glm::mat4;
using glm::vec3;

ViewControllerSlice::ViewControllerSlice()
{
	QSettings settings(GlobalPreferences::GetInstance()->ini_path(), QSettings::IniFormat);
	sharpen_level_ = static_cast<SharpenLevel>(settings.value("SLICE/default_filter", 0).toInt());

	transform_.reset(new TransformSlice());

	slice_obj_.reset(new ViewPlaneObjGL());
	slice_obj_->set_vertex_scale(glm::vec3(sqrt(2.0f)));

	nerve_.reset(new GLNerveWidget(ObjCoordSysType::TYPE_VOLUME));
	implant_.reset(new GLImplantWidget(ObjCoordSysType::TYPE_VOLUME));
	//implant_->SetAlpha(0.3f);

	nerve_->set_is_visible(false);
	implant_->set_is_visible(false);

	ApplyPreferences();
}

ViewControllerSlice::~ViewControllerSlice()
{

}

/**=================================================================================================
public functions
*===============================================================================================**/
void ViewControllerSlice::SetPlane(const glm::vec3& center_pos, const glm::vec3& right_vector, const glm::vec3& back_vector, int thickness_in_vol)
{
	const float z_spacing = Renderer().GetSpacingZ();

	glm::vec3 center_position_in_vol_gl = kInvAxisX * GLhelper::MapVolToWorldGL(center_pos, Renderer().GetVolCenter(), z_spacing);
	glm::vec3 rv = kInvAxisX * right_vector;
	glm::vec3 bv = kInvAxisX * back_vector;

	glm::vec3 model_scale = Renderer().GetModelScale();
	float major_scale = std::max(model_scale.x, model_scale.y);
	major_scale = std::max(major_scale, model_scale.z);

	float right_scale = glm::length(major_scale * glm::normalize(rv));
	float back_scale = glm::length(major_scale * glm::normalize(bv));

#if 1
	slice_obj_->set_vertex_scale(glm::vec3(glm::length(rv / right_scale), glm::length(bv / back_scale), 1.f));
#endif

	is_update_slice_obj_ = true;
	slice_thickness_ = thickness_in_vol;

	transform_->SetPlane(center_position_in_vol_gl, rv, bv);

	this->SetProjection();

	is_set_plane_ = true;
}

void ViewControllerSlice::SetSharpenLevel(const SharpenLevel& level)
{
	sharpen_level_ = level;
}

void ViewControllerSlice::ClearGL()
{
	BaseViewController::ClearGL();

	nerve_->ClearVAOVBO();
	implant_->ClearVAOVBO();
	slice_obj_->ClearVAOVBO();
	DeleteOffScreenBuff();
}

void ViewControllerSlice::ProcessViewEvent(bool* need_render)
{
	auto view_param = BaseViewController::view_param();
	if (view_param == nullptr)
	{
		*need_render = false;
		return;
	}

	switch (view_param->event_type())
	{
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
		assert(false);
		*need_render = false;
		break;
	}
}
void ViewControllerSlice::SetProjection()
{
	if (!Renderer().IsInitialize()) return;

	auto view_param = BaseViewController::view_param();
	if (view_param == nullptr) return;

	float map_scene_to_gl = 1.0f;
	transform_->SetProjection(GetPackViewProj(), map_scene_to_gl);
	view_param->set_map_scene_to_gl(map_scene_to_gl);
}

bool ViewControllerSlice::IsReady()
{
	return (Renderer().IsInitialize() && initialized() && is_set_plane_);
}

void ViewControllerSlice::SetVisibleNerve(bool is_visible)
{
	nerve_->set_is_visible(is_visible);
}

void ViewControllerSlice::SetVisibleImplant(bool is_visible)
{
	implant_->set_is_visible(is_visible);
}

void ViewControllerSlice::RenderingSlice()
{
	if (!initialized())
	{
		Initialize();
		SetProjection();
	}

	if (!Renderer().IsInitialize())
	{
		ClearGL();
		return;
	}

#if DEVELOP_MODE
	clock_t t_start, t_end;
	t_start = clock();
	common::Logger::instance()->PrintDebugMode("ViewControllerSlice::RenderingSlice");
#endif

	CW3GLFunctions::printError(__LINE__, "CW3ViewControllerSlice::RenderSlice Start");

	BaseViewController::ReadyFrameBuffer();

	this->InitVAOs();
	CW3GLFunctions::printError(__LINE__, "CW3ViewControllerSlice::InitVAOs");

	this->SetSurfaceMVP();
	CW3GLFunctions::printError(__LINE__, "CW3ViewControllerSlice::SetSurfaceMVP");

	this->RenderSlice();
	CW3GLFunctions::printError(__LINE__, "CW3ViewControllerSlice::RenderSlice");

	this->BlendingGL();
	CW3GLFunctions::printError(__LINE__, "CW3ViewControllerSlice::BlendingGL");

	this->DrawSurface();
	CW3GLFunctions::printError(__LINE__, "CW3ViewControllerSlice::DrawSurface");

#if DEVELOP_MODE
	t_end = clock();
	float elapsedTime = static_cast<float>(t_end - t_start) / CLOCKS_PER_SEC;
	common::Logger::instance()->PrintDebugMode("", QString("%1(ms)").arg(elapsedTime).toStdString());
#endif
}

void ViewControllerSlice::RenderScreen(uint dfbo)
{
	auto view_param = BaseViewController::view_param();
	if (view_param == nullptr) return;

	const QSize& view_size = view_param->view_size();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dfbo);
	glViewport(0, 0, view_size.width(), view_size.height());

	if (Renderer().GetInvertWindow())
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	else
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	if (is_set_plane_) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

		Renderer().DrawScreen(plane_obj(), pack_screen_);
	}

	CW3GLFunctions::printError(__LINE__, "CW3ViewControllerSlice::RenderScreen");
}

void ViewControllerSlice::ForceRotateMatrix(const mat4& mat)
{
	transform_->SetRotate(mat);
}

void ViewControllerSlice::MapSceneToVol(const std::vector<QPointF>& src_scene_points, std::vector<vec3>& dst_vol_points)
{
	dst_vol_points.clear();
	dst_vol_points.resize(src_scene_points.size());

	const int num_thread = omp_get_max_threads();
	omp_set_num_threads(num_thread);

#pragma omp parallel for
	for (int i = 0; i < src_scene_points.size(); ++i)
	{
		dst_vol_points[i] = this->MapSceneToVol(src_scene_points[i]);
	}
}
void ViewControllerSlice::MapVolToScene(const std::vector<glm::vec3>& src_vol_points, std::vector<QPointF>& dst_scene_points)
{
	dst_scene_points.clear();
	dst_scene_points.resize(src_vol_points.size());

	const int num_thread = omp_get_max_threads();
	omp_set_num_threads(num_thread);

	ViewRenderParam* view_param = BaseViewController::view_param();

	if (view_param == nullptr) return;

	const QRectF proj_rect = transform_->projection_rect();
	QPointF proj_center = proj_rect.center();
	proj_center.setY(-proj_center.y());
	const glm::mat4 inv_rotate = glm::inverse(transform_->rotate());
	const glm::vec3 center_pos(inv_rotate * vec4(transform_->plane_center_position(), 1.0f));
	const glm::vec3 right_vector(inv_rotate * vec4(transform_->plane_right_vector(), 1.0f));
	const glm::vec3 back_vector(inv_rotate * vec4(transform_->plane_back_vector(), 1.0f));
	const double plane_center_x = view_param->scene_center().x();
	const double plane_center_y = view_param->scene_center().y();
	const glm::vec3 vol_center = Renderer().GetVolCenter();
	const float spacing_z = Renderer().GetSpacingZ();
	const float gl_trans_x = view_param->gl_trans().x();
	const float gl_trans_y = view_param->gl_trans().y();

#pragma omp parallel for
	for (int i = 0; i < src_vol_points.size(); ++i)
	{
		const glm::vec3 disp(kInvAxisX * GLhelper::MapVolToWorldGL(src_vol_points[i], vol_center, spacing_z) - center_pos);
		const double disp_x = static_cast<double>(view_param->MapGLToScene(glm::dot(right_vector, disp) - gl_trans_x));
		const double disp_y = static_cast<double>(view_param->MapGLToScene(glm::dot(back_vector, disp) - gl_trans_y));
		dst_scene_points[i] = QPointF(plane_center_x + disp_x, plane_center_y + disp_y);
	}
}

inline glm::vec3 ViewControllerSlice::MapSceneToVol(const QPointF& pt_scene)
{
	ViewRenderParam* view_param = BaseViewController::view_param();

	if (view_param == nullptr) return glm::vec3();

	QPointF p = pt_scene - view_param->scene_center();
	p = view_param->MapSceneToGL(p) + view_param->gl_trans();
	glm::mat4 inv_rotate = glm::inverse(transform_->rotate());
	const glm::vec3 center_pos(inv_rotate * vec4(transform_->plane_center_position(), 1.0f));
	const glm::vec3 right_vector(inv_rotate * vec4(transform_->plane_right_vector(), 1.0f));
	const glm::vec3 back_vector(inv_rotate * vec4(transform_->plane_back_vector(), 1.0f));

	glm::vec3 pt_gl = kInvAxisX * ((float)p.x() * right_vector + (float)p.y() * back_vector + center_pos);
	return GLhelper::MapWorldGLtoVol(pt_gl, Renderer().GetVolCenter(), Renderer().GetSpacingZ());
}

inline QPointF ViewControllerSlice::MapVolToScene(const glm::vec3& pt_vol)
{
	ViewRenderParam* view_param = BaseViewController::view_param();

	if (view_param == nullptr)
	{
		return QPointF();
	}

	QRectF proj_rect = transform_->projection_rect();
	QPointF proj_center = proj_rect.center();
	proj_center.setY(-proj_center.y());

	glm::mat4 inv_rotate = glm::inverse(transform_->rotate());
	const glm::vec3 center_pos(inv_rotate * vec4(transform_->plane_center_position(), 1.0f));
	const glm::vec3 right_vector(inv_rotate * vec4(transform_->plane_right_vector(), 1.0f));
	const glm::vec3 back_vector(inv_rotate * vec4(transform_->plane_back_vector(), 1.0f));

	const glm::vec3 vol_center = Renderer().GetVolCenter();
	float spacing_z = Renderer().GetSpacingZ();
	const glm::vec3 disp = kInvAxisX * GLhelper::MapVolToWorldGL(pt_vol, vol_center, spacing_z) - center_pos;

	const QPointF gl_trans = view_param->gl_trans();
	double disp_x = (double)view_param->MapGLToScene(glm::dot(right_vector, disp) - gl_trans.x());
	double disp_y = (double)view_param->MapGLToScene(glm::dot(back_vector, disp) - gl_trans.y());
	QPointF plane_center = view_param->scene_center();

	return QPointF(plane_center.x() + disp_x, plane_center.y() + disp_y);
}

inline QPointF ViewControllerSlice::MapPlaneToScene(const QPointF& pt_plane)
{
	ViewRenderParam* view_param = BaseViewController::view_param();

	if (view_param == nullptr) return QPointF();

	QPointF pt_center_in_plane = GLhelper::ScaleVolToGL(QPointF(transform_->plane_width(), transform_->plane_height())) * 0.5f;

	QPointF p = GLhelper::ScaleVolToGL(pt_plane) - view_param->gl_trans() - pt_center_in_plane;

	p = view_param->MapGLToScene(p);
	return p + view_param->scene_center();
}

inline QPointF ViewControllerSlice::MapSceneToPlane(const QPointF& pt_scene)
{
	ViewRenderParam* view_param = BaseViewController::view_param();

	if (view_param == nullptr) return QPointF();

	QPointF p = pt_scene - view_param->scene_center();
	QPointF pt_center_in_plane = GLhelper::ScaleVolToGL(QPointF(transform_->plane_width(), transform_->plane_height())) * 0.5f;

	p = view_param->MapSceneToGL(p) + view_param->gl_trans() + pt_center_in_plane;

	return GLhelper::ScaleGLtoVol(p);
}

void ViewControllerSlice::GetSliceRange(float& min, float& max) const
{
	Renderer().GetSliceRange(transform_->GetUpVector(), min, max);
}

float ViewControllerSlice::GetLocationNose() const
{
	return Renderer().GetLocationNose();
}

float ViewControllerSlice::GetLocationChin() const
{
	return Renderer().GetLocationChin();
}

glm::vec3 ViewControllerSlice::GetUpVector() const
{
	return transform_->GetUpVector();
}

glm::vec3 ViewControllerSlice::GetRightVector() const
{
	return transform_->GetRightVector();
}

glm::vec3 ViewControllerSlice::GetBackVector() const
{
	return transform_->GetBackVector();
}

void ViewControllerSlice::GetWindowParams(float* window_width, float* window_level)
{
	return Renderer().GetWindowParams(window_width, window_level);
}

float ViewControllerSlice::GetIntercept() const
{
	return Renderer().GetIntercept();
}

glm::mat4 ViewControllerSlice::GetCameraMatrix() const
{
	return transform_->GetCameraMatrix();
}

glm::mat4 ViewControllerSlice::GetRotateMatrix() const
{
	return transform_->rotate() * transform_->reorien();
}

const glm::mat4& ViewControllerSlice::GetViewMatrix() const
{
	return transform_->view();
}

const bool& ViewControllerSlice::GetInvertWindow() const
{
	return Renderer().GetInvertWindow();
}

glm::vec4 ViewControllerSlice::GetDicomInfoPoint(const QPointF& pt_scene)
{
	if (!IsReady()) return glm::vec4(-1.0f);

	glm::vec3 vol_pos = MapSceneToVol(pt_scene);
	const CW3Image3D& img = ResourceContainer::GetInstance()->GetMainVolume();
	return img.GetVolumeInfo(vol_pos);
}

void ViewControllerSlice::GetDicomHULine(const QPointF& start_pt_scene, const QPointF& end_pt_scene, std::vector<short>& data)
{
	if (!IsReady()) return;

	const QPointF pt_start_plane = MapSceneToPlane(start_pt_scene);
	const QPointF pt_end_plane = MapSceneToPlane(end_pt_scene);
	std::vector<QPointF> plane_points;
	W3::GenerateSequencialPlanePointsInLine(pt_start_plane, pt_end_plane, plane_points);

	std::vector<glm::vec3> vol_points;
	vol_points.reserve(plane_points.size());
	for (const auto& plane_pt : plane_points)
	{
		const QPointF scene_pt = MapPlaneToScene(plane_pt);
		vol_points.push_back(MapSceneToVol(scene_pt));
	}

	const CW3Image3D& img = ResourceContainer::GetInstance()->GetMainVolume();
	img.GetVolumeHU(vol_points, data);
}

void ViewControllerSlice::GetDicomHURect(const QPointF& start_pt_scene, const QPointF& end_pt_scene, std::vector<short>& data)
{
	if (!IsReady()) return;

	QPointF pt_start_plane = MapSceneToPlane(start_pt_scene);
	QPointF pt_end_plane = MapSceneToPlane(end_pt_scene);
	std::vector<QPointF> plane_points;
	W3::GenerateSequencialPlanePointsInRect(pt_start_plane, pt_end_plane, plane_points);

	std::vector<glm::vec3> vol_points;
	vol_points.reserve(plane_points.size());
	for (const auto& plane_pt : plane_points)
	{
		const QPointF scene_pt = MapPlaneToScene(plane_pt);
		vol_points.push_back(MapSceneToVol(scene_pt));
	}

	const CW3Image3D& img = ResourceContainer::GetInstance()->GetMainVolume();
	img.GetVolumeHU(vol_points, data);
}

void ViewControllerSlice::RenderAndPickImplant(int* implant_id)
{
	const auto& implant_resource = ResourceContainer::GetInstance()->GetImplantResource();
	if (implant_resource.data().empty() || !implant_resource.is_visible_all())
		return;

	auto view_param = BaseViewController::view_param();
	if (view_param == nullptr) return;

	const auto& prog = Renderer().programs();

	BaseViewController::ReadyFrameBuffer();

	glDrawBuffer(pack_pick_implant_.tex_buffer);
	CW3GLFunctions::clearView(true, GL_BACK);

	implant_->set_projection(transform_->projection());
	implant_->set_view(transform_->view());
	implant_->set_world(transform_->model());
	implant_->SetTransformMat(transform_->rotate(), ARCBALL);
	implant_->SetTransformMat(transform_->reorien(), REORIENTATION);

#if 1
	glUseProgram(prog.pick_object);
	QPointF pt_view = view_param->view_mouse_curr();
	glm::vec2 pt_pick_in_gl(pt_view.x(), pt_view.y());

	glReadBuffer(pack_pick_implant_.tex_buffer);
#if 0
	* implant_id = implant_->PickSlice(prog.pick_object, glm::vec4(transform_->GetUpVector(), -transform_->gl_z_position()), pt_pick_in_gl);
#else
	glm::vec4 plane_equ = transform_->plane_equation();
	float z_position = -transform_->gl_z_position();

	*implant_id = implant_->PickSlice(prog.pick_object, glm::vec4((glm::vec3)plane_equ, plane_equ.w + z_position), pt_pick_in_gl);
#endif
#else
	glUseProgram(prog.render_slice_implant);

	QPointF pt_view = view_param->view_mouse_curr();
	glm::vec2 pt_pick_in_gl(pt_view.x(), pt_view.y());

	WGLSLprogram::setUniform(prog.render_slice_implant, "pick_mode", true);

	glReadBuffer(pack_pick_implant_.tex_buffer);
#if 0
	implant_->RenderSlice(
		prog.render_slice_implant,
		glm::vec4(transform_->GetUpVector(), -transform_->gl_z_position()));
#else
	glm::vec4 plane_equ = transform_->plane_equation();
	float z_position = -transform_->gl_z_position();

	implant_->RenderSlice(
		prog.render_slice_implant,
		glm::vec4((glm::vec3)plane_equ, plane_equ.w + z_position)
	);
#endif

	WGLSLprogram::setUniform(prog.render_slice_implant, "pick_mode", false);

	glUseProgram(0);

	//CW3GLFunctions::SaveTexture2D("c:/users/jdk/desktop/pack_pick_implant_.png", pack_pick_implant_.handler, GL_RGBA, GL_UNSIGNED_BYTE);
#endif

#if DEVELOP_MODE
	common::Logger::instance()->PrintDebugMode("ViewControllerSlice::RenderAndPickImplant");
#endif
}

const std::map<int, bool>& ViewControllerSlice::GetImplantVisibility()
{
	return implant_->visibility();
}

void ViewControllerSlice::SetFitMode(BaseTransform::FitMode mode)
{
	transform_->set_fit_mode(mode);
}

/**=================================================================================================
private functions: level 0
*===============================================================================================**/
bool ViewControllerSlice::SetRenderer()
{
	if (Renderer().IsInitialize())
	{
		auto view_param = BaseViewController::view_param();
		if (view_param == nullptr) return false;

		transform_->Initialize(Renderer().GetModelScale());
		view_param->set_base_pixel_spacing_mm(Renderer().GetBasePixelSpacingMM());
		this->SetProjection();
		is_set_plane_ = true;
		return true;
	}
	else
		return false;
}

void ViewControllerSlice::SetPackTexture()
{
	pack_screen_ = PackTexture(tex_buffer(TEX_SCREEN), tex_num(TEX_SCREEN), tex_handler(TEX_SCREEN), _tex_num(TEX_SCREEN));
	pack_pick_implant_ = PackTexture(tex_buffer(TEX_PICK_IMPLANT), tex_num(TEX_PICK_IMPLANT), tex_handler(TEX_PICK_IMPLANT), _tex_num(TEX_PICK_IMPLANT));
}

void ViewControllerSlice::SetPackTextureHandle()
{
	pack_screen_.handler = tex_handler(TEX_SCREEN);
	pack_pick_implant_.handler = tex_handler(TEX_PICK_IMPLANT);
}

void ViewControllerSlice::ReadyBufferHandles()
{
	BaseViewController::ReadyBufferHandles(DEPTH_END, TEX_END);
}

void ViewControllerSlice::SetSurfaceMVP()
{
	//if (nerve_->is_visible()) {
	//	nerve_->set_projection(transform_->projection());
	//	nerve_->set_view(transform_->view());
	//	nerve_->SetTransformMat(transform_->rotate(), ARCBALL);
	//	nerve_->SetTransformMat(transform_->reorien(), REORIENTATION);
	//}

	/*if (implant_->is_visible()) {
	  implant_->set_projection(transform_->projection_for_implant());
	  implant_->set_view(transform_->view_for_implant());
	  implant_->SetTransformMat(transform_->rotate(), ARCBALL);
	  implant_->SetTransformMat(transform_->reorien(), REORIENTATION);
	}*/
}

void ViewControllerSlice::RenderSlice()
{
	if (!initialized())
	{
		Initialize();
	}

	if (!Renderer().IsInitialize())
	{
		ClearGL();
		return;
	}

	Renderer().SetVolumeTexture();

	if (is_update_slice_obj_)
	{
		slice_obj_->ClearVAOVBO();
		is_update_slice_obj_ = false;
	}

	Renderer().DrawSlice(slice_obj_.get(), tex_buffer(TEX_SCREEN),
		transform_->mvp(), transform_->vol_tex_transform_mat(),
		transform_->GetUpVector(), slice_thickness_);

	if (sharpen_level_ != SHARPEN_OFF)
	{
		Renderer().PostProcessingSharpen(pack_screen_, sharpen_level_);
	}
}

void ViewControllerSlice::DrawSurface()
{
	if (!Renderer().IsInitialize())
	{
		ClearGL();
		return;
	}

	const auto& prog = Renderer().programs();
	if (nerve_->is_visible())
	{
		nerve_->set_projection(transform_->projection());
		nerve_->set_view(transform_->view());
		nerve_->SetTransformMat(transform_->rotate(), ARCBALL);
		nerve_->SetTransformMat(transform_->reorien(), REORIENTATION);

		glEnable(GL_DEPTH_TEST);
		glClearDepth(1.0f);
		glClear(GL_DEPTH_BUFFER_BIT);
		glDepthFunc(GL_LESS);
		glUseProgram(prog.render_slice_nerve);
		if (slice_thickness_ > 1)
		{
			float save_gl_z = transform_->gl_z_position();
			glm::mat4 upper_mvp, lower_mvp;
			transform_->TranslateZ((float)slice_thickness_);
			upper_mvp = transform_->mvp();
			transform_->TranslateZ(-(float)slice_thickness_);
			lower_mvp = transform_->mvp();
			transform_->TranslateZ(save_gl_z);

			vec4 plane_equ = transform_->plane_equation();
			vec3 plane_up_vector = vec3(transform_->plane_equation());

			vec4 slice_equ_front(plane_up_vector, plane_equ.w - slice_thickness_);
			vec4 slice_equ_back(plane_up_vector, plane_equ.w + slice_thickness_);
			glUseProgram(prog.render_slice_nerve);

			nerve_->RenderThicknessSlice(prog.render_slice_nerve, slice_obj(), upper_mvp, lower_mvp, slice_equ_front, slice_equ_back);
		}
		else
		{
			nerve_->RenderSlice(prog.render_slice_nerve, slice_obj(), transform_->mvp());
		}
	}

	glClear(GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LESS);
	if (implant_->is_visible())
	{
		if (!is_implant_wire_)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glEnable(GL_DEPTH_TEST);
			glClearDepth(1.0f);
			glClear(GL_DEPTH_BUFFER_BIT);
			glDepthFunc(GL_LESS);
			glUseProgram(prog.render_slice_implant);

			glm::vec4 plane_equ = transform_->plane_equation();
			float z_position = -transform_->gl_z_position();

			implant_->set_projection(transform_->projection());
			implant_->set_view(transform_->view());
			implant_->SetTransformMat(transform_->rotate(), ARCBALL);
			implant_->SetTransformMat(transform_->reorien(), REORIENTATION);

			implant_->RenderSlice(prog.render_slice_implant, glm::vec4((glm::vec3)plane_equ, plane_equ.w + z_position));

			glUseProgram(0);

			glDisable(GL_BLEND);
		}
		else
		{
			glClearDepth(1.0f);
			glClear(GL_DEPTH_BUFFER_BIT);

			glDisable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);

			glUseProgram(prog.render_slice_implant_wire);

			implant_->set_projection(transform_->projection_for_implant());
			implant_->set_view(transform_->view_for_implant());
			implant_->SetTransformMat(transform_->rotate(), ARCBALL);
			implant_->SetTransformMat(transform_->reorien(), REORIENTATION);

			glm::vec4 plane_equ = transform_->plane_equation();
			float z_position = -transform_->gl_z_position();

			implant_->RenderSliceWire(prog.render_slice_implant_wire, glm::vec4((glm::vec3)plane_equ, plane_equ.w + z_position));

			glUseProgram(0);
		}
	}
}

void ViewControllerSlice::ResetView()
{
	RendererManager::GetInstance().ResetDeltaWindowWL();
}

void ViewControllerSlice::LightView()
{
	const QPointF& delta_pos = BaseViewController::GetMouseDeltaWindowLevel();
	RendererManager::GetInstance().AdjustWindowWL(delta_pos.x(), delta_pos.y());
}

SliceRenderer& ViewControllerSlice::Renderer() const
{
	return RendererManager::GetInstance().renderer_slice(VOL_MAIN);
}

/**=================================================================================================
private functions: level 1
*===============================================================================================**/
ImplantData* ViewControllerSlice::GetImplantData()
{
	const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
	int selected_id = res_implant.selected_implant_id();
	if (selected_id < 0) return nullptr;

	const auto& implant_datas = res_implant.data();
	if (implant_datas.find(selected_id) == implant_datas.end()) return nullptr;

	return implant_datas.at(selected_id).get();
}

void ViewControllerSlice::ApplyPreferences()
{
	implant_->ApplyPreferences();
	is_implant_wire_ = (GlobalPreferences::GetInstance()->preferences_.objects.implant.rendering_type == GlobalPreferences::MeshRenderingType::Wire);
}

void ViewControllerSlice::DeleteOffScreenBuff()
{
	if (off_texture_pack_.handler)
	{
		glDeleteTextures(1, &off_texture_pack_.handler);
		off_texture_pack_.handler = 0;
	}

	if (off_rb_)
	{
		glDeleteRenderbuffers(1, &off_rb_);
		off_rb_ = 0;
	}

	if (off_fb_)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &off_fb_);
		off_fb_ = 0;
	}
}

const std::vector<glm::vec3>& ViewControllerSlice::GetVolVertex() const
{
	return Renderer().GetVolVertex();
}

bool ViewControllerSlice::IsNerveVisible()
{
	return nerve_->is_visible();
}

bool ViewControllerSlice::IsImplantVisible()
{
	return implant_->is_visible();
}

bool ViewControllerSlice::IsDrawSurface()
{
	if (nerve_->is_visible() || implant_->is_visible())
	{
		return true;
	}

	return false;
}

#ifndef WILL3D_VIEWER
void ViewControllerSlice::InitOffScreenBuff(const int width, const int height)
{
	DeleteOffScreenBuff();

	off_texture_pack_.tex_buffer = GL_COLOR_ATTACHMENT1;
	off_texture_pack_.tex_num = GL_TEXTURE1;

	glGenFramebuffers(1, &off_fb_);
	glBindFramebuffer(GL_FRAMEBUFFER, off_fb_);

	glGenTextures(1, &off_texture_pack_.handler);

	glActiveTexture(off_texture_pack_.tex_num);
	glBindTexture(GL_TEXTURE_2D, off_texture_pack_.handler);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenRenderbuffers(1, &off_rb_);
	glBindRenderbuffer(GL_RENDERBUFFER, off_rb_);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glFramebufferTexture2D(GL_FRAMEBUFFER, off_texture_pack_.tex_buffer, GL_TEXTURE_2D, off_texture_pack_.handler, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, off_rb_);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool ViewControllerSlice::GetTextureData(unsigned char*& out_data, const int width, const int height, const int filter, const int thickness)
{
	if (!out_data || !OffScreenDrawSlice(width, height, filter, thickness, true))
	{
		return false;
	}	

	const int size = width * height;
	const int w_4 = width * 4;
	const int w_3 = width * 3;

	float* tex_buffer = new float[size * 4]; //RGBA 

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, tex_buffer);

	float ww, wl;
	Renderer().GetWindowParams(&ww, &wl);

	float window_min = wl - (ww * 0.5f);
	float window_max = wl + (ww * 0.5f);

#pragma omp parallel for
	for (int i = 0; i < height; ++i)
	{
		unsigned char* img_buf = out_data + i * w_3;
		float* tex_buf = tex_buffer + (height - i - 1) * w_4;

		for (int j = 0; j < width; ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				float value = *tex_buf * 65535.f;
				if (value < window_min)
				{
					value = window_min;
				}
				else if (value > window_max)
				{
					value = window_max;
				}

				value = (value - window_min) / ww;
				*img_buf++ = static_cast<unsigned char>(value * 255.f);
				tex_buf++;
			}
			tex_buf++;
		}
	}

	delete[] tex_buffer;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

bool ViewControllerSlice::GetTextureData(unsigned short*& out_data, const int width, const int height, const int filter, const int thickness)
{
	if (!out_data || !OffScreenDrawSlice(width, height, filter, thickness, false))
	{
		return false;
	}

	const int size = width * height;
	const int w_4 = width * 4;
	float* tex_buffer = new float[size * 4];

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, tex_buffer);

	float slope = ResourceContainer::GetInstance()->GetMainVolume().slope();
	int pixel_representation_offset = ResourceContainer::GetInstance()->GetMainVolume().pixel_representation_offset();

#pragma omp parallel for
	for (int i = 0; i < height; ++i)
	{
		ushort* img_buf = out_data + i * width;
		float* tex_buf = tex_buffer + (height - i - 1) * w_4;

		for (int j = 0; j < width; ++j)
		{
			*img_buf++ = static_cast<ushort>(*tex_buf * 65535.f / slope - pixel_representation_offset);
			tex_buf += 4;
		}
	}

	delete[] tex_buffer;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

const int ViewControllerSlice::GetSharpenLevel()
{
	return static_cast<int>(sharpen_level_);
}

bool ViewControllerSlice::OffScreenDrawSlice(const int viewport_width, const int viewport_height, const int filter, const int thickness, bool draw_surface)
{
	InitOffScreenBuff(viewport_width, viewport_height);

	auto view_param = BaseViewController::view_param();
	QSize ori_view_size = view_param->view_size();

	view_param->SetViewPort(viewport_width, viewport_height);
	SetProjection();

	glBindFramebuffer(GL_FRAMEBUFFER, off_fb_);
	glViewport(0, 0, viewport_width, viewport_height);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		common::Logger::instance()->Print(common::LogType::ERR, " PACSViewController::OffScreenDrawSlice: invalid fbo.");
		return false;
	}

	Renderer().SetVolumeTexture();
	Renderer().DrawSlice(slice_obj_.get(), off_texture_pack_.tex_buffer,
		transform_->mvp(), transform_->vol_tex_transform_mat(),
		transform_->GetUpVector(), thickness, false, true);

	SharpenLevel sharpen_level = static_cast<SharpenLevel>(filter);
	if (sharpen_level != SHARPEN_OFF)
	{
		Renderer().PostProcessingSharpen(off_texture_pack_, sharpen_level);
	}

	if (draw_surface)
	{
		DrawSurface();
	}

	view_param->SetViewPort(ori_view_size.width(), ori_view_size.height());
	SetProjection();

	return true;
}
#endif
