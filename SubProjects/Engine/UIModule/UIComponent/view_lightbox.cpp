#include "view_lightbox.h"

#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/color_will3d.h"
#include "../../Common/Common/language_pack.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/lightbox_resource.h"
#include "../../Resource/Resource/W3Image3D.h"

#include "../UIViewController/view_controller_slice.h"

#include "scene.h"

ViewLightbox::ViewLightbox(const LightboxViewType& type, int light_box_id, QWidget* parent)
	: View(common::ViewTypeID::LIGHTBOX, parent), view_type_(type), lightbox_id_(light_box_id)
{
	controller_slice_.reset(new ViewControllerSlice);
	controller_slice_->set_is_implant_wire(true);
	controller_slice_->set_view_param(view_render_param());

	scene().InitViewItem(Viewitems::BORDER);
	scene().InitViewItem(Viewitems::RULER);
	scene().InitViewItem(Viewitems::HU_TEXT);
	scene().InitViewItem(Viewitems::SHARPEN_FILTER);
	scene().InitViewItem(Viewitems::GRID);
	scene().InitViewItem(Viewitems::SLIDER);

	scene().InitMeasure(view_type());
	connect(&scene(), &Scene::sigMeasureCreated, [=](const unsigned int& measure_id) {
		emit sigMeasureCreated(light_box_id, measure_id); });
	connect(&scene(), &Scene::sigMeasureDeleted, [=](const unsigned int& measure_id) {
		emit sigMeasureDeleted(light_box_id, measure_id); });
	connect(&scene(), &Scene::sigMeasureModified, [=](const unsigned int& measure_id) {
		emit sigMeasureModified(light_box_id, measure_id); });

	QColor view_color;
	switch (view_type_)
	{
	case LightboxViewType::AXIAL:
		view_color = ColorView::kAxial;
		break;
	case LightboxViewType::SAGITTAL:
		view_color = ColorView::kSagittal;
		break;
	case LightboxViewType::CORONAL:
		view_color = ColorView::kCoronal;
		break;
	default:
		break;
	}
	scene().SetBorderColor(view_color);
	scene().SetRulerColor(view_color);
	scene().SetSliderTextColor(view_color);
}

ViewLightbox::~ViewLightbox() {
	if (View::IsEnableGLContext()) {
		View::MakeCurrent();
		controller_slice_->ClearGL();
		View::DoneCurrent();
	}
}

void ViewLightbox::RenderSlice() {
	if (!View::IsEnableGLContext())
		return;

	MakeCurrent();
	controller_slice_->RenderingSlice();
	DoneCurrent();
}

void ViewLightbox::UpdateLightbox() {
	const auto& res_lightbox = ResourceContainer::GetInstance()->GetLightboxResource();
	if (&res_lightbox == nullptr) {
		controller_slice_->ClearPlane();
		scene().ViewEnableStatusChanged(false);
		return;
	}
	else {
		scene().ViewEnableStatusChanged(true);
	}

	SetLightboxPlane();
	RenderSlice();

	if (!res_lightbox.IsDataExist(lightbox_id_))
		return;

	scene().SetViewRulerItem(*(view_render_param()));
	scene().SetGridItem(*(view_render_param()));
	scene().SetMeasureParams(*(view_render_param()));

	UpdateMeasurePlaneInfo();
	scene().update();
}

void ViewLightbox::RenderAndUpdate() {
	RenderSlice();
	scene().update();
}

void ViewLightbox::InitSlider(int min, int max, int value) {
	scene().SetSliderRange(min, max);
	scene().SetSliderValue(value);
}

void ViewLightbox::SetSliderValue(int value) {
	scene().SetSliderValue(value);
}

void ViewLightbox::SetEnabledSharpenUI(const bool& is_enabled) {
	scene().SetEnabledItem(Viewitems::SHARPEN_FILTER, is_enabled);
}

void ViewLightbox::SetSharpenLevel(const SharpenLevel & sharpen_level) {
	if (!controller_slice_->IsReady())
	{
		return;
	}

	scene().ChangeSharpenLevel(static_cast<int>(sharpen_level));
	controller_slice_->SetSharpenLevel(sharpen_level);
	RenderAndUpdate();
}

glm::vec3 ViewLightbox::GetUpVector() const {
	return controller_slice_->GetUpVector();
}

glm::vec3 ViewLightbox::GetCenterPosition() const {
	return controller_slice_->MapSceneToVol(view_render_param()->scene_center());
}

void ViewLightbox::SetVisibleNerve(bool is_visible)
{
	controller_slice_->SetVisibleNerve(is_visible);
}

void ViewLightbox::SetVisibleImplant(bool is_visible)
{
	controller_slice_->SetVisibleImplant(is_visible);
}

