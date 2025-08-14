#include "base_view_pano_arch.h"

#include <QDebug>
#include <QMouseEvent>
#include <QOpenGLWidget>

#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/color_will3d.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/cross_section_resource.h"
#include "../../Resource/Resource/implant_resource.h"
#include "../../Resource/Resource/pano_resource.h"

#include "../UIPrimitive/implant_text_item.h"
#include "../UIPrimitive/pano_arch_item.h"

#include "../UIViewController/view_controller_slice.h"
#include "../UIViewController/view_render_param.h"
#include "../UIViewController/transform_slice.h"

#include "scene.h"

using namespace UIViewController;

BaseViewPanoArch::BaseViewPanoArch(QWidget* parent)
	: View(common::ViewTypeID::PANO_ARCH, parent) {
  controller_.reset(new ViewControllerSlice);
  controller_->set_view_param(View::view_render_param());
  controller_->SetVisibleImplant(true);
  controller_->SetVisibleNerve(true);

  for (int i = 0; i < ArchTypeID::ARCH_TYPE_END; i++) {
	arch_[i].reset(new PanoArchItem());
	arch_[i]->setZValue(5.0);
	connect(arch_[i].get(), SIGNAL(sigEndEdit()), this,
			SLOT(slotEndEditArch()));
	connect(arch_[i].get(), SIGNAL(sigUpdated()), this,
			SIGNAL(sigUpdatedArch()));
	connect(arch_[i].get(), SIGNAL(sigUpdatedFinish()), this,
			SIGNAL(sigUpdatedFinishArch()));
	connect(arch_[i].get(), SIGNAL(sigChangedArchRange()), this,
			SLOT(slotChangedArchRange()));
	scene().addItem(arch_[i].get());
  }

  scene().InitViewItem(Viewitems::RULER);
  scene().InitViewItem(Viewitems::SLIDER);
  scene().InitViewItem(Viewitems::FILTERED_TEXT);
  scene().InitViewItem(Viewitems::BORDER);
  scene().InitViewItem(Viewitems::HU_TEXT);
  scene().InitViewItem(Viewitems::GRID);
  scene().InitViewItem(Viewitems::NAVIGATION);
  scene().InitViewItem(Viewitems::DIRECTION_TEXT);
  scene().InitViewItem(Viewitems::SHARPEN_FILTER);
  scene().SetRulerColor(ColorView::kAxial);
  scene().SetBorderColor(ColorView::kAxial);
  scene().SetSliderTextColor(ColorView::kAxial);
  scene().SetDirectionTextItem(QString("R"), true);

  scene().InitMeasure(view_type());

  SetFilteredItems();

  connect(this, SIGNAL(sigProcessedZoomEvent(float)), this,
		  SLOT(slotUpdateImplantSpecPosition(float)));
}

BaseViewPanoArch::~BaseViewPanoArch() {
  if (View::IsEnableGLContext()) {
	View::MakeCurrent();
	ClearGL();
	View::DoneCurrent();
  }
}

/**=================================================================================================
public functions
*===============================================================================================**/

void BaseViewPanoArch::SetCurrentArchType(const ArchTypeID& type) {
  curr_arch_type_ = type;
#if 0
  scene().ChangeFilteredItemText(filtered_texts_[curr_arch_type_]);
#endif
  for (int i = 0; i < ARCH_TYPE_END; i++) {
	arch_[i]->setVisible(false);
  }

  arch()->setVisible(true);
}

void BaseViewPanoArch::SetPanoArchRange(float range_mm, bool is_emit_signal) {
  for (int i = 0; i < ARCH_TYPE_END; i++) {
	if (!is_emit_signal) arch_[i]->blockSignals(true);

	arch_[i]->SetPanoRange(view_render_param()->MapActualToScene(range_mm));

	if (!is_emit_signal) arch_[i]->blockSignals(false);
  }
}
void BaseViewPanoArch::SetPanoArchThickness(float thciness_mm,
											bool is_emit_signal) {
  for (int i = 0; i < ARCH_TYPE_END; i++) {
	if (!is_emit_signal) arch_[i]->blockSignals(true);

	arch_[i]->SetThickness(view_render_param()->MapActualToScene(thciness_mm));

	if (!is_emit_signal) arch_[i]->blockSignals(false);
  }
}
void BaseViewPanoArch::SetCommonToolOnOff(
	const common::CommonToolTypeOnOff& type) {
  View::SetCommonToolOnOff(type);

  if (!isVisible()) return;

  for (int i = 0; i < ARCH_TYPE_END; i++) {
	arch_[i]->SetHighlight((type == common::CommonToolTypeOnOff::NONE) ? true
																	   : false);
  }
}

