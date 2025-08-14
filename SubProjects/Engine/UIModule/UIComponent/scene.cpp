#include "scene.h"

#include "../../Common/Common/W3Logger.h"
#include "../UIPrimitive/measure_tools.h"

namespace
{
	const glm::vec3 kTempPos = glm::vec3(0.0f, 0.0f, 0.0f);
}  // end of namespace

Scene::Scene(QObject* parent) : QGraphicsScene(parent)
{
	setSceneRect(-4000.0, -4000.0, 8000.0, 8000.0);

	viewitems_ = new Viewitems();

	connect(viewitems_, &Viewitems::sigRotateMatrix, this,
		&Scene::sigRotateMatrix);
	connect(viewitems_, &Viewitems::sigSliderValueChanged, this,
		&Scene::sigSliderValueChanged);
	connect(viewitems_, &Viewitems::sigActiveFilteredItem, this,
		&Scene::sigActiveFilteredItem);
	connect(viewitems_, &Viewitems::sigActiveSharpenItem, this,
		&Scene::sigActiveSharpenItem);

	ApplyPreferences();
}

Scene::~Scene() {}

void Scene::InitViewItem(const Viewitems::ItemType& type)
{
	viewitems_->InitItem(type, this);
}

void Scene::ViewEnableStatusChanged(const bool& view_enable)
{
	viewitems_->ViewEnableStatusChanged(view_enable);
	if (!view_enable) measure_tool_->DeleteAllMeasure();
}
void Scene::SetEnabledItem(const Viewitems::ItemType& type,
	const bool& is_enabled)
{
	viewitems_->SetEnabledItem(type, is_enabled);
}

void Scene::SetRulerColor(const QColor& color)
{
	viewitems_->SetRulerColor(color);
}

void Scene::SetBorderColor(const QColor& color)
{
	viewitems_->SetBorderColor(color);
}

void Scene::SetSliderTextColor(const QColor& color)
{
	viewitems_->SetSliderTextColor(color);
}

void Scene::HideAllUI(const bool& is_hide) { viewitems_->HideAllUI(is_hide); }

void Scene::HideText(const bool& is_hide) { viewitems_->HideText(is_hide); }

void Scene::SetWorldAxisDirection(const glm::mat4& rotate_mat,
	const glm::mat4& view_mat)
{
	viewitems_->SetWorldAxisDirection(rotate_mat, view_mat);
}

void Scene::SetSliderValue(const int& value)
{
	viewitems_->SetSliderValue(value);
}

void Scene::SetSliderRange(const int& min, const int& max)
{
	viewitems_->SetSliderRange(min, max);
}

void Scene::SetSliderInvertedAppearance(const bool& is_enable)
{
	viewitems_->SetSliderInvertedAppearance(is_enable);
}

void Scene::SetSliderInterval(const int& interval)
{
	viewitems_->SetSliderInterval(interval);
}

void Scene::SetViewRulerItem(const ViewRenderParam& param)
{
	viewitems_->SetViewRulerItem(param);
}

void Scene::SetGridItem(const ViewRenderParam& view_param)
{
	viewitems_->SetGridItem(view_param);
}

void Scene::AddFilteredItem(const QString& text)
{
	viewitems_->AddFilteredItem(text);
}

void Scene::ChangeFilteredItemText(const QString& text)
{
	viewitems_->ChangeFilteredItemText(text);
}

void Scene::ChangeSharpenLevel(const int level)
{
	viewitems_->ChangeSharpenLevel(level);
}

void Scene::SetDirectionTextItem(const QString& text,
	const bool& is_align_left)
{
	viewitems_->SetDirectionTextItem(text, is_align_left);
}

bool Scene::IsMouseOnViewItems() { return viewitems_->IsUnderMouse(); }

void Scene::SetHUValue(const QString& text) { viewitems_->SetHUValue(text); }

void Scene::SetGridOnOff(const bool& visible)
{
	viewitems_->SetGridOnOff(visible);
}

void Scene::GetSliceRange(int* min, int* max)
{
	viewitems_->GetSliderRange(min, max);
}
int Scene::GetSliderValue() const { return viewitems_->GetSliderValue(); }
void Scene::resizeEvent(const ViewRenderParam& param)
{
	viewitems_->resizeEvent(param);
	if (!measure_tool_) return;

	measure_tool_->SetCenter(param.scene_center());
	measure_tool_->SetScale(param.map_scene_to_gl() * 0.5f);
}

