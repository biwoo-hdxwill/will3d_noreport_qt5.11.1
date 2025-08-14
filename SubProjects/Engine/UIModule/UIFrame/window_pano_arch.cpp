#include "window_pano_arch.h"

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/language_pack.h"

WindowPanoArch::WindowPanoArch(bool is_maximize /*= false*/, QWidget *parent /*= 0*/)
	: Window(parent)
{
	Initialize();
	if (is_maximize)
	{
		AddMaximizeButton();
	}
}

WindowPanoArch::~WindowPanoArch(void)
{

}

void WindowPanoArch::Initialize()
{
	InitSpinboxes();
	std::vector<QAbstractSpinBox*> spin_boxes;
	spin_boxes.push_back(dynamic_cast<QAbstractSpinBox*>(range_.get()));
	InitViewMenu(lang::LanguagePack::txt_axial());
	Window::AddSpinBox(spin_boxes);
}

void WindowPanoArch::InitSpinboxes()
{
	double init_panorama_range = GlobalPreferences::GetInstance()->preferences_.advanced.panorama_axial_view.panorama_default_range;

	range_.reset(new QDoubleSpinBox);
	range_->setDecimals(2);
	range_->setValue(init_panorama_range);
	range_->setSuffix(" mm");
	range_->setSingleStep(1.0);
	range_->setRange(1.0, 200.0);
	range_->setObjectName(lang::LanguagePack::txt_range());
}

void WindowPanoArch::SetRange(const double& range_mm)
{
	range_->setValue(range_mm);
}
