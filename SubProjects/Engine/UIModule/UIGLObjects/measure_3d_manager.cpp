#include "measure_3d_manager.h"

#include <QMouseEvent>

#include <Engine/Common/Common/W3Memory.h>

#include "measure_length_3d.h"
#include "measure_angle_3d.h"
#include "measure_line_3d.h"

Measure3DManager::Measure3DManager(QGraphicsScene* scene)
	: scene_(scene)
{
}

Measure3DManager::~Measure3DManager()
{
	for (auto& elem : measure_list_)
	{
		SAFE_DELETE_OBJECT(elem);
	}
	measure_list_.clear();
}

void Measure3DManager::SetType(CommonToolTypeOnOff type)
{
	if (type != CommonToolTypeOnOff::M_RULER &&
		type != CommonToolTypeOnOff::M_ANGLE &&
		type != CommonToolTypeOnOff::M_LINE &&
		type != CommonToolTypeOnOff::M_DEL)
	{
		return;
	}

	if (type_ != type)
	{
		for (auto& elem : measure_list_)
		{
			if (elem && elem->started())
			{
				elem->End();
			}
		}
	}

	type_ = type;
}

void Measure3DManager::SetVisible(const bool visible)
{
	for (auto& elem : measure_list_)
	{
		if (elem)
		{
			elem->SetVisible(visible);
		}
	}
}

void Measure3DManager::MousePressEvent(Qt::MouseButton button, const glm::vec3& volume_pos, bool& update)
{
	if (button != Qt::LeftButton)
	{
		return;
	}

	if (!measure_length_3d_)
	{
		InitMeasureLength();
	}

	if (!measure_angle_3d_)
	{
		InitMeasureAngle();
	}

	if (!measure_line_3d_)
	{
		InitMeasureLine();
	}

	if (type_ != CommonToolTypeOnOff::M_DEL)
	{
		for (auto& elem : measure_list_)
		{
			if (elem->IsSelected())
			{
				return;
			}
		}
	}

	switch (type_)
	{
	case CommonToolTypeOnOff::NONE:
		break;
	case CommonToolTypeOnOff::M_RULER:
		if (measure_angle_3d_->started())
		{
			measure_angle_3d_->End();
		}
		if (measure_line_3d_->started())
		{
			measure_line_3d_->End();
		}

		if (measure_length_3d_->started())
		{
			measure_length_3d_->End();
		}
		else
		{
			measure_length_3d_->AddPoint(volume_pos);
		}

		started_ = measure_length_3d_->started();
		update = true;
		break;
	case CommonToolTypeOnOff::M_ANGLE:
		if (measure_length_3d_->started())
		{
			measure_length_3d_->End();
		}
		if (measure_line_3d_->started())
		{
			measure_line_3d_->End();
		}

		if (measure_angle_3d_->started())
		{
			measure_angle_3d_->End();
		}
		else
		{
			measure_angle_3d_->AddPoint(volume_pos);
		}

		started_ = measure_angle_3d_->started();
		update = true;
		break;
	case CommonToolTypeOnOff::M_LINE:
		if (measure_length_3d_->started())
		{
			measure_length_3d_->End();
		}
		if (measure_angle_3d_->started())
		{
			measure_angle_3d_->End();
		}

		if (measure_line_3d_->started())
		{
			measure_line_3d_->End();
		}
		else
		{
			measure_line_3d_->AddPoint(volume_pos);
		}

		started_ = measure_line_3d_->started();
		update = true;
		break;
	case CommonToolTypeOnOff::M_DEL:
		if (measure_length_3d_->IsSelected())
		{
			measure_length_3d_->DeleteSelectedItem();

			update = true;
		}
		else if (measure_angle_3d_->IsSelected())
		{
			measure_angle_3d_->DeleteSelectedItem();

			update = true;
		}
		else if (measure_line_3d_->IsSelected())
		{
			measure_line_3d_->DeleteSelectedItem();

			update = true;
		}
		break;
	default:
		break;
	}
}

