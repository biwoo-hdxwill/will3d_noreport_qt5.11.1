#include "implant_angle_dialog.h"
#include <math.h>

#if defined(__APPLE__)
#include <glm/gtx/transform.hpp>
#else
#include <GL/glm/gtx/transform.hpp>
#endif

#include <qmath.h>
#include <qboxlayout.h>
#include <qcombobox.h>
#include <QLabel>
#include <QHeaderView>
#include <qtablewidget.h>

#include "../../Common/Common/W3Theme.h"

#include "../../Resource/Resource/implant_resource.h"
#include "../../Resource/ResContainer/resource_container.h"

ImplantAngleDialog::ImplantAngleDialog(QWidget *parent) :
	CW3Dialog("Angle between Implant", parent),
	select_implant_(new QComboBox()),
	implant_list_(new QTableWidget(1, 1, this)) {
	InitUI();
	UpdateList();
	Connections();
}

ImplantAngleDialog::~ImplantAngleDialog() {}

void ImplantAngleDialog::closeEvent(QCloseEvent * e) {
	//Disconnections();
	this->reject();
}

void ImplantAngleDialog::slotSelectImplant(const QString& text) {
	UpdateList();
}

void ImplantAngleDialog::InitUI() {
	select_implant_->setStyleSheet(CW3Theme::getInstance()->GlobalPreferencesDialogComboBoxStyleSheet());
	select_implant_->setFixedWidth(68);

	const auto& implant_resource = ResourceContainer::GetInstance()->GetImplantResource();
	const auto& implant_data = implant_resource.data();

	select_implant_->clear();
	for (const auto& data : implant_data) {
		QString contents = QString("#%1").arg(data.second->id());
		select_implant_->addItem(contents);
	}

	int selected_id = implant_resource.selected_implant_id();
	if (selected_id > 0) {
		QString contents = QString("#%1").arg(selected_id);
		select_implant_->setCurrentText(contents);
	}

	QHBoxLayout* selected_layout = new QHBoxLayout();
	selected_layout->setAlignment(Qt::AlignLeft);
	selected_layout->setContentsMargins(0, 0, 0, 0);
	selected_layout->setSpacing(9);
	selected_layout->addWidget(new QLabel("Selected : "));
	selected_layout->addWidget(select_implant_);

	implant_list_->setMouseTracking(false);
	implant_list_->setEditTriggers(QAbstractItemView::NoEditTriggers);
	implant_list_->setContextMenuPolicy(Qt::CustomContextMenu);
	implant_list_->setSelectionMode(QAbstractItemView::NoSelection);
	implant_list_->setShowGrid(false);
	implant_list_->setRowCount(1);
	const int kSectionHeight = 20;
	implant_list_->setRowHeight(0, kSectionHeight);
	implant_list_->verticalHeader()->setVisible(false);

	QLabel* placed_label = new QLabel();
	placed_label->setText(QString::fromLocal8Bit("Placed : \nAngle(°) : "));
	QHBoxLayout* placed_layout = new QHBoxLayout();
	placed_layout->setAlignment(Qt::AlignLeft);
	placed_layout->setContentsMargins(0, 0, 0, 0);
	placed_layout->setSpacing(9);
	placed_layout->addWidget(placed_label);
	placed_layout->addWidget(implant_list_);

	m_contentLayout->setContentsMargins(9, 9, 9, 9);
	m_contentLayout->setSpacing(10);
	m_contentLayout->addLayout(selected_layout);
	m_contentLayout->addLayout(placed_layout);

	this->setFixedSize(260, 120);
}

void ImplantAngleDialog::UpdateList() {
	const auto& implant_resource = ResourceContainer::GetInstance()->GetImplantResource();
	const auto& implant_data = implant_resource.data();

	QStringList placed_labels;
	placed_labels.reserve(implant_data.size());
	QString selected_content = select_implant_->currentText();
	glm::vec3 selected_axis_normal;
	for (const auto& data : implant_data) {
		QString contents = QString("#%1").arg(data.second->id());
		if (contents.compare(selected_content, Qt::CaseSensitivity::CaseInsensitive) != 0) {
			placed_labels.push_back(contents);
		} else {
			selected_axis_normal = normalize(data.second->ImplantDirection());
		}
	}
	
	implant_list_->setColumnCount(placed_labels.size());
	implant_list_->setHorizontalHeaderLabels(placed_labels);
	QHeaderView *header = implant_list_->horizontalHeader();
	header->setHighlightSections(false);
	header->setStyleSheet(CW3Theme::getInstance()->TableWidgetHeaderStyleSheet());

	for (int col = 0; col < placed_labels.size(); ++col) {
		QString label = placed_labels.at(col);
		label.remove(0, 1);
		int id = label.toInt();
		glm::vec3 target_axis_normal = normalize(implant_data.at(id)->ImplantDirection());
		float angle = acos(glm::dot(target_axis_normal, selected_axis_normal)) * 180.0f / M_PI;
		float nearest = roundf(angle * 10) / 10;

		QTableWidgetItem *item = new QTableWidgetItem(QString::number(nearest));
		item->setTextAlignment(Qt::AlignCenter);
		implant_list_->setItem(0, col, item);
		implant_list_->setColumnWidth(col, 45);
	}

	if (placed_labels.size() >= 4) {
		this->setFixedSize(260, 130);
	} else {
		this->setFixedSize(260, 120);
	}
}

void ImplantAngleDialog::Connections() {
	connect(select_implant_, SIGNAL(currentTextChanged(QString)), this, SLOT(slotSelectImplant(QString)));
}
