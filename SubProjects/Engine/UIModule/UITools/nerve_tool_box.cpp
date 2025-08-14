#include "nerve_tool_box.h"

#include <QLayout>
#include <QLabel>
#include <QCheckBox>
#include <QTextItem>
#include <QDoubleSpinBox>
#include <QToolButton>
#include <QScrollArea>

#include <Engine/Common/Common/color_dialog.h>
#include "../../Common/Common/W3Style.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/global_preferences.h"

#include "../../Common/Common/language_pack.h"
//20250123 LIN
//#ifndef WILL3D_VIEWER
#include "../../Core/W3ProjectIO/project_io_pano_engine.h"
//#endif

namespace {
const int kMarginLeft = 5;
const int kMarginRight = 12;
const int kSpacing = 2;
const int kFixedWidthFieldChkVisible = 15;
const int kFixedWidthFieldID = 20;
const int kFixedWidthFieldDia = 60;
const int kFixedWidthFieldValue = 60;
const int kFixedWidthDelete = 10;
const int kFixedHeightField = 17;
const int kPaddingLeftField = 2;
const int kColorSpacing = 2;
}

NerveToolRecord::NerveToolRecord(int record_id) : id_(record_id) {
	this->setObjectName("NerveToolRecord");

	layout_main_.reset(new QHBoxLayout);
	layout_main_->setContentsMargins(kMarginLeft, kSpacing, kMarginRight, kSpacing);
	layout_main_->setSpacing(kSpacing);

	this->setLayout(layout_main_.get());

	check_visible_.reset(new QCheckBox);
	check_visible_->setChecked(true);
	check_visible_->setFixedSize(kFixedWidthFieldChkVisible, kFixedHeightField);
	check_visible_->setContentsMargins(0, 0, 0, 0);

	text_id_.reset(new QLabel);
	text_id_->setObjectName("NerveToolRecordID");
	text_id_->setAlignment(Qt::AlignCenter);
	text_id_->setText(QString("%1").arg(record_id, 2, 10, QChar('0')));
	text_id_->setFixedSize(kFixedWidthFieldID, kFixedHeightField);
	text_id_->setContentsMargins(0, 0, 0, 0);

	spin_dia_.reset(new QDoubleSpinBox);
	spin_dia_->setFixedSize(kFixedWidthFieldDia, kFixedHeightField);
	spin_dia_->setStyle(new ViewSpinBoxStyle());
	spin_dia_->setStyleSheet(CW3Theme::getInstance()->ViewSpinBoxStyleSheet());
	spin_dia_->setDecimals(1);
	spin_dia_->setValue(GlobalPreferences::GetInstance()->preferences_.objects.nerve.default_diameter);
	spin_dia_->setContentsMargins(0, 0, 0, 0);
	spin_dia_->setSingleStep(0.1);
	spin_dia_->setRange(0.1, 10.0);

	QHBoxLayout* color_layout = new QHBoxLayout();
	color_layout->setSpacing(kColorSpacing);
	color_layout->setContentsMargins(0, 0, 0, 0);

	btn_color_.reset(new QToolButton);
	btn_color_->setFixedSize(kFixedWidthFieldValue - kFixedWidthDelete - kColorSpacing, kFixedHeightField);
	btn_color_->setObjectName("NerveColor");
	btn_color_->setContentsMargins(0, 0, 0, 0);
	this->SetColor(GlobalPreferences::GetInstance()->preferences_.objects.nerve.default_color);

	btn_delete_.reset(new QToolButton);
	btn_delete_->setFixedSize(kFixedWidthDelete, kFixedHeightField);
	btn_delete_->setObjectName("NerveDelete");
	btn_delete_->setContentsMargins(0, 0, 0, 0);
	btn_delete_->setStyleSheet(QString("QToolButton#NerveDelete {"
									   "background-image: url(:image/nervetool/nervetool_delete.png);"
									   "background-position: center;"
									   "background-repeat: no-repeat;"
									   "background-color: #00000000;"
									   "border: 0px;"
									   "}"));
	color_layout->addWidget(btn_color_.get());
	color_layout->addWidget(btn_delete_.get());

	layout_main_->addWidget(check_visible_.get());
	layout_main_->addWidget(text_id_.get());
	layout_main_->addWidget(spin_dia_.get());
	layout_main_->addLayout(color_layout);

	connect(btn_color_.get(), SIGNAL(clicked()), this, SLOT(slotClickedColor()));
	connect(btn_delete_.get(), SIGNAL(clicked()), this, SLOT(slotClickedDelete()));
	connect(check_visible_.get(), SIGNAL(stateChanged(int)), this, SLOT(slotChangedValues()));
	connect(spin_dia_.get(), SIGNAL(valueChanged(double)), this, SLOT(slotChangedValues()));
}

