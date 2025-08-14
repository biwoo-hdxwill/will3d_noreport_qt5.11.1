#include "base_view_pano_cross_section.h"

#include <QDebug>
#include <QMouseEvent>
#include <QSettings>

#include <Engine/Common/Common/global_preferences.h>

#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/color_will3d.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/cross_section_resource.h"
#include "../../Resource/Resource/nerve_resource.h"
#include "../../Resource/Resource/implant_resource.h"

#include "../UIPrimitive/W3EllipseItem.h"
#include "../UIPrimitive/W3TextItem.h"
#include "../UIPrimitive/guide_line_list_item.h"
#include "../UIPrimitive/implant_text_item.h"

#include "../UIViewController/view_controller_slice.h"

#include "scene.h"

using namespace UIViewController;

namespace
{
	const int kLineID = 0;
}

BaseViewPanoCrossSection::BaseViewPanoCrossSection(int cross_section_id, QWidget* parent)
	: cross_section_id_(cross_section_id), View(common::ViewTypeID::CROSS_SECTION, parent)
{
	controller_slice_.reset(new ViewControllerSlice);
	controller_slice_->set_view_param(view_render_param());
	controller_slice_->SetVisibleImplant(true);
	controller_slice_->SetVisibleNerve(true);
	if (GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.move_axial_line_to_view_center)
	{
		controller_slice_->SetFitMode(BaseTransform::FitMode::COVER);
	}
		
	scene().InitViewItem(Viewitems::BORDER);
	scene().InitViewItem(Viewitems::RULER);
	scene().InitViewItem(Viewitems::HU_TEXT);
	scene().InitViewItem(Viewitems::DIRECTION_TEXT);
	scene().InitViewItem(Viewitems::SHARPEN_FILTER);
	scene().InitViewItem(Viewitems::GRID);

	scene().InitMeasure(view_type(), cross_section_id_ + 1);

#if 0
	connect(&scene(), &Scene::sigMeasureCreated,
		[=](const unsigned int& measure_id)
	{
		emit sigMeasureCreated(cross_section_id, measure_id);
	});
	connect(&scene(), &Scene::sigMeasureDeleted,
		[=](const unsigned int& measure_id)
	{
		emit sigMeasureDeleted(cross_section_id, measure_id);
	});
	connect(&scene(), &Scene::sigMeasureModified,
		[=](const unsigned int& measure_id)
	{
		emit sigMeasureModified(cross_section_id, measure_id);
	});
#else
	connect(&scene(), &Scene::sigMeasureCreated, this, &BaseViewPanoCrossSection::slotMeasureCreated);
	connect(&scene(), &Scene::sigMeasureDeleted, this, &BaseViewPanoCrossSection::slotMeasureDeleted);
	connect(&scene(), &Scene::sigMeasureModified, this, &BaseViewPanoCrossSection::slotMeasureModified);
#endif

	scene().SetBorderColor(ColorView::kCrossSection);
	scene().SetRulerColor(ColorView::kCrossSection);
	scene().SetDirectionTextItem(QString("B"), false);

	text_id_.reset(new CW3TextItem(false));
	text_id_->setTextColor(ColorView::kCrossSection);
	scene().addItem(text_id_.get());

	reference_pano_line_.reset(new GuideLineListItem(GuideLineListItem::VERTICAL));
	reference_pano_line_->setZValue(0);
	reference_pano_line_->SetHighlight(false);
	reference_pano_line_->set_pen(QPen(ColorPanoItem::kLienPenColor, 2.0, Qt::SolidLine, Qt::FlatCap));
	scene().addItem(reference_pano_line_.get());

	reference_axial_line_.reset(new GuideLineListItem(GuideLineListItem::HORIZONTAL));
	reference_axial_line_->setZValue(0);
	reference_axial_line_->SetHighlight(true);
	reference_axial_line_->set_pen(QPen(ColorAxialItem::kLinePenColor, 2.0, Qt::SolidLine, Qt::FlatCap));
	scene().addItem(reference_axial_line_.get());
}

BaseViewPanoCrossSection::~BaseViewPanoCrossSection()
{
	if (View::IsEnableGLContext())
	{
		View::MakeCurrent();
		controller_slice_->ClearGL();
		View::DoneCurrent();
	}
}

