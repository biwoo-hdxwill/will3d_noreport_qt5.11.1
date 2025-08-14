#include "pacs_mpr_view_widget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

//임시
#include <QDir>
#include <QApplication>

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Math.h"

#include "../../Core/W3DicomIO/dicom_send.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/W3Image3D.h"

#include "../UIComponent/pacs_view_mpr.h"

#include "../UIPrimitive/W3DicomLoaderScrollbar.h"

namespace
{
	const int kSceneSize = 250;
	const QMargins kMarginZero(0, 0, 0, 0);

	//임시
	const QString kFolderPath = "/temp/";
	const int kViewCnt = 3;
}

PacsMPRViewWidget::PacsMPRViewWidget(QWidget* parent /*= 0*/)
	: QWidget(parent)
{
	Initialize();
	SetLayout();
	Connections();
}

PacsMPRViewWidget::~PacsMPRViewWidget()
{
	for (int i = 0; i < kViewCnt; ++i)
	{
		SAFE_DELETE_LATER(pacs_view_mpr_[i]);
	}
	SAFE_DELETE_LATER(slider_);
	SAFE_DELETE_LATER(label_image_cnt_);
	SAFE_DELETE_LATER(main_layout_);
}

void PacsMPRViewWidget::SetMPRPlaneInfo(MPRViewType mpr_type, const glm::vec3& plane_right, const glm::vec3& plane_back, const int available_depth)
{
	if (mpr_view_type_ == mpr_type || mpr_type == MPRViewType::MPR_END || mpr_type == MPRViewType::MPR_UNKNOWN)
	{
		return;
	}

	mpr_view_type_ = mpr_type;
	bool inverse = false;
	if (mpr_view_type_ == MPRViewType::CORONAL)
	{
		inverse = true;
	}

	plane_right_ = plane_right;
	plane_back_ = plane_back;
	available_depth_ = available_depth;

	for (int i = 0; i < kViewCnt; ++i)
	{
		pacs_view_mpr_[i]->set_is_inverse_up_vec(inverse);
		pacs_view_mpr_[i]->SetPlaneInfo(plane_right_, plane_back_, available_depth_);
	}
	
	if (is_translation_)
	{
		SetTranslationSliderRange();
	}
	else
	{
		SetRotationSliderRange();
	}
}

void PacsMPRViewWidget::SetCrossPos(const glm::vec3& cross_pos)
{
	cross_pos_ = cross_pos;
	for (int i = 0; i < kViewCnt; ++i)
	{
		pacs_view_mpr_[i]->set_cross_pos(cross_pos_);
	}
}

void PacsMPRViewWidget::EmitCreateDCMFile()
{
	int range = slider_->getEnd() - slider_->getStart() + 1;

	bool draw_surface = false;
	if (draw_nerve_ || draw_implant_)
	{
		draw_surface = true;
	}

	pacs_view_mpr_[0]->EmitCreateDCMFile(range, is_translation_, draw_surface);
}

void PacsMPRViewWidget::slotSliderTypeChange()
{
	is_translation_ = !is_translation_;

	if (is_translation_)
	{
		SetTranslationSliderRange();
	}
	else
	{
		SetRotationSliderRange();
	}
}

void PacsMPRViewWidget::slotRotationTypeChange()
{
	is_rot_horizontal_ = !is_rot_horizontal_;

	for (int i = 0; i < kViewCnt; ++i)
	{
		pacs_view_mpr_[i]->UpdateRotation(is_rot_horizontal_);
	}
}

void PacsMPRViewWidget::slotNerveVisibility(bool is_visible)
{
	draw_nerve_ = is_visible;
	for (int i = 0; i < kViewCnt; ++i)
	{
		pacs_view_mpr_[i]->SetVisibleNerve(is_visible);
	}
}

void PacsMPRViewWidget::slotImplantVisibility(bool is_visible)
{
	draw_implant_ = is_visible;
	for (int i = 0; i < kViewCnt; ++i)
	{
		pacs_view_mpr_[i]->SetVisibleImplant(is_visible);
	}
}

void PacsMPRViewWidget::slotUpdateInterval(const int interval)
{
	if (interval_ == interval)
	{
		return;
	}

	interval_ = interval;
	const CW3Image3D& vol_image = ResourceContainer::GetInstance()->GetMainVolume();
	float spacing_ratio = static_cast<float>(interval_) / vol_image.pixelSpacing();

	for (int i = 0; i < kViewCnt; ++i)
	{
		pacs_view_mpr_[i]->set_spacing_ratio(spacing_ratio);
	}

	slider_->setRange(0, slider_maxinum_ / spacing_ratio);

	SetTextImageCntLabel();
}

void PacsMPRViewWidget::slotUpdateAngle(const int angle)
{
	if (angle_ == angle)
	{
		return;
	}

	angle_ = angle;
	SetRotationSliderRange();
}

void PacsMPRViewWidget::slotUpdateThickness(const int thickness)
{
	for (int i = 0; i < kViewCnt; ++i)
	{
		pacs_view_mpr_[i]->UpdateThickness(thickness, is_translation_);
	}
}

void PacsMPRViewWidget::slotUpdateFilter(const int filter)
{
	for (int i = 0; i < kViewCnt; ++i)
	{
		pacs_view_mpr_[i]->UpdateFilter(filter);
	}
}