NerveToolRecord::~NerveToolRecord() {}

bool NerveToolRecord::IsNerveVisible() const {
	return check_visible_->isChecked();
}
float NerveToolRecord::GetDiameter() const {
	return (float)spin_dia_->value();
}
void NerveToolRecord::SetCheckedVisible(const bool& is_checked) {
	check_visible_->setChecked(is_checked);
}
void NerveToolRecord::slotClickedDelete() {
	emit sigDelete(id_);
}
void NerveToolRecord::slotChangedValues() {
	emit sigChangedValues(id_);
}
void NerveToolRecord::SetColor(const QColor& color) {
	curr_color_ = color;
	btn_color_->setStyleSheet(QString("QToolButton#NerveColor {"
									  "background-image: url(:/image/nervetool/nervetool_color.png);"
									  "background-position: bottom right;"
									  "background-repeat: no-repeat;"
									  "background-color: %1;"
									  "border: 0px;"
									  "}"
	).arg(curr_color_.name()));
}

void NerveToolRecord::SetDiameter(float diameter) {
	spin_dia_->setValue(diameter);
}

void NerveToolRecord::enterEvent(QEvent * event) {
	QFrame::enterEvent(event);
	emit sigHovered(id_, true);
}

void NerveToolRecord::leaveEvent(QEvent * event) {
	QFrame::leaveEvent(event);
	emit sigHovered(id_, false);
}

void NerveToolRecord::slotClickedColor() {
	ColorDialog color_dialog;
	color_dialog.SetCurrentColor(curr_color_);
	if (!color_dialog.exec())
	{
		return;
	}

	QColor color = color_dialog.SelectedColor();
	if (!color.isValid())
	{
		return;
	}

	SetColor(color);
	emit sigChangedValues(id_);
}

NerveToolBox::NerveToolBox(QWidget* parent) : QFrame(parent) {
	this->setMinimumHeight(150);
	this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

	layout_main_.reset(new QVBoxLayout());
	layout_main_->setSpacing(0);
	layout_main_->setContentsMargins(0, 0, 0, 0);

	this->InitializeFrameField();
	this->InitializeRecordArea();
	this->InitializeButtons();

	this->setLayout(layout_main_.get());
	this->setStyleSheet(CW3Theme::getInstance()->NerveToolStyleSheet());
}