bool Scene::EditSliderValue(const int& delta)
{
	return viewitems_->EditSliderValue(delta);
}

void Scene::InitMeasure(const common::ViewTypeID& view_type, int view_sub_type)
{
	measure_tool_.reset(new MeasureTools(view_type, view_sub_type, this));
	measure_tool_->ApplyPreferences();

	connect(measure_tool_.get(), &MeasureTools::sigGetProfileData, this,
		&Scene::sigGetProfileData);
	connect(measure_tool_.get(), &MeasureTools::sigGetROIData, this,
		&Scene::sigGetROIData);

	if (view_type == common::ViewTypeID::CROSS_SECTION ||
		view_type == common::ViewTypeID::LIGHTBOX ||
		view_type == common::ViewTypeID::TMJ_FRONTAL_LEFT ||
		view_type == common::ViewTypeID::TMJ_FRONTAL_RIGHT ||
		view_type == common::ViewTypeID::TMJ_LATERAL_LEFT ||
		view_type == common::ViewTypeID::TMJ_LATERAL_RIGHT)
	{
		connect(measure_tool_.get(), &MeasureTools::sigMeasureCreated, this,
			&Scene::sigMeasureCreated);
		connect(measure_tool_.get(), &MeasureTools::sigMeasureDeleted, this,
			&Scene::sigMeasureDeleted);
		connect(measure_tool_.get(), &MeasureTools::sigMeasureModified, this,
			&Scene::sigMeasureModified);
	}
}

void Scene::UpdatePlaneInfo(const glm::vec3& vp_center, const glm::vec3& vp_up,
	const glm::vec3& vp_back)
{
	if (measure_tool_) measure_tool_->Update(vp_center, vp_up, vp_back);
}

void Scene::SetMeasureType(const common::CommonToolTypeOnOff& type)
{
	if (measure_tool_)
	{
		//measure_tool_->ClearUnfinishedItem();
		measure_tool_->SetMeasureType(type);
	}
}

void Scene::DeleteAllMeasure()
{
	if (measure_tool_) measure_tool_->DeleteAllMeasure();
}

void Scene::DeleteUnfinishedMeasure()
{
	if (measure_tool_) measure_tool_->ClearUnfinishedItem();
}

void Scene::SetMeasureReconType(const common::ReconTypeID& recon_type)
{
	if (measure_tool_) measure_tool_->SetViewRenderMode(recon_type);
}

void Scene::SetMeasureParams(const ViewRenderParam& view_param)
{
	if (!measure_tool_) return;

#if 0
	if (view_param.is_valid_map_scene_to_gl())
	{
		measure_tool_->Initialize(view_param.base_pixel_spacing_mm(),
			view_param.map_scene_to_gl());
	}
	else
	{
		measure_tool_->SetPixelSpacing(view_param.base_pixel_spacing_mm());
		measure_tool_->SetScale(0.5f * view_param.map_scene_to_gl());
	}
#else
	measure_tool_->SetSceneTrans(view_param.MapGLToScene(view_param.gl_trans()));
	measure_tool_->SetPixelSpacing(view_param.base_pixel_spacing_mm());
	measure_tool_->SetScale(0.5f * view_param.map_scene_to_gl());
	measure_tool_->SetZoomFactor(view_param.scene_scale());
#endif
}

void Scene::SetMeasureParamPixelSpacing(const float& pixel_spacing)
{
	if (measure_tool_) measure_tool_->SetPixelSpacing(pixel_spacing);
}

void Scene::SetMeasureParamScale(const float& scale)
{
	if (measure_tool_) measure_tool_->SetScale(scale);
}

void Scene::SetMeasureParamZoomFactor(const float& zoom_factor)
{
	if (measure_tool_)
	{
		measure_tool_->SetZoomFactor(zoom_factor);
	}
}

void Scene::SetMeasureParamTrans(const QPointF& trans)
{
	if (measure_tool_) measure_tool_->SetSceneTrans(trans);
}