void BaseViewPanoArch::HideAllUI(bool is_hide) {
  View::HideAllUI(is_hide);
  arch()->setVisible(!is_hide);
}

// Cross section resource 를 이용하여 Cross seciton line들을 업데이트
void BaseViewPanoArch::CrossSectionUpdated() 
{
  const auto& res_cross = ResourceContainer::GetInstance()->GetCrossSectionResource();

  std::vector<glm::vec3> cross_points_in_vol;
  std::vector<int> cross_index;
  if (&res_cross) 
  {
	const auto& cross_data = res_cross.data();

	for (const auto& elem : cross_data) 
	{
	  if (elem.second->is_init())
	  {
		glm::vec4 curr_plane = glm::vec4(controller_->GetUpVector(), glm::dot(controller_->GetUpVector(), GetCenterPosition()));

		glm::vec3 ray = elem.second->arch_position_in_vol() - elem.second->center_position_in_vol();
		glm::vec3 ray_o = elem.second->center_position_in_vol();
		float t = (curr_plane.w - glm::dot(glm::vec3(curr_plane), ray_o)) / glm::dot(glm::vec3(curr_plane), ray);

		cross_points_in_vol.push_back(elem.second->center_position_in_vol() + t * ray);
		cross_index.push_back(elem.second->index());
	  }
	}
  }

  std::vector<QPointF> cross_points_in_scene;
  controller_->MapVolToScene(cross_points_in_vol, cross_points_in_scene);

  int cross_len = 0;
  float cross_thickness = 0.0f;
  if (&res_cross) 
  {
	cross_len = view_render_param()->MapVolToScene(res_cross.params().width);
	cross_thickness = view_render_param()->MapVolToScene(res_cross.params().thickness);
  }

  arch()->DrawCrossSectionLine(cross_points_in_scene, cross_index, cross_len, cross_thickness);
  scene().update();

#if DEVELOP_MODE
  common::Logger::instance()->PrintDebugMode("BaseViewPanoArch::UpdatedCrossSection");
#endif
}

void BaseViewPanoArch::UpdatedPanoRuler() {
  const auto& res_pano = ResourceContainer::GetInstance()->GetPanoResource();
  const auto& ruler_index = res_pano.ruler_index();
  arch()->DrawRuler(ruler_index.idx_min_, ruler_index.idx_max_,
					ruler_index.idx_arch_front_, ruler_index.medium_gradation_,
					ruler_index.small_gradation_);

#if DEVELOP_MODE
  common::Logger::instance()->PrintDebugMode(
	  "BaseViewPanoArch::UpdatedPanoRuler");
#endif
}

void BaseViewPanoArch::UpdateSlice() {
  this->RenderSlice();
  scene().update();
}

void BaseViewPanoArch::SetHighlightCrossSection(int cross_id,
												bool is_highlight) {
  arch()->SetHighlightCrossSection(cross_id, is_highlight);
}

void BaseViewPanoArch::SetShiftedArch(float z_pos_vol) {
  float scene_shifted_value = view_render_param()->MapVolToScene(z_pos_vol);
  if (arch()->current_spline_value() == scene_shifted_value) return;

  arch()->SetShfitedPath(scene_shifted_value);

  emit sigShiftedArch(z_pos_vol);
}

void BaseViewPanoArch::ForceRotateMatrix(const glm::mat4& mat) {
  controller_->ForceRotateMatrix(mat);

  scene().SetWorldAxisDirection(controller_->GetRotateMatrix(),
								controller_->GetViewMatrix());

  RenderSlice();
  scene().update();
}

// pano arch에 signal 이 발생하여 모든 뷰들의 axial line들을 일괄 이동한다.
void BaseViewPanoArch::SetSliceInVol(float z_pos_vol) {
  scene().SetSliderValue(static_cast<int>(z_pos_vol));
}

void BaseViewPanoArch::SetArchSliceNumber(const ArchTypeID& type)
{
	scene().SetSliderValue(drawn_arch_slice_[type]);
}

void BaseViewPanoArch::InitSliceRange(float min, float max, float value) {
  scene().SetSliderRange(static_cast<int>(min), static_cast<int>(max));
  scene().SetSliderValue(static_cast<int>(value));
}

void BaseViewPanoArch::SetSliceRange(float min, float max) {
  scene().SetSliderRange((int)min, (int)max);
}

