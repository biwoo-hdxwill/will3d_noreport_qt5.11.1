#pragma once
/**=================================================================================================

Project:		Resource
File:			measure_data.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-10-30
Last modify: 	2018-10-30

Copyright (c) 2018 HDXWILL. All rights reserved.

*===============================================================================================**/
#include <vector>
#include <qpoint.h>
#include <qstring.h>

#include "../../../Engine/Common/Common/W3Define.h"
#include "../../../Engine/Common/Common/define_measure.h"
#include "../resource_global.h"

class RESOURCE_EXPORT MeasureData {
public:
	MeasureData(const unsigned int& id,
				const common::measure::MeasureType& type,
				const common::measure::VisibilityParams& vp,
				const float& pixel_pitch, const float& scale);
	~MeasureData();

	//MeasureData& operator=(const MeasureData&);

public:
	inline void set_value(const QString& value) noexcept { value_ = value; }
	inline void set_memo(const QString& memo) noexcept { memo_ = memo; }
	inline void set_visibility_params(const common::measure::VisibilityParams& vp) noexcept { visibility_params_ = vp; }
	inline void set_note_id(const unsigned int& id) noexcept { note_id_ = id; }
	inline void set_profile_id(const unsigned int& id) noexcept { profile_id_ = id; }
	inline void set_note_txt(const QString& txt) noexcept { note_txt_ = txt; }
	inline void set_points(const std::vector<QPointF>& points) noexcept { points_ = points; }
	inline void set_pixel_pitch(const float& pixel_pitch) noexcept { pixel_pitch_ = pixel_pitch; }
	inline void set_scale(const float& scale) noexcept { scale_ = scale; }

	inline const unsigned int& id() const noexcept { return id_; }
	inline const common::measure::MeasureType& type() const noexcept { return type_; }
	inline const common::measure::VisibilityParams& visibility_params() const noexcept { return visibility_params_; }
	inline const QString& value() const noexcept { return value_; }
	inline const QString& memo() const noexcept { return memo_; }
	inline const unsigned int& note_id() const noexcept { return note_id_; }
	inline const unsigned int& profile_id() const noexcept { return profile_id_; }
	inline const QString& note_txt() const noexcept { return note_txt_; }
	inline const std::vector<QPointF>& points() const noexcept { return points_; }
	inline const float& pixel_pitch() const noexcept { return pixel_pitch_; }
	inline const float& scale() const noexcept { return scale_; }

private:
	// Measure의 unique identifier
	unsigned int id_ = 0;

	// MeasureListDlg에서 Display하기 위해 사용되는 속성들
	QString value_; // measure의 대표 값
	QString memo_; // 사용자가 입력하여 수정할 수 있는 memo 값

	// MeasureBase의 공용 속성들
	common::measure::MeasureType type_ = common::measure::MeasureType::NONE;
	common::measure::VisibilityParams visibility_params_;
	std::vector<QPointF> points_;

	// AnnotationNote에서 사용되는 속성들
	QString note_txt_;
	unsigned int note_id_ = 0;

	// MeasureProfile에서 사용되는 속성들
	unsigned int profile_id_ = 0;

	// scene의 속성들.
	float pixel_pitch_ = 0.0f;
	float scale_ = 0.0f; // scene_to_gl / vol_to_gl
};

