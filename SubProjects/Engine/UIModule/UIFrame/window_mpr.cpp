#include "window_mpr.h"
#include <Engine/Common/Common/W3Define.h>
#include <Engine/Common/Common/language_pack.h>
#include "../../Common/Common/global_preferences.h"

WindowMPR::WindowMPR(const MPRViewType& view_type, QWidget* parent)
	: Window(parent), thickness_(new QDoubleSpinBox), interval_(new QDoubleSpinBox)
{
	QString view_title;
	switch (view_type)
	{
	case MPRViewType::AXIAL:
		view_title = lang::LanguagePack::txt_axial();
		break;
	case MPRViewType::SAGITTAL:
		view_title = lang::LanguagePack::txt_sagittal();
		break;
	case MPRViewType::CORONAL:
		view_title = lang::LanguagePack::txt_coronal();
		break;
	default:
		break;
	}

	InitViewMenu(view_title);
	Initialize();
}

WindowMPR::~WindowMPR()
{

}

void WindowMPR::SyncThicknessValue(const float thickness)
{
	thickness_->setValue(thickness);
}

void WindowMPR::InitInterval(const float interval, const float minimum)
{
	interval_->setValue(interval);
	interval_->setMinimum(minimum);
}

/*
	smseo : MPR window에서는 button으로 select layout signal 이 emit 되었을
   때 light box mode를 켜기 위한 sigSelectLayout만 emit 한다. Window에서
   호출되었을 경우 기존의 레이아웃 변경 함수를 호출한다.
*/
void WindowMPR::slotSelectLayout(int row, int column)
{
	if (QObject::sender())
	{
		emit sigLightboxOn(row, column, interval_->value(), thickness_->value());
	}
	else
	{
		Window::slotSelectLayout(row, column);
	}
}

void WindowMPR::slotIntervalChanged(double slider_value)
{
	double interval;
	if (slider_value == interval_->minimum() + interval_->singleStep())
	{
		interval = 1.0;
		interval_->blockSignals(true);
		interval_->setValue(interval);
		interval_->blockSignals(false);
	}
	else
	{
		interval = slider_value;
	}

	emit sigChangeInterval(interval);
}

void WindowMPR::Initialize()
{
	auto func_gen_spin = [&](QDoubleSpinBox* spin_box)
	{
		spin_box->setDecimals(2);
		spin_box->setValue(0.0);
		spin_box->setSuffix(" mm");
		spin_box->setSingleStep(1.0);
		spin_box->setRange(0.0, common::kMPRThicknessMax);
	};

	func_gen_spin(interval_.get());
	interval_->setObjectName(lang::LanguagePack::txt_interval());
	connect(interval_.get(), SIGNAL(valueChanged(double)), this,
		SLOT(slotIntervalChanged(double)));

	func_gen_spin(thickness_.get());
	thickness_->setObjectName(lang::LanguagePack::txt_thickness());
	connect(thickness_.get(), SIGNAL(valueChanged(double)), this,
		SIGNAL(sigChangeThickness(double)));

	std::vector<QAbstractSpinBox*> spin_boxes;
	spin_boxes.push_back(dynamic_cast<QAbstractSpinBox*>(interval_.get()));
	spin_boxes.push_back(dynamic_cast<QAbstractSpinBox*>(thickness_.get()));

	Window::AddSpinBox(spin_boxes);
	Window::AddGridButton();

#ifdef WILL3D_EUROPE	
	bool btn_enable = GlobalPreferences::GetInstance()->preferences_.europe_window_btn_enable_;
	if (!btn_enable)
	{
		interval_.get()->hide();
		thickness_.get()->hide();
	}	
#endif // WILL3D_EUROPE

	Window::SetMaximumRowColCount(4, 5);
}