void BaseViewPanoArch::SetArchPointsInVol(const std::vector<glm::vec3>& points)
{
	SetArchPointsInVol(points, curr_arch_type_);
}

void BaseViewPanoArch::SetArchPointsInVol(const std::vector<glm::vec3>& points, const ArchTypeID& type)
{
	// sigDeletedArch 시그널을 보내면 인자로 받는 points가 지워지기 때문에
	// 복사해놓는다.
	std::vector<glm::vec3> ctrl_points;
	ctrl_points.assign(points.begin(), points.end());

	// 있으면 지워라
	if (arch_[type]->IsFinished())
	{
		emit sigDeletedArch();
	}

	arch_[type]->Clear();

	if (!points.empty())
	{
#if 0
		SetSliceInVol(glm::dot(controller_->GetUpVector(),
			points.front() - GetCenterPosition()) +
			scene().GetSliderValue());
#else
		//SetSliceInVol(points.front().z);
#endif
	}

	for (const auto& elem : ctrl_points)
	{
		QPointF scene_pos = controller_->MapVolToScene(elem);
#if 0
		glm::vec3 vol_pos = controller_->MapSceneToVol(scene_pos);
		qDebug() << "vol to scene :" << elem.x << elem.y << elem.z << "/" << scene_pos << "/" << vol_pos.x << vol_pos.y << vol_pos.z;
#endif
		AddPointArch(scene_pos, type);
	}

	EndEditArch();
}

void BaseViewPanoArch::GetArchPointsInVol(
	std::vector<glm::vec3>& arch_points_in_vol) const {
  const std::vector<QPointF>& arch_points_in_scene = arch()->GetPoints();
  if (arch_points_in_scene.size() == 0) return;

  arch_points_in_vol.reserve(arch_points_in_scene.size());
  for (const QPointF& ptScene : arch_points_in_scene) {
	arch_points_in_vol.push_back(controller_->MapSceneToVol(ptScene));
  }
}

void BaseViewPanoArch::GetArchCtrlPointsInVol(
	std::vector<glm::vec3>& arch_ctrl_points_in_vol) const
{
	const std::vector<QPointF>& arch_points_in_scene = arch()->GetCtrlPoints();
	if (arch_points_in_scene.size() == 0) return;

	float forward = drawn_arch_slice_[curr_arch_type_] - scene().GetSliderValue();
	arch_ctrl_points_in_vol.reserve(arch_points_in_scene.size());
	for (const QPointF& ptScene : arch_points_in_scene)
	{
		glm::vec3 vol_pos = controller_->MapSceneToVol(ptScene);
#if 0
		QPointF scene_pos = controller_->MapVolToScene(vol_pos);
		qDebug() << "scene to vol :" << ptScene << "/" << vol_pos.x << vol_pos.y << vol_pos.z << "/" << scene_pos;
#endif
		arch_ctrl_points_in_vol.push_back(vol_pos + (forward * controller_->GetUpVector()));
	}
}

float BaseViewPanoArch::GetArchRangeInVol() const {
  return view_render_param()->MapSceneToVol(arch()->range_value() * arch()->zoom_scale());
}
float BaseViewPanoArch::GetArchShiftedInVol() const {
  return view_render_param()->MapSceneToVol(arch()->current_spline_value());
}
float BaseViewPanoArch::GetArchThicknessInVol() const {
  return view_render_param()->MapSceneToVol(arch()->thickness_value());
}
glm::vec3 BaseViewPanoArch::GetUpVector() const {
  return controller_->GetUpVector();
}
glm::vec3 BaseViewPanoArch::GetCenterPosition() const {
  return controller_->MapSceneToVol(view_render_param()->scene_center());
}
bool BaseViewPanoArch::IsSetPanoArch() const { return arch()->IsFinished(); }
int BaseViewPanoArch::GetSliceInVol() const { return scene().GetSliderValue(); }

void BaseViewPanoArch::GetSliceRange(int* min, int* max) {
  scene().GetSliceRange(min, max);
}

void BaseViewPanoArch::slotEndEditArch() {
  drawn_arch_slice_[curr_arch_type_] = GetSliceInVol();
  emit sigUpdatedFinishArch();
}
/**=================================================================================================
protected functions
*===============================================================================================**/
void BaseViewPanoArch::slotActiveSharpenItem(const int index) {
	controller_->SetSharpenLevel(static_cast<SharpenLevel>(index));

  this->RenderSlice();
}
void BaseViewPanoArch::AddPointArch(const QPointF& pt_scene) {
  arch()->AddPoint(pt_scene);
}

