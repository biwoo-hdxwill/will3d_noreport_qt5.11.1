#include "measure_list_dialog.h"

#include <QBoxLayout>
#include <QGuiApplication>
#include <QImage>
#include <QLabel>
#include <QScreen>
#include <QScrollArea>
#include <QToolButton>
#include <QWindow>
#include <QFileDialog>

#include <Engine/Common/Common/W3Theme.h>
#include <Engine/Common/Common/color_will3d.h>
#include <Engine/Common/Common/language_pack.h>
#include <Engine/Common/Common/will3d_id_parser.h>
#include <Engine/Resource/Resource/include/measure_data.h>

#include <Engine/UIModule/UIPrimitive/W3InputDialog.h>

#define ENABLE_SELECT_MEASURE 0

namespace
{
	const int kDlgWidth = 675;
	const int kDlgHeight = 400;
	const int kSectionHeight = 20;

	const int kMarginLeft = 5;
	const int kMarginRight = 12;
	const int kSpacing = 2;
	const int kFixedWidthFieldNo = 38;
	const int kFixedWidthFieldType = 80;
	const int kFixedWidthFieldPosition = 120;
	const int kFixedWidthFieldValue = 160;
	const int kFixedWidthFieldMemo = 180;
	const int kFixedWidthDelete = 10;
	const int kFixedHeightField = 20;
	const int kFixedHeightRecord = 19;
	const int kPaddingLeftField = 2;

	const QString kDeleteButtonStyleSheet(
		"QToolButton#MeasureDelete {"
		"background-image: url(:image/nervetool/nervetool_delete.png);"
		"background-position: center;"
		"background-repeat: no-repeat;"
		"background-color: #00000000;"
		"border: 0;"
		"}");
}  // end of namespace

MeasureRecord::MeasureRecord(const unsigned int& record_id,
	const common::ViewTypeID& view_type,
	const unsigned int& measure_id,
	const common::measure::MeasureType& type,
	const QString& value, const QString& memo)
	: id_(record_id), measure_id_(measure_id), view_id_(view_type)
{
	layout_main_.reset(new QHBoxLayout);
	layout_main_->setContentsMargins(0, 0, 0, 0);
	layout_main_->setSpacing(0);
	this->setLayout(layout_main_.get());
	this->setObjectName("MeasureRecord");

	connect(&timer_, &QTimer::timeout, this, &MeasureRecord::slotTimerExpired);
	timer_.setSingleShot(true);

	ui_id_ =
		std::unique_ptr<QLabel>(CreateRecordLabel(QString("%1").arg(record_id)));
	ui_id_->setAlignment(Qt::AlignCenter);
	ui_id_->setFixedSize(kFixedWidthFieldNo, kFixedHeightRecord);

	ui_measure_type_ = std::unique_ptr<QLabel>(
		CreateRecordLabel(Will3DIDParser::GetMeasureTypeText(type)));
	ui_measure_type_->setFixedSize(kFixedWidthFieldType, kFixedHeightRecord);

	ui_position_ = std::unique_ptr<QLabel>(
		CreateRecordLabel(Will3DIDParser::GetMeasurePositionText(view_type)));
	ui_position_->setFixedSize(kFixedWidthFieldPosition, kFixedHeightRecord);

	ui_value_ = std::unique_ptr<QLabel>(CreateRecordLabel(value));
	ui_value_->setFixedSize(kFixedWidthFieldValue, kFixedHeightRecord);

	ui_memo_ = std::unique_ptr<QLabel>(CreateRecordLabel(memo));
	ui_memo_->setFixedSize(kFixedWidthFieldMemo, kFixedHeightRecord);

	ui_delete_.reset(new QToolButton);
	ui_delete_->setFixedSize(kFixedWidthDelete, kFixedHeightRecord);
	ui_delete_->setObjectName("MeasureDelete");
	ui_delete_->setContentsMargins(0, 0, 0, 0);
	ui_delete_->setStyleSheet(kDeleteButtonStyleSheet);
	ui_delete_->setVisible(false);
	connect(ui_delete_.get(), &QToolButton::clicked,
		[=]() { emit sigDelete(id_, view_id_, measure_id_); });
	layout_main_->addWidget(ui_delete_.get());
	setStyleSheet(CW3Theme::getInstance()->MeasureListDlgStyleSheet());
}

MeasureRecord::~MeasureRecord() {}

void MeasureRecord::ChangeRecordID(const unsigned int& new_id)
{
	id_ = new_id;
	ui_id_->setText(QString("%1").arg(id_));
}

void MeasureRecord::SetRecordSelected(const bool& select)
{
	ui_delete_->setVisible(select);

	QString label_stylesheet = GetMeasureTextLabelStyleSheet(select);
	ui_id_->setStyleSheet(label_stylesheet);
	ui_measure_type_->setStyleSheet(label_stylesheet);
	ui_position_->setStyleSheet(label_stylesheet);
	ui_value_->setStyleSheet(label_stylesheet);
	ui_memo_->setStyleSheet(label_stylesheet);
}

