#include "view_controller_implant3d.h"

#include "../../Common/Common/W3ElementGenerator.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/GLfunctions/gl_helper.h"

#include "../../Resource/Resource/pano_resource.h"
#include "../../Resource/Resource/implant_resource.h"
#include "../../Resource/ResContainer/resource_container.h"

#include "../../UIGLObjects/gl_implant_widget.h"

#include "../../UIGLObjects/W3SurfaceAxesItem.h"

#include "../../Module/Will3DEngine/renderer_manager.h"

#include "view_render_param.h"
#include "surface_items.h"

using namespace UIViewController;
using namespace UIGLObjects;
using namespace Will3DEngine;

ViewControllerImplant3D::ViewControllerImplant3D()
{
	transform_.reset(new TransformImplant3D());
	surface_items()->InitItem(SurfaceItems::NERVE, ObjCoordSysType::TYPE_VOLUME);
	surface_items()->InitItem(SurfaceItems::IMPLANT, ObjCoordSysType::TYPE_VOLUME);
	axes_item_.reset(new CW3SurfaceAxesItem());
}

ViewControllerImplant3D::~ViewControllerImplant3D() {}

void ViewControllerImplant3D::SetProjection()
{
	if (!Renderer().IsInitialize())
		return;

	auto view_param = BaseViewController::view_param();
	if (view_param == nullptr)
		return;

	const auto& res_pano = ResourceContainer::GetInstance()->GetPanoResource();
	if (&res_pano == nullptr)
		return;

	const CurveData& pano_curve = res_pano.curve_center_data();
	if (pano_curve.points().empty())
		return;

	glm::vec3 vol_center = Renderer().GetVolCenter();
	float z_spacing = Renderer().GetSpacingZ();
	int pano_point_cnt = pano_curve.points().size();
	const std::vector<glm::vec3>& pano_points = pano_curve.points();
	const std::vector<glm::vec3>& pano_upvectors = pano_curve.up_vectors();
	const glm::vec3& back_vector = res_pano.back_vector();
	const float height = static_cast<float>(res_pano.pano_3d_height());
	const float depth = static_cast<float>(res_pano.pano_3d_depth());
	const float half_depth = depth * 0.5f;

	glm::vec3 center_point = pano_points[pano_point_cnt / 2] + back_vector*height*0.5f;
	glm::vec3 center_point_gl = GLhelper::MapVolToWorldGL(center_point, vol_center, z_spacing);

	glm::vec3 gl_min = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 gl_max = glm::vec3(std::numeric_limits<float>::min());

	auto func_map_to_norm_gl = [&](const glm::vec3& pt)->glm::vec3
	{
		return glm::vec3(-1.0f, 1.0f, 1.0f)*
			GLhelper::MapVolToWorldGL(pt, vol_center, z_spacing);
	};

	for (int i = 0; i < pano_point_cnt; i++)
	{
		const glm::vec3& point = pano_points[i];
		const glm::vec3& upvector = pano_upvectors[i];

		glm::vec3 p1 = (func_map_to_norm_gl(point + upvector * half_depth));
		glm::vec3 p3 = (func_map_to_norm_gl(point + upvector * half_depth + back_vector * height));

		gl_min = glm::min(p1, gl_min);
		gl_max = glm::max(p1, gl_max);

		gl_min = glm::min(p3, gl_min);
		gl_max = glm::max(p3, gl_max);
	}

	glm::vec3 gl_center = ((gl_min + gl_max) * 0.5f);
	transform_->SetVolumeCenterPosition(gl_center);

	float world_width = GLhelper::ScaleVolToGL(abs((pano_points.back() - pano_points.front()).x));
	float world_height = GLhelper::ScaleVolToGL(depth);
	float world_depth = GLhelper::ScaleVolToGL(height);
	float cam_fov = sqrt(world_width*world_width +
		world_height * world_height +
		world_depth * world_depth);
	transform_->SetCamFOV(cam_fov);
	transform_->SetViewMatrix();

	float map_scene_to_gl;
	const float kWorldScaleOffset = 1.35f;

	transform_->SetProjectionFitIn(BaseViewController::GetPackViewProj(),
		world_width * kWorldScaleOffset,
		world_depth * kWorldScaleOffset,
		map_scene_to_gl);

	view_param->set_map_scene_to_gl(map_scene_to_gl);
}

