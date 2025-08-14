#include "pacs_dialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QStackedWidget>

#include <QDir>
#include <QApplication>

#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3MessageBox.h"
#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Memory.h"

#include "../../Core/W3DicomIO/dicom_send.h"

#include "../../Module//MPREngine/W3MPREngine.h"

#include "pacs_common_widget.h"
#include "pacs_mpr_view_widget.h"
#include "pacs_mpr_setting_widget.h"
#include "pacs_3d_view_widget.h"
#include "pacs_3d_setting_widget.h"
#include "pacs_pano_view_widget.h"
#include "pacs_pano_setting_widget.h"
#include "pacs_lightbox_setting_widget.h"

namespace
{
	const int kSceneSize = 250;

	//임시
	const QString kFolderPath = "/temp/";
}

PacsDialog::PacsDialog(TabType tab_type, const QStringList& view_list, QWidget* parent /*= 0*/)
	: CW3Dialog(lang::LanguagePack::txt_pacs(), parent)
	, tab_type_(tab_type)
	, view_list_(view_list)
{
	setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::WindowStaysOnTopHint);

	CreateWidget();
}

PacsDialog::~PacsDialog()
{
	SAFE_DELETE_LATER(common_widget_);

	SAFE_DELETE_LATER(mpr_view_widget_);
	SAFE_DELETE_LATER(mpr_setting_widget_);

	SAFE_DELETE_LATER(vr_view_widget_);
	SAFE_DELETE_LATER(vr_setting_widget_);

	SAFE_DELETE_LATER(pano_view_widget_);
	SAFE_DELETE_LATER(pano_setting_widget_);
}

void PacsDialog::EmitMPRViewInitialize(CW3MPREngine* mpr_engine)
{
	if (initial_setting_)
	{
		return;
	}

	MPRSetLayout();
	MPRConnections();

	if (!is_mpr_lightbox_)
	{
		mpr_view_widget_->SetSurfaceVisible(common_widget_->nerve_visible(), common_widget_->implant_visible());
		mpr_view_widget_->SetCrossPos(*mpr_engine->getMPRrotCenterInVol(0));
		mpr_view_type_ = MPRViewType::AXIAL;
		emit sigChangeMPRType(mpr_view_type_);
	}

	initial_setting_ = true;
}

void PacsDialog::EmitPanoViewInitialize()
{
	if (initial_setting_)
	{
		return;
	}

	PanoSetLayout();
	PanoConnections();

	pano_view_widget_->InitPanoView(common_widget_->nerve_visible(), common_widget_->implant_visible());

	initial_setting_ = true;
}

void PacsDialog::SetLightBoxViewInfo(const int filter, const int thickness)
{
	lightbox_setting_widget_->SetFilterValue(filter);
	lightbox_setting_widget_->SetThicknessValue(thickness);
}

void PacsDialog::PanoUpdate()
{
	if (pano_view_widget_ == nullptr)
	{
		return;
	}

	pano_view_widget_->PanoUpdate();
}

void PacsDialog::slotSetMPRPlaneInfo(const glm::vec3& plane_right, const glm::vec3& plane_back, const int available_depth)
{
	mpr_view_widget_->SetMPRPlaneInfo(mpr_view_type_, plane_right, plane_back, available_depth);
}

void PacsDialog::slotChangeViewType(const int index)
{
	if (tab_type_ == TabType::TAB_MPR)
	{
		if (index >= 0 && index <= 2)
		{
			//axial = 0, sagittal = 1, coronal = 2
			//swap axial = 0, sagittal = 2, coronal = 1
			MPRViewType type = static_cast<MPRViewType>(index);
			if (mpr_view_type_ != type)
			{
				mpr_view_type_ = type;
				emit sigChangeMPRType(mpr_view_type_);
			}
		}
	}
	else if (tab_type_ == TabType::TAB_PANORAMA)
	{
		//panorama = 0, cross_section = 1
		if (index == 0)
		{
			pano_setting_widget_->RestoreSliderValue();
		}
		else if (index == 1)
		{
			int filter_level, thickness;
			emit sigRequestGetPanoCrossSectionViewInfo(filter_level, thickness);

			pano_setting_widget_->SetFilterValue(filter_level);
			pano_setting_widget_->SetThicknessValue(thickness);
		}
	}
}

