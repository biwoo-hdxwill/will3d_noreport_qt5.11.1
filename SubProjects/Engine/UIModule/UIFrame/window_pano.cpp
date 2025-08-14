#include "window_pano.h"

#include <qgridlayout.h>

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3LayoutFunctions.h"
#include "../../Common/Common/language_pack.h"

/**=================================================================================================
class WindowPano
*===============================================================================================**/

WindowPano::WindowPano(QWidget *parent)
	: Window(parent)
{
	Initialize();
}

WindowPano::~WindowPano(void)
{

}

void WindowPano::Initialize() {
	InitSpinboxes();
	std::vector<QAbstractSpinBox*> spin_boxes;
	spin_boxes.push_back(dynamic_cast<QAbstractSpinBox*>(thickness_.get()));
	InitViewMenu(lang::LanguagePack::txt_panorama());
	Window::AddSpinBox(spin_boxes);

	AddMaximizeButton();
}

void WindowPano::InitSpinboxes() {
	thickness_.reset(new QDoubleSpinBox);
	thickness_->setDecimals(2);
	//thickness_->setValue(0.0);
	thickness_->setValue(GlobalPreferences::GetInstance()->preferences_.advanced.panorama_axial_view.panorama_default_thickness);
	thickness_->setSuffix(" mm");
	thickness_->setSingleStep(1.0);
	thickness_->setRange(0.0, 40.0);
	thickness_->setObjectName(lang::LanguagePack::txt_thickness());

	ApplyArchThicknessIncrements();

#ifdef WILL3D_EUROPE
	bool btn_enable = GlobalPreferences::GetInstance()->preferences_.europe_window_btn_enable_;
	if (!btn_enable)
		thickness_.get()->hide();
#endif // WILL3D_EUROPE
}

void WindowPano::ApplyPreferences() {
	ApplyArchThicknessIncrements();
}

void WindowPano::ApplyArchThicknessIncrements() {
	double arch_thickness_increments = GlobalPreferences::GetInstance()->preferences_.advanced.panorama_axial_view.panorama_thickness_increments;
	thickness_->setSingleStep(arch_thickness_increments);
}