NerveToolBox::~NerveToolBox() {}
//20250123 LIN 
//#ifndef WILL3D_VIEWER
void NerveToolBox::importProject(ProjectIOPanoEngine& in) {
	int nerve_cnt;
	in.LoadNerveCount(nerve_cnt);

	for (int index = 0; index < nerve_cnt; ++index) {
		int color_r, color_g, color_b;
		bool visible;
		float radius;
		double diameter;

		int id;
		in.LoadNerveParams(index, id, color_r, color_g, color_b,
						   visible, radius, diameter);

		records_[id].reset(new NerveToolRecord(id));

		records_[id]->setVisible(false);
		layout_records_->addWidget(records_[id].get());
		records_[id]->setVisible(true);
		records_[id]->SetCheckedVisible(visible);
		records_[id]->SetColor(QColor(color_r, color_g, color_b));
		records_[id]->SetDiameter(diameter);

		connect(records_[id].get(), SIGNAL(sigChangedValues(int)), this, SLOT(slotChangedRecordValues(int)));
		connect(records_[id].get(), SIGNAL(sigHovered(int, bool)), this, SIGNAL(sigHoveredNerveRecord(int, bool)));
		connect(records_[id].get(), SIGNAL(sigDelete(int)), this, SIGNAL(sigDeleteNerve(int)));
	}
}
//#endif
void NerveToolBox::CreateRecord(int id) {
	records_[id].reset(new NerveToolRecord(id));

	records_[id]->setVisible(false);
	layout_records_->addWidget(records_[id].get());
	records_[id]->setVisible(true);
	records_[id]->SetCheckedVisible(is_visible_check_);

	connect(records_[id].get(), SIGNAL(sigChangedValues(int)), this, SLOT(slotChangedRecordValues(int)));
	connect(records_[id].get(), SIGNAL(sigHovered(int, bool)), this, SIGNAL(sigHoveredNerveRecord(int, bool)));
	connect(records_[id].get(), SIGNAL(sigDelete(int)), this, SIGNAL(sigDeleteNerve(int)));
	emit sigChangedNerveValues(records_[id]->IsNerveVisible(), id, records_[id]->GetDiameter(), records_[id]->curr_color());
}
void NerveToolBox::InitializeFrameField() {
	frame_field_.reset(new QFrame);
	frame_field_->setContentsMargins(kMarginLeft + kPaddingLeftField, 0, kMarginRight, 0);
	frame_field_->setObjectName("NerveToolField");

	QHBoxLayout* layout_field = new QHBoxLayout();
	layout_field->setContentsMargins(0, 0, 0, 0);
	layout_field->setSpacing(kSpacing);
	frame_field_->setLayout(layout_field);

	QLabel* label = CreateFieldLabel("");
	label->setObjectName("NerveToolVisible");
	label->setFixedSize(kFixedWidthFieldChkVisible, kFixedHeightField);
	layout_field->addWidget(label);

	label = CreateFieldLabel("#");
	label->setFixedSize(kFixedWidthFieldID, kFixedHeightField);
	label->setAlignment(Qt::AlignCenter);
	layout_field->addWidget(label);

	label = CreateFieldLabel("Dia.(mm)");
	label->setFixedSize(kFixedWidthFieldDia, kFixedHeightField);
	layout_field->addWidget(label);

	label = CreateFieldLabel("Color");
	label->setFixedSize(kFixedWidthFieldValue, kFixedHeightField);
	layout_field->addWidget(label);

	layout_main_->addWidget(frame_field_.get());
}

void NerveToolBox::InitializeRecordArea() {
	layout_records_.reset(new QVBoxLayout());
	layout_records_->setContentsMargins(0, 0, 0, 0);
	layout_records_->setSpacing(0);
	layout_records_->setAlignment(Qt::AlignTop);

	QScrollArea* records_area = new QScrollArea();
	records_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	records_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	records_area->setWidgetResizable(true);

	QFrame* scroll_widget_contents = new QFrame;
	scroll_widget_contents->setObjectName("NerveToolRecordArea");

	records_area->setContentsMargins(0, 0, 0, 0);
	records_area->setWidget(scroll_widget_contents);

	scroll_widget_contents->setLayout(layout_records_.get());
	layout_main_->addWidget(records_area);
}
void NerveToolBox::InitializeButtons() {
	int SpacingM = CW3Theme::getInstance()->getToolVBarSizeInfo().spacingM;
	QMargins BoxMargins = CW3Theme::getInstance()->getToolVBarSizeInfo().marginBox;

	layout_buttons_.reset(new QHBoxLayout());
	layout_buttons_->setContentsMargins(0, BoxMargins.bottom(), 0, 0);
	layout_buttons_->setSpacing(SpacingM);
	layout_buttons_->setAlignment(Qt::AlignCenter);

	btn_draw_modify_.reset(new QToolButton());
	btn_draw_modify_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	btn_draw_modify_->setText(lang::LanguagePack::txt_draw_modify());
	btn_draw_modify_->setCheckable(true);
	btn_draw_modify_->setStyleSheet(CW3Theme::getInstance()->toolNerveDrawModifyStylesheet(btn_draw_modify_->isChecked()));

	layout_buttons_->addWidget(btn_draw_modify_.get(), 1);

	connect(btn_draw_modify_.get(), &QToolButton::toggled, this, &NerveToolBox::slotDrawModifyOn);

	layout_main_->addLayout(layout_buttons_.get());
}

