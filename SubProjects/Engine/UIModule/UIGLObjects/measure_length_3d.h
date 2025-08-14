#pragma once
/*=========================================================================
Copyright (c) 2019 All rights reserved by HDXWILL.

File:			measure_length_3d.h
Language:		C++11
Library:		Qt 5.8.0, Standard C++ Library, glew
Author:			JUNG DAE GUN
First Date:		2019-03-21
Modify Date:	2019-03-21
=========================================================================*/

#include "uiglobjects_global.h"

#include "measure_base_3d.h"

#define USE_LINE_3D 1
#if USE_LINE_3D
class CW3SurfaceDistanceItem;
#else
class CW3PathItem_anno;
#endif

class UIGLOBJECTS_EXPORT MeasureLength3D : public MeasureBase3D
{
public:
	MeasureLength3D(QGraphicsScene* scene, const bool use_label = true);
	virtual ~MeasureLength3D();
	
#ifndef WILL3D_VIEWER
	virtual void ExportProject(ProjectIOView& out) override;
	virtual void ImportProject(ProjectIOView& in) override;
#endif

	inline void set_volume_range(const glm::vec3& range) { volume_range_ = range; }

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
	void Init(CW3SurfaceDistanceItem* line);

private:
	glm::vec3 volume_range_ = glm::vec3(1.0f);

#if USE_LINE_3D
	std::vector<CW3SurfaceDistanceItem*> lines_;
#else
	std::vector<CW3PathItem_anno*> lines_;
#endif

	bool use_label_ = true;
};
