#include "base_view_pano.h"

#include <QDebug>
#include <qmath.h>
#include <QMouseEvent>

#include <Engine/Common/Common/global_preferences.h>
#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/color_will3d.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/cross_section_resource.h"
#include "../../Resource/Resource/implant_resource.h"
#include "../../Resource/Resource/nerve_resource.h"
#include "../../Resource/Resource/pano_resource.h"

#include "../UIPrimitive/W3ViewAlignTextItem.h"
#include "../UIPrimitive/guide_line_list_item.h"
#include "../UIPrimitive/implant_text_item.h"
#include "../UIPrimitive/pano_ruler_item.h"
#include "../UIViewController/view_controler_implant3d_pano.h"
#include "../UIViewController/view_controller_image.h"
#include "../UIViewController/view_controller_pano3d.h"

#include <Engine/UIModule/UIGLObjects/measure_3d_manager.h>
#include <Engine/UIModule/UIViewController/base_transform.h>
#include <Engine/Module/VREngine/W3VREngine.h>
#include <Engine/Common/GLfunctions/WGLSLprogram.h>

#include "scene.h"

using namespace UIViewController;

namespace
{
	const int kDefaultLineID = 0;
	const glm::mat4 kRotateVolToPanoCoordSys =
		glm::rotate(-(float)M_PI_2, glm::vec3(1.0f, 0.0f, 0.0f));
}  // namespace

BaseViewPano::BaseViewPano(QWidget* parent)
	: View3D(common::ViewTypeID::PANO, parent)
{
	scene().InitViewItem(Viewitems::RULER);
	scene().InitViewItem(Viewitems::SLIDER);
	scene().InitViewItem(Viewitems::FILTERED_TEXT);
	scene().InitViewItem(Viewitems::BORDER);
	scene().InitViewItem(Viewitems::HU_TEXT);
	scene().InitViewItem(Viewitems::NAVIGATION);
	scene().InitViewItem(Viewitems::DIRECTION_TEXT);
	scene().InitViewItem(Viewitems::SHARPEN_FILTER);
	scene().InitViewItem(Viewitems::ALIGN_TEXTS);
	scene().InitViewItem(Viewitems::GRID);

	scene().InitMeasure(view_type());

	scene().SetSliderInvertedAppearance(false);
	scene().SetRulerColor(ColorView::kPanorama);
	scene().SetBorderColor(ColorView::kPanorama);
	scene().SetSliderTextColor(ColorView::kPanorama);
	scene().SetEnabledItem(Viewitems::NAVIGATION, false);
	scene().SetEnabledItem(Viewitems::ALIGN_TEXTS, false);
	scene().SetDirectionTextItem(QString("R"), true);

	cross_line_.reset(new GuideLineListItem(GuideLineListItem::VERTICAL));
	cross_line_->setZValue(0);
	cross_line_->SetHighlight(true);
	cross_line_->SetMovable(true);
	cross_line_->set_pen(
		QPen(ColorCrossSectionItem::kNormal, 2.0, Qt::SolidLine, Qt::FlatCap));
	connect(cross_line_.get(), &GuideLineListItem::sigTranslateLines, this, &BaseViewPano::slotTranslateCrossSection);
	connect(cross_line_.get(), &GuideLineListItem::sigMouseReleased, this, &BaseViewPano::sigCrossSectionLineReleased);
	scene().addItem(cross_line_.get());

	reference_axial_line_.reset(
		new GuideLineListItem(GuideLineListItem::HORIZONTAL));
	reference_axial_line_->setZValue(5);
	reference_axial_line_->SetHighlight(true);
	reference_axial_line_->set_pen(
		QPen(ColorAxialItem::kLinePenColor, 2.0, Qt::SolidLine, Qt::FlatCap));
	scene().addItem(reference_axial_line_.get());

	reference_arch_line_.reset(
		new GuideLineListItem(GuideLineListItem::HORIZONTAL));
	reference_arch_line_->setZValue(0);
	reference_arch_line_->SetHighlight(false);
	reference_arch_line_->set_pen(
		QPen(ColorPanoItem::kLienPenColor, 2.0, Qt::SolidLine, Qt::FlatCap));
	scene().addItem(reference_arch_line_.get());

	pano_ruler_.reset(new PanoRulerItem);
	pano_ruler_->setZValue(0);
	scene().addItem(pano_ruler_.get());

	SetFilteredItems();
}

BaseViewPano::~BaseViewPano()
{
	if (View::IsEnableGLContext())
	{
		View::MakeCurrent();
		controller_image_->ClearGL();
		controller_3d_->ClearGL();
		if (measure_3d_manager_)
		{
			measure_3d_manager_->ClearVAOVBO();
		}
		View::DoneCurrent();
	}
	SAFE_DELETE_OBJECT(measure_3d_manager_);
}

/**=================================================================================================
public functions
*===============================================================================================**/
#ifndef WILL3D_VIEWER
void BaseViewPano::ExportProjectForMeasure3D(ProjectIOView& out)
{
	if (measure_3d_manager_)
	{
		measure_3d_manager_->ExportProject(out);
	}
}

void BaseViewPano::ImportProjectForMeasure3D(ProjectIOView& in)
{
	if (!measure_3d_manager_)
	{
		measure_3d_manager_ = new Measure3DManager(QGraphicsView::scene());
	}
	measure_3d_manager_->ImportProject(in);
}
#endif
void BaseViewPano::UpdateVR(bool is_high_quality)
{
	if (recon_type_ == RECON_3D)
	{
		if (is_high_quality)
			view_render_param()->SetRenderModeQuality();
		else
			view_render_param()->SetRenderModeFast();

		RenderPanoVolume();
		scene().update();
	}
}

void BaseViewPano::SetCliping(const std::vector<glm::vec4>& planes,
	bool is_enable)
{
	controller_3d_->SetCliping(planes, is_enable);

	if (ctrl_type_ == CTRL_3D)
	{
		this->RenderPanoVolume();
		scene().update();
	}
}

void BaseViewPano::DeleteUnfinishedMeasure()
{
	if (measure_3d_manager_)
	{
		measure_3d_manager_->DeleteUnfinishedItem();

		scene().update();
	}

	View::DeleteUnfinishedMeasure();
}

