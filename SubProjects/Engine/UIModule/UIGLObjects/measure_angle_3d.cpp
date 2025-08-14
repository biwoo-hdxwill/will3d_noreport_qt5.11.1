#include "measure_angle_3d.h"

#include <QDebug>
#include <QOpenGLFramebufferObject>

#include <Engine/Common/Common/W3Memory.h>
#include <Engine/Common/GLfunctions/W3GLFunctions.h>
#ifndef WILL3D_VIEWER
#include <Engine/Core/W3ProjectIO/project_io_view.h>
#endif
#include "W3SurfaceAngleItem.h"

MeasureAngle3D::MeasureAngle3D(QGraphicsScene* scene)
	: MeasureBase3D(scene)
{
}

MeasureAngle3D::~MeasureAngle3D()
{
	Clear();
}
#ifndef WILL3D_VIEWER
void MeasureAngle3D::ExportProject(ProjectIOView& out)
{
	const ProjectIOView::Measure3DType type = ProjectIOView::Measure3DType::ANGLE;
	const int count = angles_.size();
	out.SaveMeasure3DCount(type, count);

	for (int i = 0; i < count; ++i)
	{
		out.SaveMeasure3D(type, i, angles_.at(i)->points());
	}
}

void MeasureAngle3D::ImportProject(ProjectIOView& in)
{
	const ProjectIOView::Measure3DType type = ProjectIOView::Measure3DType::ANGLE;
	int count = 0;
	in.LoadMeasure3DCount(type, count);

	for (int i = 0; i < count; ++i)
	{
		std::vector<glm::vec3> points;
		in.LoadMeasure3D(type, i, points);
		GenerateItem();
		angles_.at(current_index_)->setPoints(points);
	}
}
#endif
void MeasureAngle3D::AddPoint(const glm::vec3& point)
{
	if (started_)
	{
		return;
	}

	GenerateItem();
	angles_.at(current_index_)->addPoint(point);
	++current_drawing_point_index_;
	started_ = true;
}

void MeasureAngle3D::MoveSelectedPoint(const glm::vec3& dest_point)
{
	if (angles_.size() <= selected_item_index_)
	{
		return;
	}

	CW3SurfaceAngleItem* angle = angles_.at(selected_item_index_);
	const int picked_point = angle->getPickIndex();;
	if (picked_point > -1 && !angle->label_selected())
	{
		angle->editPoint(picked_point, dest_point);
	}
}

void MeasureAngle3D::MoveLastPoint(const glm::vec3& dest_point)
{
	int point_count = GetPointCount(current_index_);
	if (point_count < 1)
	{
		return;
	}

	if (point_count == current_drawing_point_index_)
	{
		angles_.at(current_index_)->addPoint(dest_point);
		++point_count;
	}

	angles_.at(current_index_)->editPoint(point_count - 1, dest_point);
}

void MeasureAngle3D::Draw(GLuint program)
{
	glUseProgram(program);
	for (int i = 0; i < angles_.size(); ++i)
	{
		CW3SurfaceAngleItem* angle = angles_.at(i);
		Init(angle);

		angle->setTransformMat(reorientation_matrix_, UIGLObjects::TransformType::REORIENTATION);
		angle->setTransformMat(rotate_matrix_, UIGLObjects::TransformType::ARCBALL);
		angle->setProjViewMat(projection_matrix_, view_matrix_);
		angle->setTransformMat(scale_matrix_, UIGLObjects::TransformType::SCALE);
		angle->setSceneSizeInView(scene_size_in_view_.width(), scene_size_in_view_.height());

		//qDebug() << i << angle->getPickIndex();

		angle->draw(program);
		angle->SetAngleLabel();
	}
	glUseProgram(0);
}