void NerveToolBox::slotDrawModifyOn(bool toggle) {
	emit sigToggledDraw(toggle);

	btn_draw_modify_->setStyleSheet(CW3Theme::getInstance()->toolNerveDrawModifyStylesheet(btn_draw_modify_->isChecked()));
}

void NerveToolBox::DeleteRecord(int id) {
	if (records_.find(id) == records_.end()) {
		common::Logger::instance()->Print(common::LogType::DBG, "NerveToolBox::slotChangedRecordValues:invalid id.");
		return;
	}

	disconnect(records_[id].get(), SIGNAL(sigChangedValues(int)), this, SLOT(slotChangedRecordValues(int)));
	disconnect(records_[id].get(), SIGNAL(sigHovered(int, bool)), this, SIGNAL(sigHoveredNerveRecord(int, bool)));
	disconnect(records_[id].get(), SIGNAL(sigDelete(int)), this, SIGNAL(sigDeleteNerve(int)));

	records_[id].reset();
	records_.erase(id);
}
void NerveToolBox::PressDrawButton(bool is_disconnect_signal) {
	if (is_disconnect_signal)
		btn_draw_modify_->blockSignals(true);

	if (!btn_draw_modify_->isChecked()) {
		btn_draw_modify_->setChecked(true);
	}

	btn_draw_modify_->setStyleSheet(CW3Theme::getInstance()->toolNerveDrawModifyStylesheet(btn_draw_modify_->isChecked()));

	if (is_disconnect_signal)
		btn_draw_modify_->blockSignals(false);
}

void NerveToolBox::ReleaseDrawButton() {
	btn_draw_modify_->blockSignals(true);

	if (btn_draw_modify_->isChecked()) {
		btn_draw_modify_->setChecked(false);
	}
	btn_draw_modify_->setStyleSheet(CW3Theme::getInstance()->toolNerveDrawModifyStylesheet(btn_draw_modify_->isChecked()));


	btn_draw_modify_->blockSignals(false);
}

bool NerveToolBox::IsPressDrawButton() const {
	return btn_draw_modify_->isChecked();
}
bool NerveToolBox::IsVisibleNerveExists() const {
	for (const auto& nerve : records_) {
		if (nerve.second->IsNerveVisible())
			return true;
	}
	return false;
}
void NerveToolBox::ChangeVisibleCheck(bool is_visible) {
	is_visible_check_ = is_visible;

	for (const auto& elem : records_) {
		disconnect(elem.second.get(), SIGNAL(sigChangedValues(int)), this, SLOT(slotChangedRecordValues(int)));
		elem.second.get()->SetCheckedVisible(is_visible_check_);
		connect(elem.second.get(), SIGNAL(sigChangedValues(int)), this, SLOT(slotChangedRecordValues(int)));
	}
}
void NerveToolBox::ClearRecords() {
	records_.clear();
}
QLabel* NerveToolBox::CreateFieldLabel(const QString& text) {
	QLabel* label = new QLabel(this);
	label->setContentsMargins(0, 0, 0, 0);
	label->setFixedHeight(20);
	label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	label->setText(text);

	return label;
}

void NerveToolBox::slotChangedRecordValues(int record_id) {
	if (records_.find(record_id) == records_.end()) {
		common::Logger::instance()->Print(common::LogType::DBG,
										  "NerveToolBox::slotChangedRecordValues:invalid id.");
		return;
	}

	emit sigChangedNerveValues(records_[record_id]->IsNerveVisible(),
							   record_id,
							   records_[record_id]->GetDiameter(),
							   records_[record_id]->curr_color());
}
