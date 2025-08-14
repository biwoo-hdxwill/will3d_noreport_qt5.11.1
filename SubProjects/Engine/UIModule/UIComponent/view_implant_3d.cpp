#include "view_implant_3d.h"

#include <QMouseEvent>

#include <Engine/Common/Common/global_preferences.h>
#include "../../Common/Common/W3Enum.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/color_will3d.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/implant_resource.h"
#include "../UIViewController/view_controller_implant3d.h"
#include "../UIViewController/view_render_param.h"

#include "../UIPrimitive/implant_text_item.h"

#include <Engine/Common/Common/W3Memory.h>
#include <Engine/UIModule/UIGLObjects/measure_3d_manager.h>
#include <Engine/UIModule/UIViewController/base_transform.h>
#include <Engine/Module/VREngine/W3VREngine.h>
#include <Engine/Common/GLfunctions/WGLSLprogram.h>

#include "scene.h"

using namespace UIViewController;

ViewImplant3D::ViewImplant3D(QWidget* parent)
	: View3D(common::ViewTypeID::IMPLANT_3D, parent)
{
	controller_.reset(new ViewControllerImplant3D);
	controller_->set_view_param(view_render_param());

	scene().InitViewItem(Viewitems::ALIGN_TEXTS);
	scene().InitViewItem(Viewitems::NAVIGATION);
	scene().InitViewItem(Viewitems::RULER);
	scene().InitViewItem(Viewitems::BORDER);
	scene().InitViewItem(Viewitems::GRID);
	scene().InitMeasure(view_type());

	scene().SetRulerColor(ColorView::k3D);
	scene().SetBorderColor(ColorView::k3D);

	scene().SetMeasureReconType(common::ReconTypeID::VR);
}

ViewImplant3D::~ViewImplant3D()
{
	if (View::IsEnableGLContext())
	{
		View::MakeCurrent();
		ClearGL();
		View::DoneCurrent();
	}
}
#ifndef WILL3D_VIEWER
void ViewImplant3D::ExportProjectForMeasure3D(ProjectIOView& out)
{
	if (measure_3d_manager_)
	{
		measure_3d_manager_->ExportProject(out);
	}
}

void ViewImplant3D::ImportProjectForMeasure3D(ProjectIOView& in)
{
	if (!measure_3d_manager_)
	{
		measure_3d_manager_ = new Measure3DManager(QGraphicsView::scene());
	}
	measure_3d_manager_->ImportProject(in);
}
#endif
void ViewImplant3D::ResetVolume()
{
	if (View::IsEnableGLContext())
	{
		View::MakeCurrent();
		ClearGL();
		if (measure_3d_manager_)
		{
			measure_3d_manager_->ClearVAOVBO();
		}
		View::DoneCurrent();
	}
	SAFE_DELETE_OBJECT(measure_3d_manager_);

	View::SetViewEvent(EVIEW_EVENT_TYPE::UPDATE);
	ActiveControllerViewEvent();

	scene().update();
}

void ViewImplant3D::DeleteImplant(const int& implant_id)
{
	const auto& res_implant =
		ResourceContainer::GetInstance()->GetImplantResource();
	const auto& implant_datas = res_implant.data();

	auto iter = implant_specs_.find(implant_id);
	if (iter == implant_specs_.end()) return;

	implant_specs_[implant_id]->setVisible(false);
}

void ViewImplant3D::DeleteAllImplants()
{
	for (auto& spec : implant_specs_) scene().removeItem(spec.second.get());
	implant_specs_.clear();
}

void ViewImplant3D::UpdateImplantSpec()
{
	const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
	const auto& implant_datas = res_implant.data();
	if (implant_datas.empty())
	{
		return;
	}

	int selected_id = res_implant.selected_implant_id();

	// selected 가 아닌 Spec Text들의 visible 상태와 position을 결정
	this->UpdateNotSelectedImplantSpec(implant_datas, selected_id);

	// selected 인 spec text 및 handle의 visible 상태와 position을 결정
	this->UpdateSelectedImplantSpec(implant_datas, selected_id);
}

