#include "cross_section_resource.h"

#include <QDebug>
#include <QTextCodec>

#if defined(__APPLE__)
#include <glm/gtx/transform.hpp>
#else
#include <GL/glm/gtx/transform.hpp>
#endif

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/common.h"
#include "../../Common/Common/W3Logger.h"

using glm::vec3;
CrossSectionResource::CrossSectionResource(const CurveData& pano_curve,
	const glm::vec3& pano_back_vector,
	const int& pano_front_index,
	const int& pano_height,
	const float& arch_position_in_pano) :
	pano_curve_(pano_curve),
	pano_back_vector_(pano_back_vector),
	pano_front_index_(pano_front_index),
	pano_height_(pano_height),
	arch_position_in_pano_(arch_position_in_pano)
{

}

void CrossSectionResource::SetPanoCurveData(const CurveData& pano_curve, const int& pano_front_index)
{
	pano_curve_ = pano_curve;
	pano_front_index_ = pano_front_index;
	EditShiftedValue(0.0f);
}

void CrossSectionResource::SetCrossSectionParams(const Params& params)
{
	if (params_.count != params.count)
	{
		data_.clear();
		for (int i = 0; i < params.count; i++)
			data_[i].reset(new CrossSectionData());
	}
	params_ = params;
	EditShiftedValue(0.0f);
}

void CrossSectionResource::EditShiftedValue(float shifted_value)
{
	float max_shifted, min_shifted;
	GetAvailableShiftedValue(&min_shifted, &max_shifted);
#if 0
	shifted_value_ += shifted_value;

	shifted_value_ = std::max(shifted_value_, min_shifted);
	shifted_value_ = std::min(shifted_value_, max_shifted);
#else // 마우스로 cross section line 이동할 때도 interval 적용
	float new_shifted_value = shifted_value_ + shifted_value;

	if (new_shifted_value < min_shifted ||
		new_shifted_value > max_shifted)
	{
		return;
	}

	/*int factor = static_cast<int>(shifted_value_ / params_.interval);
	new_shifted_value = params_.interval * factor;*/

	qDebug() << new_shifted_value << "=" << shifted_value_ << "+" << shifted_value;

	shifted_value_ = new_shifted_value;

	qDebug() << "shifted_value_ :" << shifted_value_;
#endif
	UpdateCurveData();
}

void CrossSectionResource::ResetDeltaShiftedValue()
{
	GlobalPreferences* global_preferences = GlobalPreferences::GetInstance();
	bool shifted_by_interval = global_preferences->GetShiftedByInterval();
	if (shifted_by_interval)
	{
		shifted_value_ = static_cast<float>(CalculateShiftedValueByInterval());
	}
}

void CrossSectionResource::SetCenterPositionsInPanoPlane(const std::map<int, QPointF>& center_positions)
{
	for (const auto& elem : center_positions)
	{
		auto iter = data_.find(elem.first);
		if (iter == data_.end())
		{
			auto logger = common::Logger::instance();
			logger->Print(common::LogType::ERR, "CrossSectionResource::SetCenterPositionsInPanoPlane: invalid cross id.");
			assert(false);
			return;
		}
		iter->second->set_center_position_in_pano_plane(elem.second);
	}
}
void CrossSectionResource::SetArchPositionsInPanoPlane(const std::map<int, QPointF>& arch_positions)
{
	for (const auto& elem : arch_positions)
	{
		auto iter = data_.find(elem.first);
		if (iter == data_.end())
		{
			auto logger = common::Logger::instance();
			logger->Print(common::LogType::ERR, "CrossSectionResource::SetCenterPositionsInPanoPlane: invalid cross id.");
			assert(false);
			return;
		}
		iter->second->set_arch_position_in_pano_plane(elem.second);
	}
}

int CrossSectionResource::CalculateShiftedValueByInterval()
{
	int shifted_value = 0;
	int factor = static_cast<int>(shifted_value_ / params_.interval);
	float new_shifted_value = params_.interval * factor;

	if (new_shifted_value > 0)
	{
		shifted_value = (int)(new_shifted_value + 0.5f);
	}
	else
	{
		shifted_value = (int)(new_shifted_value);
	}

	qDebug() << new_shifted_value << "=" << params_.interval << "*" << factor;
	qDebug() << "shifted_value :" << shifted_value;

	return shifted_value;
}