#ifndef WILL3D_VIEWER
void ViewLightbox::RequestedCreateDCMFiles(const QString& time, const bool nerve_visible, const bool implant_visible, const int filter, const int thickness)
{
	int width = 0;
	int height = 0;
	RenewalWH(width, height);

	if (width == 0 || height == 0)
	{
		return;
	}

	QString middle_path = time;
	if (view_type_ == LightboxViewType::AXIAL)
	{
		middle_path += "Axial_";
	}
	else if (view_type_ == LightboxViewType::SAGITTAL)
	{
		middle_path += "Sagittal_";
	}
	else if (view_type_ == LightboxViewType::CORONAL)
	{
		middle_path += "Coronal_";
	}

	QString path = middle_path + QString::asprintf("%.4d.dcm", lightbox_id_);

	int size = width * height;
	if (!nerve_visible && !implant_visible)
	{
		unsigned short* buf = new unsigned short[size];
		MakeCurrent();
		bool is_ok = controller_slice_->GetTextureData(buf, width, height, filter, thickness);
		DoneCurrent();

		if (is_ok)
		{
			emit sigCreateDCMFiles_ushort(buf, path, lightbox_id_, height, width);
		}
		delete[] buf;
	}
	else
	{
		bool ori_nerve_visible = controller_slice_->IsNerveVisible();
		bool ori_implant_visible = controller_slice_->IsImplantVisible();

		controller_slice_->SetVisibleNerve(nerve_visible);
		controller_slice_->SetVisibleImplant(implant_visible);

		unsigned char* buf = new unsigned char[size * 3];
		MakeCurrent();
		bool is_ok = controller_slice_->GetTextureData(buf, width, height, filter, thickness);
		DoneCurrent();

		if (is_ok)
		{
			emit sigCreateDCMFiles_uchar(buf, path, lightbox_id_, height, width);
		}
		delete[] buf;

		controller_slice_->SetVisibleNerve(ori_nerve_visible);
		controller_slice_->SetVisibleImplant(ori_implant_visible);
	}
}

const int ViewLightbox::GetFilterValue() const
{
	return controller_slice_->GetSharpenLevel();
}
#endif // !WILL3D_VIEWER

void ViewLightbox::slotActiveSharpenItem(const int index) {
	controller_slice_->SetSharpenLevel(static_cast<SharpenLevel>(index));
	emit sigSetSharpen(static_cast<SharpenLevel>(index));
}

void ViewLightbox::slotChangedValueSlider(int value) {
	if (!controller_slice_->IsReady())
		return;

	emit sigTranslate(lightbox_id_, value);
}

void ViewLightbox::slotGetProfileData(const QPointF & start_pt_scene,
	const QPointF & end_pt_scene,
	std::vector<short>& data) {
	QPointF pt_start_plane = controller_slice_->MapSceneToPlane(start_pt_scene);
	QPointF pt_end_plane = controller_slice_->MapSceneToPlane(end_pt_scene);
	emit sigGetProfileData(lightbox_id_, pt_start_plane, pt_end_plane, data);
}

void ViewLightbox::slotGetROIData(const QPointF & start_pt_scene,
	const QPointF & end_pt_scene,
	std::vector<short>& data) {
	QPointF pt_start_plane = controller_slice_->MapSceneToPlane(start_pt_scene);
	QPointF pt_end_plane = controller_slice_->MapSceneToPlane(end_pt_scene);
	emit sigGetROIData(lightbox_id_, pt_start_plane, pt_end_plane, data);
}

void ViewLightbox::InitializeController() {
	controller_slice_->Initialize();
}

bool ViewLightbox::IsReadyController() {
	return controller_slice_->IsReady();
}

void ViewLightbox::ClearGL() {
	controller_slice_->ClearGL();
}

void ViewLightbox::ActiveControllerViewEvent() {
	bool need_render = false;
	controller_slice_->ProcessViewEvent(&need_render);

	if (tool_type() == common::CommonToolTypeOnOff::V_LIGHT)
		need_render = true;

	if (need_render)
		RenderSlice();
}

void ViewLightbox::drawBackground(QPainter * painter, const QRectF & rect) {
	View::drawBackground(painter, rect);

	if (!IsReadyController()) {
		if (controller_slice_->GetInvertWindow())
			painter->fillRect(rect, Qt::white);
		else
			painter->fillRect(rect, Qt::black);
		return;
	}

	if (IsUpdateController()) {
		SetLightboxPlane();
		controller_slice_->RenderingSlice();
		UpdateDoneContoller();
	}

	painter->beginNativePainting();
	RenderScreen();
	painter->endNativePainting();
}

void ViewLightbox::leaveEvent(QEvent * event) {
	View::leaveEvent(event);
	scene().SetEnabledItem(Viewitems::HU_TEXT, false);
}

void ViewLightbox::enterEvent(QEvent * event) {
	View::enterEvent(event);
	scene().SetEnabledItem(Viewitems::HU_TEXT, true);
}

void ViewLightbox::resizeEvent(QResizeEvent * pEvent) {
	View::resizeEvent(pEvent);
	controller_slice_->SetProjection();
}

void ViewLightbox::mouseMoveEvent(QMouseEvent * event) {
	View::mouseMoveEvent(event);
	RequestDICOMInfo(pt_scene_current());
}