void MeasureAngle3D::Pick(const QSize& viewport_size, const QPointF& mouse_pos, bool* update, GLuint program)
{
	for (int i = 0; i < angles_.size(); ++i)
	{
		CW3SurfaceAngleItem* angle = angles_.at(i);
		if (angle->label_selected())
		{
			selected_item_index_ = i;
			*update = true;
			return;
		}
	}

	selected_item_index_ = -1;

	QOpenGLFramebufferObject fbo(viewport_size.width(), viewport_size.height(), QOpenGLFramebufferObject::Depth);
	fbo.bind();
	glViewport(0, 0, viewport_size.width(), viewport_size.height());

	glUseProgram(program);

#if 0
	for (int i = 0; i < angles_.size(); ++i)
	{
		angles_.at(i)->SetSelected(false);
	}
#endif

	for (int i = 0; i < angles_.size(); ++i)
	{
		bool each_update = false;
		CW3SurfaceAngleItem* angle = angles_.at(i);
		angle->pick(mouse_pos, &each_update, program);
		*update |= each_update;
		if (angle->getPickIndex() > -1 || angle->line_selected())
		{
			selected_item_index_ = i;
			break;
		}
	}

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	fbo.release();
	glUseProgram(0);
}

void MeasureAngle3D::ClearVAOVBO()
{
	for (int i = 0; i < angles_.size(); ++i)
	{
		angles_.at(i)->clearVAOVBO();
	}
}

void MeasureAngle3D::End()
{
	if (!started_)
	{
		return;
	}

	if (GetPointCount(current_index_) > 2)
	{
		started_ = false;
		current_drawing_point_index_ = 0;
	}
	else
	{
		++current_drawing_point_index_;
	}
}

void MeasureAngle3D::Clear()
{
	started_ = false;
	for (int i = 0; i < angles_.size(); ++i)
	{
		CW3SurfaceAngleItem* angle = angles_.at(i);
		angle->clear();
		SAFE_DELETE_OBJECT(angle);
	}
	angles_.clear();
}

void MeasureAngle3D::DeleteSelectedItem()
{
	started_ = false;
	if (angles_.size() <= selected_item_index_)
	{
		return;
	}

	CW3SurfaceAngleItem* angle = angles_.at(selected_item_index_);
	angle->clear();
	angle->clearVAOVBO();
	SAFE_DELETE_OBJECT(angle);

	angles_.erase(angles_.begin() + selected_item_index_);

	selected_item_index_ = -1;
	current_drawing_point_index_ = 0;
}

void MeasureAngle3D::DeleteLastItem()
{
	selected_item_index_ = angles_.size() - 1;

	DeleteSelectedItem();
}

void MeasureAngle3D::SetVisible(const bool visible)
{
	for (int i = 0; i < angles_.size(); ++i)
	{
		angles_.at(i)->SetVisible(visible);
	}
}

const int MeasureAngle3D::GetPointCount(const int index)
{
	if (angles_.size() > index)
	{
		return angles_.at(index)->points().size();
	}
	else
	{
		return -1;
	}
}

void MeasureAngle3D::Init(CW3SurfaceAngleItem* angle)
{
	angle->set_line_width(0.008f);
	angle->set_pixel_spacing(pixel_spacing_);
	angle->set_slice_thickness(slice_thickness_);
}

void MeasureAngle3D::GenerateItem()
{
	CW3SurfaceAngleItem* angle = new CW3SurfaceAngleItem(scene_, CW3SurfaceAngleItem::SHAPE::SIMPLE_LINE);
	Init(angle);

	angle->ApplyPreferences();

	angles_.push_back(angle);

	current_index_ = angles_.size() - 1;
}

const bool MeasureAngle3D::IsSelected()
{
	for (int i = 0; i < angles_.size(); ++i)
	{
		CW3SurfaceAngleItem* angle = angles_.at(i);
		if (angle->label_selected() || 
			angle->getPickIndex() > -1 || 
			angle->line_selected())
		{
			return true;
		}
	}

	return false;
}

const bool MeasureAngle3D::IsLineSelected()
{
	for (int i = 0; i < angles_.size(); ++i)
	{
		CW3SurfaceAngleItem* angle = angles_.at(i);
		if (angle->line_selected())
		{
			return true;
		}
	}

	return false;
}

void MeasureAngle3D::ApplyPreferences()
{
	for (int i = 0; i < angles_.size(); ++i)
	{
		angles_.at(i)->ApplyPreferences();
	}
}

void MeasureAngle3D::SetNodeVisible(const bool visible)
{
	for (int i = 0; i < angles_.size(); ++i)
	{
		angles_.at(i)->SetNodeVisible(visible);
	}
}
