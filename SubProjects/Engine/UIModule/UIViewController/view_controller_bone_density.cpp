#include "view_controller_bone_density.h"

#include <qmath.h>
#include <QApplication>

#include "../../Common/Common/W3Cursor.h"
#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/GLfunctions/gl_helper.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/implant_resource.h"

#include "../../UIGLObjects/implant_obj_gl.h"
#include "../../UIGLObjects/view_plane_obj_gl.h"

#include "../../Module/Will3DEngine/renderer_manager.h"

#include "surface_items.h"
#include "view_render_param.h"

using namespace UIViewController;
using namespace UIGLObjects;
using namespace Will3DEngine;

ViewControllerBoneDensity::ViewControllerBoneDensity() : transform_(new TransformBasicVR)
{

}

ViewControllerBoneDensity::~ViewControllerBoneDensity()
{

}

void ViewControllerBoneDensity::RenderingBoneDensity()
{
	auto view_param = BaseViewController::view_param();

	if (view_param == nullptr || !IsReady())
	{
		ClearGL();
		return;
	}

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

	if (!IsReady() && !Renderer().IsInitialize() && initialized())
	{
		common::Logger::instance()->Print(common::LogType::ERR, "ViewControllerBoneDensity::RenderingBoneDensity: not ready.");

		assert(false);
		return;
	}
	CW3GLFunctions::printError(__LINE__, "BaseViewControllerVR::RenderVolume Start");

	BaseViewController::ReadyFrameBuffer();
	glDrawBuffer(pack_screen_.tex_buffer);

	ReadyImplantObject();
	SetProjection();
	CW3GLFunctions::clearView(true, GL_BACK, 0.19f, 0.52f, 0.77f, 1.0f);

	CW3GLFunctions::clearDepth(GL_LESS);

	const glm::mat4& trans = implant_->GetTranslate(TYPE_VOLUME);
	glm::mat4 rotate = implant_->GetRotate(TYPE_VOLUME);

	const glm::mat4& world = glm::scale(Renderer().GetModelScale());

	// vol_tex_transform은 볼륨좌표계의 3D 텍스쳐를 읽기 때문에
	//항상 임플란트의 볼륨좌표계를 가져온다.
	glm::mat4 vol_tex_transform = glm::translate(vec3(0.5f)) *
		glm::scale(vec3(-0.5f, 0.5f, 0.5f)) *
		glm::inverse(world) * trans * rotate;

	glm::mat4 view_implant_model = rotate;

	glm::mat4 view_mat = camera_ * transform_->rotate() * transform_->reorien();
	Renderer().DrawBoneDensity((GLObject*)implant_.get(), view_implant_model,
		view_mat, transform_->projection(), vol_tex_transform);
	glDisable(GL_DEPTH_TEST);

#if DEVELOP_MODE
	common::Logger::instance()->PrintDebugMode(
		"ViewControllerBoneDensity::RenderingBoneDensity");
#endif
}

void ViewControllerBoneDensity::RenderScreen(uint dfbo)
{
	auto view_param = BaseViewController::view_param();
	if (view_param == nullptr) return;

	const QSize& view_size = view_param->view_size();

	const float kViewPortMargin = view_size.width() * 0.14f;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dfbo);
	glViewport(kViewPortMargin, 0, view_size.width(), view_size.height());
	CW3GLFunctions::clearView(true, GL_BACK);

	Renderer().DrawTexture(plane_obj(), pack_screen_);

	glDisable(GL_DEPTH_TEST);

	CW3GLFunctions::printError(__LINE__, "ViewControllerBoneDensity::RenderScreen");
}

void ViewControllerBoneDensity::SyncImplant3DCameraMatrix(const glm::mat4& rotate_mat, const glm::mat4& reorien_mat, const glm::mat4& view_mat)
{
	transform_->ForceRotate(rotate_mat);
	transform_->SetReorien(reorien_mat);

	glm::vec3 cam_dir(view_mat[0][2], view_mat[1][2], view_mat[2][2]);
	glm::vec3 cam_up(view_mat[0][1], view_mat[1][1], view_mat[2][1]);
	glm::vec3 cam_pos = cam_dir * glm::length(Renderer().GetModelScale());
	camera_ = (glm::lookAt(cam_pos, glm::vec3(0.0f), cam_up));
}