void ViewControllerImplant3D::InitVAOs()
{
	if (clip_pano_area_)
	{
		InitVAOVBOArchVolume();
	}
	else
	{
		BaseViewController3D::InitVAOs();
	}
}

void ViewControllerImplant3D::ClearGL()
{
	BaseViewController3D::ClearGL();

	if (vbo_vol_.size())
		glDeleteBuffers(vbo_vol_.size(), &vbo_vol_[0]);
	vbo_vol_.clear();

	axes_item_->clearVAOVBO();
}
void ViewControllerImplant3D::SelectImplant(int* implant_id)
{
	if (GetImplantData() == nullptr)
		return;

	auto view_param = BaseViewController::view_param();
	if (view_param == nullptr)
		return;

	if (!IsReady())
	{
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

bool ViewControllerImplant3D::IsPickImplant() const
{
	return axes_item_->isPicking();
}

int ViewControllerImplant3D::GetPickImplantID() const
{
	ImplantData* implant_data = GetImplantData();

	if (implant_data == nullptr)
		return -1;
	else
		return implant_data->id();
}
const glm::mat4& ViewControllerImplant3D::GetRotateMatrix() const
{
	return transform_->rotate_arcball();
}
const glm::mat4& ViewControllerImplant3D::GetReorienMatrix() const
{
	return transform_->reorien();
}
glm::mat4 ViewControllerImplant3D::GetNavigatorViewMatrix()
{
	return glm::lookAt(glm::vec3(0.0f, -transform_->cam_fov(), 0.0f),
		glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
}
glm::mat4 ViewControllerImplant3D::GetCollisionProjectionViewMatrix() const
{
	TransformROIVR transform;

	PackViewProj arg = BaseViewController::GetPackViewProj();

	float map_scene_to_gl;
	transform.Initialize(Renderer().GetModelScale());
	transform.SetProjection(arg, map_scene_to_gl);

	return transform.projection() * transform.view();
}

void ViewControllerImplant3D::MoveImplant(int* implant_id,
	glm::vec3* delta_translate,
	glm::vec3* rotate_axes,
	float* delta_degree)
{
	if (!axes_item_->isPicking())
	{
		return;
	}

	ImplantData* implant_data = GetImplantData();

	if (implant_data == nullptr || !implant_data->is_visible())
		return;

	*implant_id = implant_data->id();

	auto view_param = BaseViewController::view_param();

	if (view_param == nullptr)
		return;

	QRectF proj_rect = transform_->projection_rect();

	if (axes_item_->isSelectTranslate())
	{
		QPointF delta_scene = view_param->scene_mouse_curr() - view_param->scene_mouse_prev();
		QPointF proj_center = proj_rect.center();
		proj_center.setY(-proj_center.y()); //화면 Y축 반전.. 하..

		QPointF delta_gl = view_param->MapSceneToGL(delta_scene) - proj_center;
		QPointF delta_norm_gl(delta_gl.x() / (proj_rect.width()*0.5f),
			delta_gl.y() / (proj_rect.height()*0.5f));

		*delta_translate = axes_item_->translate(glm::vec3(delta_norm_gl.x(), -delta_norm_gl.y(), 0.0f), false);
	}
	else if (axes_item_->isSelectRotate())
	{
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
			glm::vec3(gl_norm_prev.x(), -gl_norm_prev.y(), 0.0f), false);

		*rotate_axes = rot.second;
		*delta_degree = glm::degrees(rot.first);
	}

#if DEVELOP_MODE
	common::Logger::instance()->PrintDebugMode("ViewControllerImplant3D::MoveImplant");
#endif
}
void ViewControllerImplant3D::PickAxesItem(bool* is_update_scene)
{
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

void ViewControllerImplant3D::RenderForPickAxes()
{
	if (!IsReady())
	{
		common::Logger::instance()->Print(common::LogType::ERR,
			"ViewControllerImplant3D::RenderForPickAxes: not ready.");
		assert(false);
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

void ViewControllerImplant3D::InitVAOVBOArchVolume()
{
	if (vao_vol() != 0 || vbo_vol_.size())
		return;

	const auto& res_pano = ResourceContainer::GetInstance()->GetPanoResource();

	if (&res_pano == nullptr)
		return;

	const CurveData& pano_curve = res_pano.curve_center_data();

	if (pano_curve.points().empty())
		return;

	glm::vec3 texel_size = Renderer().GetTexelSize();
	glm::vec3 vol_center = Renderer().GetVolCenter();
	float z_spacing = Renderer().GetSpacingZ();

	auto func_map_to_norm_gl = [&](const glm::vec3& pt)->glm::vec3
	{
		return glm::vec3(-texel_size.x, texel_size.y, texel_size.z)*
			GLhelper::MapVolToWorldGL(pt, vol_center, z_spacing);
	};

	int pano_point_cnt = pano_curve.points().size();
	const std::vector<glm::vec3>& pano_points = pano_curve.points();
	const std::vector<glm::vec3>& pano_upvectors = pano_curve.up_vectors();
	const float height = (float)res_pano.pano_3d_height();
	const float depth = (float)res_pano.pano_3d_depth();
	const float half_depth = depth*0.5f;
	const glm::vec3& back_vector = res_pano.back_vector();

	std::vector<glm::vec3> upper_top_line, upper_bot_line,
		lower_top_line, lower_bot_line;
	std::vector<glm::vec3> upper_left_edge_line, upper_right_edge_line,
		lower_left_edge_line, lower_right_edge_line, upper_center_line, lower_center_line;

	upper_top_line.reserve(pano_point_cnt);
	upper_bot_line.reserve(pano_point_cnt);
	lower_top_line.reserve(pano_point_cnt);
	lower_bot_line.reserve(pano_point_cnt);

	for (int i = 0; i < pano_point_cnt; i++)
	{
		const glm::vec3& point = pano_points[i];
		const glm::vec3& upvector = pano_upvectors[i];

		upper_top_line.push_back(func_map_to_norm_gl(point + upvector * half_depth));
		upper_bot_line.push_back(func_map_to_norm_gl(point - upvector * half_depth));

		lower_top_line.push_back(func_map_to_norm_gl(point + upvector * half_depth + back_vector*height));
		lower_bot_line.push_back(func_map_to_norm_gl(point - upvector * half_depth + back_vector*height));
	}

	upper_left_edge_line.push_back(upper_top_line.front());
	upper_left_edge_line.push_back(upper_bot_line.front());
	upper_right_edge_line.push_back(upper_top_line.back());
	upper_right_edge_line.push_back(upper_bot_line.back());

	lower_left_edge_line.push_back(lower_top_line.front());
	lower_left_edge_line.push_back(lower_bot_line.front());
	lower_right_edge_line.push_back(lower_top_line.back());
	lower_right_edge_line.push_back(lower_bot_line.back());

	upper_center_line.push_back(upper_bot_line.front());
	upper_center_line.push_back(upper_bot_line.back());

	lower_center_line.push_back(lower_bot_line.front());
	lower_center_line.push_back(lower_bot_line.back());

	std::vector<glm::vec3> vert, norm;
	std::vector<uint> indices;

	//Face1
	CW3ElementGenerator::generateRectFace(upper_bot_line, upper_top_line, vert, norm, indices);

	//Face2
	CW3ElementGenerator::generateRectFace(lower_top_line, lower_bot_line, vert, norm, indices);

	//Face3
	CW3ElementGenerator::generateRectFace(upper_top_line, lower_top_line, vert, norm, indices);

	//Face4
	CW3ElementGenerator::generateRectFace(lower_center_line, upper_center_line, vert, norm, indices);

	//Face5
	CW3ElementGenerator::generateRectFace(lower_left_edge_line, upper_left_edge_line, vert, norm, indices);

	//Face6
	CW3ElementGenerator::generateRectFace(upper_right_edge_line, lower_right_edge_line, vert, norm, indices);

	//Face7
	CW3ElementGenerator::generateRectFace(upper_center_line, upper_bot_line, vert, norm, indices);

	//Face8
	CW3ElementGenerator::generateRectFace(lower_bot_line, lower_center_line, vert, norm, indices);

	uint prev_vao = vao_vol();
	if (prev_vao)
	{
		glDeleteVertexArrays(1, &prev_vao);
	}
	if (vbo_vol_.size())
		glDeleteBuffers(vbo_vol_.size(), &vbo_vol_[0]);
	vbo_vol_.clear();

	unsigned int vao = 0;
	vbo_vol_.resize(3, 0);
	CW3GLFunctions::initVAOVBO(&vao, &vbo_vol_[0], vert, norm, indices);
	set_vao_vol(vao);
	set_indices_vol(indices.size());

#if DEVELOP_MODE
	common::Logger::instance()->PrintDebugMode("ViewControllerImplant3D::InitVAOVBOArchVolume");
#endif
}

ImplantData* ViewControllerImplant3D::GetImplantData() const
{
	const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
	int selected_id = res_implant.selected_implant_id();
	if (selected_id < 0)
		return nullptr;

	const auto& implant_datas = res_implant.data();
	if (implant_datas.find(selected_id) == implant_datas.end())
		return nullptr;

	return implant_datas.at(selected_id).get();
}

void ViewControllerImplant3D::SetSurfaceMVP()
{
	BaseViewController3D::SetSurfaceMVP();

	this->SetSurfaceAxesMVP();
}

void ViewControllerImplant3D::SetSurfaceAxesMVP()
{
	is_selected_implant_ = false;
	axes_item_->setShown(false);

	ImplantData* implant_data = GetImplantData();
	if (implant_data == nullptr || !implant_data->is_visible())
		return;

	//glm::vec3 world_scale = Renderer().GetWorldScale();
	//glm::mat4 axes_model = glm::scale(glm::vec3(glm::length(world_scale)*0.3f));
	//
	//axes_item_->setProjViewMat(transform_->projection(), transform_->view());
	//axes_item_->setTransformMat(axes_model, TransformType::SCALE);
	//axes_item_->setTransformMat(implant_data->translate_in_vol(), TransformType::TRANSLATE);
	//axes_item_->setTransformMat(implant_data->rotate_in_vol(), TransformType::ROTATE);
	//axes_item_->setTransformMat(transform_->rotate(), TransformType::ARCBALL);
	//axes_item_->setShown(true);

	is_selected_implant_ = true;
}

void ViewControllerImplant3D::DrawOverwriteSurface()
{
	BaseViewController3D::DrawOverwriteSurface();

	const auto& prog = Renderer().programs();
	if (axes_item_->isShown())
	{
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

/**=====================================================================================
private functions: level 1
*====================================================================================**/
VolumeRenderer& ViewControllerImplant3D::Renderer() const
{
	return RendererManager::GetInstance().renderer_vol(VOL_MAIN);
}

void ViewControllerImplant3D::SetPackTexture()
{
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
void ViewControllerImplant3D::SetPackTextureHandle()
{
	BaseViewController3D::SetPackTextureHandle();

	pack_pick_implant_.handler = tex_handler(TEX_PICK_IMPLANT);
	pack_pick_axes_.handler = tex_handler(TEX_PICK_AXIS);
}
void ViewControllerImplant3D::ReadyBufferHandles()
{
	BaseViewController::ReadyBufferHandles(DEPTH_END, TEX_IMP_END);
}

/**=====================================================================================
private functions: level 2
*====================================================================================**/
BaseTransform& ViewControllerImplant3D::transform() const
{
	return *(dynamic_cast<BaseTransform*>(transform_.get()));
}

void ViewControllerImplant3D::ApplyPreferences()
{
	BaseViewController3D::ApplyPreferences();
}

void ViewControllerImplant3D::ClipPanoArea(const bool clip)
{
	clip_pano_area_ = clip;

	ClearGL();
}

void ViewControllerImplant3D::SetFitMode(BaseTransform::FitMode mode)
{
	transform_->set_fit_mode(mode);
}
