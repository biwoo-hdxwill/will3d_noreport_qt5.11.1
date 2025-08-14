#include "viewitems.h"

#include <QDebug>
#include <QGraphicsProxyWidget>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/define_ui.h"
#include "../../Common/Common/color_will3d.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/W3ImageHeader.h"

#include "../UIPrimitive/W3ViewRuler.h"
#include "../UIPrimitive/grid_lines.h"
#include "../UIPrimitive/W3ViewAlignTextItem.h"
#include "../UIPrimitive/W3Slider_2DView.h"
#include "../UIPrimitive/W3FilteredTextItem.h"
#include <Engine/UIModule/UIPrimitive/image_filter_selector.h>
#include "../UIPrimitive/view_border_item.h"
#include "../UIPrimitive/simple_text_item.h"
#include "../UIPrimitive/W3TextItem.h"
#include "../UIPrimitive/direction_text_item.h"

#include "../UIViewController/view_navigator_item.h"


using namespace std;
Viewitems::Viewitems() {
	casted_items_.resize(ITEM_TYPE_END, nullptr);
	enabled_items_.resize(ITEM_TYPE_END, true);
}
Viewitems::~Viewitems() 
{
	SAFE_DELETE_OBJECT(slider_.content);
	SAFE_DELETE_OBJECT(slider_.text);
	for (int i = 0; i < ITEM_TYPE_END; ++i)
	{
		QGraphicsItem* item = casted_items_.at(i);
		if (!item)
		{
			return;
		}

		SAFE_DELETE_OBJECT(item);
	}
}

///////////////////////////////////////////////////////////////////////////
/// public functions
///////////////////////////////////////////////////////////////////////////
void Viewitems::InitItem(const ItemType & type, QGraphicsScene* scene) {
	switch (type) {
	case RULER:
		InitViewRulerItem(scene);
		break;
	case ALIGN_TEXTS:
		InitAlignItem(scene);
		break;
	case NAVIGATION:
		InitNavigationItem(scene);
		break;
	case FILTERED_TEXT:
		InitFilteredItem(scene);
		break;
	case SLIDER:
		InitSlider(scene);
		break;
	case BORDER:
		InitBorder(scene);
		break;
	case HU_TEXT:
		InitHUValueLabel(scene);
		break;
	case GRID:
		InitGridItem(scene);
		break;
	case DIRECTION_TEXT:
		InitDirectionItem(scene);
		break;
	case SHARPEN_FILTER:
		InitSharpenItem(scene);
		break;
	default:
		assert(false);
		break;
	}

	ApplyPreferences();
}

void Viewitems::SetEnabledItem(const ItemType & type, const bool& isEnable) {
	if (type == ItemType::SLIDER) {
		if (isEnable) {
			slider_.text->setVisible(isEnable);
		} else {
			slider_.text->setVisible(false);
		}
	}

	if (type == ItemType::HU_TEXT) {
		bool is_text_visible = !(hide_all_ui_);
		if (is_text_visible) {
			enabled_items_[type] = isEnable;
			SetVisibleItem(type, isEnable);
		} else {
			enabled_items_[type] = false;
			SetVisibleItem(type, false);
		}
	} else {
		enabled_items_[type] = isEnable;
		SetVisibleItem(type, isEnable);
	}
}

void Viewitems::SetRulerColor(const QColor & color) {
	if (view_ruler_)
		view_ruler_->SetColor(color);
}

void Viewitems::SetBorderColor(const QColor& color) {
	if (view_border_)
		view_border_->SetColor(color);
}

void Viewitems::SetSliderTextColor(const QColor& color) {
	if (slider_.text)
		slider_.text->setTextColor(color);
}

void Viewitems::HideAllUI(const bool& is_hide) {
	hide_all_ui_ = is_hide;

	SetVisibleItems();
}

void Viewitems::HideText(const bool& is_hide) {
	// HideUI 로 통일
}

