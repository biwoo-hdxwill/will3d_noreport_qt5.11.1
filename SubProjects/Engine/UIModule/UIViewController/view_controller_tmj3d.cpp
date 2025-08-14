#include "view_controller_tmj3d.h"

#include "../../Common/Common/W3ElementGenerator.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/GLfunctions/gl_helper.h"
#include "../../Common/GLfunctions/gl_transform_functions.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/tmj_resource.h"
#include "../../Resource/Resource/W3Image3D.h"

#include "../../Module/Will3DEngine/renderer_manager.h"

#include "view_render_param.h"

using namespace UIViewController;
using namespace Will3DEngine;
namespace {
	const float kWorldScaleOffset = 1.35f;
	const glm::vec3 kInvAxisX(-1.0f, 1.0f, 1.0f);
}

ViewControllerTMJ3D::ViewControllerTMJ3D(const TMJDirectionType& type) :
	direction_type_(type){
	transform_.reset(new TransformROIVR());
	
}

ViewControllerTMJ3D::~ViewControllerTMJ3D() {}

void ViewControllerTMJ3D::UpdateCutting(const int& curr_step) {
	if (!cut_info_.is_cut)
		return;

	const auto& res_tmj = ResourceContainer::GetInstance()->GetTMJResource();
	if (&res_tmj == nullptr)
		return;

	const auto& res_frontal = res_tmj.frontal(direction_type_);
	if (&res_frontal == nullptr)
		return;

	const auto& res_lateral = res_tmj.lateral(direction_type_);
	if (&res_lateral == nullptr)
		return;

	cut_info_.curr_step = curr_step;
	int width = (int)res_frontal.param().width;
	int height = (int)res_lateral.param().width;
	int depth = (int)res_tmj.height();

	cut_info_.is_update = true;

	glm::vec3 vol_center = Renderer().GetVolCenter();
	float spacing_z = Renderer().GetSpacingZ();
	glm::vec3 world_center = GLhelper::MapVolToWorldGL(res_tmj.GetTMJRectCenter(direction_type_),
													   vol_center, spacing_z);

	glm::mat4 rotate = res_tmj.GetTMJRoateMatrix(direction_type_);

	const glm::vec3 vol_size = 1.0f / Renderer().GetTexelSize();
	glm::mat4 voltex_to_world = glm::scale(vol_size) *
		glm::scale(vec3(2.0f))*
		glm::translate(vec3(-0.5f));
	glm::mat4 world_to_tmjtex = glm::inverse(
		rotate * glm::scale(glm::vec3((float)width,
		(float)height,
		(float)depth)) *
		glm::scale(vec3(2.0f))*
		glm::translate(vec3(-0.5f)));

	cut_info_.map_vol_to_mask = world_to_tmjtex *
		glm::translate(-world_center)*voltex_to_world;
}

void ViewControllerTMJ3D::SetCutting(bool is_enable) {
	cut_info_.is_cut = is_enable;
}