void BaseViewPano::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type)
{
#if 0
	if (type == common::CommonToolTypeOnOff::V_ZOOM_R &&
		recon_type_ == ReconType::RECON_3D)
	{
		return;
	}
#endif

	View::SetCommonToolOnOff(type);

	//if (ctrl_type_ == CTRL_3D && measure_3d_manager_)
	if (measure_3d_manager_)
	{
		measure_3d_manager_->SetType(type);
	}

	if (!isVisible()) return;

	if (type == common::CommonToolTypeOnOff::NONE)
	{
		reference_axial_line_->SetHighlight(true);
		cross_line_->SetHighlight(true);
		cross_line_->SetMovable(true);
	}
	else
	{
		reference_axial_line_->SetHighlight(false);
		cross_line_->SetHighlight(false);
		cross_line_->SetMovable(false);
	}
}

void BaseViewPano::SetHighlightCrossSection(int cross_id, bool is_highlight)
{
	if (is_highlight)
	{
		cross_line_->SetColorSpecificLine(ColorCrossSectionItem::kHighlight,
			cross_id);
		id_highlighted_cross_line_ = cross_id;
	}
	else
	{
		cross_line_->SetColorSpecificLine(ColorCrossSectionItem::kNormal, cross_id);
		id_highlighted_cross_line_ = -1;
	}
}

void BaseViewPano::ClearCrossSectionLine() { cross_line_->ClearLines(); }

// Cross section resource 를 이용하여 Cross seciton line들을 업데이트
void BaseViewPano::CrossSectionUpdated()
{
	if (recon_type_ == ReconType::RECON_3D) return;

	const auto& res_cross =
		ResourceContainer::GetInstance()->GetCrossSectionResource();

	if (ctrl_type_ != CTRL_IMAGE || !controller_image_->IsReady() ||
		!&res_cross)
	{
		cross_line_->ClearLines();
		return;
	}

	std::vector<QPointF> cross_position_center_in_pano_plane,
		cross_position_arch_in_pano_plane;
	const auto& cross_data = res_cross.data();
	for (const auto& elem : cross_data)
	{
		if (elem.second->is_init())
		{
			cross_position_center_in_pano_plane.push_back(
				elem.second->center_position_in_pano_plane());
			cross_position_arch_in_pano_plane.push_back(
				elem.second->arch_position_in_pano_plane());
		}
	}

	float thickness = res_cross.params().thickness;
	qreal thickness_in_scene =
		(qreal)view_render_param()->MapVolToScene(thickness);
	QPen pen_cross_section_line(ColorCrossSectionItem::kNormal, 2.0,
		Qt::SolidLine, Qt::FlatCap);
	if (thickness_in_scene >= 2.0)
		pen_cross_section_line.setWidthF(thickness_in_scene);
	cross_line_->set_pen(pen_cross_section_line);
	cross_line_->setVisible(IsUIVisible());

	std::vector<QPointF> cross_positions_in_scene, cross_arch_positions_in_scene;
	controller_image_->MapImageToScene(cross_position_center_in_pano_plane,
		cross_positions_in_scene);
	controller_image_->MapImageToScene(cross_position_arch_in_pano_plane,
		cross_arch_positions_in_scene);

	QPointF line_left_top = controller_image_->MapImageToScene(QPointF(0.0, 0.0));
	QPointF line_right_bottom = controller_image_->MapImageToScene(
		QPointF((double)controller_image_->GetImageWidth(),
		(double)controller_image_->GetImageHeight()));

	cross_line_->SetRangeScene(line_left_top.x(), line_right_bottom.x());
	cross_line_->set_length(
		(float)std::fabs(line_right_bottom.ry() - line_left_top.ry()));

	float radian = glm::radians(res_cross.params().degree);
	QVector2D rotated_direction(-sin(radian), cos(radian));
	for (int i = 0; i < cross_positions_in_scene.size(); i++)
		cross_line_->SetLine(i, cross_positions_in_scene[i], rotated_direction);

	for (int i = 0; i < cross_arch_positions_in_scene.size(); i++)
		cross_line_->SetRotatePointAtLine(i, cross_arch_positions_in_scene[i]);

	if (id_highlighted_cross_line_ >= 0)
		cross_line_->SetColorSpecificLine(ColorCrossSectionItem::kHighlight,
			id_highlighted_cross_line_);

#if DEVELOP_MODE
	common::Logger::instance()->PrintDebugMode(
		"BaseViewPano::UpdatedCrossSection");
#endif
}

// view manager 에서 호출하는 함수
void BaseViewPano::SetAxialLine(const QPointF& axial_position_in_pano_plane)
{
	if (axial_position_in_pano_plane == QPointF())
	{
		reference_axial_line_->ClearLines();
		return;
	}

	QPointF axial_position_in_scene = controller_image_->MapImageToScene(
		QPointF(controller_image_->GetImageWidth() / 2,
			axial_position_in_pano_plane.y()));

	this->SetAxialLineItem(axial_position_in_scene);
#if DEVELOP_MODE
	common::Logger::instance()->PrintDebugMode("BaseViewPano::SetAxialLine");
#endif
}

void BaseViewPano::SetArchLine(const QPointF& arch_position_in_pano_plane)
{
	if (arch_position_in_pano_plane == QPointF())
	{
		reference_arch_line_->ClearLines();
		return;
	}

	QPointF arch_position_in_scene = controller_image_->MapImageToScene(QPointF(
		controller_image_->GetImageWidth() / 2, arch_position_in_pano_plane.y()));

	this->SetArchLineItem(arch_position_in_scene);
#if DEVELOP_MODE
	common::Logger::instance()->PrintDebugMode("BaseViewPano::SetarchLine");
#endif
}

void BaseViewPano::SetEnableSlider(bool is_enable)
{
	scene().SetEnabledItem(Viewitems::SLIDER, is_enable);
}

void BaseViewPano::SetSliceRange(float min, float max)
{
	scene().SetSliderRange((int)min, (int)max);
}

void BaseViewPano::SetSliceNum(int num) { scene().SetSliderValue(num); }

int BaseViewPano::GetSliceNum() const { return scene().GetSliderValue(); }

void BaseViewPano::UpdatedPano()
{
	if (ctrl_type_ == CTRL_IMAGE)
	{
		this->SetPanoImage();
		this->UpdateMeasurePlaneInfo();
	}
	else
	{
		if (controller_3d_->IsReady())
		{
			controller_3d_->Initialize();
			this->RenderPanoVolume();
		}
	}

	if (controller_image_->IsImageSizeChanged())
	{
		this->SetPanoRuler();
		this->UpdateImplantHandleAndSpec();
	}

	scene().SetViewRulerItem(*(view_render_param()));
	scene().SetGridItem(*(view_render_param()));
	scene().SetMeasureParams(*(view_render_param()));
	scene().update();
#if DEVELOP_MODE
	common::Logger::instance()->PrintDebugMode("BaseViewPano::UpdatedPano");
#endif
}

