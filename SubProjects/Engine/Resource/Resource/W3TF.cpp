#include "W3TF.h"

#include <math.h>

#include "../../Common/Common/W3Memory.h"

CW3TF::CW3TF(const std::weak_ptr<std::vector<TF_BGRA>>& tf_colors_, int offset){
	ref_tf_colors_ = tf_colors_;
	offset_ = offset;
}

CW3TF::~CW3TF(void) {
}

void CW3TF::FindMinMax() {
	min_value_ = 65535;
	max_value_ = 0;

	bool flag = false;
	std::vector<TF_BGRA>* ptf_colors_ = ref_tf_colors_.lock().get();

	int size = ptf_colors_->size();

	for (int i = 0; i < size; i++) {
		float a = ptf_colors_->at(i).a;

		if (i == 0) {
			if (a > 0.0f) {
				min_value_ = 0;
				flag = true;
			}
		} else {
			if (a > 0.0f && !flag) {
				if (min_value_ > i) {
					min_value_ = i;
				}

				flag = true;
			}

			if (a > 0.0f && flag) {
				if (max_value_ < i) {
					max_value_ = i;
				}

				flag = false;

			}
		}
	}

	if (max_value_ == 0) {
		max_value_ = size - 1;
	}
}