void Measure3DManager::MouseMoveEvent(Qt::MouseButtons buttons, const glm::vec3& volume_pos, bool& update)
{
	if (buttons == Qt::NoButton)
	{
		MeasureBase3D* measure = nullptr;
		switch (type_)
		{
		case CommonToolTypeOnOff::M_RULER:
			measure = measure_length_3d_;
			break;
		case CommonToolTypeOnOff::M_ANGLE:
			measure = measure_angle_3d_;
			break;
		case CommonToolTypeOnOff::M_LINE:
			measure = measure_line_3d_;
			break;
		default:
			break;
		}

		if (!measure)
		{
			return;
		}

		if (measure->started())
		{
			measure->MoveLastPoint(volume_pos);
		}

		update = true;
	}
	else if (buttons == Qt::LeftButton)
	{
		update = false;
		for (auto& elem : measure_list_)
		{
			if (elem && elem->IsSelected() && !elem->IsLineSelected())
			{
				elem->MoveSelectedPoint(volume_pos);
				update = true;
			}
		}
	}
}

void Measure3DManager::MouseReleaseEvent(Qt::MouseButton button, const glm::vec3& volume_pos, bool& update)
{
	/*if (button != Qt::LeftButton)
	{
		return;
	}

	if (!measure_length_3d_)
	{
		InitMeasureLength();
	}

	if (!measure_angle_3d_)
	{
		InitMeasureAngle();
	}

	switch (type_)
	{
	case CommonToolTypeOnOff::NONE:
		break;
	case CommonToolTypeOnOff::M_RULER:
		if (measure_length_3d_->IsSelected() || measure_angle_3d_->IsSelected())
		{
			return;
		}

		if (measure_angle_3d_->started())
		{
			measure_angle_3d_->End();
		}

		if (measure_length_3d_->started())
		{
			measure_length_3d_->End();
		}
		else
		{
			measure_length_3d_->AddPoint(volume_pos);
		}

		started_ = measure_length_3d_->started();
		update = true;
		break;
	case CommonToolTypeOnOff::M_ANGLE:
		if (measure_length_3d_->IsSelected() || measure_angle_3d_->IsSelected())
		{
			return;
		}

		if (measure_length_3d_->started())
		{
			measure_length_3d_->End();
		}

		if (measure_angle_3d_->started())
		{
			measure_angle_3d_->End();
		}
		else
		{
			measure_angle_3d_->AddPoint(volume_pos);
		}

		started_ = measure_angle_3d_->started();
		update = true;
		break;
	case CommonToolTypeOnOff::M_DEL:
		if (measure_length_3d_->IsSelected())
		{
			measure_length_3d_->DeleteSelectedItem();

			update = true;
		}
		else if (measure_angle_3d_->IsSelected())
		{
			measure_angle_3d_->DeleteSelectedItem();

			update = true;
		}
		break;
	default:
		break;
	}*/
}

void Measure3DManager::Draw(GLuint program)
{
	if (measure_length_3d_)
	{
		measure_length_3d_->set_pixel_spacing(pixel_spacing_);
		measure_length_3d_->set_slice_thickness(slice_thickness_);
		glm::vec3 volume_range(scale_matrix_[0][0], scale_matrix_[1][1], scale_matrix_[2][2]);
		measure_length_3d_->set_volume_range(volume_range);
		measure_length_3d_->set_scene_size_in_view(scene_size_in_view_);

		measure_length_3d_->set_reorientation_matrix(reorientation_matrix_);
		measure_length_3d_->set_scale_matrix(scale_matrix_);
		measure_length_3d_->set_rotate_matrix(rotate_matrix_);
		measure_length_3d_->set_projection_matrix(projection_matrix_);
		measure_length_3d_->set_view_matrix(view_matrix_);

		measure_length_3d_->Draw(program);
	}

	if (measure_angle_3d_)
	{
		measure_angle_3d_->set_pixel_spacing(pixel_spacing_);
		measure_angle_3d_->set_slice_thickness(slice_thickness_);
		measure_angle_3d_->set_scene_size_in_view(scene_size_in_view_);

		measure_angle_3d_->set_reorientation_matrix(reorientation_matrix_);
		measure_angle_3d_->set_scale_matrix(scale_matrix_);
		measure_angle_3d_->set_rotate_matrix(rotate_matrix_);
		measure_angle_3d_->set_projection_matrix(projection_matrix_);
		measure_angle_3d_->set_view_matrix(view_matrix_);
		measure_angle_3d_->Draw(program);
	}

	if (measure_line_3d_)
	{
		measure_line_3d_->set_reorientation_matrix(reorientation_matrix_);
		measure_line_3d_->set_scale_matrix(scale_matrix_);
		measure_line_3d_->set_rotate_matrix(rotate_matrix_);
		measure_line_3d_->set_projection_matrix(projection_matrix_);
		measure_line_3d_->set_view_matrix(view_matrix_);
		measure_line_3d_->Draw(program);
	}
}

