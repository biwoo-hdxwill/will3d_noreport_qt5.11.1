#include "cross_section_data.h"

#if defined(__APPLE__)
#include <glm/glm/gtx/transform.hpp>
#else
#include <GL/glm/gtx/transform.hpp>
#endif

CrossSectionData::CrossSectionData() {}

CrossSectionData::~CrossSectionData() {}

void CrossSectionData::Initialize(const glm::vec3& right_vector,
	const glm::vec3& up_vector,
	const glm::vec3 & center_pos,
	const glm::vec3& arch_pos,
	const int& index, const bool& is_flip)
{
	center_position_in_vol_ = center_pos;
	arch_position_in_vol_ = arch_pos;
	index_ = index;
	right_vector_ = right_vector;
	up_vector_ = up_vector;
	flip_ = is_flip;
	is_init_ = true;
	back_vector_ = glm::normalize(glm::cross(up_vector_, right_vector));
}

void CrossSectionData::Clear()
{
	center_position_in_vol_ = up_vector_ =
		right_vector_ = glm::vec3(0.0f);
	center_position_in_pano_plane_ = proj_nerve_ = ctrl_nerve_ = QPointF(-1.0, -1.0);
	index_ = -1;
	is_init_ = false;
	flip_ = false;
}
