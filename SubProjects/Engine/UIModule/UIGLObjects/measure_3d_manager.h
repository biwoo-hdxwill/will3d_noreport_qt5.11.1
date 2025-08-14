#pragma once
/*=========================================================================
Copyright (c) 2019 All rights reserved by HDXWILL.

File:			measure_3d_manager.h
Language:		C++11
Library:		Qt 5.8.0, Standard C++ Library, glm
Author:			JUNG DAE GUN
First Date:		2019-04-02
Modify Date:	2019-04-02
=========================================================================*/

#include "uiglobjects_global.h"

#include <vector>

#include <QSize>
#include <QPoint>

#include <Engine/Common/Common/define_view.h>
#include <Engine/Common/GLfunctions/WGLHeaders.h>

class QGraphicsScene;
class QMouseEvent;

class MeasureBase3D;
class MeasureLength3D;
class MeasureAngle3D;
class MeasureLine3D;
#ifndef WILL3D_VIEWER
class ProjectIOView;
#endif

using namespace common;

class UIGLOBJECTS_EXPORT Measure3DManager
{
public:
#if 0
	enum class Type
	{
		NONE,
		LENGTH,
		ANGLE,
		DELETE,
		DELETE_ALL
	};
#endif

	Measure3DManager(QGraphicsScene* scene);
	virtual ~Measure3DManager();

#ifndef WILL3D_VIEWER
	void ExportProject(ProjectIOView& out);
	void ImportProject(ProjectIOView& in);
#endif

	void SetType(CommonToolTypeOnOff type);
	void SetVisible(const bool visible);
	void MousePressEvent(Qt::MouseButton button, const glm::vec3& volume_pos, bool& update);
	void MouseMoveEvent(Qt::MouseButtons buttons, const glm::vec3& volume_pos, bool& update);
	void MouseReleaseEvent(Qt::MouseButton button, const glm::vec3& volume_pos, bool& update);
	void Draw(GLuint program);
	void Pick(const QSize& viewport_size, const QPointF& mouse_pos, bool* update, GLuint program);
	void ClearVAOVBO();
	void Clear();
	void SetSceneSizeInView(QSizeF size);
	const bool IsSelected();
	void DeleteUnfinishedItem();

	inline void set_volume_range(const glm::vec3& range) { volume_range_ = range; }
	inline void set_pixel_spacing(const float pixel_spacing) { pixel_spacing_ = pixel_spacing; }
	inline void set_slice_thickness(const float slice_thickness) { slice_thickness_ = slice_thickness; }
	inline void set_reorientation_matrix(const glm::mat4& mat) { reorientation_matrix_ = mat; }
	inline void set_scale_matrix(const glm::mat4& mat) { scale_matrix_ = mat; }
	inline void set_rotate_matrix(const glm::mat4& mat) { rotate_matrix_ = mat; }
	inline void set_projection_matrix(const glm::mat4& mat) { projection_matrix_ = mat; }
	inline void set_view_matrix(const glm::mat4& mat) { view_matrix_ = mat; }

	inline const bool started() { return started_; }

	void ApplyPreferences();

private:
	void InitMeasureLength();
	void InitMeasureAngle();
	void InitMeasureLine();
	void InitMeasureFreedraw();

private:
	CommonToolTypeOnOff type_ = CommonToolTypeOnOff::NONE;

	QGraphicsScene* scene_ = nullptr;

	glm::vec3 volume_range_ = glm::vec3(1.0f);
	float pixel_spacing_ = 1.0f;
	float slice_thickness_ = 1.0f;
	glm::mat4 reorientation_matrix_;
	glm::mat4 scale_matrix_;
	glm::mat4 rotate_matrix_;
	glm::mat4 projection_matrix_;
	glm::mat4 view_matrix_;
	QSizeF scene_size_in_view_ = QSizeF(1.0f, 1.0f);

	bool started_ = false;

	MeasureLength3D* measure_length_3d_ = nullptr;
	MeasureAngle3D* measure_angle_3d_ = nullptr;
	MeasureLine3D* measure_line_3d_ = nullptr;

	std::vector<MeasureBase3D*> measure_list_;
};