void Measure3DManager::Pick(const QSize& viewport_size, const QPointF& mouse_pos, bool* update, GLuint program)
{
	for (auto& elem : measure_list_)
	{
		if (elem)
		{
			elem->Pick(viewport_size, mouse_pos, update, program);
		}
	}
}

void Measure3DManager::ClearVAOVBO()
{
	started_ = false;

	for (auto& elem : measure_list_)
	{
		if (elem)
		{
			elem->ClearVAOVBO();
		}
	}
}

void Measure3DManager::Clear()
{
	type_ = CommonToolTypeOnOff::NONE;

	started_ = false;

	for (auto& elem : measure_list_)
	{
		if (elem)
		{
			elem->Clear();
		}
	}
}

void Measure3DManager::SetSceneSizeInView(QSizeF size)
{
	scene_size_in_view_ = size;

	for (auto& elem : measure_list_)
	{
		if (elem)
		{
			elem->set_scene_size_in_view(scene_size_in_view_);
		}
	}
}

void Measure3DManager::InitMeasureLength()
{
	if (!scene_)
	{
		return;
	}

	if (!measure_length_3d_)
	{
		measure_length_3d_ = new MeasureLength3D(scene_);
		measure_list_.push_back(measure_length_3d_);
	}
	measure_length_3d_->set_pixel_spacing(pixel_spacing_);
	measure_length_3d_->set_slice_thickness(slice_thickness_);
#if 1
	glm::vec3 volume_range(scale_matrix_[0][0], scale_matrix_[1][1], scale_matrix_[2][2]);
	measure_length_3d_->set_volume_range(volume_range);
#else
	measure_length_3d_->set_volume_range(volume_range_);
#endif
	measure_length_3d_->set_scene_size_in_view(scene_size_in_view_);
}

void Measure3DManager::InitMeasureAngle()
{
	if (!scene_)
	{
		return;
	}

	if (!measure_angle_3d_)
	{
		measure_angle_3d_ = new MeasureAngle3D(scene_);
		measure_list_.push_back(measure_angle_3d_);
	}
	measure_angle_3d_->set_pixel_spacing(pixel_spacing_);
	measure_angle_3d_->set_slice_thickness(slice_thickness_);
	measure_angle_3d_->set_scene_size_in_view(scene_size_in_view_);
}

void Measure3DManager::InitMeasureLine()
{
	if (!scene_)
	{
		return;
	}

	if (!measure_line_3d_)
	{
		measure_line_3d_ = new MeasureLine3D();
		measure_list_.push_back(measure_line_3d_);
	}
}

void Measure3DManager::InitMeasureFreedraw()
{

}

const bool Measure3DManager::IsSelected()
{
	bool selected = false;

	for (auto& elem : measure_list_)
	{
		if (elem)
		{
			selected |= elem->IsSelected();
		}
	}

	return selected;
}

void Measure3DManager::DeleteUnfinishedItem()
{
	started_ = false;

	MeasureBase3D* measure = nullptr;
	switch (type_)
	{
	case CommonToolTypeOnOff::M_RULER:
		measure = measure_length_3d_;
		break;
	case CommonToolTypeOnOff::M_ANGLE:
		measure = measure_angle_3d_;
		break;
	case CommonToolTypeOnOff::M_LINE:
		measure = measure_line_3d_;
		break;
	default:
		break;
	}

	if (measure && measure->started())
	{
		measure->DeleteLastItem();
	}
}
#ifndef WILL3D_VIEWER
void Measure3DManager::ExportProject(ProjectIOView& out)
{
	for (auto& elem : measure_list_)
	{
		if (elem)
		{
			elem->ExportProject(out);
		}
	}
}

void Measure3DManager::ImportProject(ProjectIOView& in)
{
	if (!measure_length_3d_)
	{
		measure_length_3d_ = new MeasureLength3D(scene_);
		measure_list_.push_back(measure_length_3d_);
	}

	if (!measure_angle_3d_)
	{
		measure_angle_3d_ = new MeasureAngle3D(scene_);
		measure_list_.push_back(measure_angle_3d_);
	}

	if (!measure_line_3d_)
	{
		measure_line_3d_ = new MeasureLine3D();
		measure_list_.push_back(measure_line_3d_);
	}

	for (auto& elem : measure_list_)
	{
		if (elem)
		{
			elem->ImportProject(in);
		}
	}
}
#endif
void Measure3DManager::ApplyPreferences()
{
	for (auto& elem : measure_list_)
	{
		if (elem)
		{
			elem->ApplyPreferences();
		}
	}
}