/*
ResourceContainer 에 selection 변수가 변경되기 전에 불릴 수 있기 때문에
implant_id == res_implant.selected_implant_id() 대신 selected 변수를 받았다.
*/
QPointF BaseViewPano::GetImplantSpecPosition(const int& implant_id,
	const bool& selected) const
{
	const auto& res_implant =
		ResourceContainer::GetInstance()->GetImplantResource();
	glm::vec3 pt_pano_plane_3d;

	switch (recon_type_)
	{
	case BaseViewPano::RECON_MPR:
		if (selected)
		{
			pt_pano_plane_3d =
				res_implant.data().at(implant_id)->axis_point_in_pano_plane();
		}
		else
		{
			pt_pano_plane_3d =
				res_implant.data().at(implant_id)->position_in_pano_plane();
		}
		return controller_image_->MapImageToScene(
			QPointF(pt_pano_plane_3d.x, pt_pano_plane_3d.y));
	case BaseViewPano::RECON_XRAY:
		if (selected)
		{
			pt_pano_plane_3d =
				res_implant.data().at(implant_id)->axis_point_in_pano();
		}
		else
		{
			pt_pano_plane_3d =
				res_implant.data().at(implant_id)->position_in_pano();
		}
		return controller_image_->MapImageToScene(
			QPointF(pt_pano_plane_3d.x, pt_pano_plane_3d.y));
	case BaseViewPano::RECON_3D:
		if (selected)
		{
			pt_pano_plane_3d =
				res_implant.data().at(implant_id)->axis_point_in_pano();
		}
		else
		{
			pt_pano_plane_3d =
				res_implant.data().at(implant_id)->position_in_pano();
		}
		return controller_3d_->MapVolToScene(pt_pano_plane_3d);
	default:
		assert(false);
		return QPointF();
		break;
	}
}

void BaseViewPano::SetControllerImage(ViewControllerImage* controller)
{
	controller_image_.reset(controller);
	controller_image_->set_view_param(view_render_param());
	if (GlobalPreferences::GetInstance()->preferences_.advanced.panorama_view.move_axial_line_to_view_center)
	{
		controller_image_->SetFitMode(BaseTransform::FitMode::COVER);
	}
}

void BaseViewPano::SetController3D(ViewControllerPano3D* controller)
{
	controller_3d_.reset(new ViewControllerPano3D);
	controller_3d_->set_view_param(view_render_param());
	if (GlobalPreferences::GetInstance()->preferences_.advanced.panorama_view.move_axial_line_to_view_center)
	{
		controller_3d_->SetFitMode(BaseTransform::FitMode::COVER);
	}
}

void BaseViewPano::SetController3D(
	const std::shared_ptr<ViewControllerImplant3Dpano>& controller)
{
	controller_3d_ = std::static_pointer_cast<ViewControllerPano3D>(controller);
	controller_3d_->set_view_param(view_render_param());
	if (GlobalPreferences::GetInstance()->preferences_.advanced.panorama_view.move_axial_line_to_view_center)
	{
		controller_3d_->SetFitMode(BaseTransform::FitMode::COVER);
	}
}

void BaseViewPano::HideAllUI(bool is_hide)
{
	View::HideAllUI(is_hide);
	SetVisibleLineItems(!is_hide);
	UpdateImplantHandleAndSpec();
}

/**=================================================================================================
protected functions
*===============================================================================================**/
void BaseViewPano::slotActiveSharpenItem(const int index)
{
	controller_image_->SetSharpenLevel(static_cast<SharpenLevel>(index));
}
void BaseViewPano::slotActiveFilteredItem(const QString& text)
{
	if (!IsReadyController()) return;

	MakeCurrent();
	ClearGL();
	DoneCurrent();

	ControllerType prev_ctrl_type = ctrl_type_;
	common::ReconTypeID view_render_mode;
	if (text == filtered_texts_[RECON_MPR])
	{
		ctrl_type_ = CTRL_IMAGE;
		recon_type_ = RECON_MPR;
		view_render_mode = common::ReconTypeID::MPR;

		SetVisibleLineItems(IsUIVisible());
		scene().SetEnabledItem(Viewitems::SLIDER, true);
		scene().SetEnabledItem(Viewitems::HU_TEXT, true);
		scene().SetEnabledItem(Viewitems::NAVIGATION, false);
		scene().SetEnabledItem(Viewitems::DIRECTION_TEXT, true);
		scene().SetEnabledItem(Viewitems::SHARPEN_FILTER, true);
		scene().SetEnabledItem(Viewitems::ALIGN_TEXTS, false);
	}
	else if (text == filtered_texts_[RECON_XRAY])
	{
		ctrl_type_ = CTRL_IMAGE;
		recon_type_ = RECON_XRAY;
		view_render_mode = common::ReconTypeID::X_RAY;

		SetVisibleLineItems(false);
		scene().SetEnabledItem(Viewitems::SLIDER, false);
		scene().SetEnabledItem(Viewitems::HU_TEXT, true);
		scene().SetEnabledItem(Viewitems::NAVIGATION, false);
		scene().SetEnabledItem(Viewitems::DIRECTION_TEXT, true);
		scene().SetEnabledItem(Viewitems::SHARPEN_FILTER, true);
		scene().SetEnabledItem(Viewitems::ALIGN_TEXTS, false);
	}
	else if (text == filtered_texts_[RECON_3D])
	{
		ctrl_type_ = CTRL_3D;
		recon_type_ = RECON_3D;
		view_render_mode = common::ReconTypeID::VR;

		SetVisibleLineItems(false);
		scene().SetEnabledItem(Viewitems::SLIDER, false);
		scene().SetEnabledItem(Viewitems::HU_TEXT, false);
		scene().SetEnabledItem(Viewitems::NAVIGATION, true);
		scene().SetEnabledItem(Viewitems::DIRECTION_TEXT, false);
		scene().SetEnabledItem(Viewitems::SHARPEN_FILTER, false);
		scene().SetEnabledItem(Viewitems::ALIGN_TEXTS, true);
	}
	this->RestoreViewRenderParamByChangeController(prev_ctrl_type, ctrl_type_);
	scene().SetMeasureReconType(view_render_mode);
	QResizeEvent force_resize(this->size(), this->size());
	this->resizeEvent(&force_resize);
	// this->resize(this->geometry().width(), this->geometry().height());
	// scene().resizeEvent(*view_render_param().get());

	if (measure_3d_manager_)
	{
		measure_3d_manager_->SetVisible(ctrl_type_ == CTRL_3D && scene().MeasureVisibleFromToolbar());
	}

	if (recon_type_ == RECON_3D)
	{
		this->SetRenderModeQuality();

		if (!controller_3d_->IsReady())
		{
			emit sigReconPanoResource();
			UpdatedPano();
		}
	}
	else
	{
		emit sigReconPanoResource();
		UpdatedPano();
		CrossSectionUpdated();
	}

	UpdateImplantHandleAndSpec();

	emit sigReconTypeChanged();
}

