#pragma once
/*=========================================================================
Copyright (c) 2019 All rights reserved by HDXWILL.

File:			base_view_3d.h
Language:		C++11
Library:		Qt 5.8.0, Standard C++ Library
Author:			JUNG DAE GUN
First Date:		2019-04-19
Modify Date:	2019-04-19
=========================================================================*/

#include "view.h"

#include <memory>
#include <vector>

#if defined(__APPLE__)
#include <glm/detail/type_vec.hpp>
#else
#include <GL/glm/detail/type_vec.hpp>
#endif

class Measure3DManager;
class BaseViewController3D;
#ifndef WILL3D_VIEWER
class ProjectIOView;
#endif

class UICOMPONENT_EXPORT BaseView3D : public View
{
public:
	BaseView3D(const common::ViewTypeID& view_type, QWidget* parent = 0);
	virtual ~BaseView3D();

#ifndef WILL3D_VIEWER
	void ExportProjectForMeasure3D(ProjectIOView& out);
	void ImportProjectForMeasure3D(ProjectIOView& in);
#endif

	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;

	inline BaseViewController3D* controller_3d() const { return controller_3d_.get(); }
	inline Measure3DManager* measure_3d_manager() const { return measure_3d_manager_; }

protected:
	virtual void resizeEvent(QResizeEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	
	virtual void HideMeasure(bool toggled) override;
	virtual void DeleteAllMeasure() override;

	glm::vec3 VolumeToGLVertex(glm::vec3 volume_pos);

private:
	std::shared_ptr<BaseViewController3D> controller_3d_;

	Measure3DManager* measure_3d_manager_ = nullptr;
};