void BaseViewPanoArch::AddPointArch(const QPointF& pt_scene, const ArchTypeID& type)
{
	arch_[type]->AddPoint(pt_scene);
}

void BaseViewPanoArch::ClearArch() { arch()->Clear(); }

void BaseViewPanoArch::EndEditArch() {
  if (!arch()->EndEdit()) arch()->Clear();
}

void BaseViewPanoArch::TransformItems(const QTransform& transform) {
  for (int i = 0; i < ArchTypeID::ARCH_TYPE_END; i++) {
	arch_[i]->TransformItem(transform);
  }

  for (auto& spec : implant_specs_) spec.second->TransformItems(transform);
}

void BaseViewPanoArch::resizeEvent(QResizeEvent* pEvent) {
  View::resizeEvent(pEvent);

  controller_->SetProjection();
}

void BaseViewPanoArch::mouseMoveEvent(QMouseEvent* event) {
  RequestDICOMInfo(mapToScene(event->pos()));
  View::mouseMoveEvent(event);
}

void BaseViewPanoArch::leaveEvent(QEvent* event) {
  View::leaveEvent(event);
  scene().SetEnabledItem(Viewitems::HU_TEXT, false);
}

void BaseViewPanoArch::enterEvent(QEvent* event) {
  View::enterEvent(event);
  scene().SetEnabledItem(Viewitems::HU_TEXT, true);
}

/*
ResourceContainer 에 selection 변수가 변경되기 전에 불릴 수 있기 때문에
implant_id == res_implant.selected_implant_id() 대신 selected 변수를 받았다.
*/
QPointF BaseViewPanoArch::GetImplantSpecPosition(const int& implant_id,
												 const bool& selected) const {
  const auto& res_implant =
	  ResourceContainer::GetInstance()->GetImplantResource();

  glm::vec3 pt_vol = res_implant.data().at(implant_id)->axis_point_in_vol();
  QPointF pt_scene = controller()->MapVolToScene(pt_vol);
  if (selected) {
	pt_scene.setY(pt_scene.y());
  }
  return pt_scene;
}

void BaseViewPanoArch::UpdateImplantSpecPosition() {
  if (implant_specs_.empty()) return;

  const auto& res_implant =
	  ResourceContainer::GetInstance()->GetImplantResource();
  for (const auto& spec : implant_specs_) {
	spec.second->setPos(GetImplantSpecPosition(
		spec.first, spec.first == res_implant.selected_implant_id()));
  }

  // view implant pano only
  UpdateImplantHandlePosition();
}

void BaseViewPanoArch::RenderSlice() {
  if (!View::IsEnableGLContext()) return;

  this->UpdateMeasurePlaneInfo();

  MakeCurrent();
  controller()->set_is_implant_wire(true);
  controller_->RenderingSlice();
  DoneCurrent();
}

void BaseViewPanoArch::UpdateMeasurePlaneInfo() {
  scene().UpdatePlaneInfo(
	  controller_->MapSceneToVol(
		  view_render_param()->scene_center() -
		  view_render_param()->MapGLToScene(view_render_param()->gl_trans())),
	  controller_->GetUpVector(), controller_->GetRightVector());
}

/**=================================================================================================
private slots
*===============================================================================================**/

void BaseViewPanoArch::slotActiveFilteredItem(const QString& text) {
  if (!IsReadyController()) return;

  for (int i = 0; i < ARCH_TYPE_END; i++) {
	if (text == filtered_texts_[i]) {
	  curr_arch_type_ = (ArchTypeID)i;
	  emit sigChangedArchType(curr_arch_type_);
	  break;
	}
  }
}
void BaseViewPanoArch::SetFilteredItems() {
#if 1
	return;
#endif

  filtered_texts_[ARCH_MAXILLA] = QString("Arch: Maxilla");
  filtered_texts_[ARCH_MANDLBLE] = QString("Arch: Mandible");

  for (int i = 0; i < ARCH_TYPE_END; i++)
	scene().AddFilteredItem(filtered_texts_[i]);

  scene().ChangeFilteredItemText(filtered_texts_[ARCH_MANDLBLE]);
}
void BaseViewPanoArch::slotChangedValueSlider(int value) {
  if (!controller_->IsReady()) return;

  const auto& vol = ResourceContainer::GetInstance()->GetMainVolume();
  if (&vol == nullptr) return;

  float vol_width = (float)vol.width();
  float vol_height = (float)vol.height();
  float vol_depth = (float)vol.depth();

  controller_->SetPlane(
	  glm::vec3(vol_width * 0.5f, vol_height * 0.5f, (float)value),
	  vol_width * UIViewController::kRightVector,
	  vol_height * UIViewController::kBackVector, 0);

  scene().SetViewRulerItem(*(view_render_param()));
  scene().SetGridItem(*(view_render_param()));
  scene().SetMeasureParams(*(view_render_param()));

  this->CrossSectionUpdated();
  RenderSlice();
  scene().update();

  emit sigTranslateZ(value);
}