void ViewImplant3D::ChangeSelectedImplantSpec()
{
	const auto& res_implant =
		ResourceContainer::GetInstance()->GetImplantResource();
	const auto& implant_datas = res_implant.data();
	if (implant_datas.size() == 0) return;

	int selected_id = res_implant.selected_implant_id();
	if (selected_id < 0) return;

	auto iter = implant_datas.find(selected_id);
	if (iter != implant_datas.end())
	{
		implant_specs_[selected_id]->ChangeImplantSpec(iter->second.get());
	}
}

void ViewImplant3D::SyncBoneDensityRotateMatrix(const glm::mat4& rotate_mat)
{
	controller_->ForceRotateMatrix(rotate_mat);
	scene().SetWorldAxisDirection(controller_->GetRotateMatrix(),
		controller_->GetNavigatorViewMatrix());
}

void ViewImplant3D::UpdateVolume()
{
	this->RenderVolume();
	this->RenderForPickAxes();
	scene().update();
}

void ViewImplant3D::DeleteUnfinishedMeasure()
{
	if (measure_3d_manager_)
	{
		measure_3d_manager_->DeleteUnfinishedItem();

		scene().update();
	}

	View::DeleteUnfinishedMeasure();
}

void ViewImplant3D::SetCommonToolOnOff(
	const common::CommonToolTypeOnOff& type)
{
	if (type == common::CommonToolTypeOnOff::V_ZOOM_R)
	{
		return;
	}

	if (measure_3d_manager_)
	{
		measure_3d_manager_->SetType(type);
	}

	View::SetCommonToolOnOff(type);
}
const glm::mat4& ViewImplant3D::GetViewMatrix() const
{
	return controller_->GetViewMatrix();
}
const glm::mat4& ViewImplant3D::GetReorienMatrix() const
{
	return controller_->GetReorienMatrix();
}
const glm::mat4& ViewImplant3D::GetRotateMatrix() const
{
	return controller_->GetRotateMatrix();
}
glm::mat4 ViewImplant3D::GetProjectionViewMatrix() const
{
	return controller_->GetCollisionProjectionViewMatrix();
}
void ViewImplant3D::slotRotateMatrix(const glm::mat4& mat)
{
	controller_->ForceRotateMatrix(mat);

	scene().SetWorldAxisDirection(controller_->GetRotateMatrix(),
		controller_->GetNavigatorViewMatrix());

	this->RenderVolume();
	this->RenderForPickAxes();
	scene().update();

	emit sigRotated();
}

void ViewImplant3D::UpdateNotSelectedImplantSpec(
	const std::map<int, std::unique_ptr<ImplantData>>& implant_datas,
	int selected_id)
{
	// selected 가 아닌 Spec Text들의 visible 상태와 position을 결정
	for (const auto& elem : implant_datas)
	{
		if (implant_specs_.find(elem.first) == implant_specs_.end())
		{
			CreateImplantSpec(elem.second.get());
		}

		bool always_show_implant_id = GlobalPreferences::GetInstance()->preferences_.objects.implant.always_show_implant_id;
		if (always_show_implant_id)
		{
			implant_specs_[elem.first]->setVisible(IsTextVisible() && elem.second->is_visible());
		}
		else
		{
			implant_specs_[elem.first]->setVisible(false);
		}

		if (selected_id == elem.first) continue;

		QPointF pt_scene_specs;

		const glm::vec3& pt_vol = elem.second->position_in_vol();
		pt_scene_specs = controller_->MapVolToScene(pt_vol);

		implant_specs_[elem.first]->SetSelected(false, pt_scene_specs);
	}
}