/**=================================================================================================
public functions
*===============================================================================================**/
/*
불리는 조건
1. Implant selection
2. 임플란트가 이동했을 때 Handle 과 Spec 위치를 업데이트
*/
void BaseViewPanoCrossSection::UpdateImplantHandleAndSpec()
{
	const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
	const auto& implant_datas = res_implant.data();

	int selected_id = res_implant.selected_implant_id();

	if (implant_datas.size() == 0)
	{
		DeleteImplantSpecs();
		return;
	}

	for (const auto& elem : implant_specs_)
	{
		elem.second->setVisible(false);
		elem.second->SetSelected(false);
	}
	
	bool always_show_implant_id = GlobalPreferences::GetInstance()->preferences_.objects.implant.always_show_implant_id;

	const std::map<int, bool>& visibility_map = controller_slice_->GetImplantVisibility();

	// selected 가 아닌 Spec Text들의 visible 상태와 position을 결정
	for (const auto& elem : implant_datas)
	{
		if (implant_specs_.find(elem.first) == implant_specs_.end())
		{
			CreateImplantSpec(elem.second.get());
		}
		else
		{
			implant_specs_[elem.first]->ChangeImplantSpec(elem.second.get());
		}

		if (selected_id == elem.first)
			continue;

		QPointF pt_scene = controller_slice()->MapVolToScene(elem.second->position_in_vol());
		implant_specs_[elem.first]->SetSelected(false, pt_scene);

		if (visibility_map.find(elem.first) == visibility_map.end())
		{
			continue;
		}

		if (always_show_implant_id && res_implant.add_implant_id() != elem.first)
		{
			bool visibility = visibility_map.at(elem.first);
			implant_specs_[elem.first]->setVisible(visibility && IsTextVisible() && elem.second->is_visible());
		}
		else
		{
			implant_specs_[elem.first]->setVisible(false);
		}
	}

	// selected 인 spec text 및 handle의 visible 상태와 position을 결정
	if (selected_id > 0)
	{
		auto iter = implant_datas.find(selected_id);
		if (iter != implant_datas.end())
		{
			QPointF pt_scene = controller_slice()->MapVolToScene(iter->second->axis_point_in_vol());
			pt_scene.setY(pt_scene.y());

			if (implant_specs_.find(selected_id) == implant_specs_.end())
			{
				CreateImplantSpec(iter->second.get());
			}
			else
			{
				implant_specs_[selected_id]->ChangeImplantSpec(iter->second.get());
			}

			implant_specs_[selected_id]->setPos(pt_scene);
			implant_specs_[selected_id]->SetSelected(IsTextVisible() && iter->second->is_visible());

			if (visibility_map.find(iter->first) != visibility_map.end())
			{
				bool visibility = visibility_map.at(iter->first);
				implant_specs_[selected_id]->setVisible(visibility && IsTextVisible() && iter->second->is_visible());
			}
		}
	}

	UpdateSelectedImplantHandle(implant_datas, selected_id);
}

void BaseViewPanoCrossSection::SetEnabledSharpenUI(const bool& is_enabled)
{
	scene().SetEnabledItem(Viewitems::SHARPEN_FILTER, is_enabled);
}

void BaseViewPanoCrossSection::SetSharpen(const SharpenLevel& level)
{
	scene().ChangeSharpenLevel(static_cast<int>(level));
	controller_slice_->SetSharpenLevel(level);
	RenderSlice();
	scene().update();
}

void BaseViewPanoCrossSection::SetPanoThickness(float thickness_mm)
{
	if (!isVisible()) return;

	QPen pen_pano_line(ColorPanoItem::kLienPenColor, 2.0, Qt::SolidLine,
		Qt::FlatCap);

	float scene_thickness = view_render_param()->MapActualToScene(thickness_mm);
	if (scene_thickness >= 2.0)
	{
		pen_pano_line.setWidthF(scene_thickness);
	}
	reference_pano_line_->set_pen(pen_pano_line);
	reference_pano_line_->UpdateLine(kLineID);
}