void PacsDialog::slotSendButtonClick()
{
	int server_cnt = common_widget_->server_cnt();
	if (server_cnt == 0)
	{
		return;
	}

	int index = common_widget_->cur_server_index();
	QStringList info = GlobalPreferences::GetInstance()->GetPACSServerInfo(index);

	if (DicomSend::NetworkConnectionCheck(info[1], info[2], info[3].toInt()) == false)
	{
		QString error_msg = "network error";
		common::Logger::instance()->Print(common::LogType::ERR, error_msg.toStdString());
		CW3MessageBox msg_box("Will3D", error_msg, CW3MessageBox::Critical);
		msg_box.exec();
		return;
	}

	QDir dir(QApplication::applicationDirPath() + kFolderPath);
	if (dir.exists())
	{
		dir.removeRecursively();
	}

	if (tab_type_ == TabType::TAB_MPR)
	{
		if (is_mpr_lightbox_)
		{
			bool is_nerve = common_widget_->nerve_visible();
			bool is_implant = common_widget_->implant_visible();
			int filter = lightbox_setting_widget_->GetFilterValue();
			int thickness = lightbox_setting_widget_->GetThicknessValue();

			emit sigRequestCreateDCMFiles(is_nerve, is_implant, filter, thickness);
		}
		else
		{
			if (mpr_view_widget_->isVisible())
			{
				mpr_view_widget_->EmitCreateDCMFile();
			}
			else
			{
				vr_view_widget_->EmitCreateDCMFile();
			}
		}
	}
	else if (tab_type_ == TabType::TAB_PANORAMA)
	{
		int index = common_widget_->GetViewListIndex();
		if (index == 0) //panorama
		{
			pano_view_widget_->EmitCreateDCMFile();
		}
		else if (index == 1) //cross_section
		{
			bool is_nerve = common_widget_->nerve_visible();
			bool is_implant = common_widget_->implant_visible();
			int filter = pano_setting_widget_->GetFilterValue();
			int thickness = pano_setting_widget_->GetThicknessValue();

			emit sigRequestCreateDCMFiles(is_nerve, is_implant, filter, thickness);
		}
	}

	emit sigPACSSend(info);

	GlobalPreferences::GetInstance()->AddCountPACSSerisNumber();
}

void PacsDialog::CreateWidget()
{
	common_widget_ = new PacsCommonWidget(tab_type_, view_list_, this);

	if (tab_type_ == TabType::TAB_MPR)
	{
		if (view_list_.size() == 1)
		{
			is_mpr_lightbox_ = true;
			lightbox_setting_widget_ = new PacsLightboxSettingWidget(this);
		}
		else
		{
			mpr_view_widget_ = new PacsMPRViewWidget(this);
			mpr_setting_widget_ = new PacsMPRSettingWidget(this);

			vr_view_widget_ = new Pacs3DViewWidget(this);
			vr_setting_widget_ = new Pacs3DSettingWidget(this);
		}
	}
	else if (tab_type_ == TabType::TAB_PANORAMA)
	{
		pano_setting_widget_ = new PacsPanoSettingWidget(this);
		pano_view_widget_ = new PacsPanoViewWidget(this);
	}
}

void PacsDialog::MPRSetLayout()
{
	m_contentLayout->setContentsMargins(kMarginZero);
	m_contentLayout->setSpacing(0);
	m_contentLayout->addLayout(CreateMPRContentsLayout());
}

