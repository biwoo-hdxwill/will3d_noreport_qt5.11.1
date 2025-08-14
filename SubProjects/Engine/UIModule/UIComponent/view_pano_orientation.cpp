#include "view_pano_orientation.h"

ViewPanoOrientation::ViewPanoOrientation(const ReorientViewID& type,
										 QWidget* parent) 
	: BaseViewOrientation(common::ViewTypeID::PANO_ORIENTATION, type, true, parent) {
}

ViewPanoOrientation::~ViewPanoOrientation() {
}