void BaseViewPanoCrossSection::SetAxialLine(const QPointF& axial_position_in_cross_plane)
{
	const auto& res_cross = ResourceContainer::GetInstance()->GetCrossSectionResource();

	if (&res_cross == nullptr || axial_position_in_cross_plane == QPointF())
	{
		reference_axial_line_->ClearLines();
		return;
	}

	double width = (double)res_cross.params().width;
	double height = (double)res_cross.params().height;

	QPointF axial_position_in_scene = controller_slice_->MapPlaneToScene(QPointF(width * 0.5, axial_position_in_cross_plane.y()));

	auto iter = reference_axial_line_->line_positions().find(kLineID);
	if (iter != reference_axial_line_->line_positions().end() && iter->second == axial_position_in_scene)
	{
		return;
	}

	QPointF line_left_top = controller_slice_->MapPlaneToScene(QPointF(0.0, 0.0));
	QPointF line_right_bottom = controller_slice_->MapPlaneToScene(QPointF(width, height));

	float len_width = (float)abs(line_right_bottom.x() - line_left_top.x());

	reference_axial_line_->SetRangeScene(line_left_top.y(), line_right_bottom.y());
	reference_axial_line_->set_length(len_width);
	reference_axial_line_->SetLine(kLineID, axial_position_in_scene, QVector2D(1.0, 0.0));

#if DEVELOP_MODE
	common::Logger::instance()->PrintDebugMode("BaseViewPanoCrossSection::SetAxialLine");
#endif
}

glm::vec3 BaseViewPanoCrossSection::GetUpVector() const
{
	return controller_slice_->GetUpVector();
}

void BaseViewPanoCrossSection::UpdateCrossSection()
{
	const auto& res_cross = ResourceContainer::GetInstance()->GetCrossSectionResource();
	if (&res_cross == nullptr)
	{
		controller_slice_->ClearPlane();

		text_id_->setVisible(false);
		scene().ViewEnableStatusChanged(false);
		reference_pano_line_->ClearLines();
		reference_axial_line_->ClearLines();
		return;
	}
	else
	{
		scene().ViewEnableStatusChanged(true);
	}

	SetCrossSectionPlane();
	RenderSlice();

	const auto& cross_data = res_cross.data();
	const auto& iter_data = cross_data.find(cross_section_id_);
	if (iter_data == cross_data.end()) return;

	UpdateMeasurePlaneInfo();
	UpdateImplantHandleAndSpec();

	text_id_->setPlainText(QString("%1").arg(iter_data->second->index()));
	text_id_->setVisible(IsTextVisible());

	double width = (double)res_cross.params().width;
	double height = (double)res_cross.params().height;

	QPointF line_left_top = controller_slice_->MapPlaneToScene(QPointF(0.0, 0.0));
	QPointF line_right_bottom = controller_slice_->MapPlaneToScene(QPointF(width, height));

	float len_height = (float)abs(line_right_bottom.ry() - line_left_top.ry());

	reference_pano_line_->SetRangeScene(line_left_top.x(), line_right_bottom.x());
	reference_pano_line_->set_length(len_height);
	reference_pano_line_->SetLine(kLineID, (line_left_top + line_right_bottom) * 0.5, QVector2D(0, 1));

	scene().SetDirectionTextItem("B", iter_data->second->flip());
	scene().update();

#if DEVELOP_MODE
	common::Logger::instance()->PrintDebugMode(
		"BaseViewPanoCrossSection::UpdatedCrossSection");
#endif
}
void BaseViewPanoCrossSection::RenderSlice()
{
	if (!View::IsEnableGLContext()) return;

	MakeCurrent();
	controller_slice_->RenderingSlice();
	DoneCurrent();
}
void BaseViewPanoCrossSection::HideAllUI(bool is_hide)
{
	View::HideAllUI(is_hide);
	text_id_->setVisible(!is_hide);
	reference_pano_line_->setVisible(!is_hide);
	reference_axial_line_->setVisible(!is_hide);

	UpdateImplantHandleAndSpec();
}

void BaseViewPanoCrossSection::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type)
{
	View::SetCommonToolOnOff(type);
}