/**=================================================================================================
protected functions
*===============================================================================================**/
void BaseViewPano::UpdateNotSelectedImplantSpec(const std::map<int, std::unique_ptr<ImplantData>>& implant_datas, int selected_id, int add_implant_id)
{
	for (const auto& spec : implant_specs_)
	{
		spec.second->setVisible(false);
	}

	if (implant_datas.empty())
	{
		return;
	}

	// selected 가 아닌 Spec Text들의 visible 상태와 position을 결정
	for (const auto& elem : implant_datas)
	{
		if (implant_specs_.find(elem.first) == implant_specs_.end())
		{
			CreateImplantSpec(elem.second.get());
			implant_specs_[elem.first]->setVisible(false);
		}
		else
		{
			implant_specs_[elem.first]->ChangeImplantSpec(elem.second.get());
		}

		if (add_implant_id == elem.first) continue;

		bool always_show_implant_id = GlobalPreferences::GetInstance()->preferences_.objects.implant.always_show_implant_id;
		if (always_show_implant_id)
		{
			implant_specs_[elem.first]->setVisible(IsTextVisible() && elem.second->is_visible());
		}

		if (selected_id == elem.first) continue;

		QPointF pt_scene_specs;
		switch (recon_type())
		{
		case ReconType::RECON_MPR:
		{
			const glm::vec3& pt_pano_plane = elem.second->position_in_pano_plane();
			pt_scene_specs = controller_image()->MapImageToScene(QPointF(pt_pano_plane.x, pt_pano_plane.y));
			break;
		}
		case ReconType::RECON_3D:
		{
			const glm::vec3& pt_pano = elem.second->position_in_pano();
			pt_scene_specs = controller_3d()->MapVolToScene(pt_pano);
			break;
		}
		case ReconType::RECON_XRAY:
		{
			const glm::vec3& pt_pano = elem.second->position_in_pano();
			pt_scene_specs = controller_image()->MapImageToScene(QPointF(pt_pano.x, pt_pano.y));
			break;
		}
		default:
			assert(false);
			break;
		}
		implant_specs_[elem.first]->SetSelected(false, pt_scene_specs);
	}
}

void BaseViewPano::UpdateSelectedImplantSpec(const std::map<int, std::unique_ptr<ImplantData>>& implant_datas, int selected_id)
{
	if (selected_id > 0)
	{
		auto iter = implant_datas.find(selected_id);
		if (iter == implant_datas.end())
		{
			return;
		}

		else
		{
			implant_specs_[selected_id]->ChangeImplantSpec(iter->second.get());
		}

		QPointF pt_scene_specs;
		switch (recon_type())
		{
		case ReconType::RECON_MPR:
		{
			glm::vec3 pt_pano_plane = iter->second->axis_point_in_pano_plane();
			pt_scene_specs = controller_image()->MapImageToScene(
				QPointF(pt_pano_plane.x, pt_pano_plane.y));
			break;
		}
		case ReconType::RECON_3D:
		{
			glm::vec3 pt_pano = iter->second->axis_point_in_pano();
			pt_scene_specs = controller_3d()->MapVolToScene(pt_pano);
			break;
		}
		case ReconType::RECON_XRAY:
		{
			glm::vec3 pt_pano = iter->second->axis_point_in_pano();
			pt_scene_specs = controller_image()->MapImageToScene(
				QPointF(pt_pano.x, pt_pano.y));
			break;
		}
		default:
			assert(false);
			break;
		}
		implant_specs()[selected_id]->setPos(pt_scene_specs);
		const bool visible = IsTextVisible() && iter->second->is_visible();
		implant_specs()[selected_id]->SetSelected(visible);
		implant_specs()[selected_id]->setVisible(visible);
	}
}

void BaseViewPano::ActiveControllerViewEvent()
{
	bool need_render = false;
	GetCurrentController().ProcessViewEvent(&need_render);

	if (need_render && ctrl_type_ == CTRL_3D)
	{
		RenderPanoVolume();
	}
	EVIEW_EVENT_TYPE event_type = view_render_param()->event_type();
	if (event_type == UIViewController::ROTATE)
	{
		UpdateImplantHandleAndSpec();
	}
}

void BaseViewPano::TransformItems(const QTransform& transform)
{
	if (ctrl_type_ == CTRL_IMAGE)
	{
		cross_line_->TransformItems(transform);
		reference_axial_line_->TransformItems(transform);
		reference_arch_line_->TransformItems(transform);
#if 0
		pano_ruler_->TransformItems(transform);
#else
		SetPanoRuler();
#endif
	}
	for (auto& spec : implant_specs_) spec.second->TransformItems(transform);
}

void BaseViewPano::resizeEvent(QResizeEvent* pEvent)
{
	View::resizeEvent(pEvent);

	if (!view_render_param() || !is_view_ready()) return;

	if (ctrl_type_ == ControllerType::CTRL_IMAGE)
	{
		controller_image_->SetProjection();
	}
	else if (ctrl_type_ == ControllerType::CTRL_3D)
	{
		controller_3d_->SetProjection();
	}

	if (measure_3d_manager_)
	{
		QPointF scene_center = GetSceneCenterPos();
		QSizeF scene_size_in_view(scene_center.x(), scene_center.y());

		measure_3d_manager_->SetSceneSizeInView(scene_size_in_view);
	}
}