void PacsMPRViewWidget::slotSliderValueChange(int index, float value)
{
	if (is_translation_)
	{
		pacs_view_mpr_[index]->UpdateTranslationSliderValue(static_cast<int>(value));
	}
	else
	{
		pacs_view_mpr_[index]->UpdateRotationSliderValue(static_cast<int>(value), is_rot_horizontal_);
	}

	if (index != 1)
	{
		SetTextImageCntLabel();
	}
}

void PacsMPRViewWidget::Initialize()
{
	main_layout_ = new QVBoxLayout();
	main_layout_->setContentsMargins(0, 0, 0, 0);
	main_layout_->setSpacing(0);

	setLayout(main_layout_);

	for (int i = 0; i < kViewCnt; ++i)
	{
		QSizePolicy size_policy(QSizePolicy::Fixed, QSizePolicy::Fixed);

		pacs_view_mpr_[i] = new PACSViewMPR(this);
		pacs_view_mpr_[i]->setSizePolicy(size_policy);
		pacs_view_mpr_[i]->setSceneRect(QRectF(0, 0, kSceneSize, kSceneSize));
	}

	slider_ = new CW3DicomLoaderScrollbar(this);

	GlobalPreferences::PACSDefaultSetting& setting = GlobalPreferences::GetInstance()->preferences_.pacs_default_setting;

	is_rot_horizontal_ = !setting.mpr_is_2d_vertical;
	interval_ = setting.mpr_interval;
	angle_ = setting.mpr_2d_angle;
}

void PacsMPRViewWidget::SetLayout()
{
	main_layout_->addLayout(CreateMPRContentsLayout());
}

void PacsMPRViewWidget::Connections()
{
	if (slider_)
	{
		connect(slider_, &CW3DicomLoaderScrollbar::sigScrollTranslated, this, &PacsMPRViewWidget::slotSliderValueChange);
	}

	for (int i = 0; i < kViewCnt; ++i)
	{
		connect(pacs_view_mpr_[i], &PACSViewMPR::sigCreateDCMFiles_ushort, this, &PacsMPRViewWidget::sigCreateDCMFiles_ushort);
		connect(pacs_view_mpr_[i], &PACSViewMPR::sigCreateDCMFiles_uchar, this, &PacsMPRViewWidget::sigCreateDCMFiles_uchar);
	}
}

QVBoxLayout* PacsMPRViewWidget::CreateMPRContentsLayout()
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
				view_layout->addWidget(pacs_view_mpr_[i]);
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

void PacsMPRViewWidget::SetTranslationSliderRange()
{
	if (slider_ == nullptr)
	{
		return;
	}

	const CW3Image3D& vol_image = ResourceContainer::GetInstance()->GetMainVolume();
	int width = vol_image.width();
	int height = vol_image.height();
	int depth = vol_image.depth();

	glm::vec3 vol_range = glm::vec3(width, height, depth) - glm::vec3(1.f);
	glm::vec3 vol_center = vol_range * 0.5f;

	glm::vec3 up = glm::normalize(glm::cross(plane_back_, plane_right_));
	if (mpr_view_type_ != MPRViewType::CORONAL)
	{
		up = -up;
	}

	glm::vec3 dir = up * static_cast<float>(available_depth_);

	glm::vec3 destination_min = cross_pos_ - dir;
	glm::vec3 destination_max = cross_pos_ + dir;

	destination_min = W3::FitVolRange(vol_range, cross_pos_, destination_min);
	destination_max = W3::FitVolRange(vol_range, cross_pos_, destination_max);

	slider_maxinum_ = glm::length(destination_max - destination_min);

	//offset
	glm::vec3 center_to_dest = destination_min - vol_center;
	float dot = glm::dot(glm::normalize(center_to_dest), glm::normalize(plane_back_));
	float arc = acosf(dot);
	int vertical_distance = static_cast<int>(glm::length(center_to_dest) * sin(arc));

	glm::vec3 vol_center_min = W3::FitVolRange(vol_range, vol_center, vol_center - dir);
	int min_slider_value = static_cast<int>(glm::length(vol_center - vol_center_min));

	int half_depth = static_cast<int>(available_depth_ * 0.5f);
	int start_offset = half_depth - min_slider_value;
	int offset = half_depth - vertical_distance - start_offset;
	if (offset < 0)
	{
		offset = 0;
	}

	float pixel_spacing = vol_image.pixelSpacing();
	float spacing_ratio = static_cast<float>(interval_) / pixel_spacing;

	for (int i = 0; i < kViewCnt; ++i)
	{
		pacs_view_mpr_[i]->set_spacing_ratio(spacing_ratio);
		pacs_view_mpr_[i]->set_offset(offset);
	}

	slider_->setRange(0, slider_maxinum_ / spacing_ratio);

	SetTextImageCntLabel();
}

void PacsMPRViewWidget::SetRotationSliderRange()
{
	if (slider_ == nullptr)
	{
		return;
	}

	slider_maxinum_ = 359;

	float radian = glm::radians(static_cast<float>(angle_));
	for (int i = 0; i < kViewCnt; ++i)
	{
		pacs_view_mpr_[i]->set_rotation_angle(radian);
	}

	slider_->setRange(0, slider_maxinum_ / angle_);

	SetTextImageCntLabel();
}

void PacsMPRViewWidget::SetTextImageCntLabel()
{
	if (slider_ == nullptr || label_image_cnt_ == nullptr)
	{
		return;
	}

	int num = slider_->getEnd() - slider_->getStart() + 1;
	label_image_cnt_->setText(QString::number(num));
}