void Viewitems::ViewEnableStatusChanged(const bool& view_enable) {
	view_enable_ = view_enable;

	SetVisibleItems();
}
//TODO 아이템 setpos 함수를 재정의하여 인자로 view size를 받도록 하고
//안에서 maptoscene을 한 position으로 setpos하도록 변경한다.
void Viewitems::resizeEvent(const ViewRenderParam& view_param) {
	int view_width = view_param.view_size_width();
	int view_height = view_param.view_size_height();
	double scene_width = view_param.scene_size_width();
	double scene_height = view_param.scene_size_height();

	if (view_ruler_)
		this->SetViewRulerItem(view_param);

	if (grid_)
		this->SetGridItem(view_param);

	if (align_item_)
		align_item_->setPosItem(view_width);

	int max = scene_width < scene_height ? scene_width : scene_height;
	int size = max * 0.275f;
	int haf = size * 0.5f;
	if (navigation_item_) {
		//navigation_item_->SetSize(100, 100);
		//navigation_item_->setPos(scene_width - 65.0, scene_height - 65.0);
		navigation_item_->SetSize(size, size);
		navigation_item_->setPos(scene_width - haf, scene_height - haf);
	}

	if (slider_.content) {
		slider_.proxy->resize(10, std::min(scene_height * 0.33, 159.0));
		slider_.proxy->setPos(
			scene_width - slider_.proxy->sceneBoundingRect().width()*1.5,
			scene_height*0.5 - slider_.proxy->sceneBoundingRect().height() * 0.5);

		slider_.text->setPos(
			slider_.proxy->sceneBoundingRect().right() - slider_.text->sceneBoundingRect().width(),
			(scene_height + slider_.proxy->sceneBoundingRect().height()) * 0.5f
		);
	}

	if (sharpen_item_) {
		int pos_y = common::ui_define::kViewFilterOffsetY;

		if (align_item_ && enabled_items_[ALIGN_TEXTS])
			pos_y += common::ui_define::kViewSpacing;

		sharpen_item_->setPos(
			scene_width - sharpen_item_->sceneBoundingRect().width() - common::ui_define::kViewMarginWidth,
			pos_y);
	}

	if (filtered_item_) {
		int pos_y = common::ui_define::kViewFilterOffsetY;

		if (align_item_ && enabled_items_[ALIGN_TEXTS])
			pos_y += common::ui_define::kViewSpacing;
		if (sharpen_item_ && enabled_items_[SHARPEN_FILTER])
			pos_y += common::ui_define::kViewSpacing;

		filtered_item_->setPos(
			scene_width - filtered_item_->sceneBoundingRect().width() - common::ui_define::kViewMarginWidth,
			pos_y);
	}

	if (view_border_) {
		QPointF scene_center = view_param.scene_center();
		view_border_->SetRect(QRectF(scene_center.x() - (double)scene_width / 2.0 - 1.0,
									 scene_center.y() - (double)scene_height / 2.0 - 1.0,
									 scene_width + 4.0, scene_height + 4.0));
	}

	if (HU_value_label_) {
		HU_value_label_->setPos(30, scene_height - 70);
	}

	if (direction_item_) {
		direction_item_->SetSceneSize(scene_width, scene_height);
		direction_item_->UpdatePosition();
	}

}

bool Viewitems::EditSliderValue(const int& delta) {
	if (slider_.content /*&& slider_.content->isVisible()*/) {
		const int max = slider_.content->maximum();
		const int min = slider_.content->minimum();

		const int sign = slider_.content->invertedAppearance() ? -1 : 1;
		const int degrees = (delta*sign);

		int value = slider_.content->value();
		value += degrees*slider_.interval;

		value = std::min(value, max);
		value = std::max(value, min);

		slider_.content->setValue(value);
		return true;
	} else {
		return false;
	}
}

void Viewitems::SetViewRulerItem(const ViewRenderParam & view_param) {
	if (!view_ruler_)
		return;

	if (view_param.scene_size() == QSize())
		return;

	QPointF view_size_in_scene = QPointF(
		view_param.scene_size().width(),
		view_param.scene_size().height());

	QPointF view_length = view_param.MapSceneToActual(view_size_in_scene);

	QPointF scene_offset = view_param.scene_offset();
	scene_offset.setY(scene_offset.y() - 1.0f);

	if (view_length.x() > 10000.0f || view_length.y() > 10000.0f) {
		common::Logger::instance()->Print(common::LogType::ERR, "Viewitems::SetViewRulerItem: view_length err.");

		qDebug() << "view_size_in_scene :" << view_size_in_scene;
		qDebug() << "view_length :" << view_length;
		qDebug() << "scene_offset :" << scene_offset;

		//assert(false);
		return;
	}

	view_ruler_->setViewRuler(view_size_in_scene.x(), view_size_in_scene.y(),
							  view_length.x(), view_length.y(),
							  view_param.scene_center() - scene_offset);
}

