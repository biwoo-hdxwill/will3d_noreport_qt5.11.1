#include "base_view_3d.h"

#include <QMouseEvent>

#include <Engine/Common/Common/W3Memory.h>

#include <Engine/UIModule/UIGLObjects/measure_3d_manager.h>
#include <Engine/UIModule/UIViewController/base_view_controller_3d.h>
#include <Engine/UIModule/UIViewController/base_transform.h>

#include <Engine/Module/VREngine/W3VREngine.h>

#include "scene.h"

BaseView3D::BaseView3D(const common::ViewTypeID& view_type, QWidget* parent)
	: View(view_type, parent)
{
}

BaseView3D::~BaseView3D()
{
	if (View::IsEnableGLContext())
	{
		View::MakeCurrent();
		if (measure_3d_manager_)
		{
			measure_3d_manager_->ClearVAOVBO();
		}
		View::DoneCurrent();
	}
	SAFE_DELETE_OBJECT(measure_3d_manager_);
}
#ifndef WILL3D_VIEWER
void BaseView3D::ExportProjectForMeasure3D(ProjectIOView& out)
{
	if (measure_3d_manager_)
	{
		measure_3d_manager_->ExportProject(out);
	}
}

void BaseView3D::ImportProjectForMeasure3D(ProjectIOView& in)
{
	if (measure_3d_manager_)
	{
		measure_3d_manager_->ImportProject(in);
	}
}
#endif
void BaseView3D::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type)
{
	View::SetCommonToolOnOff(type);

	if (measure_3d_manager_)
	{
		measure_3d_manager_->SetType(type);
	}
}

void BaseView3D::resizeEvent(QResizeEvent* event)
{
	View::resizeEvent(event);

	if (measure_3d_manager_)
	{
		QPointF scene_center = GetSceneCenterPos();
		QSizeF scene_size_in_view(scene_center.x(), scene_center.y());

		measure_3d_manager_->SetSceneSizeInView(scene_size_in_view);
	}
}

void BaseView3D::mousePressEvent(QMouseEvent* event)
{
	View::mousePressEvent(event);
}

void BaseView3D::mouseMoveEvent(QMouseEvent* event)
{
	if (measure_3d_manager_)
	{
		if (!measure_3d_manager_->started() && event->buttons() == Qt::NoButton)
		{
			bool update = false;
			uint pick_program = CW3VREngine::GetInstance()->getPROGpickWithCoord();
			MakeCurrent();
			measure_3d_manager_->Pick(size(), event->pos(), &update, pick_program);
			DoneCurrent();

			if (update)
			{
				scene().update();
			}
		}
		else
		{
			MakeCurrent();
			vec3 volume_pos = controller_3d_->MapSceneToVol(event->pos());
			bool volume_picked = volume_pos.x > 0.0f;
			DoneCurrent();

			if (volume_picked)
			{
				bool update;
				measure_3d_manager_->MouseMoveEvent(event->buttons(), VolumeToGLVertex(volume_pos), update);

				if (update)
				{
					scene().update();
				}
			}
		}
	}
}

void BaseView3D::HideMeasure(bool toggled)
{
	View::HideMeasure(toggled);
}

void BaseView3D::DeleteAllMeasure()
{
	View::DeleteAllMeasure();
}

glm::vec3 BaseView3D::VolumeToGLVertex(glm::vec3 volume_pos)
{
	glm::vec3 gl_vertex_pos;

	glm::mat4 scale_matrix = controller_3d_->transform().model();
	glm::vec3 volume_range(scale_matrix[0][0], scale_matrix[1][1], scale_matrix[2][2]);
	gl_vertex_pos = volume_pos / volume_range * 2.0f - 1.0f;
	gl_vertex_pos.x = -gl_vertex_pos.x;

	return gl_vertex_pos;
}