void BaseViewPanoArch::slotChangedArchRange() {
  float range_mm = view_render_param()->MapSceneToActual(arch()->range_value() * arch()->zoom_scale());
  emit sigChangedArchRange(range_mm);
}
void BaseViewPanoArch::slotGetProfileData(const QPointF& start_pt_scene,
										  const QPointF& end_pt_scene,
										  std::vector<short>& data) {
  controller_->GetDicomHULine(start_pt_scene, end_pt_scene, data);
}

void BaseViewPanoArch::slotGetROIData(const QPointF& start_pt_scene,
									  const QPointF& end_pt_scene,
									  std::vector<short>& data) {
  controller_->GetDicomHURect(start_pt_scene, end_pt_scene, data);
}

void BaseViewPanoArch::slotUpdateImplantSpecPosition(float) {
  UpdateImplantSpecPosition();
}

/**=================================================================================================
private functions
*===============================================================================================**/

void BaseViewPanoArch::drawBackground(QPainter* painter, const QRectF& rect) {
  View::drawBackground(painter, rect);

  if (!controller_->IsReady()) {
	if (controller_->GetInvertWindow())
	  painter->fillRect(rect, Qt::white);
	else
	  painter->fillRect(rect, Qt::black);
	return;
  }

  painter->beginNativePainting();
  if (IsUpdateController()) {
	controller_->RenderingSlice();
	UpdateDoneContoller();
  }

  controller_->RenderScreen(View::GetDefaultFrameBufferObject());
  painter->endNativePainting();
}

void BaseViewPanoArch::SetGraphicsItems() {
  View::SetGraphicsItems();
  emit sigRequestInitialize();
  SyncMeasureResourceCounterparts(true);
}

void BaseViewPanoArch::InitializeController() {
  controller_->Initialize();

  const auto& vol = ResourceContainer::GetInstance()->GetMainVolume();
  if (&vol == nullptr) return;

  float vol_width = (float)vol.width();
  float vol_height = (float)vol.height();
  float vol_depth = (float)vol.depth();

  controller_->SetPlane(glm::vec3(vol_width * 0.5f, vol_height * 0.5f,
								  (float)scene().GetSliderValue()),
						vol_width * UIViewController::kRightVector,
						vol_height * UIViewController::kBackVector, 0);

  scene().SetWorldAxisDirection(controller_->GetRotateMatrix(),
								controller_->GetViewMatrix());
}

bool BaseViewPanoArch::IsReadyController() { return controller_->IsReady(); }

void BaseViewPanoArch::ClearGL() { controller_->ClearGL(); }

void BaseViewPanoArch::ActiveControllerViewEvent() {
  bool need_render = false;
  controller_->ProcessViewEvent(&need_render);

  if (need_render) RenderSlice();
}

void BaseViewPanoArch::RequestDICOMInfo(const QPointF& pt_scene) {
  glm::vec4 vol_info = controller_->GetDicomInfoPoint(pt_scene);
  DisplayDICOMInfo(vol_info);
}

void BaseViewPanoArch::DisplayDICOMInfo(const glm::vec4& vol_info) {
  float window_width, window_level;
  controller_->GetWindowParams(&window_width, &window_level);
  int ww = static_cast<int>(window_width);
  int wl = static_cast<int>(window_level + controller_->GetIntercept());
  if (vol_info.w != common::dicom::kInvalidHU) {
	scene().SetHUValue(QString("WL %1\nWW %2\n(%3, %4, %5), %6")
						   .arg(wl)
						   .arg(ww)
						   .arg(static_cast<int>(vol_info.x))
						   .arg(static_cast<int>(vol_info.y))
						   .arg(static_cast<int>(vol_info.z))
						   .arg(vol_info.w));
  } else {
	scene().SetHUValue(QString("WL %1\nWW %2\n(-, -, -)").arg(wl).arg(ww));
  }
}

void BaseViewPanoArch::ApplyPreferences()
{
	controller_->ApplyPreferences();

	View::ApplyPreferences();
}

void BaseViewPanoArch::ClearArch(const ArchTypeID& type)
{
	arch_[type]->Clear();
}
