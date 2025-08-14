#include "measure_base.h"

#include "../../Resource/Resource/include/measure_data.h"

namespace {
const float kEpsEqual = 0.5f;
} // end of namespace

unsigned int MeasureBase::id() const noexcept {
	MeasureData* data = data_.lock().get();
	if (data != nullptr) {
		return data->id();
	} else {
		return 0;
	}
}

common::measure::MeasureType MeasureBase::type() const noexcept {
	MeasureData* data = data_.lock().get();
	if (data != nullptr) {
		return data->type();
	} else {
		return common::measure::MeasureType::NONE;
	}
}

const common::measure::VisibilityParams& MeasureBase::GetVisibilityParams() {
	return data_.lock().get()->visibility_params();
}

bool MeasureBase::VisibilityCheck(common::measure::VisibilityParams& view_vp) const {
	const auto& data_ptr = data_.lock().get();

	if (!data_ptr)
	{
		return false;
	}

	common::measure::VisibilityParams vp = data_ptr->visibility_params();
	if (view_vp.thickness <= 0.0f) {
		if (glm::length(vp.center - view_vp.center) < kEpsEqual &&
			glm::length(vp.up - view_vp.up)*100.0f < kEpsEqual &&
			glm::length(vp.back - view_vp.back)*100.0f < kEpsEqual) {
			return true;
		} else {
			return false;
		}
	} else {
		/*
			- direction vector는 동일하게 체크한다.
			- item의 pos가 MeasureTools의 pos, direction,
				thickness를 이용하여 만들어진 연장선상에 포함되는지 체크한다.
		*/
		return true;
	}
}

void MeasureBase::SetMeasureData(const std::weak_ptr<MeasureData>& data) {
	data_ = data;
	//UpdateMeasure();
}