void ViewImplant3D::UpdateSelectedImplantSpec(
	const std::map<int, std::unique_ptr<ImplantData>>& implant_datas,
	int selected_id)
{
	if (selected_id > 0)
	{
		auto iter = implant_datas.find(selected_id);
		if (iter != implant_datas.end())
		{
			implant_specs_[selected_id]->ChangeImplantSpec(iter->second.get());

			QPointF pt_scene_specs;

			const glm::vec3& pt_vol = iter->second->axis_point_in_vol();
			pt_scene_specs = controller_->MapVolToScene(pt_vol);

			implant_specs_[selected_id]->setPos(pt_scene_specs);
			const bool visible = IsTextVisible() && iter->second->is_visible();
			implant_specs_[selected_id]->SetSelected(visible);
			implant_specs_[selected_id]->setVisible(visible);
		}
		else
		{
			common::Logger::instance()->Print(
				common::LogType::ERR, "BaseViewPano::UpdateSelectedImplantSpec");
		}
	}
}
/*	무조건 만든다. 밖에서 std::map 중복 체크 하고 들어와야 한다. */
void ViewImplant3D::CreateImplantSpec(ImplantData* implant_data)
{
	if (!implant_data)
	{
		return;
	}

	int implant_id = implant_data->id();

	implant_specs_[implant_id].reset(new ImplantTextItem(implant_data));
	scene().addItem(implant_specs_[implant_id].get());
}
bool ViewImplant3D::IsEventImplantSelected() const
{
	return (controller_->is_selected_implant() && !View::IsSetTool()) ? true
		: false;
}
void ViewImplant3D::SelectImplant()
{
	View::MakeCurrent();
	{
		int selected_id = -1;
		controller_->SelectImplant(&selected_id);

		if (selected_id > 0)
		{
			emit sigSelectImplant(selected_id);
			UpdateImplantSpec();
		}
	}
	View::DoneCurrent();
}
void ViewImplant3D::UpdateImplantImages()
{
	if (controller_->IsPickImplant())
	{
		int pick_id = controller_->GetPickImplantID();

		if (pick_id > 0) emit sigUpdateImplantImages(pick_id);
	}
}
void ViewImplant3D::PickAndMoveImplant()
{
	bool is_need_render_vol = false;

	if (is_pressed())
	{
		glm::vec3 delta_translate;
		glm::vec3 rotate_axes;
		float delta_degree = 0.0f;
		int implant_id = -1;

		controller_->MoveImplant(&implant_id, &delta_translate, &rotate_axes,
			&delta_degree);

		if (delta_translate != glm::vec3())
		{
			emit sigTranslateImplant(implant_id, delta_translate);
			this->RenderForPickAxes();
			is_need_render_vol = true;
		}
		if (delta_degree != 0.0f)
		{
			emit sigRotateImplant(implant_id, rotate_axes, delta_degree);
			this->RenderForPickAxes();
			is_need_render_vol = true;
		}
	}
	else
	{
		View::MakeCurrent();
		controller_->PickAxesItem(&is_need_render_vol);
		View::DoneCurrent();
	}

	if (is_need_render_vol)
	{
		this->RenderVolume();
		scene().update();
	}
}

void ViewImplant3D::drawBackground(QPainter* painter, const QRectF& rect)
{
	View::drawBackground(painter, rect);

	if (!controller_->IsReady())
	{
		if (controller_->GetInvertWindow())
			painter->fillRect(rect, Qt::white);
		else
			painter->fillRect(rect, Qt::black);
		return;
	}
	painter->beginNativePainting();

	if (IsUpdateController())
	{
		controller_->RenderingVolume();
		UpdateDoneContoller();
	}

	controller_->RenderScreen(View::GetDefaultFrameBufferObject());

	if (measure_3d_manager_)
	{
		uint surface_program = CW3VREngine::GetInstance()->getPROGsurface();
		glUseProgram(surface_program);
		WGLSLprogram::setUniform(surface_program, "Light.Intensity", vec3(1.0f));
		vec4 lightPos = vec4(0.0f, -controller_->transform().cam_fov(), 0.0f, 1.0f);
		WGLSLprogram::setUniform(surface_program, "Light.Position",
			glm::lookAt(glm::vec3(0.0f, -controller_->transform().cam_fov(), 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)) * lightPos);
		glUseProgram(0);

		measure_3d_manager_->set_reorientation_matrix(controller_->transform().reorien());
		measure_3d_manager_->set_scale_matrix(controller_->transform().model());
		measure_3d_manager_->set_rotate_matrix(controller_->transform().rotate());
		measure_3d_manager_->set_projection_matrix(controller_->transform().projection());
		measure_3d_manager_->set_view_matrix(controller_->transform().view());
		measure_3d_manager_->Draw(surface_program);
	}

	painter->endNativePainting();
}

