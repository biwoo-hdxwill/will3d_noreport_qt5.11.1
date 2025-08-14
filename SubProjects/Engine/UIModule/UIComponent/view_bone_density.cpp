#include "view_bone_density.h"

#include <QMouseEvent>

#include "../../Common/Common/W3Memory.h"
#include "../UIPrimitive/simple_text_item.h"
#include "../UIViewController/view_controller_bone_density.h"
#include "../UIViewController/view_render_param.h"

#include "scene.h"

namespace
{
	const QString kBoneDensityNumber[] =
	{
		"1250",
		"850",
		"350",
		"150",
	};

	const QColor kBoneDensityColor[] =
	{
		Qt::white,
		Qt::blue,
		Qt::green,
		QColor(255, 255, 0),  // yellow
		QColor(255, 128, 0),  // orange
		Qt::red,
		Qt::black
	};

	const QString kDensityLevels[] = { "D1", "D2", "D3", "D4", "D5" };

	const float kTextYPosOffset = 12.0f;
}  // namespace

using namespace UIViewController;

ViewBoneDensity::ViewBoneDensity(QWidget* parent)
	: View(common::ViewTypeID::IMPLANT_BONEDENSITY, parent) 
{
	controller_.reset(new ViewControllerBoneDensity);
	controller_->set_view_param(view_render_param());

	//this->setFixedHeight(170);
}

ViewBoneDensity::~ViewBoneDensity() 
{ 
	ClearGL(); 
}

/**=================================================================================================
public functions
*===============================================================================================**/
void ViewBoneDensity::SyncImplant3DCameraMatrix(const glm::mat4& rotate_mat,
	const glm::mat4& reorien_mat,
	const glm::mat4& view_mat) 
{
	controller_->SyncImplant3DCameraMatrix(rotate_mat, reorien_mat, view_mat);
}

void ViewBoneDensity::UpdateBoneDensity() 
{
	if (!isVisible()) return;

	this->RenderBoneDensity();
	scene().update();
}

const glm::mat4& ViewBoneDensity::GetRotateMatrix() const 
{
	return controller_->GetRotateMatrix();
}

/**=================================================================================================
private functions
*===============================================================================================**/
void ViewBoneDensity::SyncPopupStatus(bool popup) 
{
	popup_mode_ = popup;
	//this->setFixedHeight(popup_mode_ ? 500 : 170);
}

void ViewBoneDensity::drawBackground(QPainter* painter, const QRectF& rect) 
{
	View::drawBackground(painter, rect);

	painter->beginNativePainting();

	if (IsUpdateController()) {
		controller_->RenderingBoneDensity();
		UpdateDoneContoller();
		DrawBoneDensityBar();
	}

	controller_->RenderScreen(View::GetDefaultFrameBufferObject());
	painter->endNativePainting();
}

void ViewBoneDensity::mouseMoveEvent(QMouseEvent* event) 
{
	if (controller_->IsReady() && event->buttons() & Qt::RightButton) {
		View::SetViewEvent(EVIEW_EVENT_TYPE::ROTATE);
		emit sigRotated();
	}

	View::mouseMoveEvent(event);
}

void ViewBoneDensity::mouseDoubleClickEvent(QMouseEvent* event) 
{
	popup_mode_ = !popup_mode_;
	//
	//this->setFixedHeight(popup_mode_ ? 500 : 170);
	emit sigBoneDensityPopupMode(popup_mode_);
}

void ViewBoneDensity::InitializeController() 
{ 
	controller_->Initialize(); 
}

bool ViewBoneDensity::IsReadyController() 
{ 
	return controller_->IsReady(); 
}

void ViewBoneDensity::TransformItems(const QTransform& transform) 
{
	//DrawBoneDensityBar();
}

void ViewBoneDensity::ClearGL()
{
	if (View::IsEnableGLContext()) 
	{
		View::MakeCurrent();
		controller_->ClearGL();
		View::DoneCurrent();
	}
}

