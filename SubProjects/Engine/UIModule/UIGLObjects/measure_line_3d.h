#pragma once
/*=========================================================================
Copyright (c) 2019 All rights reserved by HDXWILL.

File:			measure_line_3d.h
Language:		C++11
Library:		Qt 5.8.0, Standard C++ Library, glew
Author:			JUNG DAE GUN
First Date:		2020-08-18
Modify Date:	2020-08-18
=========================================================================*/

#include "uiglobjects_global.h"

#include "measure_base_3d.h"

class CW3SurfaceLineItem;

class UIGLOBJECTS_EXPORT MeasureLine3D : public MeasureBase3D
{
public:
	MeasureLine3D();
	virtual ~MeasureLine3D();

#ifndef WILL3D_VIEWER
	virtual void ExportProject(ProjectIOView& out) override;
	virtual void ImportProject(ProjectIOView& in) override;
#endif

	virtual void AddPoint(const glm::vec3& point) override;
	virtual void MoveSelectedPoint(const glm::vec3& dest_point) override;
	virtual void MoveLastPoint(const glm::vec3& dest_point) override;
	virtual void Draw(GLuint program) override;
	virtual void Pick(const QSize& viewport_size, const QPointF& mouse_pos, bool* update, GLuint program) override;
	virtual void ClearVAOVBO() override;
	virtual void End() override;
	virtual void Clear() override;
	virtual void DeleteSelectedItem() override;
	virtual void DeleteLastItem() override;
	virtual void SetVisible(const bool visible) override;
	virtual const bool IsSelected() override;
	virtual const bool IsLineSelected() override;

	virtual void ApplyPreferences() override;

	virtual void SetNodeVisible(const bool visible) override;

protected:
	virtual const int GetPointCount(const int index) override;
	virtual void GenerateItem() override;

private:
	void Init(CW3SurfaceLineItem* line);

private:
	std::vector<CW3SurfaceLineItem*> lines_;
};