void Viewitems::SetGridItem(const ViewRenderParam & view_param) {
	if (!grid_)
		return;

	if (view_param.scene_size() == QSize())
		return;

	QPointF view_size_in_scene = QPointF(
		view_param.scene_size().width(),
		view_param.scene_size().height());

	QPointF view_length = view_param.MapSceneToActual(view_size_in_scene);

	QPointF scene_offset = view_param.scene_offset();
	scene_offset.setY(scene_offset.y() - 1.0f);

	int index = GlobalPreferences::GetInstance()->preferences_.general.display.grid_spacing;
	int spacing = GlobalPreferences::GetInstance()->preferences_.general.display.grid_spacing_preset[index];
	grid_->SetGrid(view_size_in_scene.x(), view_size_in_scene.y(),
				   view_length.x(), view_length.y(),
				   view_param.scene_center() - scene_offset, static_cast<float>(spacing));
	grid_->setVisible(enabled_items_[GRID]);
}

void Viewitems::SetWorldAxisDirection(const glm::mat4 & rotate_mat, const glm::mat4& view_mat) {
	if (!navigation_item_)
	{
		return;
	}

	navigation_item_->SetWorldAxisDirection(rotate_mat, view_mat);
}

void Viewitems::SetSliderValue(const int& value) {
	slider_.content->setValue(value);
}
void Viewitems::SetSliderRange(const int& min, const int& max) {
	int old_min = slider_.content->minimum();
	int old_max = slider_.content->maximum();
	if (old_min < old_max)
	{
		int old_val = (slider_.content->value() - old_min) / (old_max - old_min);
		int new_val = (old_val * (max - min)) + min;

		slider_.content->setRange(min, max);
		//slider_.content->setValue(new_val);
	}
	else
	{
		slider_.content->setRange(min, max);
	}
}
void Viewitems::GetSliderRange(int* min, int* max) {
	*min = slider_.content->minimum();
	*max = slider_.content->maximum();
}

void Viewitems::SetSliderInvertedAppearance(const bool& is_enable) {
	slider_.content->setInvertedAppearance(is_enable);
}

void Viewitems::SetSliderInterval(const int& interval) {
	slider_.interval = interval;
}

void Viewitems::AddFilteredItem(const QString & text) {
	filtered_item_->addText(text);
}

void Viewitems::ChangeFilteredItemText(const QString & text) {
	filtered_item_->changeText(text);
}

void Viewitems::ChangeSharpenLevel(const int level) {
	sharpen_item_->SetLevel(level);
}

void Viewitems::SetDirectionTextItem(const QString & text, const bool& is_align_left) {
	if (!direction_item_)
	{
		return;
	}

	direction_item_->SetText(text, is_align_left);
	direction_item_->UpdatePosition();
}

bool Viewitems::IsUnderMouse() {
	if (align_item_ && align_item_->isVisible() && align_item_->IsHovered()) {
		return true;
	}
	if (slider_.content && slider_.content->isVisible()) {
		if(slider_.content->pressed() || slider_.content->hovered())
			return true;
	}
	if (filtered_item_ && filtered_item_->isVisible() && filtered_item_->isHovered()) {
		return true;
	}
	if (sharpen_item_ && sharpen_item_->isVisible() && sharpen_item_->IsHovered()) {
		return true;
	}

	return false;
}

const bool Viewitems::IsTextVisible() const {
	return !(hide_all_ui_);
}

const bool Viewitems::IsUIVisible() const {
	return !hide_all_ui_;
}

int Viewitems::GetSliderValue() const {
	return slider_.content->value();
}

