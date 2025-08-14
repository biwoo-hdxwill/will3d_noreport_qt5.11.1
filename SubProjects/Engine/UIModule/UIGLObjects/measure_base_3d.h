#pragma once
/*=========================================================================
Copyright (c) 2019 All rights reserved by HDXWILL.

File:			measure_base_3d.h
Language:		C++11
Library:		Qt 5.8.0, Standard C++ Library, glew
Author:			JUNG DAE GUN
First Date:		2019-03-21
Modify Date:	2019-03-21
=========================================================================*/

#include "uiglobjects_global.h"

#include <vector>

#include <QSize>

#include "../../Common/GLfunctions/WGLHeaders.h"

class QPointF;
class QGraphicsScene;
class CW3SurfaceAngleItem;
class ProjectIOView;

class UIGLOBJECTS_EXPORT MeasureBase3D
{
public:
	MeasureBase3D(QGraphicsScene* scene);
	virtual ~MeasureBase3D();

#ifndef WILL3D_VIEWER
	virtual void ExportProject(ProjectIOView& out) = 0;
	virtual void ImportProject(ProjectIOView& in) = 0;
#endif

	inline void set_pixel_spacing(const float pixel_spacing) { pixel_spacing_ = pixel_spacing; }
	inline void set_slice_thickness(const float slice_thickness) { slice_thickness_ = slice_thickness; }
	inline void set_reorientation_matrix(const glm::mat4& mat) { reorientation_matrix_ = mat; }
	inline void set_scale_matrix(const glm::mat4& mat) { scale_matrix_ = mat; }
	inline void set_rotate_matrix(const glm::mat4& mat) { rotate_matrix_ = mat; }
	inline void set_projection_matrix(const glm::mat4& mat) { projection_matrix_ = mat; }
	inline void set_view_matrix(const glm::mat4& mat) { view_matrix_ = mat; }
	inline void set_scene_size_in_view(QSizeF size) { scene_size_in_view_ = size; }

	virtual void AddPoint(const glm::vec3& point) = 0;
	virtual void MoveSelectedPoint(const glm::vec3& dest_point) = 0;
	virtual void MoveLastPoint(const glm::vec3& dest_point) = 0;
	virtual void Draw(GLuint program) = 0;
	virtual void Pick(const QSize& viewport_size, const QPointF& mouse_pos, bool* update, GLuint program) = 0;
	virtual void ClearVAOVBO() = 0;
	virtual void End() = 0;
	virtual void Clear() = 0;
	virtual void DeleteSelectedItem() = 0;
	virtual void DeleteLastItem() = 0;
	virtual void SetVisible(const bool visible) = 0;
	virtual const bool IsSelected() = 0;
	virtual const bool IsLineSelected() = 0;

	inline const bool started() { return started_; }

	virtual void ApplyPreferences() = 0;

	virtual void SetNodeVisible(const bool visible) = 0;

protected:
	virtual const int GetPointCount(const int index) = 0;
	virtual void GenerateItem() = 0;

protected:
	QGraphicsScene* scene_ = nullptr;

	float pixel_spacing_ = 1.0f;
	float slice_thickness_ = 1.0f;

	glm::mat4 reorientation_matrix_;
	glm::mat4 scale_matrix_;
	glm::mat4 rotate_matrix_;
	glm::mat4 projection_matrix_;
	glm::mat4 view_matrix_;
	QSizeF scene_size_in_view_ = QSizeF(1.0f, 1.0f);

	int current_index_ = 0;
	int selected_item_index_ = -1;

	bool started_ = false;
};