void CrossSectionResource::UpdateCurveData()
{
	int shifted_value;

	GlobalPreferences* global_preferences = GlobalPreferences::GetInstance();
	bool shifted_by_interval = global_preferences->GetShiftedByInterval();
	if (shifted_by_interval)
	{
		shifted_value = CalculateShiftedValueByInterval();
	}
	else
	{
		if (shifted_value_ > 0)
		{
			shifted_value = (int)(shifted_value_ + 0.5f);
		}
		else
		{
			shifted_value = (int)(shifted_value_);
		}
	}

	const std::vector<glm::vec3>& tmp_pano_points = pano_curve_.points();
	const std::vector<glm::vec3>& tmp_pano_up_vectors = pano_curve_.up_vectors();

	int begin_idx = GetBeginCrossIdxAtPanoCurve() + shifted_value;

	if (begin_idx > (int)(tmp_pano_points.size() - 1) || begin_idx < 0)
		return;

	std::vector<glm::vec3> pano_points, pano_up_vectors;
	pano_points.assign(tmp_pano_points.begin(), tmp_pano_points.end());
	pano_up_vectors.assign(tmp_pano_up_vectors.begin(), tmp_pano_up_vectors.end());

	glm::vec3 tail = pano_points.back()*2.0f - pano_points[pano_points.size() - 2];
	pano_points.push_back(tail);
	pano_points.push_back(tail);

	pano_up_vectors.push_back(pano_up_vectors.back());
	pano_up_vectors.push_back(pano_up_vectors.back());

	int data_id = 0;

	float pano_height_haf = (float)(pano_height_)*0.5f;
	float accum_interval = 0.0f;
	bool flip_slices_across_the_arch_centerline = GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.flip_slices_across_the_arch_centerline;
	GlobalPreferences::Direction direction = GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.direction;

	for (int i = 0; i < params_.count; i++)
	{
		int idx = std::min(begin_idx + (int)accum_interval, (int)pano_points.size() - 2);

		glm::mat4 rotate = glm::rotate(glm::radians(params_.degree), pano_up_vectors[idx]);
		glm::vec3 right_vector = glm::normalize(pano_up_vectors[idx]);

		glm::vec3 p1 = pano_points[idx] + right_vector * ((float)params_.width / 2.f);
		glm::vec3 p2 = pano_points[idx] - right_vector * ((float)params_.width / 2.f);

		bool is_flip = false;
		if (flip_slices_across_the_arch_centerline)
		{
			is_flip = idx > pano_front_index_ ? false : true;
		}

		std::vector<glm::vec3> cross_point;
		glm::vec3 up_vector = glm::normalize(pano_points[idx + 1] - pano_points[idx]);
		up_vector = glm::vec3(rotate * glm::vec4(up_vector, 1.0f));

		if (direction == GlobalPreferences::Direction::Inverse)
		{
			glm::vec3 temp = p1;
			p1 = p2;
			p2 = temp;
			up_vector = -up_vector;
		}

		if (is_flip && flip_slices_across_the_arch_centerline)
		{
			cross_point.push_back(p2);
			cross_point.push_back(p1);
			up_vector = -up_vector;
		}
		else
		{
			cross_point.push_back(p1);
			cross_point.push_back(p2);
		}

		glm::vec3 cross_right_vector = glm::normalize(cross_point[1] - cross_point[0]);

#if 0
		bool is_buccal = 
			(glm::dot(cross_right_vector, glm::vec3(0.0f, 1.0f, 0.0f)) > 0.0f)
			? true : false;
#else
		bool is_buccal =
			(glm::dot(cross_right_vector, glm::vec3(is_flip ? 1.0 : -1.0f, 0.0f, 0.0f)) > 0.0f)
			? true : false;
#endif

#if 0
		qDebug();
		qDebug() << static_cast<int>(direction) << flip_slices_across_the_arch_centerline;
		qDebug() << "idx :" << idx << " / pano_front_index_ :" << pano_front_index_;
		qDebug() << "is_buccal :" << is_buccal << " / is_flip :" << is_flip;
		qDebug();
#endif

#if 0
		bool reversed = false;
		switch (direction)
		{
		case GlobalPreferences::Direction::Normal:
			if (flip_slices_across_the_arch_centerline)
			{
				if (is_flip == is_buccal)
				{
					reversed = true;
				}
			}
			else
			{
				if (!is_buccal)
				{
					reversed = true;
				}
			}
			break;
		case GlobalPreferences::Direction::Inverse:
			if (flip_slices_across_the_arch_centerline)
			{
				if (is_flip != is_buccal)
				{
					reversed = true;
				}
			}
			else
			{
				if (is_buccal)
				{
					reversed = true;
				}
			}
			break;
		default:
			break;
		}

		if (reversed)
		{
			glm::vec3 temp = cross_point[0];
			cross_point[0] = cross_point[1];
			cross_point[1] = temp;
			up_vector = -up_vector;

			cross_right_vector = glm::normalize(cross_point[1] - cross_point[0]);
			is_buccal = !is_buccal;
		}
#endif

		const glm::vec3 to_pano_center = pano_back_vector_ * pano_height_haf;
		const glm::vec3 center_pos = pano_points[idx] + to_pano_center;
		const glm::vec3 arch_pos = pano_points[idx] + pano_back_vector_ * arch_position_in_pano_;

		const glm::vec3 cross_center = vec3(rotate *
			glm::vec4((center_pos - arch_pos), 1.0f)) + arch_pos;

		data_[data_id]->Initialize(cross_right_vector, up_vector,
			cross_center, arch_pos,
			idx - pano_front_index_, 
			(direction == GlobalPreferences::Direction::Inverse) ? is_flip : !is_flip);

		accum_interval += params_.interval;
		++data_id;
	}
}

void CrossSectionResource::GetAvailableShiftedValue(float* min, float* max) const
{
	*min = std::numeric_limits<float>::max();
	*max = std::numeric_limits<float>::min();

	const std::vector<glm::vec3>& pano_points = pano_curve_.points();
	*min = (float)(-GetBeginCrossIdxAtPanoCurve());
	*max = (float)((int)pano_points.size() - 1 - GetEndCrossIdxAtPanoCurve());
}

int CrossSectionResource::GetBeginCrossIdxAtPanoCurve() const
{
	const std::vector<glm::vec3>& pano_points = pano_curve_.points();
	int idx = pano_front_index_;
	idx = std::max(0, idx);
	return idx;
}

int CrossSectionResource::GetEndCrossIdxAtPanoCurve() const
{
	const std::vector<glm::vec3>& pano_points = pano_curve_.points();

	int idx = GetBeginCrossIdxAtPanoCurve() + (int)(params_.interval*(float)(params_.count - 1));
	idx = std::min((int)pano_points.size() - 1, idx);
	return idx;
}
