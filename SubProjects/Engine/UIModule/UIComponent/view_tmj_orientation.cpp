#include "view_tmj_orientation.h"

ViewTMJorientation::ViewTMJorientation(const ReorientViewID& type,
										 QWidget* parent) 
	: BaseViewOrientation(common::ViewTypeID::TMJ_ORIENTATION, type, true, parent) {
}

ViewTMJorientation::~ViewTMJorientation() {
}