void Viewitems::SetHUValue(const QString & text) {
	if (HU_value_label_)
		HU_value_label_->SetText(text);
}

void Viewitems::SetGridOnOff(const bool& visible) {
	if (!grid_)
		return;

	SetEnabledItem(ItemType::GRID, visible);
	grid_->setVisible(visible);
}

void Viewitems::ApplyPreferences() {
	bool is_text_visible = IsTextVisible();
	show_rulers_ = GlobalPreferences::GetInstance()->preferences_.general.display.show_rulers;
	if (view_ruler_)
		view_ruler_->setVisible(!hide_all_ui_ && show_rulers_);

	show_slice_numbers_ = GlobalPreferences::GetInstance()->preferences_.general.display.show_slice_numbers;
	if (slider_.text) {
		slider_.text->setVisible(is_text_visible && show_slice_numbers_);
		slider_.content->setVisible(!hide_all_ui_);
	}

	int index = GlobalPreferences::GetInstance()->preferences_.general.display.grid_spacing;
	int spacing = GlobalPreferences::GetInstance()->preferences_.general.display.grid_spacing_preset[index];
	if (grid_)
		grid_->SetGridSpacing(spacing);
}

///////////////////////////////////////////////////////////////////////////
/// private  functions
///////////////////////////////////////////////////////////////////////////
void Viewitems::InitViewRulerItem(QGraphicsScene * scene) {
	if (view_ruler_) {
		return PrintLogAndAssert("Viewitems::InitViewRulerItem: Already set.");
	}

	view_ruler_ = new CW3ViewRuler();
	scene->addItem(view_ruler_);

	casted_items_[RULER] = (QGraphicsItem*)view_ruler_;
}
void Viewitems::InitAlignItem(QGraphicsScene * scene) {
	if (align_item_) {
		return PrintLogAndAssert("Viewitems::InitAlignItem: Already set.");
	}

	align_item_ = new CW3ViewAlignTextItem();
	scene->addItem(align_item_);
	connect(align_item_, SIGNAL(sigRotateMatrix(glm::mat4)), this, SIGNAL(sigRotateMatrix(glm::mat4)));

	casted_items_[ALIGN_TEXTS] = (QGraphicsItem*)align_item_;
}
void Viewitems::InitNavigationItem(QGraphicsScene * scene) {
	if (navigation_item_) {
		return PrintLogAndAssert("Viewitems::ViewNavigatorItem: Already set.");
	}

	navigation_item_ = new ViewNavigatorItem();
	navigation_item_->setFlag(QGraphicsItem::ItemIsSelectable, true);
	navigation_item_->setZValue(0);
	scene->addItem(navigation_item_);
	//connect(navigation_item_.get(), SIGNAL(sigRotateMatrix(glm::mat4)), this, SIGNAL(sigRotateMatrix(glm::mat4)));

	casted_items_[NAVIGATION] = (QGraphicsItem*)navigation_item_;
}
void Viewitems::InitSlider(QGraphicsScene * scene) {
	if (slider_.content) {
		return PrintLogAndAssert("Viewitems::InitSlider: Already set.");
	}

	slider_.content = new CW3Slider_2DView();
	slider_.content->setInvertedAppearance(true);
	slider_.content->setRange(0, 0);

	slider_.text = new CW3TextItem(false);
	scene->addItem(slider_.text);

	const int kGLSingleStep = 2;
	slider_.content->setSingleStep(kGLSingleStep);
	slider_.proxy = (scene->addWidget(slider_.content));
	slider_.proxy->setZValue(10.0f);

	connect(slider_.content, SIGNAL(valueChanged(int)), this, SLOT(slotSliderValueChanged(int)));

	casted_items_[SLIDER] = (QGraphicsItem*)slider_.proxy;
}
void Viewitems::InitBorder(QGraphicsScene* scene) {
	if (view_border_) {
		return PrintLogAndAssert("Viewitems::InitBorder: Already set.");
	}

	view_border_ = new ViewBorderItem();
	scene->addItem(view_border_);
	casted_items_[BORDER] = (QGraphicsItem*)view_border_;
}
void Viewitems::InitHUValueLabel(QGraphicsScene * scene) {
	if (HU_value_label_) {
		return PrintLogAndAssert("Viewitems::InitHUValueLabel: Already set.");
	}

	HU_value_label_ = new SimpleTextItem();
	scene->addItem(HU_value_label_);
	casted_items_[HU_TEXT] = (QGraphicsItem*)HU_value_label_;
}
void Viewitems::InitFilteredItem(QGraphicsScene * scene) {
	if (filtered_item_) {
		return PrintLogAndAssert("Viewitems::InitFilteredItem: Already set.");
	}

	filtered_item_ = new CW3FilteredTextItem();
	scene->addItem(filtered_item_);

	connect(filtered_item_, SIGNAL(sigPressed(QString)),
			this, SIGNAL(sigActiveFilteredItem(QString)));

	casted_items_[FILTERED_TEXT] = (QGraphicsItem*)filtered_item_;
}
void Viewitems::InitGridItem(QGraphicsScene * scene) {
	if (grid_) {
		return PrintLogAndAssert("Viewitems::InitGridItem: Already set.");
	}

	grid_ = new GridLines();
	grid_->setZValue(0.0);
	scene->addItem(grid_);
	casted_items_[GRID] = (QGraphicsItem*)grid_;
	SetEnabledItem(GRID, false);
}