void ViewControllerBoneDensity::ClearGL()
{
	BaseViewController::ClearGL();

	if (implant_)
	{
		implant_->ClearVAOVBO();
	}
}

void ViewControllerBoneDensity::ProcessViewEvent(bool* need_render)
{
	*need_render = false;

	auto view_param = BaseViewController::view_param();
	if (view_param == nullptr)
	{
		return;
	}

	if (view_param->event_type() == EVIEW_EVENT_TYPE::ROTATE)
	{
		RotateArcBall();
		*need_render = true;
	}

	else if (view_param->event_type() == EVIEW_EVENT_TYPE::UPDATE)
	{
		this->SetProjection();
		*need_render = true;
	}
}

void ViewControllerBoneDensity::SetProjection()
{
	if (!this->IsReady())
	{
		return;
	}

	if (!implant_)
	{
		common::Logger::instance()->Print(common::LogType::ERR, "ViewControllerBoneDensity::SetProjection: invalid implant.");
		return;
	}

	glm::vec3 min, max;
	implant_->GetBoundingBoxRange(&max, &min);

	float bonding_length = GLhelper::ScaleVolToGL(glm::length(max - min));
	this->SetProjectionFitIn(bonding_length, bonding_length);
}

bool ViewControllerBoneDensity::IsReady()
{
	auto res_container = ResourceContainer::GetInstance();
	const ImplantResource& res_implant = res_container->GetImplantResource();

	int selected_id = GetSelectedImplantID();

	const auto& implant_datas = res_implant.data();
	if (implant_datas.find(selected_id) == implant_datas.end())
	{
		return false;
	}

	return true;
}

void ViewControllerBoneDensity::InitRotateMatrix()
{
	transform_->ForceRotate(glm::mat4(1.0f));
}

const glm::mat4& ViewControllerBoneDensity::GetRotateMatrix() const
{
	return transform_->rotate();
}

float ViewControllerBoneDensity::GetRotateAngle()
{
	return transform_->angle();
}

void ViewControllerBoneDensity::ApplyPreferences()
{

}

void ViewControllerBoneDensity::SetFitMode(BaseTransform::FitMode mode)
{
	transform_->set_fit_mode(mode);
}

void ViewControllerBoneDensity::ReadyImplantObject()
{
	int selected_id = GetSelectedImplantID();
	if (selected_id < 0)
	{
		return;
	}

	if (!implant_ || IsImplantModelChanged(selected_id))
	{
		implant_.reset(new ImplantObjGL(selected_id));

		// set implant spec data
		const ImplantResource& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
		const auto& selected_implant_data = res_implant.data().at(res_implant.selected_implant_id()).get();

		spec_.file_name = GetFileName();
		spec_.manufacturer = selected_implant_data->manufacturer();
		spec_.product = selected_implant_data->product();
		spec_.length = selected_implant_data->length();
		spec_.diameter = selected_implant_data->diameter();
	}
}

VolumeRenderer& ViewControllerBoneDensity::Renderer() const
{
	return RendererManager::GetInstance().renderer_vol(VOL_MAIN);
}

void ViewControllerBoneDensity::RotateArcBall()
{
	auto view_param = BaseViewController::view_param();
	if (view_param == nullptr)
	{
		return;
	}

	glm::vec3 v1 = GetMouseVector(view_param->scene_mouse_prev());
	glm::vec3 v2 = GetMouseVector(view_param->scene_mouse_curr());

	glm::vec3 rot_axis(1.f, 0.f, 0.f);
	float rot_angle = 0.0f;
	if (glm::length(v1 - v2) > 1e-5f)
	{
		rot_angle = std::acos(std::min(1.0f, glm::dot(v1, v2))) * 180.0f / M_PI;
		rot_angle *= common::kArcballSensitivity;

		rot_axis = glm::cross(v1, v2);
		rot_axis = glm::normalize(rot_axis);
	}

	transform().Rotate(rot_angle, rot_axis);

	QApplication::setOverrideCursor(CW3Cursor::ArrowCursor());
}

