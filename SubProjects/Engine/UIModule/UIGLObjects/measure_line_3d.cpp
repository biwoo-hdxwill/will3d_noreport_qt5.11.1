#include "measure_line_3d.h"

#include <QDebug>
#include <QOpenGLFramebufferObject>
#include <QGraphicsScene>

#include <Engine/Common/Common/W3Memory.h>
#include <Engine/Common/GLfunctions/W3GLFunctions.h>
#ifndef WILL3D_VIEWER
#include <Engine/Core/W3ProjectIO/project_io_view.h>
#endif
#include "W3SurfaceLineItem.h"

MeasureLine3D::MeasureLine3D()
	: MeasureBase3D(nullptr)
{
}

MeasureLine3D::~MeasureLine3D()
{
	Clear();
}
#ifndef WILL3D_VIEWER
void MeasureLine3D::ExportProject(ProjectIOView& out)
{
	const ProjectIOView::Measure3DType type = ProjectIOView::Measure3DType::LINE;
	const int count = lines_.size();
	out.SaveMeasure3DCount(type, count);

	for (int i = 0; i < count; ++i)
	{
		out.SaveMeasure3D(type, i, lines_.at(i)->points());
	}
}

void MeasureLine3D::ImportProject(ProjectIOView& in)
{
	const ProjectIOView::Measure3DType type = ProjectIOView::Measure3DType::LINE;
	int count = 0;
	in.LoadMeasure3DCount(type, count);

	for (int i = 0; i < count; ++i)
	{
		std::vector<glm::vec3> points;
		in.LoadMeasure3D(type, i, points);
		GenerateItem();
		lines_.at(current_index_)->setPoints(points);
	}
}
#endif
void MeasureLine3D::AddPoint(const glm::vec3& point)
{
	if (started_)
	{
		return;
	}

	GenerateItem();
	lines_.at(current_index_)->addPoint(point);
	started_ = true;
}

void MeasureLine3D::MoveSelectedPoint(const glm::vec3& dest_point)
{
	if (lines_.size() <= selected_item_index_)
	{
		return;
	}

	CW3SurfaceLineItem* line = lines_.at(selected_item_index_);
	const int picked_point = line->getPickIndex();
	if (picked_point > -1)
	{
		line->editPoint(picked_point, dest_point);
	}
}

void MeasureLine3D::MoveLastPoint(const glm::vec3& dest_point)
{
	int point_count = GetPointCount(current_index_);
	if (point_count < 1)
	{
		return;
	}

	switch (point_count)
	{
	case 1:
		lines_.at(current_index_)->addPoint(dest_point);
		++point_count;
		break;
	default:
		break;
	}

	lines_.at(current_index_)->editPoint(point_count - 1, dest_point);
}

void MeasureLine3D::Draw(GLuint program)
{
	glUseProgram(program);
	for (int i = 0; i < lines_.size(); ++i)
	{
		CW3SurfaceLineItem* line = lines_.at(i);
		Init(line);

		line->setTransformMat(reorientation_matrix_, UIGLObjects::TransformType::REORIENTATION);
		line->setTransformMat(rotate_matrix_, UIGLObjects::TransformType::ARCBALL);
		line->setProjViewMat(projection_matrix_, view_matrix_);
		line->setTransformMat(scale_matrix_, UIGLObjects::TransformType::SCALE);

		line->draw(program);
	}
	glUseProgram(0);
}

void MeasureLine3D::Pick(const QSize& viewport_size, const QPointF& mouse_pos, bool* update, GLuint program)
{
	selected_item_index_ = -1;

	QOpenGLFramebufferObject fbo(viewport_size.width(), viewport_size.height(), QOpenGLFramebufferObject::Depth);
	fbo.bind();
	glViewport(0, 0, viewport_size.width(), viewport_size.height());

	glUseProgram(program);

	for (int i = 0; i < lines_.size(); ++i)
	{
		bool each_update = false;
		CW3SurfaceLineItem* line = lines_.at(i);
		line->pick(mouse_pos, &each_update, program);
		*update |= each_update;
		if (line->getPickIndex() > -1 || line->line_selected())
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

void MeasureLine3D::ClearVAOVBO()
{
	for (int i = 0; i < lines_.size(); ++i)
	{
		lines_.at(i)->clearVAOVBO();
	}
}

void MeasureLine3D::End()
{
	started_ = false;
}

void MeasureLine3D::Clear()
{
	started_ = false;
	for (int i = 0; i < lines_.size(); ++i)
	{
		CW3SurfaceLineItem* line = lines_.at(i);
		line->clear();
		SAFE_DELETE_OBJECT(line);
	}
	lines_.clear();
}

void MeasureLine3D::DeleteSelectedItem()
{
	started_ = false;
	if (lines_.size() <= selected_item_index_)
	{
		return;
	}

	CW3SurfaceLineItem* line = lines_.at(selected_item_index_);
	line->clear();
	line->clearVAOVBO();

	SAFE_DELETE_OBJECT(line);

	lines_.erase(lines_.begin() + selected_item_index_);

	selected_item_index_ = -1;
}

void MeasureLine3D::DeleteLastItem()
{
	selected_item_index_ = lines_.size() - 1;

	DeleteSelectedItem();
}

void MeasureLine3D::SetVisible(const bool visible)
{
	for (int i = 0; i < lines_.size(); ++i)
	{
		lines_.at(i)->SetVisible(visible);
	}
}

const int MeasureLine3D::GetPointCount(const int index)
{
	if (lines_.size() > index)
	{
		return lines_.at(index)->points().size();
	}
	else
	{
		return -1;
	}
}

void MeasureLine3D::Init(CW3SurfaceLineItem* line)
{
	line->set_line_width(0.008f);
}

void MeasureLine3D::GenerateItem()
{
	CW3SurfaceLineItem* line = new CW3SurfaceLineItem(CW3SurfaceLineItem::SHAPE::SIMPLE_LINE, true);
	Init(line);

	line->ApplyPreferences();

	lines_.push_back(line);
	current_index_ = lines_.size() - 1;
}

const bool MeasureLine3D::IsSelected()
{
	for (int i = 0; i < lines_.size(); ++i)
	{
		CW3SurfaceLineItem* line = lines_.at(i);
		if (line->getPickIndex() > -1 || 
			line->line_selected())
		{
			return true;
		}
	}

	return false;
}

const bool MeasureLine3D::IsLineSelected()
{
	for (int i = 0; i < lines_.size(); ++i)
	{
		CW3SurfaceLineItem* line = lines_.at(i);
		if (line->line_selected())
		{
			return true;
		}
	}

	return false;
}

void MeasureLine3D::ApplyPreferences()
{
	for (int i = 0; i < lines_.size(); ++i)
	{
		lines_.at(i)->ApplyPreferences();
	}
}

void MeasureLine3D::SetNodeVisible(const bool visible)
{
	for (int i = 0; i < lines_.size(); ++i)
	{
		lines_.at(i)->SetNodeVisible(visible);
	}
}
