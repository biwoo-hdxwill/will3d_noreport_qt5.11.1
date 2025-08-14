#include "sagittal_resource.h"

#if defined(__APPLE__)
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#else
#include <GL/glm/glm.hpp>
#include <GL/glm/gtx/transform.hpp>
#endif

#include "../../Common/Common/W3Logger.h"

SagittalResource::SagittalResource() { 
}

SagittalResource::~SagittalResource() {}

void SagittalResource::SetSagittalParams(const Params& params) {
	params_ = params;
}

void SagittalResource::SetSagittal(const glm::vec3& vol_position,
										   const CurveData& pano_curve,
										   const glm::vec3& pano_back_vector,
										   const QPointF& pano_position) {
	// check : param initialize
	if (params_.width == 0 || params_.height == 0) {
		common::Logger::instance()->Print(common::LogType::ERR,
										  "SagittalResource::SetSagittal: invalid params.");
		assert(false);
		return;
	}

	// pano_index range check
	int pano_index = static_cast<int>(pano_position.x());
	if (pano_index >= pano_curve.points().size() ||
		pano_index < 0) {
		common::Logger::instance()->Print(common::LogType::ERR,
										  "SagittalResource::SetSagittal: invalid pano_index.");
		assert(false);
		return;
	}

	back_vector_ = pano_back_vector;

	const glm::mat4 rotate = glm::rotate(glm::radians(params_.degree), back_vector_);
	up_vector_ = pano_curve.up_vectors().at(pano_index);
	up_vector_ = glm::vec3(rotate * glm::vec4(up_vector_, 1.0f));
	right_vector_ = glm::normalize(glm::cross(back_vector_, up_vector_));

	const glm::vec3 pano_curve_pt(pano_curve.points().at(pano_index));
	const glm::vec3 vol_pt_dir = vol_position - pano_curve_pt;
	float du = glm::dot(up_vector_, vol_pt_dir);
	float dv = glm::dot(right_vector_, vol_pt_dir);
	const glm::vec3 vol_pt_in_pano_plane = pano_curve_pt + up_vector_ * du + right_vector_ * dv;

	float pano_displacement = static_cast<float>(pano_position.y());
	const glm::vec3 to_pano_center = back_vector_ * pano_displacement;

	center_position_ = vol_pt_in_pano_plane + to_pano_center;
	is_valid_ = true;
}