bool Scene::IsMeasureInteractionAvailable(
	const common::CommonToolTypeOnOff& anno_type)
{
#if 0
	bool is_under_mouse_item = viewitems_->IsUnderMouse();
	bool is_measure_available =
		measure_tool_ && measure_tool_->IsMeasureInteractionAvailable(anno_type);
	return !is_under_mouse_item && is_measure_available;
#else
	if (measure_tool_)
	{
		return measure_tool_->IsMeasureInteractionAvailable(anno_type);
	}
	else
	{
		return false;
	}
#endif
}

void Scene::TransformMeasure(const QTransform& transform, const bool translate)
{
	if (measure_tool_) measure_tool_->TransformItems(transform, translate);
}

void Scene::HideMeasure(const bool& toggled)
{
	if (measure_tool_) measure_tool_->SetVisibleFromToolbar(!toggled);
}

void Scene::MeasureMousePress(const QPointF& scene_pos)
{
	if (measure_tool_) measure_tool_->ProcessMousePressed(scene_pos, kTempPos);
}

bool Scene::MeasureMouseMove(Qt::MouseButtons buttons,
	const common::CommonToolTypeOnOff& anno_type,
	const QPointF& scene_pos)
{
	if (IsMeasureInteractionAvailable(anno_type))
	{
		if (measure_tool_->IsDrawing())
		{
			measure_tool_->ProcessMouseMove(scene_pos, kTempPos);
			return true;
		}

		if (measure_tool_->IsSelected())
		{
			measure_tool_->ProcessMouseMove(scene_pos, kTempPos);
			return true;
		}
	}
	return false;
}

void Scene::MeasureMouseRelease(Qt::MouseButton button,
	const QPointF& scene_pos)
{
	if (measure_tool_)
		measure_tool_->ProcessMouseReleased(button, scene_pos, kTempPos);
}

void Scene::MeasureMouseDoubleClick(Qt::MouseButton button, const QPointF& scene_pos)
{
	if (measure_tool_)
		measure_tool_->ProcessMouseDoubleClick(button, scene_pos, kTempPos);
}

void Scene::ImportMeasureResource(const bool& is_update_resource)
{
	if (measure_tool_) measure_tool_->ImportMeasureResource(is_update_resource);
}

void Scene::SyncMeasureResourceSiblings(const bool& is_update_resource)
{
	if (!measure_tool_)
	{
		return;
	}

	measure_tool_->SyncMeasureResourceSiblings(is_update_resource);
}

void Scene::SyncMeasureResourceCounterparts(const bool& is_update_resource, const bool need_transform)
{
	if (!measure_tool_)
	{
		return;
	}

	measure_tool_->SyncMeasureResourceCounterparts(is_update_resource, need_transform);
}

void Scene::TransformMeasureForCounterparts()
{
	if (!measure_tool_)
	{
		return;
	}

	measure_tool_->TransformPointsForCounterparts();
}

void Scene::SyncCreateMeasureUI(const unsigned int& measure_id)
{
	if (measure_tool_) measure_tool_->SyncCreateMeasureUI(measure_id);
}

void Scene::SyncDeleteMeasureUI(const unsigned int& measure_id)
{
	if (measure_tool_) measure_tool_->SyncDeleteMeasureUI(measure_id);
}

void Scene::SyncModifyMeasureUI(const unsigned int& measure_id)
{
	if (measure_tool_) measure_tool_->SyncModifyMeasureUI(measure_id);
}

void Scene::GetMeasureParams(
	const common::ViewTypeID& view_type, const unsigned int& measure_id,
	common::measure::VisibilityParams* measure_params)
{
	if (measure_tool_)
		measure_tool_->GetMeasureParams(view_type, measure_id, measure_params);
}

void Scene::ApplyPreferences()
{
	if (measure_tool_) measure_tool_->ApplyPreferences();
	viewitems_->ApplyPreferences();
	update();
}

const bool Scene::IsTextVisible() const { return viewitems_->IsTextVisible(); }

const bool Scene::IsUIVisible() const { return viewitems_->IsUIVisible(); }

const bool Scene::IsMeasureSelected() const
{
	if (measure_tool_) return measure_tool_->IsSelected();

	return false;
}

const bool Scene::MeasureVisibleFromToolbar()
{
	if (!measure_tool_)
	{
		return false;
	}

	return measure_tool_->visible_from_toolbar();
}

void Scene::SetVisible(const bool visible)
{
	if (!measure_tool_)
	{
		return;
	}

	measure_tool_->SetVisibleFromView(visible);
}