void ViewImplant3D::resizeEvent(QResizeEvent* pEvent)
{
	View::resizeEvent(pEvent);
	controller_->SetProjection();

	if (measure_3d_manager_)
	{
		QPointF scene_center = GetSceneCenterPos();
		QSizeF scene_size_in_view(scene_center.x(), scene_center.y());

		measure_3d_manager_->SetSceneSizeInView(scene_size_in_view);
	}

	UpdateImplantSpec();
}

void ViewImplant3D::mousePressEvent(QMouseEvent* event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	if (measure_3d_manager_ &&
		(tool_type() == CommonToolTypeOnOff::M_RULER ||
			tool_type() == CommonToolTypeOnOff::M_ANGLE ||
			tool_type() == CommonToolTypeOnOff::M_DEL))
	{
		MakeCurrent();
		vec3 volume_pos = controller_->MapSceneToVol(event->pos());
		bool volume_picked = volume_pos.x > 0.0f;
		DoneCurrent();

		if (volume_picked || tool_type() == CommonToolTypeOnOff::M_DEL)
		{
			bool update;
			measure_3d_manager_->MousePressEvent(event->button(), VolumeToGLVertex(volume_pos), update);

			if (update)
			{
				scene().update();
			}
		}
	}

	View::mousePressEvent(event);
}

void ViewImplant3D::mouseMoveEvent(QMouseEvent* event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	if (measure_3d_manager_)
	{
		if (!measure_3d_manager_->started() && event->buttons() == Qt::NoButton)
		{
			bool update = false;
			uint pick_program = CW3VREngine::GetInstance()->getPROGpickWithCoord();
			MakeCurrent();
			measure_3d_manager_->Pick(size(), event->pos(), &update, pick_program);
			DoneCurrent();

			if (update)
			{
				scene().update();
			}
		}
		else
		{
			MakeCurrent();
			vec3 volume_pos = controller_->MapSceneToVol(event->pos());
			bool volume_picked = volume_pos.x > 0.0f;
			DoneCurrent();

			if (volume_picked)
			{
				bool update;
				measure_3d_manager_->MouseMoveEvent(event->buttons(), VolumeToGLVertex(volume_pos), update);

				if (update)
				{
					scene().update();
				}
			}
		}
	}

	if (is_pressed()) View::SetRenderModeFast();

	View::mouseMoveEvent(event);

	if (IsEventRightButton(event)/* && !View::IsSetTool()*/)
	{
		View::SetViewEvent(EVIEW_EVENT_TYPE::ROTATE);
		scene().SetWorldAxisDirection(controller_->GetRotateMatrix(),
			controller_->GetNavigatorViewMatrix());
		emit sigRotated();
	}

	if (!IsEventRightButton(event) && IsEventImplantSelected())
	{
		PickAndMoveImplant();
	}
}

void ViewImplant3D::mouseReleaseEvent(QMouseEvent* event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		if (event->button() == Qt::RightButton)
		{
			emit sigShowButtonListDialog(event->globalPos());
			return;
		}
	}
#endif // WILL3D_EUROPE

	if (event->button() == Qt::LeftButton)
	{
		this->SelectImplant();
		this->UpdateImplantImages();
	}

	View::SetRenderModeQuality();
	this->RenderForPickAxes();

	View::mouseReleaseEvent(event);
}

void ViewImplant3D::wheelEvent(QWheelEvent* event)
{
	View::wheelEvent(event);
	View::ProcessZoomWheelEvent(event);
}