void ViewBoneDensity::ActiveControllerViewEvent() 
{
	bool need_render = false;
	controller_->ProcessViewEvent(&need_render);
	
	if (need_render) 
	{
		this->RenderBoneDensity();
	}
}

void ViewBoneDensity::RenderBoneDensity() 
{
	if (!View::IsEnableGLContext()) return;

	View::MakeCurrent();
	controller_->RenderingBoneDensity();
	View::DoneCurrent();
}

void ViewBoneDensity::resizeEvent(QResizeEvent* pEvent) 
{
	View::resizeEvent(pEvent);
	controller_->SetProjection();

	//DrawBoneDensityBar();
}

void ViewBoneDensity::DrawBoneDensityBar() 
{
	RemoveItems();
	float color_bar_width = 0.13f * width();
	AddGradient(color_bar_width);

	float interval = height() / 5.f;
	for (int i = 0; i < 5; ++i) 
	{
		float hu_pos_y = interval * i - kTextYPosOffset;
		if (i > 0) AddHUText(i - 1, interval, color_bar_width, hu_pos_y);

		AddDLevelText(i, interval, hu_pos_y);

		if (i < 4) AddHSpacer(i, interval, color_bar_width);
	}
}

void ViewBoneDensity::RemoveItems() 
{
	if (color_bar_) 
	{
		scene().removeItem(color_bar_);
		SAFE_DELETE_OBJECT(color_bar_);
	}

	for (int i = 0; i < hu_level_text_.size(); ++i) 
	{
		scene().removeItem(hu_level_text_.at(i));
		SAFE_DELETE_OBJECT(hu_level_text_.at(i));
	}
	hu_level_text_.clear();

	for (int i = 0; i < d_level_text_.size(); ++i) 
	{
		scene().removeItem(d_level_text_.at(i));
		SAFE_DELETE_OBJECT(d_level_text_.at(i));
	}
	d_level_text_.clear();

	for (int i = 0; i < spacer_.size(); ++i) 
	{
		scene().removeItem(spacer_.at(i));
		SAFE_DELETE_OBJECT(spacer_.at(i));
	}
	spacer_.clear();
}

void ViewBoneDensity::AddGradient(float box_width) 
{
	color_bar_ = scene().addRect(0, 0, box_width, height());

	QRectF rect = color_bar_->rect();
	QLinearGradient gradient(0, rect.top(), 0, rect.bottom());
	int color_count = 7;
	int d_count = 5;
	float step_range = 1.0f / d_count;
	for (int i = 0; i < color_count; ++i) 
	{
		float pos = (float)i / d_count - step_range * 0.5f;
		if (pos < 0.0f)
			pos = 0.0f;
		else if (pos > 1.0f)
			pos = 1.0f;
		gradient.setColorAt(pos, kBoneDensityColor[i]);
	}
	color_bar_->setBrush(gradient);
}

void ViewBoneDensity::AddHUText(int index, float interval, float hu_pos_x, float hu_pos_y) 
{
	SimpleTextItem* hu_value = new SimpleTextItem;
	hu_value->SetText(kBoneDensityNumber[index]);
	hu_value->setPos(hu_pos_x, hu_pos_y);
	hu_level_text_.push_back(hu_value);
	scene().addItem(hu_value);
}

void ViewBoneDensity::AddDLevelText(int index, float interval, float curr_hu_pos_y) 
{
	SimpleTextItem* d_value = new SimpleTextItem;
	d_value->SetText(kDensityLevels[index]);

	float next_hu_pos_y = interval * (index + 1) - kTextYPosOffset;
	float d_pos_y = (curr_hu_pos_y + next_hu_pos_y) * 0.5f;
	d_value->setPos(0, d_pos_y);

	d_level_text_.push_back(d_value);
	scene().addItem(d_value);
}

void ViewBoneDensity::AddHSpacer(int index, float interval,	float spacer_length)
{
	float line_pos_y = interval * (index + 1);
	auto* line = scene().addLine(0, line_pos_y, spacer_length, line_pos_y);
	line->setPen(QPen(Qt::black));
	spacer_.push_back(line);
}