void ViewControllerTMJ3D::SetCliping(const std::vector<glm::vec4>& planes, bool is_enable) {
	const auto& res_tmj = ResourceContainer::GetInstance()->GetTMJResource();
	if (&res_tmj == nullptr)
		return;

	const auto& res_frontal = res_tmj.frontal(direction_type_);
	if (&res_frontal == nullptr)
		return;

	const auto& res_lateral = res_tmj.lateral(direction_type_);
	if (&res_lateral == nullptr)
		return;

	glm::vec3 vol_range = Renderer().GetModelScale();
	const glm::vec3 texel_size = Renderer().GetTexelSize();
	float z_spacing = Renderer().GetSpacingZ();
	glm::vec3 vol_center = Renderer().GetVolCenter();

	const glm::vec3& up_vector = glm::vec3(-1.0f, 1.0f, 1.0f)*res_frontal.up_vector();
	const glm::vec3& back_vector = glm::vec3(-1.0f, 1.0f, 1.0f)*res_tmj.back_vector();
	const glm::vec3 right_vector = glm::normalize(glm::cross(up_vector, back_vector));

	float norm_width = glm::length(GLhelper::ScaleVolToGL(res_frontal.param().width * right_vector) / vol_range);
	float norm_height = glm::length(GLhelper::ScaleVolToGL(res_lateral.param().width * up_vector) / vol_range);
	float norm_depth = glm::length(GLhelper::ScaleVolToGL(res_tmj.height() * back_vector) / vol_range);

	glm::vec3 norm_center = glm::vec3(-1.0f, 1.0f, 1.0f)*GLhelper::MapVolToNormGL(res_tmj.GetTMJRectCenter(direction_type_),
													 vol_center, vol_range, z_spacing);

	PackClipping pack_clipping;
	pack_clipping.planes.reserve(planes.size());
	for (const auto& elem : planes) {
		glm::vec3 plane_vector = glm::vec3(-1.0f, 1.0f, 1.0f)*glm::normalize(glm::vec3(elem));
		float to_center = glm::dot(plane_vector, norm_center);
		float clip_range = glm::dot(plane_vector,
									plane_vector*elem.w*vec3(norm_width, norm_height, norm_depth))*0.5f;

		pack_clipping.planes.push_back(vec4(vec3(elem), clip_range + to_center));
	}
	pack_clipping.is_clipping = is_enable;
	set_pack_clipping(pack_clipping);
}

void ViewControllerTMJ3D::SetProjection() {
	BaseViewController3D::SetProjection();

	if (!Renderer().IsInitialize())
		return;

	auto view_param = BaseViewController::view_param();
	if (view_param == nullptr)
		return;

	const auto& res_tmj = ResourceContainer::GetInstance()->GetTMJResource();
	if (&res_tmj == nullptr)
		return;

	const auto& res_frontal = res_tmj.frontal(direction_type_);
	if (&res_frontal == nullptr)
		return;

	const auto& res_lateral = res_tmj.lateral(direction_type_);
	if (&res_lateral == nullptr)
		return;


	glm::vec3 vol_center = Renderer().GetVolCenter();
	float z_spacing = Renderer().GetSpacingZ();

	auto func_map_to_gl = [&](const glm::vec3& pt)->glm::vec3 {
		return glm::vec3(-1.0f, 1.0f, 1.0f)*
			GLhelper::MapVolToWorldGL(pt, vol_center, z_spacing);
	};

	glm::vec3 gl_center = func_map_to_gl(res_tmj.GetTMJRectCenter(direction_type_));
	transform_->SetVolumeCenterPosition(gl_center);

	float world_width = GLhelper::ScaleVolToGL(res_frontal.param().width);
	float world_height = GLhelper::ScaleVolToGL(res_lateral.param().width);
	float world_depth = GLhelper::ScaleVolToGL(res_tmj.height());
	float cam_fov = sqrt(world_width * world_width +
						 world_height * world_height +
						 world_depth * world_depth);
	transform_->SetCamFOV(cam_fov);
	transform_->SetViewMatrix();

	float map_scene_to_gl;

	transform_->SetProjectionFitIn(BaseViewController::GetPackViewProj(),
								   world_width*kWorldScaleOffset,
								   world_depth*kWorldScaleOffset,
								   map_scene_to_gl);

	view_param->set_map_scene_to_gl(map_scene_to_gl);
}

void ViewControllerTMJ3D::InitVAOs() {
	this->InitVAOVBOROIVolume();
}

void ViewControllerTMJ3D::ClearGL() {
	BaseViewController3D::ClearGL();

	if (vbo_vol_.size())
		glDeleteBuffers(vbo_vol_.size(), &vbo_vol_[0]);
	vbo_vol_.clear();
}

glm::mat4 ViewControllerTMJ3D::GetRotateMatrix() {
	return transform_->rotate_arcball()*transform_->reorien();
}