void ViewControllerBoneDensity::SetProjectionFitIn(float world_fit_width, float world_fit_height)
{
	if (!this->IsReady())
	{
		return;
	}

	auto view_param = BaseViewController::view_param();
	if (view_param == nullptr)
	{
		return;
	}

	PackViewProj arg = BaseViewController::GetPackViewProj();

	float map_scene_to_gl;
	transform().SetProjectionFitIn(arg, world_fit_width, world_fit_height, map_scene_to_gl);

	view_param->set_map_scene_to_gl(map_scene_to_gl);
}

bool ViewControllerBoneDensity::SetRenderer()
{
	if (Renderer().IsInitialize())
	{
		auto view_param = BaseViewController::view_param();
		if (view_param == nullptr)
		{
			return false;
		}

		transform().Initialize(Renderer().GetModelScale());
		view_param->set_base_pixel_spacing_mm(Renderer().GetBasePixelSpacingMM());
		return true;
	}
	else
		return false;
}

void ViewControllerBoneDensity::SetPackTexture()
{
	pack_screen_ = PackTexture(tex_buffer(TEX_SCREEN), tex_num(TEX_SCREEN),
		tex_handler(TEX_SCREEN), _tex_num(TEX_SCREEN));
}

void ViewControllerBoneDensity::SetPackTextureHandle()
{
	pack_screen_.handler = tex_handler(TEX_SCREEN);
}

void ViewControllerBoneDensity::ReadyBufferHandles()
{
	BaseViewController::ReadyBufferHandles(DEPTH_END, TEX_END);
}

BaseTransform& ViewControllerBoneDensity::transform()
{
	return *(dynamic_cast<BaseTransform*>(transform_.get()));
}

int ViewControllerBoneDensity::GetSelectedImplantID() const
{
	const ImplantResource& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
	return res_implant.selected_implant_id();
}

bool ViewControllerBoneDensity::IsImplantModelChanged(int selected_id)
{
	if (implant_->id() != selected_id)
	{
		return true;
	}

	const ImplantResource& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
	const auto& selected_implant_data = res_implant.data().at(res_implant.selected_implant_id()).get();

	if (GetFileName() != spec_.file_name)
		return true;
	else if (selected_implant_data->manufacturer() != spec_.manufacturer)
		return true;
	else if (selected_implant_data->product() != spec_.product)
		return true;
	else if (selected_implant_data->length() != spec_.length)
		return true;
	else if (selected_implant_data->diameter() != spec_.diameter)
		return true;
	else
		return false;
}

glm::vec3 ViewControllerBoneDensity::GetMouseVector(const QPointF& pt_scene)
{
	auto view_param = BaseViewController::view_param();
	if (view_param == nullptr)
	{
		return glm::vec3();
	}

	const QSize& scene_size = view_param->scene_size();

	QPointF pt_norm_scene(pt_scene.x() / scene_size.width() - 0.5f, pt_scene.y() / scene_size.height() - 0.5f);
	QPointF pt_gl = GLhelper::ScaleSceneToGL(pt_norm_scene);

	return GetArcBallVector(pt_gl);
}

glm::vec3 ViewControllerBoneDensity::GetArcBallVector(const QPointF& pt_gl)
{
	vec3 arcball_vector(pt_gl.x(), 0.0f, -pt_gl.y());
	float sqrt_xz = arcball_vector.x * arcball_vector.x + arcball_vector.z * arcball_vector.z;

	if (sqrt_xz < 1.0f)
	{
		arcball_vector.y = sqrt(1.0f - sqrt_xz);
	}
	else
	{
		arcball_vector = glm::normalize(arcball_vector);
	}

	return arcball_vector;
}

QString ViewControllerBoneDensity::GetFileName()
{
	const ImplantResource& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
	const auto& selected_implant_data = res_implant.data().at(res_implant.selected_implant_id()).get();

	QString full_path = selected_implant_data->file_path();
	QStringList list = full_path.split("/");
	if (list.isEmpty())
	{
		return QString();
	}

	return list.last();
}