void ViewImplant3D::InitializeController()
{
	controller_->Initialize();

	if (controller_->IsReady()) controller_->RenderingVolume();

	scene().SetWorldAxisDirection(controller_->GetRotateMatrix(),
		controller_->GetNavigatorViewMatrix());

	QPointF scene_center = GetSceneCenterPos();
	QSizeF scene_size_in_view(scene_center.x(), scene_center.y());

	if (!measure_3d_manager_)
	{
		measure_3d_manager_ = new Measure3DManager(QGraphicsView::scene());
	}
	measure_3d_manager_->set_pixel_spacing(view_render_param()->base_pixel_spacing_mm());
	measure_3d_manager_->set_slice_thickness(view_render_param()->base_pixel_spacing_mm());
	measure_3d_manager_->SetSceneSizeInView(scene_size_in_view);
	measure_3d_manager_->SetType(tool_type());
}

bool ViewImplant3D::IsReadyController() { return controller_->IsReady(); }

void ViewImplant3D::TransformItems(const QTransform& transform)
{
	View::TransformItems(transform);
	for (auto& spec : implant_specs_)
	{
		spec.second->TransformItems(transform);
	}
}

void ViewImplant3D::ClearGL()
{
	controller_->ClearGL();
	if (measure_3d_manager_)
	{
		measure_3d_manager_->ClearVAOVBO();
	}
}

void ViewImplant3D::ActiveControllerViewEvent()
{
	EVIEW_EVENT_TYPE event_type = view_render_param()->event_type();

	bool need_render = false;
	controller_->ProcessViewEvent(&need_render);
	if (need_render)
	{
		this->RenderVolume();

#if 1
		if (event_type == UIViewController::ROTATE)
		{
			this->UpdateImplantSpec();
		}
		else if (event_type == UIViewController::UPDATE)
		{
			this->UpdateImplantSpec();
			this->RenderForPickAxes();
		}
#endif
	}
}

void ViewImplant3D::RenderVolume()
{
	if (!View::IsEnableGLContext()) return;

	View::MakeCurrent();
	controller_->RenderingVolume();
	View::DoneCurrent();
}

void ViewImplant3D::RenderForPickAxes()
{
	if (!View::IsEnableGLContext()) return;

	View::MakeCurrent();
	controller_->RenderForPickAxes();
	View::DoneCurrent();
}

void ViewImplant3D::HideMeasure(bool toggled)
{
	View::HideMeasure(toggled);

	if (measure_3d_manager_)
	{
		measure_3d_manager_->SetVisible(!toggled);
		scene().update();
	}
}

void ViewImplant3D::DeleteAllMeasure()
{
	View::DeleteAllMeasure();

	if (measure_3d_manager_)
	{
		measure_3d_manager_->Clear();
		scene().update();
	}
}

glm::vec3 ViewImplant3D::VolumeToGLVertex(glm::vec3 volume_pos)
{
	glm::vec3 gl_vertex_pos;

	glm::mat4 scale_matrix = controller_->transform().model();
	glm::vec3 volume_range(scale_matrix[0][0], scale_matrix[1][1], scale_matrix[2][2]);
	gl_vertex_pos = volume_pos / volume_range * 2.0f - 1.0f;
	gl_vertex_pos.x = -gl_vertex_pos.x;

	return gl_vertex_pos;
}

BaseViewController3D* ViewImplant3D::controller_3d()
{
	return (BaseViewController3D*)controller_.get();
}

void ViewImplant3D::keyPressEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->modifiers().testFlag(Qt::ControlModifier))
	{
		is_control_key_on_ = true;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	View3D::keyPressEvent(event);
}

void ViewImplant3D::keyReleaseEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->key() == Qt::Key_Control)
	{
		is_control_key_on_ = false;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	View3D::keyReleaseEvent(event);
}

void ViewImplant3D::SetWorldAxisDirection()
{
	scene().SetWorldAxisDirection(controller_->GetRotateMatrix(), controller_->GetNavigatorViewMatrix());
	emit sigRotated();
}

void ViewImplant3D::ApplyPreferences()
{
	controller_->ApplyPreferences();

	View3D::ApplyPreferences();
	UpdateImplantSpec();
}

void ViewImplant3D::HideAllUI(bool is_hide)
{
	View3D::HideAllUI(is_hide);
	UpdateImplantSpec();
}

void ViewImplant3D::Clip3DOnOff(const bool clip_on)
{
	controller_->ClipPanoArea(clip_on);

	UpdateVolume();
}