void BaseViewPano::mouseMoveEvent(QMouseEvent* event)
{
	if (ctrl_type_ == CTRL_3D && measure_3d_manager_)
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
			vec3 volume_pos = controller_3d_->MapSceneToVol(event->pos());
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

	RequestDICOMInfo(mapToScene(event->pos()));

	if (is_pressed()) View::SetRenderModeFast();

	if (IsEventRightButton(event))
	{
		if (IsEventRotate3D())
		{
			View::SetViewEvent(EVIEW_EVENT_TYPE::ROTATE);
			scene().SetWorldAxisDirection(
				controller_3d_->GetRotateMatrix() * kRotateVolToPanoCoordSys,
				controller_3d_->GetViewMatrix());
		}
		else if (IsEventCrossSectionHovered())
		{
			this->MouseCrossSectionRotateEvent();
			is_measure_delete_event_ = false;
			is_current_measure_available_ = false;
		}
	}
	else if (IsEventLeftButton(event))
	{
		// sync가 되어 있어서 view에서 처리함.
		// axial slice가 움직이면 외부에서 setting 한다.
		if (IsEventAxialLineHovered())
		{
			QPointF pt_curr_pano_plane = controller_image_->MapSceneToImage(pt_scene_current());
			emit sigSetAxialSlice(ClampImagePosition(pt_curr_pano_plane));
		}
	}
	View::mouseMoveEvent(event);
}