glm::mat4 ViewControllerTMJ3D::GetViewMatrix() {
	return glm::lookAt(glm::vec3(0.0f, -transform_->cam_fov(), 0.0f),
					   glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
}

bool ViewControllerTMJ3D::IsReady() {
	return (Renderer().IsInitialize() && initialized() && is_tmj_);
}

void ViewControllerTMJ3D::InitVAOVBOROIVolume() {
	if (vao_vol() != 0 || vbo_vol_.size())
		return;

	const auto& res_tmj = ResourceContainer::GetInstance()->GetTMJResource();

	if (&res_tmj == nullptr) {
		is_tmj_ = false;
		return;
	}

	const auto& res_frontal = res_tmj.frontal(direction_type_);
	if (&res_frontal == nullptr) {
		is_tmj_ = false;
		return;
	}

	const auto& res_lateral = res_tmj.lateral(direction_type_);
	if (&res_lateral == nullptr) {
		is_tmj_ = false;
		return;
	}
	glm::vec3 vol_range = Renderer().GetModelScale();
	float max_axis_size = std::max(vol_range.x, vol_range.y);
	max_axis_size = std::max(vol_range.z, max_axis_size);
	glm::mat4 ratio_scale = glm::scale(glm::vec3((vol_range / max_axis_size)));
	glm::vec3 vol_center = Renderer().GetVolCenter();
	float z_spacing = Renderer().GetSpacingZ();

	const glm::vec3& up_vector =   kInvAxisX * res_frontal.up_vector();
	const glm::vec3& back_vector = kInvAxisX * res_tmj.back_vector();
	const glm::vec3 right_vector = glm::normalize(glm::cross(up_vector, back_vector));

	glm::vec3 world_center = kInvAxisX*GLhelper::MapVolToWorldGL(res_tmj.GetTMJRectCenter(direction_type_),
												   vol_center, z_spacing);

	glm::vec3 x_offset = (res_frontal.param().width)*right_vector;
	glm::vec3 y_offset = (res_lateral.param().width)*up_vector;
	glm::vec3 z_offset = (res_tmj.height())*back_vector;

	std::vector<glm::vec3> vert, norm;
	vert.push_back((world_center + (x_offset + y_offset + z_offset)) / vol_range);
	vert.push_back((world_center + (x_offset + y_offset - z_offset)) / vol_range);
	vert.push_back((world_center + (-x_offset + y_offset - z_offset)) / vol_range);
	vert.push_back((world_center + (-x_offset + y_offset + z_offset)) / vol_range);
	vert.push_back((world_center + (x_offset - y_offset + z_offset)) / vol_range);
	vert.push_back((world_center + (x_offset - y_offset - z_offset)) / vol_range);
	vert.push_back((world_center + (-x_offset - y_offset - z_offset)) / vol_range);
	vert.push_back((world_center + (-x_offset - y_offset + z_offset)) / vol_range);

	std::vector<unsigned int> indices;

	CW3ElementGenerator::GenerateTMJCube(vert, norm, indices);

	uint prev_vao = vao_vol();
	if (prev_vao) {
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

	is_tmj_ = true;
#if DEVELOP_MODE
	common::Logger::instance()->PrintDebugMode("ViewControllerTMJ3D::InitVAOVBOArchVolume");
#endif 
}

/**=====================================================================================
private functions: level 1
*====================================================================================**/
bool ViewControllerTMJ3D::IsValidVol(const CW3Image3D& vol) const {
	if (&vol == nullptr || vol.width() == 0 || vol.height() == 0 || vol.depth() == 0)
		return false;
	else
		return true;
}

void ViewControllerTMJ3D::SetCutMaskTexture() {
	CW3GLFunctions::printError(__LINE__, "ViewControllerTMJ3D::SetCutMaskTexture Start");

	const auto& res_tmj = ResourceContainer::GetInstance()->GetTMJResource();
	if (&res_tmj == nullptr)
		return;

	const CW3Image3D& vol = res_tmj.tmj_mask(direction_type_);
	if (&vol == nullptr)
		return;

	if (cut_info_.pack.handler) {
		glDeleteTextures(1, &cut_info_.pack.handler);
		cut_info_.pack.handler = 0;
	}

	if (!IsValidVol(vol)) {
		return;
	}

	glActiveTexture(cut_info_.pack.tex_num);

	CW3GLFunctions::InitVRCutVol3DTex2Dpointer(cut_info_.pack.handler,
										 vol.width(), vol.height(), vol.depth(),
										 vol.getData());

	CW3GLFunctions::printError(__LINE__, "ViewControllerTMJ3D::SetCutMaskTexture End");
}

void ViewControllerTMJ3D::RayCasting() {
	//if (cut_info_.is_cut && cut_info_.is_update) {
	//	SetCutMaskTexture();
	//	cut_info_.is_update = false;
	//}
	this->SetRayStepSize();

	Renderer().SetVolumeTexture();
	Renderer().SetTFtexture();

	Renderer().DrawFrontFace(
		vao_vol(),
		indices_vol(),
		pack_front().tex_buffer,
		transform().mvp());

	Renderer().DrawBackFace(
		vao_vol(),
		indices_vol(),
		pack_back().tex_buffer,
		transform().mvp());

	glDepthFunc(GL_LESS);
	this->DrawBackFaceSurface();

	Renderer().DrawFinalFrontFace(
		(GLObject*)plane_obj(),
		pack_front(),
		pack_back(),
		pack_clipping());

	CW3GLFunctions::setDepthStencilAttarch(depth_handler(DEPTH_RAYCASTING));

	glDrawBuffer(pack_raycasting().tex_buffer); {
		CW3GLFunctions::clearView(true, GL_BACK);

		this->DrawSurface();
		if (cut_info_.is_cut && cut_info_.curr_step > -1) {
			if (cut_info_.is_update) {
				SetCutMaskTexture();
				cut_info_.is_update = false;
			}

			Renderer().DrawRaycastingCut((GLObject*)plane_obj(), pack_raycasting().tex_buffer,
										 cut_info_.curr_step,
										 transform().mvp(), cut_info_.map_vol_to_mask, pack_front(),
										 pack_back(), cut_info_.pack);
		} else {
			Renderer().DrawRaycasting((GLObject*)plane_obj(), pack_raycasting().tex_buffer,
									  transform().mvp(), pack_front(), pack_back());
		}

		this->DrawOverwriteSurface();
	}

	CW3GLFunctions::setDepthStencilAttarch(depth_handler(DEPTH_DEFAULT));

	Renderer().DrawTextureToTexture((GLObject*)plane_obj(), pack_screen().tex_buffer, pack_raycasting());
}

VolumeRenderer& ViewControllerTMJ3D::Renderer() const {
	return RendererManager::GetInstance().renderer_vol(VOL_MAIN);
}
void ViewControllerTMJ3D::SetPackTexture() {
	cut_info_.pack = PackTexture(tex_buffer(TEX_CUT_TMJ_MASK),
									  tex_num(TEX_CUT_TMJ_MASK),
									  tex_handler(TEX_CUT_TMJ_MASK),
									  _tex_num(TEX_CUT_TMJ_MASK));
	BaseViewController3D::SetPackTexture();
}
void ViewControllerTMJ3D::ReadyBufferHandles() {
	BaseViewController::ReadyBufferHandles(GL_DEPTH_HANDLE::DEPTH_END, TEX_IMP_END);
}
/**=====================================================================================
private functions: level 2
*====================================================================================**/
BaseTransform& ViewControllerTMJ3D::transform() const {
	return *(dynamic_cast<BaseTransform*>(transform_.get()));
}

void ViewControllerTMJ3D::ApplyPreferences()
{
	BaseViewController3D::ApplyPreferences();
}

void ViewControllerTMJ3D::SetFitMode(BaseTransform::FitMode mode)
{
	transform_->set_fit_mode(mode);
}