QLabel* MeasureRecord::CreateRecordLabel(const QString& text)
{
	QLabel* label = new QLabel(this);
	label->setObjectName("MeasureTextLabel");
	label->setContentsMargins(0, 0, 0, 0);
	label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	label->setText(text);
	label->setStyleSheet(GetMeasureTextLabelStyleSheet(false));
	layout_main_->addWidget(label);

	return label;
}

QString MeasureRecord::GetMeasureTextLabelStyleSheet(bool selected)
{
	return QString("QLabel#MeasureTextLabel { color: %1; %2 }")
		.arg(ColorGeneral::kBorder.name())
		.arg(selected ? "font-weight: bold;" : "");
}

void MeasureRecord::mousePressEvent(QMouseEvent * event)
{
	QFrame::mousePressEvent(event);
	if (timer_.isActive())
	{
		timer_.stop();
		// edit memo dialog here
		CW3InputDialog dlg(CW3InputDialog::InputMode::TextInput);
		dlg.setTitle("Edit Memo");
		dlg.setFixedWidth(300);
		dlg.raise();

		if (dlg.exec())
		{
			QString inputText = dlg.getInputText();
			ui_memo_->setText(inputText);
			emit sigChangeMemo(view_id_, measure_id_, inputText);
		}
	}
	else
	{
		timer_.start(250);
	}
}

void MeasureRecord::slotTimerExpired()
{
#if ENABLE_SELECT_MEASURE
	emit sigSelect(id_, view_id_, measure_id_);
#endif
}

MeasureListDialog::MeasureListDialog(
	const common::measure::MeasureDataContainer& datas, QWidget* parent)
	: CW3Dialog(lang::LanguagePack::txt_measure_list(), parent),
	close_(new QToolButton),
	capture_(new QToolButton)
{
	InitUI();
	UpdateList(datas);
	Connections();
}

MeasureListDialog::~MeasureListDialog() {}

void MeasureListDialog::slotClose() { this->reject(); }

void MeasureListDialog::slotCapture()
{
	QPixmap pixmap = grab();
#if 0
	QString save_path = QFileDialog::getSaveFileName(this, "Save image", "", "Image files (*.png)");
	pixmap.save(save_path, "png");
#else
	QImage image = pixmap.toImage();
	sigMeasureListCapture(image);
#endif
}

void MeasureListDialog::slotDeleteRecord(unsigned int record_id,
	const common::ViewTypeID& view_type,
	const unsigned int& measure_id)
{
	// 아래 signal의 동작
	// 1. MeasureResourceMgr 에서 해당 MeasureResource를 삭제
	// 2. 현재 활성화되어 있는 Tab > ViewMgr > View > DeleteMeasureUI
	emit sigMeasureDelete(view_type, measure_id);

	// UI Sync
	unsigned int vector_index = record_id - 1;
	disconnect(records_[vector_index].get(), &MeasureRecord::sigDelete, this,
		&MeasureListDialog::slotDeleteRecord);
	disconnect(records_[vector_index].get(), &MeasureRecord::sigSelect, this,
		&MeasureListDialog::slotSelectRecord);
	disconnect(records_[vector_index].get(), &MeasureRecord::sigChangeMemo, this,
		&MeasureListDialog::sigMeasureChangeMemo);

	records_[vector_index].reset();
	records_.erase(records_.begin() + vector_index);

	for (int idx = vector_index; idx < records_.size(); ++idx)
	{
		unsigned int new_record_id = idx + 1;
		records_[idx]->ChangeRecordID(new_record_id);
	}
}

void MeasureListDialog::slotSelectRecord(unsigned int record_id,
	const common::ViewTypeID& view_type,
	const unsigned int& measure_id)
{
#if ENABLE_SELECT_MEASURE
	emit sigMeasureSelect(view_type, measure_id);
#endif

	// UI Sync
	unsigned int vector_index = record_id - 1;
	for (int idx = 0; idx < records_.size(); ++idx)
	{
		bool visible = (vector_index == idx) ? true : false;
		records_[idx]->SetRecordSelected(visible);
	}
}

void MeasureListDialog::InitUI()
{
	setFixedSize(kDlgWidth, kDlgHeight);

	InitFieldUI();
	InitRecordAreaUI();
	QHBoxLayout* button_layout = InitButtonUI();

	m_contentLayout->setContentsMargins(18, 10, 18, 10);
	m_contentLayout->setSpacing(0);
	m_contentLayout->addLayout(button_layout);

	setStyleSheet(CW3Theme::getInstance()->MeasureListDlgStyleSheet());
}

