#include "measure_data.h"

MeasureData::MeasureData(const unsigned int& id,
						 const common::measure::MeasureType& type,
						 const common::measure::VisibilityParams& vp,
						 const float& pixel_pitch, const float& scale) :
	id_(id), type_(type), visibility_params_(vp),
	pixel_pitch_(pixel_pitch), scale_(scale) {
}

MeasureData::~MeasureData() {
}

//MeasureData & MeasureData::operator=(const MeasureData &data) {
//	id_ = data.id_;
//	value_ = data.value_;
//	memo_ = data.memo_;
//	type_ = data.type_;
//	visibility_params_.back = data.visibility_params_.back;
//	visibility_params_.center = data.visibility_params_.center;
//	visibility_params_.up = data.visibility_params_.up;
//	visibility_params_.thickness = data.visibility_params_.thickness;
//	points_ = data.points_;
//	note_txt_ = data.note_txt_;
//	note_id_ = data.note_id_;
//	profile_id_ = data.profile_id_;
//
//	return *this;
//}
