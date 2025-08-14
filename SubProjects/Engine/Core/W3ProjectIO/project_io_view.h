#pragma once
/*=========================================================================

File:			class ProjectIOView
Language:		C++11
Library:        Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-07-16
Last modify:	2016-07-16

=========================================================================*/
#include <memory>
#include <vector>

#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif
#include <H5Cpp.h>

#include "w3projectio_global.h"

class CW3ViewPlane;

namespace project {
typedef struct _MeasureViewParams MeasureViewInfo;
}

class W3PROJECTIO_EXPORT ProjectIOView {
public:
	enum class Measure3DType
	{
		LENGTH,
		ANGLE,
		LINE,
		END
	};

	ProjectIOView(const std::shared_ptr<H5::H5File>& file,
				  const H5::Group& curr_view_group);
	~ProjectIOView();

	ProjectIOView(const ProjectIOView&) = delete;
	ProjectIOView& operator=(const ProjectIOView&) = delete;

public:
	void SaveRotateMatrix(const glm::mat4& mat);
	void SaveVolRange(const glm::vec3& vol_range);
	void SaveViewInfo(const float& scale, const float& scale_scene_to_gl,
		const float& gl_trans_x, const float& gl_trans_y);
	void SaveViewInfo(const float& scale, const float& scale_scene_to_gl,
		const float& gl_trans_x, const float& gl_trans_y,
		const int window_center, const int window_width);
	void SaveMeasure3DCount(const Measure3DType type, const int count);
	void SaveMeasure3D(const Measure3DType type, const int index, const std::vector<glm::vec3>& points);
	void SaveViewPlane(CW3ViewPlane* view_plane);

	void LoadRotateMatrix(glm::mat4& mat);
	void LoadVolRange(glm::vec3& vol_range);
	void LoadViewInfo(float& scale, float& scale_scene_to_gl,
		float& gl_trans_x, float& gl_trans_y);
	void LoadViewInfo(float& scale, float& scale_scene_to_gl,
		float& gl_trans_x, float& gl_trans_y,
		int& window_center, int& window_width);
	void LoadMeasure3DCount(const Measure3DType type, int& count);
	void LoadMeasure3D(const Measure3DType type, const int index, std::vector<glm::vec3>& points);
	void LoadViewPlane(CW3ViewPlane* view_plane);

	void GetVersionInfo(int& version);

private:
	H5::Group view_group_;
	std::shared_ptr<H5::H5File> file_;
};