void MeasureListDialog::InitFieldUI()
{
	QHBoxLayout* layout_field = new QHBoxLayout();
	layout_field->setContentsMargins(0, 0, 0, 0);
	layout_field->setSpacing(0);

	QLabel* label = CreateFieldLabel(tr("NO"));
	label->setAlignment(Qt::AlignCenter);
	label->setFixedSize(kFixedWidthFieldNo, kFixedHeightField);
	layout_field->addWidget(label);

	label = CreateFieldLabel(lang::LanguagePack::txt_type());
	label->setFixedSize(kFixedWidthFieldType, kFixedHeightField);
	layout_field->addWidget(label);

	label = CreateFieldLabel(lang::LanguagePack::txt_position());
	label->setFixedSize(kFixedWidthFieldPosition, kFixedHeightField);
	layout_field->addWidget(label);

	label = CreateFieldLabel(lang::LanguagePack::txt_value());
	label->setFixedSize(kFixedWidthFieldValue, kFixedHeightField);
	layout_field->addWidget(label);

	label = CreateFieldLabel(lang::LanguagePack::txt_memo());
	label->setFixedSize(kFixedWidthFieldMemo, kFixedHeightField);
	layout_field->addWidget(label);

	field_area_.reset(new QFrame);
	field_area_->setContentsMargins(0, 0, 0, 0);
	field_area_->setObjectName("MeasureListFieldArea");
	field_area_->setStyleSheet(
		CW3Theme::getInstance()->MeasureListDlgStyleSheet());
	field_area_->setLayout(layout_field);
	m_contentLayout->addWidget(field_area_.get());
}

void MeasureListDialog::InitRecordAreaUI()
{
	layout_records_.reset(new QVBoxLayout());
	layout_records_->setContentsMargins(0, 0, 0, 0);
	layout_records_->setSpacing(0);
	layout_records_->setAlignment(Qt::AlignTop);

	QScrollArea* records_area = new QScrollArea();
	records_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	records_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	records_area->setWidgetResizable(true);

	QFrame* scroll_widget_contents = new QFrame;
	scroll_widget_contents->setObjectName("MeasureListRecordArea");
	scroll_widget_contents->setStyleSheet(
		CW3Theme::getInstance()->MeasureListDlgStyleSheet());

	records_area->setContentsMargins(0, 0, 0, 0);
	records_area->setWidget(scroll_widget_contents);

	scroll_widget_contents->setLayout(layout_records_.get());
	m_contentLayout->addWidget(records_area);
}

QHBoxLayout* MeasureListDialog::InitButtonUI()
{
	close_->setText(lang::LanguagePack::txt_close());
	capture_->setText(lang::LanguagePack::txt_capture());

	QHBoxLayout* button_layout = new QHBoxLayout;
	button_layout->setAlignment(Qt::AlignCenter);
	button_layout->setContentsMargins(0, 9, 0, 0);
	button_layout->setSpacing(9);
	button_layout->addWidget(capture_.get());
	button_layout->addWidget(close_.get());

	return button_layout;
}

void MeasureListDialog::UpdateList(
	const common::measure::MeasureDataContainer& datas)
{
	for (const auto& data : datas)
	{
		if (data.second.empty()) continue;

		for (const auto& measure_data : data.second)
		{
			CreateRecord(data.first, measure_data);
		}
	}
}

void MeasureListDialog::Connections()
{
	connect(close_.get(), &QToolButton::clicked, this,
		&MeasureListDialog::slotClose);
	connect(capture_.get(), &QToolButton::clicked, this,
		&MeasureListDialog::slotCapture);
}

void MeasureListDialog::CreateRecord(
	const common::ViewTypeID& view_type,
	const std::weak_ptr<MeasureData>& measure_data)
{
	MeasureData* data = measure_data.lock().get();
	MeasureRecord* record =
		new MeasureRecord(next_record_id_++, view_type, data->id(), data->type(),
			data->value(), data->memo());
	record->setVisible(false);
	layout_records_->addWidget(record);
	record->setVisible(true);
	connect(record, &MeasureRecord::sigDelete, this,
		&MeasureListDialog::slotDeleteRecord);
	connect(record, &MeasureRecord::sigSelect, this,
		&MeasureListDialog::slotSelectRecord);
	connect(record, &MeasureRecord::sigChangeMemo, this,
		&MeasureListDialog::sigMeasureChangeMemo);
	records_.push_back(std::unique_ptr<MeasureRecord>(record));
}

QLabel* MeasureListDialog::CreateFieldLabel(const QString& text)
{
	QLabel* label = new QLabel(this);
	label->setContentsMargins(0, 0, 0, 0);
	label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	label->setText(text);

	return label;
}

void MeasureListDialog::closeEvent(QCloseEvent* e) { this->slotClose(); }