void PacsDialog::MPRConnections()
{
	if (!is_mpr_lightbox_)
	{
		//Common
		connect(common_widget_, &PacsCommonWidget::sigChangeViewType, this, &PacsDialog::slotChangeViewType);
		connect(common_widget_, &PacsCommonWidget::sigNerveVisibility, mpr_view_widget_, &PacsMPRViewWidget::slotNerveVisibility);
		connect(common_widget_, &PacsCommonWidget::sigImplantVisibility, mpr_view_widget_, &PacsMPRViewWidget::slotImplantVisibility);
		connect(common_widget_, &PacsCommonWidget::sigNerveVisibility, vr_view_widget_, &Pacs3DViewWidget::slotNerveVisibility);
		connect(common_widget_, &PacsCommonWidget::sigImplantVisibility, vr_view_widget_, &Pacs3DViewWidget::slotImplantVisibility);

		//MPRView - mpr
		connect(mpr_view_widget_, &PacsMPRViewWidget::sigCreateDCMFiles_ushort, this, &PacsDialog::sigCreateDCMFiles_ushort);
		connect(mpr_view_widget_, &PacsMPRViewWidget::sigCreateDCMFiles_uchar, this, &PacsDialog::sigCreateDCMFiles_uchar);

		connect(mpr_setting_widget_, &PacsMPRSettingWidget::sigSliderTypeChange, mpr_view_widget_, &PacsMPRViewWidget::slotSliderTypeChange);
		connect(mpr_setting_widget_, &PacsMPRSettingWidget::sigRotationTypeChange, mpr_view_widget_, &PacsMPRViewWidget::slotRotationTypeChange);
		connect(mpr_setting_widget_, &PacsMPRSettingWidget::sigUpdateInterval, mpr_view_widget_, &PacsMPRViewWidget::slotUpdateInterval);
		connect(mpr_setting_widget_, &PacsMPRSettingWidget::sigUpdateAngle, mpr_view_widget_, &PacsMPRViewWidget::slotUpdateAngle);
		connect(mpr_setting_widget_, &PacsMPRSettingWidget::sigUpdateThickness, mpr_view_widget_, &PacsMPRViewWidget::slotUpdateThickness);
		connect(mpr_setting_widget_, &PacsMPRSettingWidget::sigUpdateFilter, mpr_view_widget_, &PacsMPRViewWidget::slotUpdateFilter);

		//MPRView - vr
		connect(vr_view_widget_, &Pacs3DViewWidget::sigCreateDCMFiles_uchar, this, &PacsDialog::sigCreateDCMFiles_uchar);

		connect(vr_setting_widget_, &Pacs3DSettingWidget::sigRotationTypeChange, vr_view_widget_, &Pacs3DViewWidget::slotRotationTypeChange);
		connect(vr_setting_widget_, &Pacs3DSettingWidget::sigRotationDirChange, vr_view_widget_, &Pacs3DViewWidget::slotRotationDirChange);
		connect(vr_setting_widget_, &Pacs3DSettingWidget::sigUpdateAngle, vr_view_widget_, &Pacs3DViewWidget::slotUpdateAngle);
	}	
}

void PacsDialog::PanoSetLayout()
{
	m_contentLayout->setContentsMargins(kMarginZero);
	m_contentLayout->setSpacing(0);
	m_contentLayout->addLayout(CreatePanoContentsLayout());
}

void PacsDialog::PanoConnections()
{
	//Common
	connect(common_widget_, &PacsCommonWidget::sigChangeViewType, this, &PacsDialog::slotChangeViewType);
	connect(common_widget_, &PacsCommonWidget::sigNerveVisibility, pano_view_widget_, &PacsPanoViewWidget::slotNerveVisibility);
	connect(common_widget_, &PacsCommonWidget::sigImplantVisibility, pano_view_widget_, &PacsPanoViewWidget::slotImplantVisibility);

	//Pano
	connect(pano_view_widget_, &PacsPanoViewWidget::sigPanoUpdate, this, &PacsDialog::sigPanoUpdate);
	connect(pano_view_widget_, &PacsPanoViewWidget::sigCreateDCMFiles_ushort, this, &PacsDialog::sigCreateDCMFiles_ushort);
	connect(pano_view_widget_, &PacsPanoViewWidget::sigCreateDCMFiles_uchar, this, &PacsDialog::sigCreateDCMFiles_uchar);

	connect(pano_setting_widget_, &PacsPanoSettingWidget::sigUpdateFilter, pano_view_widget_, &PacsPanoViewWidget::slotUpdateFilter);
	connect(pano_setting_widget_, &PacsPanoSettingWidget::sigUpdateThickness, this, &PacsDialog::sigPanoUpdateThickness);
}

QHBoxLayout* PacsDialog::CreateMPRContentsLayout()
{
	QHBoxLayout* constents_layout = new QHBoxLayout();
	{
		if (!is_mpr_lightbox_)
		{
			constents_layout->addLayout(CreateMPRContentsLeftLayout());
		}

		constents_layout->addLayout(CreateMPRContentsRightLayout());
	}

	return constents_layout;
}

QVBoxLayout* PacsDialog::CreateMPRContentsLeftLayout()
{
	QVBoxLayout* left_layout = new QVBoxLayout();
	{
		QStackedWidget* stacked_widget = new QStackedWidget(this);

		stacked_widget->addWidget(mpr_view_widget_);
		stacked_widget->addWidget(vr_view_widget_);

		connect(common_widget_, &PacsCommonWidget::sigChangeViewType, [=](int index)
		{
			int cur_index = stacked_widget->currentIndex();
			if (cur_index == 0 && index == 3) //2D
			{
				stacked_widget->setCurrentIndex(1);
			}
			else if (cur_index == 1 && index < 3) //3D
			{
				stacked_widget->setCurrentIndex(0);
			}
		});

		left_layout->addWidget(stacked_widget);
	}

	return left_layout;
}

