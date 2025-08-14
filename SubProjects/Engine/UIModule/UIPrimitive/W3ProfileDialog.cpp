#include "W3ProfileDialog.h"

#include <qboxlayout.h>
#include <qtoolbutton.h>
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>

#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/language_pack.h"
#include "plotter_module.h"

CW3ProfileDialog::CW3ProfileDialog(const QString& name, QWidget *parent) :
	CW3Dialog(name, parent),
	capture_(new QToolButton(this)), close_(new QToolButton(this)),
	plotter_(new PlotterModule(this)),
	info_display_(new QTableWidget(0, 4, this)) {
	this->setStyleSheet(QString(
		"QToolButton {"
		"border: 1px solid #FF3A3A3A; padding: 0px;"
		"width: 76px; height: 19px;"
		"background-color: qlineargradient(spread:pad x1:0, y1:0, x2:0, y2:1, stop:0 #FF848484, stop:1 #FF686868);"
		"}"
		"QToolButton:hover {background-color: qlineargradient(spread:pad x1:0, y1:0, x2:0, y2:1, stop:0 #FF676767, stop:1 #FFA4A4A4);}"
		"QToolButton:pressed {background-color: qlineargradient(spread:pad x1:0, y1:0, x2:0, y2:1, stop:0 #FF303030, stop:1 #FF686868);}"
	));

	//setWindowFlags(Qt::WindowStaysOnTopHint);
	setFocusPolicy(Qt::StrongFocus);

	capture_->setText(lang::LanguagePack::txt_capture());
	close_->setText(lang::LanguagePack::txt_close());

	QHBoxLayout *pLayoutBtn = new QHBoxLayout();
	pLayoutBtn->setAlignment(Qt::AlignCenter);
	pLayoutBtn->setSpacing(5);
	pLayoutBtn->setContentsMargins(0, 5, 0, 5);
	pLayoutBtn->addWidget(capture_);
	pLayoutBtn->addWidget(close_);

	QHBoxLayout* info_layout = new QHBoxLayout();
	info_layout->setContentsMargins(50, 0, 20, 0);
	info_layout->addWidget(info_display_);

	m_contentLayout->setContentsMargins(0, 0, 0, 0);
	m_contentLayout->addWidget(plotter_);
	m_contentLayout->addLayout(info_layout);
	m_contentLayout->addLayout(pLayoutBtn);

	info_display_->verticalHeader()->setVisible(false);
	info_display_->setEditTriggers(QAbstractItemView::NoEditTriggers);
	info_display_->setContextMenuPolicy(Qt::CustomContextMenu);
	//info_display_->setSelectionMode(QAbstractItemView::NoSelection);

	info_display_->setSelectionBehavior(QAbstractItemView::SelectRows);
	info_display_->setSelectionMode(QAbstractItemView::SingleSelection);

	info_display_->setShowGrid(false);
	//info_display_->setFixedHeight(50);
	info_display_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	info_display_->setContentsMargins(5, 0, 5, 0);

	QStringList info_labels = {
		lang::LanguagePack::txt_coordinate_system(),
		lang::LanguagePack::txt_min(),
		lang::LanguagePack::txt_max(),
		lang::LanguagePack::txt_cursor()
	};
	info_display_->setHorizontalHeaderLabels(info_labels);
	QHeaderView *header = info_display_->horizontalHeader();
	header->setDefaultAlignment(Qt::AlignCenter);
	header->resizeSection(0, 180);
	header->resizeSection(1, 85);
	header->resizeSection(2, 85);
	header->setStretchLastSection(true);
	header->setHighlightSections(false);
	header->setStyleSheet(CW3Theme::getInstance()->TableWidgetHeaderStyleSheet());

	connect(plotter_, SIGNAL(sigDisplayInfo(int, short)), this, SLOT(slotDisplayInfo(int, short)));
	connect(capture_, SIGNAL(clicked()), this, SLOT(slotCaptureBtnPressed()));
	connect(close_, SIGNAL(clicked()), this, SLOT(close()));

	connect(plotter_, SIGNAL(sigChangeLengthStartPos(float)), this, SIGNAL(sigChangeLengthStartPos(float)));
	connect(plotter_, SIGNAL(sigChangeLengthEndPos(float)), this, SIGNAL(sigChangeLengthEndPos(float)));
}

void CW3ProfileDialog::closeEvent(QCloseEvent *event) {
	emit sigPlotterWasClosed();
}

CW3ProfileDialog::~CW3ProfileDialog(void) {
	SAFE_DELETE_OBJECT(plotter_);
}

void CW3ProfileDialog::initialize(
	const QPointF& start_pos,
	const QPointF& end_pos,
	const std::vector<short>& data,
	short min,
	short max,
	float pixel_pitch,
	float length
)
{
	info_display_->setRowCount(1);

	profile_length_ = data.size();
	start_pos_ = start_pos;
	end_pos_ = end_pos;

	QString txt_pos = QString("(%1, %2) - (%3, %4)")
		.arg(QString::number(static_cast<int>(start_pos.x())))
		.arg(QString::number(static_cast<int>(start_pos.y())))
		.arg(QString::number(static_cast<int>(end_pos.x())))
		.arg(QString::number(static_cast<int>(end_pos.y())));
	QTableWidgetItem* item_pos = new QTableWidgetItem(txt_pos);
	item_pos->setTextAlignment(Qt::AlignCenter);
	info_display_->setItem(0, 0, item_pos);

	QTableWidgetItem* item_min = new QTableWidgetItem(QString::number(min));
	item_min->setTextAlignment(Qt::AlignCenter);
	info_display_->setItem(0, 1, item_min);

	QTableWidgetItem* item_max = new QTableWidgetItem(QString::number(max));
	item_max->setTextAlignment(Qt::AlignCenter);
	info_display_->setItem(0, 2, item_max);

	QTableWidgetItem* item_curesor = new QTableWidgetItem(QString(""));
	item_curesor->setTextAlignment(Qt::AlignCenter);
	info_display_->setItem(0, 3, item_curesor);

	plotter_->initialize(data, min, max, pixel_pitch, length);
}

void CW3ProfileDialog::GetMinMax(short & min, short & max) {
	min = plotter_->min_hu();
	max = plotter_->max_hu();
}


void CW3ProfileDialog::slotCaptureBtnPressed() {
	if (!plotter_)
		return;

	plotter_->captureImage();
}

void CW3ProfileDialog::slotDisplayInfo(int profile_index, short hu_value) {
	QString txt_cursor;
	if (profile_index >= 0) {
		int profile_max_index = profile_length_ - 1;
		int curr_x = (profile_index*end_pos_.x() + (profile_max_index - profile_index)*start_pos_.x()) / profile_max_index;
		int curr_y = (profile_index*end_pos_.y() + (profile_max_index - profile_index)*start_pos_.y()) / profile_max_index;
		txt_cursor = QString("(%1, %2) - HU : %3")
			.arg(QString::number(static_cast<int>(curr_x)))
			.arg(QString::number(static_cast<int>(curr_y)))
			.arg(QString::number(static_cast<int>(hu_value)));
	} else {
		txt_cursor = QString();
	}

	QTableWidgetItem* item_cursor = new QTableWidgetItem(txt_cursor);
	item_cursor->setTextAlignment(Qt::AlignCenter);
	info_display_->setItem(0, 3, item_cursor);
}