void BaseViewPano::mousePressEvent(QMouseEvent* event)
{
	if (ctrl_type_ == CTRL_3D)
	{
		if (measure_3d_manager_ &&
			(tool_type() == CommonToolTypeOnOff::M_RULER ||
				tool_type() == CommonToolTypeOnOff::M_ANGLE ||
				tool_type() == CommonToolTypeOnOff::M_DEL))
		{
			MakeCurrent();
			vec3 volume_pos = controller_3d_->MapSceneToVol(event->pos());
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
	}

	if ((event->buttons() == Qt::LeftButton || event->buttons() == Qt::RightButton) && 
		IsEventCrossSectionHovered())
	{
		QGraphicsView::mousePressEvent(event);
	}
	else
		View::mousePressEvent(event);
	// View::MousePressEvent(event, !IsEventCrossSectionHovered());
}

void BaseViewPano::mouseReleaseEvent(QMouseEvent* event)
{
	if (ctrl_type_ == CTRL_3D)
	{
		View::SetRenderModeQuality();

		if (measure_3d_manager_ &&
			(tool_type() == CommonToolTypeOnOff::M_RULER ||
				tool_type() == CommonToolTypeOnOff::M_ANGLE ||
				tool_type() == CommonToolTypeOnOff::M_DEL))
		{
			MakeCurrent();
			vec3 volume_pos = controller_3d_->MapSceneToVol(event->pos());
			bool volume_picked = volume_pos.x > 0.0f;
			DoneCurrent();

			if (volume_picked)
			{
				bool update;
				measure_3d_manager_->MouseReleaseEvent(event->button(), VolumeToGLVertex(volume_pos), update);

				if (update)
				{
					scene().update();
				}
			}
		}
	}

	is_pressed_double_click_ = false;

	View::mouseReleaseEvent(event);
}

void BaseViewPano::mouseDoubleClickEvent(QMouseEvent* event)
{
	View::mouseDoubleClickEvent(event);

	is_pressed_double_click_ = true;
}

void BaseViewPano::wheelEvent(QWheelEvent* event)
{
	View::wheelEvent(event);

	if (recon_type_ == RECON_3D) View::ProcessZoomWheelEvent(event);
}

void BaseViewPano::leaveEvent(QEvent* event)
{
	View::leaveEvent(event);
	scene().SetEnabledItem(Viewitems::HU_TEXT, false);
}

void BaseViewPano::enterEvent(QEvent* event)
{
	View::enterEvent(event);
	if (recon_type_ == ReconType::RECON_MPR || recon_type_ == ReconType::RECON_XRAY)
	{
		const auto& res_pano = ResourceContainer::GetInstance()->GetPanoResource();
		if(&res_pano == nullptr && res_pano.GetPanoImageWeakPtr().lock().get() == nullptr);
		{
			return; 
		}

		if (res_pano.GetPanoImageWeakPtr().lock().get()->Width() != 0)
		{
			scene().SetEnabledItem(Viewitems::HU_TEXT, true);
		}
	}
}

bool BaseViewPano::IsEventRotate3D() const
{
	return (ctrl_type_ == CTRL_3D/* && !View::IsSetTool()*/) ? true : false;
}

bool BaseViewPano::IsEventCrossSectionHovered() const
{
	return ((recon_type_ == ReconType::RECON_MPR ||
		recon_type_ == ReconType::RECON_XRAY) && cross_line_->is_hovered() &&
		!View::IsSetTool())
		? true
		: false;
}

bool BaseViewPano::IsEventAxialLineHovered() const
{
	return ((recon_type_ == ReconType::RECON_MPR ||
		recon_type_ == ReconType::RECON_XRAY) && reference_axial_line_->is_hovered() &&
		!View::IsSetTool())
		? true
		: false;
}

void BaseViewPano::SetAxialLineItem(const QPointF& pt_scene)
{
	if (!controller_image_->IsReady())
	{
		reference_axial_line_->ClearLines();
		return;
	}

	QPointF line_left_top = controller_image_->MapImageToScene(QPointF(0.0, 0.0));
	QPointF line_right_bottom = controller_image_->MapImageToScene(
		QPointF((double)controller_image_->GetImageWidth(),
		(double)controller_image_->GetImageHeight()));

	reference_axial_line_->SetRangeScene(line_left_top.y(),
		line_right_bottom.y());
	reference_axial_line_->setVisible(IsUIVisible());
	reference_axial_line_->set_length(
		(float)abs(line_right_bottom.rx() - line_left_top.rx()));
	reference_axial_line_->SetLine(kDefaultLineID, pt_scene, QVector2D(1.0, 0.0));
}

void BaseViewPano::SetArchLineItem(const QPointF& pt_scene)
{
	if (!controller_image_->IsReady())
	{
		reference_arch_line_->ClearLines();
		return;
	}

	QPointF line_left_top = controller_image_->MapImageToScene(QPointF(0.0, 0.0));
	QPointF line_right_bottom = controller_image_->MapImageToScene(
		QPointF((double)controller_image_->GetImageWidth(),
		(double)controller_image_->GetImageHeight()));

	reference_arch_line_->SetRangeScene(line_left_top.y(), line_right_bottom.y());
	reference_arch_line_->setVisible(IsUIVisible());
	reference_arch_line_->set_length(
		(float)abs(line_right_bottom.rx() - line_left_top.rx()));
	reference_arch_line_->SetLine(kDefaultLineID, pt_scene, QVector2D(1.0, 0.0));
}

/*	무조건 만든다. 밖에서 std::map 중복 체크 하고 들어와야 한다. */
void BaseViewPano::CreateImplantSpec(ImplantData* implant_data)
{
	if (!implant_data)
	{
		return;
	}

	int implant_id = implant_data->id();

	implant_specs_[implant_id].reset(new ImplantTextItem(implant_data));
	scene().addItem(implant_specs_[implant_id].get());
}

/**=================================================================================================
private functions
*===============================================================================================**/
void BaseViewPano::SetGraphicsItems()
{
	View::SetGraphicsItems();
	SyncMeasureResourceCounterparts(true);
}

void BaseViewPano::InitializeController()
{
	controller_image_->Initialize();
	controller_3d_->Initialize();

	scene().SetWorldAxisDirection(
		controller_3d_->GetRotateMatrix() * kRotateVolToPanoCoordSys,
		controller_3d_->GetViewMatrix());

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

bool BaseViewPano::IsReadyController()
{
	return GetCurrentController().IsReady();
}

void BaseViewPano::ClearGL()
{
	controller_3d_->ClearGL();
	controller_image_->ClearGL();
	if (measure_3d_manager_)
	{
		measure_3d_manager_->ClearVAOVBO();
	}
}

void BaseViewPano::drawBackground(QPainter* painter, const QRectF& rect)
{
	if (!IsReadyController())
	{
		bool is_invert;
		switch (ctrl_type_)
		{
		case BaseViewPano::CTRL_IMAGE:
			is_invert = controller_image_->GetInvertWindow();
			if (IsUpdateController())
			{
				UpdatedPano();
			}

			break;
		case BaseViewPano::CTRL_3D:
			is_invert = controller_3d_->GetInvertWindow();

			painter->beginNativePainting();
			if (IsUpdateController() && controller_3d_->IsReady())
			{
				controller_3d_->RenderingVolume();
				UpdateDoneContoller();
			}
			painter->endNativePainting();

			break;
		default:
			assert(false);
			break;
		}

		if (is_invert)
			painter->fillRect(rect, Qt::white);
		else
			painter->fillRect(rect, Qt::black);

		return;
	}

	View::drawBackground(painter, rect);

	painter->beginNativePainting();
	RenderScreen();

	if (ctrl_type_ == CTRL_3D && measure_3d_manager_)
	{
		uint surface_program = CW3VREngine::GetInstance()->getPROGsurface();
		glUseProgram(surface_program);
		WGLSLprogram::setUniform(surface_program, "Light.Intensity", vec3(1.0f));
		vec4 lightPos = vec4(0.0f, -controller_3d_->transform().cam_fov(), 0.0f, 1.0f);
		WGLSLprogram::setUniform(surface_program, "Light.Position",
			glm::lookAt(glm::vec3(0.0f, -controller_3d_->transform().cam_fov(), 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)) * lightPos);
		glUseProgram(0);

		measure_3d_manager_->set_reorientation_matrix(controller_3d_->transform().reorien());
		measure_3d_manager_->set_scale_matrix(controller_3d_->transform().model());
		measure_3d_manager_->set_rotate_matrix(controller_3d_->transform().rotate());
		measure_3d_manager_->set_projection_matrix(controller_3d_->transform().projection());
		measure_3d_manager_->set_view_matrix(controller_3d_->transform().view());
		measure_3d_manager_->Draw(surface_program);
	}

	painter->endNativePainting();
}

void BaseViewPano::RenderPanoVolume()
{
	if (!View::IsEnableGLContext()) return;
	View::MakeCurrent();
	controller_3d_->RenderingVolume();
	View::DoneCurrent();
}

void BaseViewPano::MouseCrossSectionRotateEvent()
{
	int hovered_cross_id = cross_line_->GetHoveredLineID();
	if (hovered_cross_id < 0) return;

	QPointF line_rotate_pos =
		cross_line_->GetLineRotatePosition(hovered_cross_id);
	QVector2D prev_vec =
		QVector2D(view_render_param()->scene_mouse_prev() - line_rotate_pos)
		.normalized();
	QVector2D curr_vec =
		QVector2D(view_render_param()->scene_mouse_curr() - line_rotate_pos)
		.normalized();

	float delta_angle = (float)(std::asin(prev_vec.x() * curr_vec.y() -
		prev_vec.y() * curr_vec.x())) *
		(180 / M_PI);
	SetDisableMeasureEvent();
	emit sigRotateCrossSection(delta_angle);
}

void BaseViewPano::SetFilteredItems()
{
	filtered_texts_[RECON_MPR] = QString("MPR");
	filtered_texts_[RECON_XRAY] = QString("X-Ray");
	filtered_texts_[RECON_3D] = QString("3D");

	for (int i = 0; i < RECON_TYPE_END; i++)
	{
#ifndef _WIN64
		if (i == RECON_XRAY)
		{
			continue;
		}
#endif
		scene().AddFilteredItem(filtered_texts_[i]);
	}
}

void BaseViewPano::RenderScreen()
{
	if (ctrl_type_ == CTRL_IMAGE)
		controller_image_->RenderScreen(GetDefaultFrameBufferObject());
	else if (ctrl_type_ == CTRL_3D)
		controller_3d_->RenderScreen(GetDefaultFrameBufferObject());
}

void BaseViewPano::SetPanoImage()
{
	const auto& res_pano = ResourceContainer::GetInstance()->GetPanoResource();
	if (&res_pano == nullptr)
	{
		return;
	}
	const auto* image = res_pano.GetPanoImageWeakPtr().lock().get();
	if (image && image->Width() == 0)
	{
		scene().ViewEnableStatusChanged(false);
		this->SetVisibleLineItems(false);
	}
	else
	{
		scene().ViewEnableStatusChanged(true);
		this->SetVisibleLineItems(IsUIVisible());
	}
	controller_image_->SetImage(res_pano.GetPanoImageWeakPtr());
	controller_image_->SetNerveMask(res_pano.GetMaskNerveImageWeakPtr());
	controller_image_->SetImplantMask(res_pano.GetMaskimplantImageWeakPtr());
}

void BaseViewPano::UpdateMeasurePlaneInfo()
{
	const auto& res_pano = ResourceContainer::GetInstance()->GetPanoResource();

	if (&res_pano == nullptr || !res_pano.is_valid() ||
		res_pano.GetCurrentCurveData().GetCurveLength() == 0) return;

	const auto& curve_data = res_pano.GetCurrentCurveData();

	float center_x = (float)res_pano.GetPanoPlaneWidth() * 0.5f;
	float center_y = (float)res_pano.GetPanoPlaneHeight() * 0.5f;

	float shifted_value = res_pano.shifted_value();
	glm::vec3 pano_center = glm::vec3(center_x, center_y, shifted_value);
	scene().UpdatePlaneInfo(pano_center, glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	//glm::vec3 pano_center =
	//    curve_data.points()[center_x] + center_y * res_pano.back_vector();
	//
	//scene().UpdatePlaneInfo(pano_center, curve_data.up_vectors()[center_x],
	//                        res_pano.back_vector());
}

void BaseViewPano::SetPanoRuler()
{
	if (ctrl_type_ != CTRL_IMAGE) return;

	const auto& res_pano = ResourceContainer::GetInstance()->GetPanoResource();
	if (&res_pano == nullptr)
	{
		return;
	}

	const auto& ruler_index = res_pano.ruler_index();

	if (!ruler_index.is_set_)
	{
		pano_ruler_->DeleteRulerItem();
		return;
	}

	if (!res_pano.GetPanoImageWeakPtr().lock().get())
	{
		return;
	}

	auto FuncMapImageXtoSceneX = [&](int x) -> double
	{
		return (double)controller_image_->MapImageToScene(QPointF(x, 0)).x();
	};

	std::vector<double> large_gradation = {
		FuncMapImageXtoSceneX(ruler_index.idx_min_),
		FuncMapImageXtoSceneX(ruler_index.idx_max_),
		FuncMapImageXtoSceneX(ruler_index.idx_arch_front_) };

	std::vector<double> medium_gradation_in_scene;
	medium_gradation_in_scene.reserve(ruler_index.medium_gradation_.size());
	for (const auto& elem : ruler_index.medium_gradation_)
	{
		medium_gradation_in_scene.push_back(FuncMapImageXtoSceneX(elem));
	}

	std::vector<double> small_gradation_in_scene;
	small_gradation_in_scene.reserve(ruler_index.small_gradation_.size());
	for (const auto& elem : ruler_index.small_gradation_)
	{
		small_gradation_in_scene.push_back(FuncMapImageXtoSceneX(elem));
	}

#if 0
	QPointF pt_bottom = controller_image_->MapImageToScene(
		QPointF(0.0f, (double)controller_image_->GetImageHeight()));
#else
	QPointF pt_bottom(0.0f, height());
#endif
	QPointF ruler_position(0.0f, pt_bottom.y() - 20.0f);

	pano_ruler_->setVisible(IsUIVisible());
	pano_ruler_->set_pixel_spacing(res_pano.GetPanoImageWeakPtr().lock()->Pitch());
	pano_ruler_->SetRuler(
		ruler_index.idx_min_, ruler_index.idx_max_, ruler_index.idx_arch_front_, 
		large_gradation[0], large_gradation[1], large_gradation[2],
		ruler_index.medium_gradation_, ruler_index.small_gradation_,
		medium_gradation_in_scene, small_gradation_in_scene,
		ruler_position
	);
}

void BaseViewPano::SetVisibleLineItems(bool visible)
{
	bool line_visible = visible && recon_type_ == ReconType::RECON_MPR;
	cross_line_->setVisible(line_visible);
	reference_axial_line_->setVisible(line_visible);
	reference_arch_line_->setVisible(line_visible);
	pano_ruler_->setVisible(line_visible);
}

BaseViewController& BaseViewPano::GetCurrentController() const
{
	if (ctrl_type_ == CTRL_IMAGE)
	{
		return *(dynamic_cast<BaseViewController*>(controller_image_.get()));
	}
	else if (ctrl_type_ == CTRL_3D)
	{
		return *(dynamic_cast<BaseViewController*>(controller_3d_.get()));
	}
	else
	{
		assert(false);
		return *(dynamic_cast<BaseViewController*>(controller_image_.get()));
	}
}

void BaseViewPano::RestoreViewRenderParamByChangeController(ControllerType prev_ctrl_type, ControllerType curr_ctrl_type)
{
	if (prev_ctrl_type == curr_ctrl_type)
	{
		return;
	}

	ViewRenderParam prev_view_param = *(View::view_render_param().get());
	prev_view_param.set_is_set_viewport(false);
	prev_view_render_param_[prev_ctrl_type] = prev_view_param;

	if (prev_view_render_param_[curr_ctrl_type].is_set_viewport())
	{
		std::shared_ptr<ViewRenderParam> curr_view_param;
		curr_view_param.reset(new ViewRenderParam(prev_view_render_param_[curr_ctrl_type]));
		View::SetViewRenderParam(curr_view_param);
	}

	GetCurrentController().set_view_param(View::view_render_param());
}

void BaseViewPano::RequestDICOMInfo(const QPointF& pt_scene)
{
	if ((recon_type_ == ReconType::RECON_MPR ||
		recon_type_ == ReconType::RECON_XRAY) &&
		controller_image_->IsReady())
	{
		QPointF pt_pano_plane = controller_image_->MapSceneToImage(pt_scene);

		glm::vec4 vol_info;
		emit sigDisplayDICOMInfo(pt_pano_plane, vol_info);
		DisplayDICOMInfo(vol_info);
	}
}

void BaseViewPano::DisplayDICOMInfo(const glm::vec4& vol_info)
{
	float window_width, window_level;
	controller_image_->GetWindowParams(&window_width, &window_level);
	int ww = static_cast<int>(window_width);
	int wl = static_cast<int>(window_level + controller_image_->GetIntercept());
	if (vol_info.w != common::dicom::kInvalidHU)
	{
		scene().SetHUValue(QString("WL %1\nWW %2\n(%3, %4, %5), %6")
			.arg(wl)
			.arg(ww)
			.arg(vol_info.x)
			.arg(vol_info.y)
			.arg(vol_info.z)
			.arg(vol_info.w));
	}
	else
	{
		scene().SetHUValue(QString("WL %1\nWW %2\n(-, -, -)").arg(wl).arg(ww));
	}
}

QPointF BaseViewPano::ClampImagePosition(const QPointF& image_pos)
{
	QPointF clamp_pt;
	clamp_pt.setX(std::min(image_pos.x(),
		(double)(controller_image_->GetImageWidth() - 1)));
	clamp_pt.setX(std::max(clamp_pt.x(), 0.0));
	clamp_pt.setY(std::min(image_pos.y(),
		(double)(controller_image_->GetImageHeight() - 1)));
	clamp_pt.setY(std::max(clamp_pt.y(), 0.0));
	return clamp_pt;
}
/**=================================================================================================
private slots
*===============================================================================================**/
void BaseViewPano::slotChangedValueSlider(int value)
{
	emit sigTranslateZ(value);
}
void BaseViewPano::slotRotateMatrix(const glm::mat4& mat)
{
	glm::mat4 rotate;

	if (mat == UIPrimitive::kRotateA)
	{
		rotate = mat;
	}
	else if (mat == UIPrimitive::kRotateP)
	{
		rotate = glm::rotate(glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	else if (mat == UIPrimitive::kRotateS)
	{
		rotate = glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	}
	else if (mat == UIPrimitive::kRotateI)
	{
		rotate = glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	}
	else if (mat == UIPrimitive::kRotateL)
	{
		rotate = glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	else if (mat == UIPrimitive::kRotateR)
	{
		rotate = glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	controller_3d_->ForceRotateMatrix(rotate);

	scene().SetWorldAxisDirection(
		controller_3d_->GetRotateMatrix() * kRotateVolToPanoCoordSys,
		controller_3d_->GetViewMatrix());

	this->RenderPanoVolume();
	scene().update();
}

void BaseViewPano::slotTranslateCrossSection(const QPointF& trans)
{
	// 이동 주체가 GuideLineItem이기 때문에 내부에서 처리된 결과만 보냄.
	// cross-section은 이 뷰에서 관리되는 item이기 때문에
	float trans_x = view_render_param()->MapSceneToVol((float)trans.x());
	emit sigTranslatedCrossSection(trans_x);
}

void BaseViewPano::slotGetProfileData(const QPointF& start_pt_scene,
	const QPointF& end_pt_scene,
	std::vector<short>& data)
{
	QPointF start_pt_plane = controller_image_->MapSceneToImage(start_pt_scene);
	QPointF end_pt_plane = controller_image_->MapSceneToImage(end_pt_scene);
	emit sigGetProfileData(start_pt_plane, end_pt_plane, data);
}

void BaseViewPano::slotGetROIData(const QPointF& start_pt_scene,
	const QPointF& end_pt_scene,
	std::vector<short>& data)
{
	QPointF start_pt_plane = controller_image_->MapSceneToImage(start_pt_scene);
	QPointF end_pt_plane = controller_image_->MapSceneToImage(end_pt_scene);
	emit sigGetROIData(start_pt_plane, end_pt_plane, data);
}

glm::vec3 BaseViewPano::VolumeToGLVertex(glm::vec3 volume_pos)
{
	glm::vec3 gl_vertex_pos;

	glm::mat4 scale_matrix = controller_3d_->transform().model();
	glm::vec3 volume_range(scale_matrix[0][0], scale_matrix[1][1], scale_matrix[2][2]);
	gl_vertex_pos = volume_pos / volume_range * 2.0f - 1.0f;
	gl_vertex_pos.x = -gl_vertex_pos.x;

	return gl_vertex_pos;
}

void BaseViewPano::HideMeasure(bool toggled)
{
	View::HideMeasure(toggled);

	if (measure_3d_manager_)
	{
		measure_3d_manager_->SetVisible(!toggled && ctrl_type_ == CTRL_3D && scene().MeasureVisibleFromToolbar());

		scene().update();
	}
}

void BaseViewPano::DeleteAllMeasure()
{
	View::DeleteAllMeasure();

	if (measure_3d_manager_)
	{
#if 0
		measure_3d_manager_->SetType(Measure3DManager::Type::DELETE_ALL);
#else
		measure_3d_manager_->Clear();

		if (ctrl_type_ == CTRL_3D)
		{
			scene().update();
		}
#endif
	}
}

void BaseViewPano::ResetView()
{
	View::ResetView();

	slotRotateMatrix(glm::mat4());
}

BaseViewController3D* BaseViewPano::controller_3d()
{
	return (BaseViewController3D*)controller_3d_.get();
}

void BaseViewPano::keyPressEvent(QKeyEvent* event)
{
	if (ctrl_type_ == CTRL_3D)
	{
		View3D::keyPressEvent(event);
	}
	else if (ctrl_type_ == CTRL_IMAGE)
	{
		View::keyPressEvent(event);
		if (event->key() == Qt::Key_W)
		{
			emit sigToggleImplantRenderingType();
		}
	}
}

void BaseViewPano::SetWorldAxisDirection()
{
	scene().SetWorldAxisDirection(controller_3d()->GetRotateMatrix() * kRotateVolToPanoCoordSys, controller_3d()->GetViewMatrix());
}

void BaseViewPano::RotateOneDegree(RotateOneDegreeDirection direction)
{
	glm::vec3 rotate_axis(0.0f, 1.0f, 0.0f);

	if (direction == RotateOneDegreeDirection::UP ||
		direction == RotateOneDegreeDirection::DOWN)
	{
		rotate_axis = glm::vec3(1.0f, 0.0f, 0.0f);
	}

	rotate_axis = glm::vec3(-glm::vec4(rotate_axis, 0.0f) * kRotateVolToPanoCoordSys);

	if (direction == RotateOneDegreeDirection::LEFT ||
		direction == RotateOneDegreeDirection::UP)
	{
		controller_3d()->transform().Rotate(-1.0f, rotate_axis);
	}
	else if (direction == RotateOneDegreeDirection::RIGHT ||
		direction == RotateOneDegreeDirection::DOWN)
	{
		controller_3d()->transform().Rotate(1.0f, rotate_axis);
	}

	View::MakeCurrent();
	controller_3d()->RenderingVolume();
	View::DoneCurrent();

	SetWorldAxisDirection();

	SceneUpdate();
}

void BaseViewPano::DeleteImplant(int implant_id)
{
	auto iter = implant_specs().find(implant_id);
	if (iter != implant_specs().end())
	{
		scene().removeItem(iter->second.get());
		implant_specs().erase(iter);
	}
}

void BaseViewPano::DeleteAllImplants()
{
	for (auto& spec : implant_specs())
	{
		scene().removeItem(spec.second.get());
	}
	implant_specs().clear();
}

void BaseViewPano::ApplyPreferences()
{
	controller_3d_->ApplyPreferences();
	controller_image_->ApplyPreferences();
	UpdateImplantHandleAndSpec();

	View::ApplyPreferences();
}

void BaseViewPano::MoveAxialLineToViewCenter()
{
	QPointF center_scene_position = mapToScene(QPoint(width(), height()) * 0.5f);
	QPointF axial_line_position = reference_axial_line_->GetLineCenterPosition(kDefaultLineID);

	QPointF gl_translate = view_render_param()->gl_trans() + view_render_param()->MapSceneToGL(axial_line_position - center_scene_position);

	SetPanTranslate(QPointF(0.0f, gl_translate.y()));
}