void Viewitems::InitDirectionItem(QGraphicsScene* scene) {
	if (direction_item_) {
		return PrintLogAndAssert("Viewitems::InitDirectionItem: Already set.");
	}

	direction_item_ = new DirectionTextItem();
	direction_item_->setZValue(0.0);
	scene->addItem(direction_item_);
	casted_items_[DIRECTION_TEXT] = (QGraphicsItem*)direction_item_;
}

void Viewitems::InitSharpenItem(QGraphicsScene* scene) {
	if (sharpen_item_) {
		return PrintLogAndAssert("Viewitems::InitSharpenItem: Already set.");
	}

	QSettings settings(GlobalPreferences::GetInstance()->ini_path(), QSettings::IniFormat);

	sharpen_item_ = new ImageFilterSelector();
	sharpen_item_->SetLevel(settings.value("SLICE/default_filter", 0).toInt());
	sharpen_item_->setZValue(0.0);

	connect(sharpen_item_, SIGNAL(sigPressed(int)), this, SIGNAL(sigActiveSharpenItem(int)));

	scene->addItem(sharpen_item_);

	casted_items_[SHARPEN_FILTER] = (QGraphicsItem*)sharpen_item_;
}

void Viewitems::PrintLogAndAssert(const char* msg) {
	common::Logger::instance()->Print(common::LogType::WRN, msg);
	assert(false);
}

void Viewitems::SetVisibleItems() {
	const bool is_text_visible = IsTextVisible();
	for (int i = 0; i < ITEM_TYPE_END; i++) {
		if (i == RULER) {
			SetVisibleItem(RULER, !hide_all_ui_ && show_rulers_ && view_enable_);
		} else if (i == FILTERED_TEXT || i == ALIGN_TEXTS || i == SHARPEN_FILTER
				   || i == DIRECTION_TEXT) {
			SetVisibleItem((ItemType)i, is_text_visible && view_enable_);
		} else if (i == HU_TEXT) {
			continue;
		} else {
			SetVisibleItem((ItemType)i, !hide_all_ui_ && view_enable_);
		}
	}

	if (slider_.text)
		slider_.text->setVisible(is_text_visible && show_slice_numbers_
								 && view_enable_);
}

void Viewitems::SetVisibleItem(const ItemType& type, const bool& isVisible) {
	if (!casted_items_[type])
		return;

	if (enabled_items_[type])
		casted_items_[type]->setVisible(isVisible);
	else
		casted_items_[type]->setVisible(false);
}

void Viewitems::slotSliderValueChanged(int value) {
	slider_.text->setPlainText(QString("%1").arg(value + 1));
	slider_.text->setPos(
		slider_.proxy->sceneBoundingRect().right() - slider_.text->sceneBoundingRect().width(),
		slider_.text->sceneBoundingRect().y()
	);
	emit sigSliderValueChanged(value);
}
