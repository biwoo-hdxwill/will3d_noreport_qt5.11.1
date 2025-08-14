#include "view_mpr_orientation.h"

ViewMPROrientation::ViewMPROrientation(const ReorientViewID& type, QWidget* parent) 
	: BaseViewOrientation(common::ViewTypeID::PANO_ORIENTATION, type, false, parent) {
}

ViewMPROrientation::~ViewMPROrientation() {
}
