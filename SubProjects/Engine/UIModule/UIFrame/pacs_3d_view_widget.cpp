#include "pacs_3d_view_widget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Math.h"

#include "../../Core/W3DicomIO/dicom_send.h"

#include "../UIComponent/pacs_view_3d.h"

#include "../UIPrimitive/W3DicomLoaderScrollbar.h"

namespace
{
	const int kSceneSize = 250;
	const QMargins kMarginZero(0, 0, 0, 0);

	const glm::vec3 kHorizontalAxis = glm::vec3(0.f, 0.f, -1.f);
	const glm::vec3 kVerticalAxis = glm::vec3(1.f, 0.f, 0.f);

	const int kViewCnt = 3;
}

Pacs3DViewWidget::Pacs3DViewWidget(QWidget* parent /*= 0*/)
	: QWidget(parent)
{
	Initialize();
	SetLayout();
	Connections();

	SetRotationSliderRange();
}

Pacs3DViewWidget::~Pacs3DViewWidget()
{
	for (int i = 0; i < kViewCnt; ++i)
	{
		SAFE_DELETE_LATER(pacs_view_3d_[i]);
	}
	SAFE_DELETE_LATER(slider_);
	SAFE_DELETE_LATER(label_image_cnt_);
	SAFE_DELETE_LATER(main_layout_);
}

void Pacs3DViewWidget::EmitCreateDCMFile()
{
	int cnt = slider_->getEnd() - slider_->getStart() + 1;
	pacs_view_3d_[0]->EmitCreateDCMFile(cnt);
}

void Pacs3DViewWidget::slotRotationTypeChange()
{
	is_rot_horizontal_ = !is_rot_horizontal_;

	UpdateView(0, static_cast<int>(slider_->getStart()));
	UpdateView(1, static_cast<int>(slider_->getMiddle()));
	UpdateView(2, static_cast<int>(slider_->getEnd()));
}

void Pacs3DViewWidget::slotRotationDirChange()
{
	is_dir_anterior_ = !is_dir_anterior_;

	UpdateView(0, static_cast<int>(slider_->getStart()));
	UpdateView(1, static_cast<int>(slider_->getMiddle()));
	UpdateView(2, static_cast<int>(slider_->getEnd()));
}

void Pacs3DViewWidget::slotNerveVisibility(bool is_visible)
{
	for (int i = 0; i < kViewCnt; ++i)
	{
		pacs_view_3d_[i]->SetVisibleNerve(is_visible);
	}
}

void Pacs3DViewWidget::slotImplantVisibility(bool is_visible)
{
	for (int i = 0; i < kViewCnt; ++i)
	{
		pacs_view_3d_[i]->SetVisibleImplant(is_visible);
	}
}

void Pacs3DViewWidget::slotUpdateAngle(const int angle)
{
	if (angle_ == angle)
	{
		return;
	}

	angle_ = angle;
	SetRotationSliderRange();
}

void Pacs3DViewWidget::slotSliderValueChange(int index, float value)
{
	UpdateView(index, static_cast<int>(value));

	if (index != 1)
	{
		SetTextImageCntLabel();
	}
}

void Pacs3DViewWidget::Initialize()
{
	main_layout_ = new QVBoxLayout();
	main_layout_->setContentsMargins(0, 0, 0, 0);
	main_layout_->setSpacing(0);

	setLayout(main_layout_);

	for (int i = 0; i < kViewCnt; ++i)
	{
		QSizePolicy size_policy(QSizePolicy::Fixed, QSizePolicy::Fixed);

		pacs_view_3d_[i] = new PACSView3D(this);
		pacs_view_3d_[i]->setSizePolicy(size_policy);
		pacs_view_3d_[i]->setSceneRect(QRectF(0, 0, kSceneSize, kSceneSize));
	}

	slider_ = new CW3DicomLoaderScrollbar(this);

	GlobalPreferences::PACSDefaultSetting& setting = GlobalPreferences::GetInstance()->preferences_.pacs_default_setting;

	is_rot_horizontal_ = !setting.mpr_is_3d_vertical;
	is_dir_anterior_ = !setting.mpr_is_dir_posterior;

	angle_ = setting.mpr_3d_angle;
}

void Pacs3DViewWidget::SetLayout()
{
	main_layout_->addLayout(CreateMPRContentsLayout());
}

void Pacs3DViewWidget::Connections()
{
	if (slider_)
	{
		connect(slider_, &CW3DicomLoaderScrollbar::sigScrollTranslated, this, &Pacs3DViewWidget::slotSliderValueChange);
	}

	for (int i = 0; i < kViewCnt; ++i)
	{
		connect(pacs_view_3d_[i], &PACSView3D::sigCreateDCMFiles_uchar, this, &Pacs3DViewWidget::sigCreateDCMFiles_uchar);
	}
}

QVBoxLayout* Pacs3DViewWidget::CreateMPRContentsLayout()
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
				view_layout->addWidget(pacs_view_3d_[i]);
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

void Pacs3DViewWidget::SetRotationSliderRange()
{
	if (slider_ == nullptr)
	{
		return;
	}

	slider_->setRange(0, 359 / angle_);

	SetTextImageCntLabel();
}

void Pacs3DViewWidget::SetTextImageCntLabel()
{
	if (slider_ == nullptr || label_image_cnt_ == nullptr)
	{
		return;
	}

	int num = slider_->getEnd() - slider_->getStart() + 1;
	label_image_cnt_->setText(QString::number(num));
}

void Pacs3DViewWidget::UpdateView(const int index, const int value)
{
	float radian = glm::radians(static_cast<float>(angle_)) * static_cast<int>(value);
	if (is_dir_anterior_ == false)
	{
		radian += glm::pi<float>();
	}

	glm::vec3 axis;
	if (is_rot_horizontal_)
	{
		axis = kHorizontalAxis;
	}
	else
	{
		axis = kVerticalAxis;
	}

	glm::mat4 rot_mat = glm::rotate(radian, axis);
	pacs_view_3d_[index]->ForceRotateMatrix(rot_mat);

	pacs_view_3d_[index]->set_angle(angle_);
	pacs_view_3d_[index]->set_num(value);
	pacs_view_3d_[index]->set_anterior(is_dir_anterior_);
	pacs_view_3d_[index]->set_horizontal(is_rot_horizontal_);
}