void ViewLightbox::mouseReleaseEvent(QMouseEvent * event) {
	if (tool_type() == common::CommonToolTypeOnOff::V_LIGHT && IsEventLeftButton(event)) {
		emit sigLightboxWindowingDone();
	}
	else if (tool_type() == common::CommonToolTypeOnOff::V_ZOOM ||
		tool_type() == common::CommonToolTypeOnOff::V_ZOOM_R) {
		emit sigLightboxZoomDone(view_render_param()->scene_scale());
	}
	else if (tool_type() == common::CommonToolTypeOnOff::V_PAN ||
		tool_type() == common::CommonToolTypeOnOff::V_PAN_LR) {
		emit sigLightboxPanDone(view_render_param()->gl_trans());
	}

	View::mouseReleaseEvent(event);
}

void ViewLightbox::mouseDoubleClickEvent(QMouseEvent * event) {
	View::mouseDoubleClickEvent(event);

	if (tool_type() == common::CommonToolTypeOnOff::NONE && IsEventLeftButton(event)) {
		emit sigMaximize(lightbox_id_);
	}
}

void ViewLightbox::RenderScreen() {
	controller_slice_->RenderScreen(GetDefaultFrameBufferObject());
}

void ViewLightbox::SetLightboxPlane() {
	const auto& res_lightbox = ResourceContainer::GetInstance()->GetLightboxResource();
	if (&res_lightbox == nullptr) {
		controller_slice_->ClearPlane();
		return;
	}

	if (!res_lightbox.IsDataExist(lightbox_id_)) {
		controller_slice_->ClearPlane();
		return;
	}

	const glm::vec3 back_vector = glm::normalize(res_lightbox.GetPlaneBottom());
	const glm::vec3 right_vector = glm::normalize(res_lightbox.GetPlaneRight());

	controller_slice_->SetPlane(res_lightbox.GetPlaneCenter(lightbox_id_),
		right_vector*(float)res_lightbox.GetPlaneWidth(),
		back_vector*(float)res_lightbox.GetPlaneHeight(),
		(int)res_lightbox.GetLightboxThickness());

	controller_slice_->SetProjection();
}

void ViewLightbox::UpdateMeasurePlaneInfo() {
	const auto& res_lightbox = ResourceContainer::GetInstance()->GetLightboxResource();
	if (&res_lightbox == nullptr)
		return;

	if (!res_lightbox.IsDataExist(lightbox_id_))
		return;

	scene().UpdatePlaneInfo(res_lightbox.GetPlaneCenter(lightbox_id_),
		res_lightbox.GetLighboxDirection(),
		res_lightbox.GetPlaneBottom());
}

void ViewLightbox::RequestDICOMInfo(const QPointF & pt_scene) {
	if (!controller_slice_->IsReady())
		return;

	const auto& res_lightbox = ResourceContainer::GetInstance()->GetLightboxResource();
	if (&res_lightbox == nullptr)
		return;

	QPointF pt_cross_section_plane = controller_slice_->MapSceneToPlane(pt_scene);
	glm::vec4 vol_info;
	emit sigDisplayDICOMInfo(lightbox_id_, pt_cross_section_plane, vol_info);
	DisplayDICOMInfo(vol_info);

}

void ViewLightbox::DisplayDICOMInfo(const glm::vec4 & vol_info) {
	float window_width, window_level;
	controller_slice_->GetWindowParams(&window_width, &window_level);
	int ww = static_cast<int>(window_width);
	int wl = static_cast<int>(window_level + controller_slice_->GetIntercept());
	if (vol_info.w != common::dicom::kInvalidHU) {
		scene().SetHUValue(QString("WL %1\nWW %2\n(%3, %4, %5), %6")
			.arg(wl).arg(ww)
			.arg(vol_info.x)
			.arg(vol_info.y)
			.arg(vol_info.z)
			.arg(vol_info.w));
	}
	else {
		scene().SetHUValue(QString("WL %1\nWW %2\n(-, -, -)")
			.arg(wl).arg(ww));
	}
}

void ViewLightbox::RenewalWH(int& out_width, int& out_height)
{
	const CW3Image3D& main_volume = ResourceContainer::GetInstance()->GetMainVolume();
	float vol_width = main_volume.width();
	float vol_height = main_volume.height();
	float vol_depth = main_volume.depth();

	glm::vec3 vol_range = glm::vec3(vol_width, vol_height, vol_depth);
	glm::vec3 vol_center = (vol_range - glm::vec3(1.f)) * 0.5f;

	std::vector<glm::vec3> vol_vertex = controller_slice_->GetVolVertex();

	int new_width = 0;
	int	new_height = 0;
	int temp_value = 0;

	glm::vec3 right = controller_slice_->GetRightVector();
	glm::vec3 back = controller_slice_->GetBackVector();

	for (int i = 0; i < 8; ++i)
	{
		glm::vec3 vec = vol_vertex[i] - vol_center;

		temp_value = glm::dot(vec, right);
		new_width = std::max(new_width, std::abs(temp_value));

		temp_value = glm::dot(vec, back);
		new_height = std::max(new_height, std::abs(temp_value));
	}

	out_width = new_width * 2;
	out_height = new_height * 2;
}