/**=================================================================================================
protected functions
*===============================================================================================**/
void BaseViewPanoCrossSection::slotActiveSharpenItem(const int index)
{
	SharpenLevel level = static_cast<SharpenLevel>(index);

	controller_slice_->SetSharpenLevel(level);
	emit sigSetSharpen(level);
}
void BaseViewPanoCrossSection::TransformItems(const QTransform& transform)
{
	reference_pano_line_->TransformItems(transform);
	reference_axial_line_->TransformItems(transform);

	for (auto& spec : implant_specs_) spec.second->TransformItems(transform);
}

void BaseViewPanoCrossSection::leaveEvent(QEvent* event)
{
	View::leaveEvent(event);

	scene().SetEnabledItem(Viewitems::HU_TEXT, false);

	scene().SetBorderColor(ColorView::kCrossSection);
	scene().SetRulerColor(ColorView::kCrossSection);
	text_id_->setTextColor(ColorView::kCrossSection);

	emit sigHoverView(false);
}
void BaseViewPanoCrossSection::enterEvent(QEvent* event)
{
	View::enterEvent(event);

	if (!IsReadyController()) return;

	scene().SetEnabledItem(Viewitems::HU_TEXT, true);

	scene().SetBorderColor(ColorView::kCrossSectionSelected);
	scene().SetRulerColor(ColorView::kCrossSectionSelected);
	text_id_->setTextColor(ColorView::kCrossSectionSelected);
	emit sigHoverView(true);
}
void BaseViewPanoCrossSection::wheelEvent(QWheelEvent* event)
{
	View::wheelEvent(event);

	if (!IsReadyController()) return;

	GlobalPreferences::Direction direction = GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.mouse_wheel_direction;

	const int kSingleStep = 120;
	int degrees = (int)((event->delta()) / kSingleStep);

	if (direction == GlobalPreferences::Direction::Inverse)
	{
		degrees *= -1;
	}

	emit sigWheelDelta(degrees);
}

void BaseViewPanoCrossSection::keyPressEvent(QKeyEvent* event)
{
	View::keyPressEvent(event);
	if (!IsReadyController())
	{
		return;
	}

	const int kSingleStep = 1;
	if (event->key() == Qt::Key_Up)
	{
		emit sigWheelDelta(kSingleStep);
	}
	else if (event->key() == Qt::Key_Down)
	{
		emit sigWheelDelta(-kSingleStep);
	}
	else if (event->key() == Qt::Key_W)
	{
		controller_slice_->set_is_implant_wire(!controller_slice_->is_implant_wire());
		RenderSlice();
		scene().update();
	}
}

void BaseViewPanoCrossSection::resizeEvent(QResizeEvent* pEvent)
{
	View::resizeEvent(pEvent);

	QPointF scene_center = view_render_param()->scene_center();
	QSize scene_size = view_render_param()->scene_size();
	text_id_->setPos(scene_center.x() + (double)scene_size.width() / 2.0 - 40.0,
		scene_center.y() + (double)scene_size.height() / 2.0 - 40.0);

	controller_slice_->SetProjection();
}

void BaseViewPanoCrossSection::mouseMoveEvent(QMouseEvent* event)
{
	View::mouseMoveEvent(event);
	QPointF scene_pos = pt_scene_current();

	RequestDICOMInfo(scene_pos);

	if (IsEventAxialLineHovered() && IsEventLeftButton(event))
	{
		emit sigSetAxialSlice(cross_section_id_,
			controller_slice_->MapSceneToVol(scene_pos));
	}
}

void BaseViewPanoCrossSection::mousePressEvent(QMouseEvent* event)
{
	View::mousePressEvent(event);
}

void BaseViewPanoCrossSection::mouseReleaseEvent(QMouseEvent* event)
{
	View::mouseReleaseEvent(event);
}

void BaseViewPanoCrossSection::mouseDoubleClickEvent(QMouseEvent* event)
{
	View::mouseDoubleClickEvent(event);

	if (IsEventLeftButton(event) && tool_type() == common::CommonToolTypeOnOff::NONE)
	{
		maximize_ = !maximize_;
#if 0
		emit sigMaximize();
#endif
	}
}