QVBoxLayout* PacsDialog::CreateMPRContentsRightLayout()
{
	QVBoxLayout* right_layout = new QVBoxLayout();
	{
		QVBoxLayout* setting_layout = new QVBoxLayout();
		{
			setting_layout->setAlignment(Qt::AlignTop);
			setting_layout->setMargin(10);
			setting_layout->setSpacing(10);

			setting_layout->addWidget(common_widget_);

			if (!is_mpr_lightbox_)
			{
				QStackedWidget* stacked_widget = new QStackedWidget(this);

				stacked_widget->addWidget(mpr_setting_widget_);
				stacked_widget->addWidget(vr_setting_widget_);

				connect(common_widget_, &PacsCommonWidget::sigChangeViewType, [=](int index)
				{
					int cur_index = stacked_widget->currentIndex();
					if (cur_index == 0 && index == 3) //2D
					{
						stacked_widget->setCurrentIndex(1);
					}
					else if (cur_index == 1 && index < 3) //3D
					{
						stacked_widget->setCurrentIndex(0);
					}
				});

				setting_layout->addWidget(stacked_widget);
			}
			else
			{
				setting_layout->addWidget(lightbox_setting_widget_);
			}
		}

		right_layout->addLayout(setting_layout);
		right_layout->addLayout(CreateButtonLayout());
	}

	return right_layout;
}

QHBoxLayout* PacsDialog::CreatePanoContentsLayout()
{
	QHBoxLayout* constents_layout = new QHBoxLayout();
	{
		QStackedWidget* stacked_widget = new QStackedWidget(this);

		QWidget* temp_widget = new QWidget(this);

		stacked_widget->addWidget(pano_view_widget_);
		stacked_widget->addWidget(temp_widget);

		connect(common_widget_, &PacsCommonWidget::sigChangeViewType, [=](int index)
		{
			if (index == 0)
			{
				pano_view_widget_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
				temp_widget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

				connect(pano_setting_widget_, &PacsPanoSettingWidget::sigUpdateFilter, pano_view_widget_, &PacsPanoViewWidget::slotUpdateFilter);
				connect(pano_setting_widget_, &PacsPanoSettingWidget::sigUpdateThickness, this, &PacsDialog::sigPanoUpdateThickness);
			}
			else if (index == 1)
			{
				pano_view_widget_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
				temp_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

				disconnect(pano_setting_widget_, &PacsPanoSettingWidget::sigUpdateFilter, pano_view_widget_, &PacsPanoViewWidget::slotUpdateFilter);
				disconnect(pano_setting_widget_, &PacsPanoSettingWidget::sigUpdateThickness, this, &PacsDialog::sigPanoUpdateThickness);
			}

			stacked_widget->setCurrentIndex(index);
			this->adjustSize();
		});

		constents_layout->addWidget(stacked_widget);
		constents_layout->addLayout(CreatePanoContentsRightLayout());
	}

	return constents_layout;
}

QVBoxLayout* PacsDialog::CreatePanoContentsRightLayout()
{
	QVBoxLayout* right_layout = new QVBoxLayout();
	{
		right_layout->setMargin(0);
		right_layout->setSpacing(0);
		QVBoxLayout* top_layout = new QVBoxLayout();
		{
			top_layout->setAlignment(Qt::AlignTop);
			top_layout->setMargin(10);
			top_layout->setSpacing(10);

			top_layout->addWidget(common_widget_);
			top_layout->addWidget(pano_setting_widget_);
		}

		right_layout->addLayout(top_layout);
		right_layout->addLayout(CreateButtonLayout());
	}

	return right_layout;
}

QHBoxLayout* PacsDialog::CreateButtonLayout()
{
	QHBoxLayout* button_layout = new QHBoxLayout();
	{
		button_layout->setAlignment(Qt::AlignBottom | Qt::AlignRight);
		button_layout->setMargin(10);
		button_layout->setSpacing(10);

		QToolButton* send_button = new QToolButton(this);
		QToolButton* cancel_button = new QToolButton(this);

		send_button->setText(lang::LanguagePack::txt_send());
		cancel_button->setText(lang::LanguagePack::txt_cancel());

		button_layout->addWidget(send_button);
		button_layout->addWidget(cancel_button);

		positive_button_ = send_button;

		connect(send_button, &QToolButton::clicked, this, &PacsDialog::slotSendButtonClick);
		connect(cancel_button, &QToolButton::clicked, this, &PacsDialog::reject);
	}

	return button_layout;
}
