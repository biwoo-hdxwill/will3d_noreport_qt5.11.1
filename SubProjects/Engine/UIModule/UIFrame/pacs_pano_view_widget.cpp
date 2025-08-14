#include "pacs_pano_view_widget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

//임시
#include <QDir>
#include <QApplication>
//

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Math.h"

#include "../../Core/W3DicomIO/dicom_send.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/pano_resource.h"

#include "../UIComponent/pacs_view_pano.h"

#include "../UIPrimitive/W3DicomLoaderScrollbar.h"

namespace
{
	const int kViewCnt = 3;
	const int kSceneSize = 250;
	const QMargins kMarginZero(0, 0, 0, 0);

	const glm::vec3 kHorizontalAxis = glm::vec3(0.f, 0.f, -1.f);
	const glm::vec3 kVerticalAxis = glm::vec3(1.f, 0.f, 0.f);

	//임시
	const QString kFolderPath = "/temp/";
}

PacsPanoViewWidget::PacsPanoViewWidget(QWidget* parent /*= 0*/)
	: QWidget(parent)
{
	Initialize();
	SetLayout();
	Connections();
}

PacsPanoViewWidget::~PacsPanoViewWidget()
{
	for (int i = 0; i < kViewCnt; ++i)
	{
		SAFE_DELETE_LATER(pacs_view_pano_[i]);
	}
	SAFE_DELETE_LATER(slider_);
	SAFE_DELETE_LATER(label_image_cnt_);
	SAFE_DELETE_LATER(main_layout_);
}

void PacsPanoViewWidget::PanoUpdate()
{
	for (int i = 0; i < kViewCnt; ++i)
	{
		pacs_view_pano_[i]->UpdatedPano();
	}
}

void PacsPanoViewWidget::InitPanoView(bool nerve, bool implant)
{
	for (int i = 0; i < kViewCnt; ++i)
	{
		pacs_view_pano_[i]->InitPano(nerve, implant);
	}
}

void PacsPanoViewWidget::EmitCreateDCMFile()
{
	int range = slider_->getEnd() - slider_->getStart() + 1;
	pacs_view_pano_[0]->EmitCreateDCMFile(range);
}

void PacsPanoViewWidget::slotNerveVisibility(bool is_visible)
{
	for (int i = 0; i < kViewCnt; ++i)
	{
		pacs_view_pano_[i]->SetNerveVisibility(is_visible);
	}
}

void PacsPanoViewWidget::slotImplantVisibility(bool is_visible)
{
	for (int i = 0; i < kViewCnt; ++i)
	{
		pacs_view_pano_[i]->SetImplantVisibility(is_visible);
	}
}

void PacsPanoViewWidget::slotUpdateFilter(const int filter)
{
	for (int i = 0; i < kViewCnt; ++i)
	{
		pacs_view_pano_[i]->SetSharpenLevel(filter);
	}
}

void PacsPanoViewWidget::slotSliderValueChange(int index, float value)
{
	int new_value = static_cast<int>(value);
	pacs_view_pano_[index]->SetCurPosValue(new_value);

	if (index != 1)
	{
		SetTextImageCntLabel();
	}
}

void PacsPanoViewWidget::Initialize()
{
	main_layout_ = new QVBoxLayout();
	main_layout_->setContentsMargins(0, 0, 0, 0);
	main_layout_->setSpacing(0);

	setLayout(main_layout_);

	for (int i = 0; i < kViewCnt; ++i)
	{
		QSizePolicy size_policy(QSizePolicy::Fixed, QSizePolicy::Fixed);

		pacs_view_pano_[i] = new PACSViewPano(this);
		pacs_view_pano_[i]->setSizePolicy(size_policy);
		pacs_view_pano_[i]->setSceneRect(QRectF(0, 0, kSceneSize, kSceneSize));
	}

	slider_ = new CW3DicomLoaderScrollbar(this);

	const auto& res_pano = ResourceContainer::GetInstance()->GetPanoResource();
	slider_->setRange(0, res_pano.pano_3d_depth() - 1);

	pacs_view_pano_[0]->SetCurPosValue(slider_->getStart());
	pacs_view_pano_[1]->SetCurPosValue(slider_->getMiddle());
	pacs_view_pano_[2]->SetCurPosValue(slider_->getEnd());

	SetTextImageCntLabel();
}

void PacsPanoViewWidget::SetLayout()
{
	main_layout_->addLayout(CreateContentsLayout());
}

void PacsPanoViewWidget::Connections()
{
	if (slider_)
	{
		connect(slider_, &CW3DicomLoaderScrollbar::sigScrollTranslated, this, &PacsPanoViewWidget::slotSliderValueChange);
	}

	for (int i = 0; i < kViewCnt; ++i)
	{		
		connect(pacs_view_pano_[i], &PACSViewPano::sigPanoUpdate, [=](int value)
		{
			emit sigPanoUpdate(value);
		});

		connect(pacs_view_pano_[i], &PACSViewPano::sigCreateDCMFiles_ushort, this, &PacsPanoViewWidget::sigCreateDCMFiles_ushort);
		connect(pacs_view_pano_[i], &PACSViewPano::sigCreateDCMFiles_uchar, this, &PacsPanoViewWidget::sigCreateDCMFiles_uchar);
	}
}

QVBoxLayout* PacsPanoViewWidget::CreateContentsLayout()
{
	QVBoxLayout* contents_layout = new QVBoxLayout();
	{
		contents_layout->setMargin(10);
		contents_layout->setSpacing(10);

		QHBoxLayout* view_layout = new QHBoxLayout();
		{
			view_layout->setContentsMargins(kMarginZero);
			view_layout->setSpacing(10);

			for (int i = 0; i < kViewCnt; ++i)
			{
				view_layout->addWidget(pacs_view_pano_[i]);
			}
		}

		QVBoxLayout* slider_layout = new QVBoxLayout();
		{
			slider_layout->setSpacing(0);
			slider_layout->setContentsMargins(kMarginZero);

			QHBoxLayout* label_layout = new QHBoxLayout();
			{
				label_layout->setSpacing(0);
				label_layout->setContentsMargins(kMarginZero);
				label_layout->setAlignment(Qt::AlignLeft);

				QLabel* label_caption = new QLabel(this);
				label_caption->setText(lang::LanguagePack::txt_total_number_of_selected_images() + " : ");

				label_image_cnt_ = new QLabel(this);

				label_layout->addWidget(label_caption);
				label_layout->addWidget(label_image_cnt_);
			}

			slider_layout->addLayout(label_layout);
			slider_layout->addWidget(slider_);
		}

		contents_layout->addLayout(view_layout);
		contents_layout->addLayout(slider_layout);
	}

	return contents_layout;
}

void PacsPanoViewWidget::SetTextImageCntLabel()
{
	if (slider_ == nullptr || label_image_cnt_ == nullptr)
	{
		return;
	}

	int num = slider_->getEnd() - slider_->getStart() + 1;
	label_image_cnt_->setText(QString::number(num));
}