/*
ResourceContainer 에 selection 변수가 변경되기 전에 불릴 수 있기 때문에
implant_id == res_implant.selected_implant_id() 대신 selected 변수를 받았다.
*/
QPointF BaseViewPanoCrossSection::GetImplantSpecPosition(
	const int& implant_id, const bool& selected) const
{
	const auto& res_implant =
		ResourceContainer::GetInstance()->GetImplantResource();

	glm::vec3 pt_vol = res_implant.data().at(implant_id)->axis_point_in_vol();
	QPointF pt_scene = controller_slice_->MapVolToScene(pt_vol);
	if (selected)
	{
		pt_scene.setY(pt_scene.y());
	}
	return pt_scene;
}

void BaseViewPanoCrossSection::UpdateImplantSpecPosition()
{
	if (implant_specs_.empty()) return;

	const auto& res_implant =
		ResourceContainer::GetInstance()->GetImplantResource();
	for (const auto& spec : implant_specs_)
	{
		spec.second->setPos(GetImplantSpecPosition(
			spec.first, spec.first == res_implant.selected_implant_id()));
	}

	// view implant pano only
	UpdateImplantHandlePosition();
}

void BaseViewPanoCrossSection::DeleteImplantSpec(const int & implant_id)
{
	auto iter = implant_specs_.find(implant_id);
	if (iter != implant_specs_.end())
	{
		scene().removeItem(iter->second.get());
		implant_specs_.erase(iter);
	}
}

void BaseViewPanoCrossSection::DeleteImplantSpecs()
{
	for (auto& spec : implant_specs_)
		scene().removeItem(spec.second.get());
	implant_specs_.clear();
}

bool BaseViewPanoCrossSection::IsInCrossSectionPlane(const int& x,
	const int& y) const
{
	const auto& res_cross =
		ResourceContainer::GetInstance()->GetCrossSectionResource();

	if (&res_cross == nullptr)
	{
		return false;
	}

	int width = static_cast<int>(res_cross.params().width);
	int height = static_cast<int>(res_cross.params().height);

	if (x >= 0 && x < width && y >= 0 && y < height)
		return true;
	else
		return false;
}

/**=================================================================================================
private functions
*===============================================================================================**/
// Counterpart tab 이 없는 상태에서 불림
void BaseViewPanoCrossSection::SetGraphicsItems()
{
	View::SetGraphicsItems();
	bool is_update_needed = false;
	emit sigMeasureResourceUpdateNeeded(cross_section_id_, is_update_needed);
	SyncMeasureResourceCounterparts(is_update_needed, cross_section_id_ == 0);
}

void BaseViewPanoCrossSection::InitializeController()
{
	controller_slice_->Initialize();
}

bool BaseViewPanoCrossSection::IsReadyController()
{
	return controller_slice_->IsReady();
}

void BaseViewPanoCrossSection::ClearGL() { controller_slice_->ClearGL(); }

void BaseViewPanoCrossSection::ActiveControllerViewEvent()
{
	bool need_render = false;
	controller_slice_->ProcessViewEvent(&need_render);
	if (need_render)
	{
		RenderSlice();
	}
}

void BaseViewPanoCrossSection::drawBackground(QPainter* painter,
	const QRectF& rect)
{
	View::drawBackground(painter, rect);

	if (!IsReadyController())
	{
		if (controller_slice_->GetInvertWindow())
			painter->fillRect(rect, Qt::white);
		else
			painter->fillRect(rect, Qt::black);
		return;
	}

	painter->beginNativePainting();
	if (IsUpdateController())
	{
		this->SetCrossSectionPlane();
		controller_slice_->RenderingSlice();
		UpdateDoneContoller();
	}

	RenderScreen();
	painter->endNativePainting();
}

/*	무조건 만든다. 밖에서 std::map 중복 체크 하고 들어와야 한다. */
void BaseViewPanoCrossSection::CreateImplantSpec(ImplantData* implant_data)
{
	if (!implant_data)
	{
		return;
	}

	int implant_id = implant_data->id();

	implant_specs_[implant_id].reset(new ImplantTextItem(implant_data));
	//implant_specs_[implant_id]->setZValue(10.0);
	scene().addItem(implant_specs_[implant_id].get());
}

void BaseViewPanoCrossSection::RenderScreen()
{
	controller_slice_->RenderScreen(GetDefaultFrameBufferObject());
}

