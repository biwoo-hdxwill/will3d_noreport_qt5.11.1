#pragma once
/*=========================================================================

File:			class ProjectIOMPR
Language:		C++11
Library:        Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-07-11
Last modify:	2016-07-11

=========================================================================*/
#include <memory>
#include <vector>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif
#include <qpoint>

#include "datatypes.h"
#include "w3projectio_global.h"

namespace H5
{
	class H5File;
}
class ProjectIOView;

class W3PROJECTIO_EXPORT ProjectIOMPR
{
public:
	ProjectIOMPR(const project::Purpose& purpose, const std::shared_ptr<H5::H5File>& file);
	~ProjectIOMPR();

	ProjectIOMPR(const ProjectIOMPR&) = delete;
	ProjectIOMPR& operator=(const ProjectIOMPR&) = delete;

	enum class ViewType
	{
		AXIAL, SAGITTAL, CORONAL, VR, ZOOM3D
	};

public:
	void InitMPRTab();
	bool IsInit();
	void InitializeView(const ProjectIOMPR::ViewType& view_type);
	ProjectIOView& GetViewIO();

	void SaveRegistrationParams(float* params, int count);
	void LoadRegistrationParams(float* params);

	void SaveSecondTransformMatrix(const glm::mat4& mat);
	void LoadSecondTransformMatrix(glm::mat4& mat);

	void SaveRotateMatrix(const glm::mat4& mat);
	void LoadRotateMatrix(glm::mat4& mat);

	void SaveTransformStatusCount(const int& cnt);
	void LoadTransformStatusCount(int& cnt);

	void SaveThicknessInterval(const float& thickness, const float& interval);
	void LoadThicknessInterval(float& thickenss, float& interval);

	void SaveTransformStatusForMeasure(
		const unsigned int& measure_id, const glm::mat4& axial,
		const glm::mat4& sagittal, const glm::mat4& coronal,
		const glm::vec3& rot_center, const float& axial_angle,
		const float& sagittal_angle, const float& coronal_angle);
	void LoadTransformStatusForMeasure(
		unsigned int& measure_id, glm::mat4& axial,
		glm::mat4& sagittal, glm::mat4& coronal,
		glm::vec3& rot_center, float& axial_angle,
		float& sagittal_angle, float& coronal_angle);

	void SaveCrossController(const QPointF& center_pos, const float rotate_angle);
	bool LoadCrossController(QPointF& center_pos, float& rotate_angle);

	void SaveCrossControllerDelta(const QPointF& delta_pos, const float rotate_angle);
	void LoadCrossControllerDelta(QPointF& delta_pos, float& rotate_angle);

private:
	std::shared_ptr<H5::H5File> file_;
	std::unique_ptr<ProjectIOView> view_io_;
	ViewType curr_view_type_;
	int curr_cnt_transform_status_ = 0;
};
