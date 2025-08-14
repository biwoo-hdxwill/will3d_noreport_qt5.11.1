#include "W3ToolHBar.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>

#include <Engine/Common/Common/W3Theme.h>
#include <Engine/Common/Common/W3Memory.h>
#include <Engine/Resource/Resource/W3ImageHeader.h>

#include <Engine/UIModule/UITools/tool_box.h>
#include <Engine/UIModule/UITools/tool_mgr.h>
#include "dicom_info_box.h"

CW3ToolHBar::CW3ToolHBar(QWidget* parent) : QWidget(parent) {
	this->setStyleSheet(CW3Theme::getInstance()->toolbarStyleSheet());
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	QLabel* empty_space = new QLabel();
	empty_space->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	empty_space->setObjectName("EmptySpace");

	dicom_info_ = new DicomInfoBox();
	dicom_info_->setVisible(false);

	main_layout_ = new QHBoxLayout();
	main_layout_->setSpacing(1);
	main_layout_->setAlignment(Qt::AlignLeft);
	main_layout_->setContentsMargins(4, 0, 4, 0);
	main_layout_->addLayout(ToolMgr::instance()->GetCommonTaskToolLayout());
	main_layout_->addWidget(dicom_info_);
	main_layout_->addWidget(empty_space);

	this->setLayout(main_layout_);
}

CW3ToolHBar::~CW3ToolHBar() {
	SAFE_DELETE_OBJECT(dicom_info_);
}

void CW3ToolHBar::SetDicomInfo(CW3ImageHeader* header) {
	if (!header)
		return;

	dicom_info_->SetPatientID(header->getPatientID());
	dicom_info_->SetPatientNameGender(header->getPatientName(), header->getPatientSex());
	dicom_info_->SetPatientAge(header->getPatientAge());
	dicom_info_->SetImageKvp(header->getKVP());
	dicom_info_->SetImageMa(header->getXRayTubeCurrent());
	dicom_info_->SetScanDate(header->getSeriesDate());
	dicom_info_->setVisible(true);
}