void BaseViewPanoCrossSection::SetCrossSectionPlane()
{
	const auto& res_cross = ResourceContainer::GetInstance()->GetCrossSectionResource();
	if (&res_cross == nullptr)
	{
		controller_slice_->ClearPlane();
		return;
	}

	const auto& cross_datas = res_cross.data();
	const auto& iter_data = cross_datas.find(cross_section_id_);
	if (iter_data == cross_datas.end())
	{
		controller_slice_->ClearPlane();
		return;
	}

	const auto& cross_data = iter_data->second;

	const glm::vec3& up_vector = cross_data->up_vector();
	const glm::vec3 back_vector = glm::normalize(cross_data->back_vector());
	const glm::vec3 right_vector =
		glm::normalize(glm::cross(back_vector, up_vector));
	const auto& cross_params = res_cross.params();

	controller_slice_->SetPlane(cross_data->center_position_in_vol(),
		right_vector * (float)cross_params.width,
		back_vector * (float)cross_params.height,
		(int)cross_params.thickness);

	scene().SetViewRulerItem(*(view_render_param()));
	scene().SetGridItem(*(view_render_param()));
	scene().SetMeasureParams(*(view_render_param()));
}

void BaseViewPanoCrossSection::UpdateMeasurePlaneInfo()
{
	const auto& res_cross =
		ResourceContainer::GetInstance()->GetCrossSectionResource();
	if (&res_cross == nullptr) return;

	const auto& cross_data = res_cross.data();
	const auto& iter_data = cross_data.find(cross_section_id_);

	if (iter_data == cross_data.end()) return;

	scene().UpdatePlaneInfo(iter_data->second->center_position_in_vol(),
		iter_data->second->up_vector(),
		iter_data->second->back_vector());
}

void BaseViewPanoCrossSection::RequestDICOMInfo(const QPointF& pt_scene)
{
	glm::vec4 vol_info = controller_slice_->GetDicomInfoPoint(pt_scene);
	DisplayDICOMInfo(vol_info);
}

void BaseViewPanoCrossSection::DisplayDICOMInfo(const glm::vec4& vol_info)
{
	float window_width, window_level;
	controller_slice_->GetWindowParams(&window_width, &window_level);
	int ww = static_cast<int>(window_width);
	int wl = static_cast<int>(window_level + controller_slice_->GetIntercept());
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

bool BaseViewPanoCrossSection::IsEventAxialLineHovered() const
{
	return (reference_axial_line_->is_highlight() && !View::IsSetTool()) ? true
		: false;
}

/**=================================================================================================
private slots
*===============================================================================================**/
void BaseViewPanoCrossSection::slotGetProfileData(const QPointF& start_pt_scene,
	const QPointF& end_pt_scene,
	std::vector<short>& data)
{
	controller_slice_->GetDicomHULine(start_pt_scene, end_pt_scene, data);
}

void BaseViewPanoCrossSection::slotGetROIData(const QPointF& start_pt_scene,
	const QPointF& end_pt_scene,
	std::vector<short>& data)
{
	controller_slice_->GetDicomHURect(start_pt_scene, end_pt_scene, data);
}

void BaseViewPanoCrossSection::slotMeasureCreated(const unsigned int& measure_id)
{
	emit sigMeasureCreated(cross_section_id_, measure_id);
}

void BaseViewPanoCrossSection::slotMeasureDeleted(const unsigned int& measure_id)
{
	emit sigMeasureDeleted(cross_section_id_, measure_id);
}

void BaseViewPanoCrossSection::slotMeasureModified(const unsigned int& measure_id)
{
	emit sigMeasureModified(cross_section_id_, measure_id);
}

void BaseViewPanoCrossSection::ApplyPreferences()
{
	controller_slice_->ApplyPreferences();
	View::ApplyPreferences();
	UpdateImplantHandleAndSpec();
}

void BaseViewPanoCrossSection::MoveAxialLineToViewCenter()
{
	QPointF center_scene_position = mapToScene(QPoint(width(), height()) * 0.5f);
	QPointF axial_line_position = reference_axial_line_->GetLineCenterPosition(kLineID);

	QPointF gl_translate = view_render_param()->gl_trans() + view_render_param()->MapSceneToGL(axial_line_position - center_scene_position);

	SetPanTranslate(QPointF(0.0f, gl_translate.y()));
}
