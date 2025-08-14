#include "measure_length_3d.h"

#include <QDebug>
#include <QOpenGLFramebufferObject>
#include <QGraphicsScene>

#include <Engine/Common/Common/W3Memory.h>
#include <Engine/Common/GLfunctions/W3GLFunctions.h>
#ifndef WILL3D_VIEWER
#include <Engine/Core/W3ProjectIO/project_io_view.h>
#endif
#if USE_LINE_3D
#include "W3SurfaceDistanceItem.h"
#else
#include <Engine/UIModule/UIPrimitive/W3PathItem_anno.h>
#endif

MeasureLength3D::MeasureLength3D(QGraphicsScene* scene, const bool use_label)
	: use_label_(use_label), MeasureBase3D(scene)
{
}

MeasureLength3D::~MeasureLength3D()
{
	Clear();
}
#ifndef WILL3D_VIEWER
void MeasureLength3D::ExportProject(ProjectIOView& out)
{
	const ProjectIOView::Measure3DType type = ProjectIOView::Measure3DType::LENGTH;
	const int count = lines_.size();
	out.SaveMeasure3DCount(type, count);

	for (int i = 0; i < count; ++i)
	{
		out.SaveMeasure3D(type, i, lines_.at(i)->points());
	}
}

void MeasureLength3D::ImportProject(ProjectIOView& in)
{
	const ProjectIOView::Measure3DType type = ProjectIOView::Measure3DType::LENGTH;
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
void MeasureLength3D::AddPoint(const glm::vec3& point)
{
	if (started_)
	{
		return;
	}

	GenerateItem();
#if USE_LINE_3D
	lines_.at(current_index_)->addPoint(point);
#else
	glm::mat4 mvp = projection_matrix_ * view_matrix_ * rotate_matrix_ * scale_matrix_;
	glm::vec4 viewport_position = mvp * glm::vec4(point, 1.0f);
	QPointF scene_position =
		QPointF(viewport_position.x * scene_size_in_view_.width(), (viewport_position.y * -1.0f) * scene_size_in_view_.height()) +
		QPointF(scene_size_in_view_.width(), scene_size_in_view_.height());

	lines_.at(current_index_)->addPoint(scene_position);
	lines_.at(current_index_)->drawingCurPath(scene_position);
#endif
	started_ = true;
}

void MeasureLength3D::MoveSelectedPoint(const glm::vec3& dest_point)
{
#if USE_LINE_3D
	if (lines_.size() <= selected_item_index_)
	{
		return;
	}

	CW3SurfaceDistanceItem* line = lines_.at(selected_item_index_);
	const int picked_point = line->getPickIndex();
	if (picked_point > -1 && !line->label_selected())
	{
		line->editPoint(picked_point, dest_point);
	}
#else
#endif
}

void MeasureLength3D::MoveLastPoint(const glm::vec3& dest_point)
{
#if USE_LINE_3D
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
#else
	glm::mat4 mvp = projection_matrix_ * view_matrix_ * rotate_matrix_ * scale_matrix_;
	glm::vec4 viewport_position = mvp * glm::vec4(dest_point, 1.0f);
	QPointF scene_position =
		QPointF(viewport_position.x * scene_size_in_view_.width(), (viewport_position.y * -1.0f) * scene_size_in_view_.height()) +
		QPointF(scene_size_in_view_.width(), scene_size_in_view_.height());

	lines_.at(current_index_)->drawingCurPath(scene_position);
#endif
}

void MeasureLength3D::Draw(GLuint program)
{
#if USE_LINE_3D
	glUseProgram(program);
	for (int i = 0; i < lines_.size(); ++i)
	{
		CW3SurfaceDistanceItem* line = lines_.at(i);
		Init(line);

		line->setTransformMat(reorientation_matrix_, UIGLObjects::TransformType::REORIENTATION);
		line->setTransformMat(rotate_matrix_, UIGLObjects::TransformType::ARCBALL);
		line->setProjViewMat(projection_matrix_, view_matrix_);
		line->setTransformMat(scale_matrix_, UIGLObjects::TransformType::SCALE);
		line->setSceneSizeInView(scene_size_in_view_.width(), scene_size_in_view_.height());

		//qDebug() << i << line->getPickIndex();

		line->draw(program);
		if (use_label_)
		{
			line->SetDistanceLabel();
		}
	}
	glUseProgram(0);
#endif
}

void MeasureLength3D::Pick(const QSize& viewport_size, const QPointF& mouse_pos, bool* update, GLuint program)
{
	for (int i = 0; i < lines_.size(); ++i)
	{
		CW3SurfaceDistanceItem* line = lines_.at(i);
		if (line->label_selected())
		{
			selected_item_index_ = i;
			*update = true;
			return;
		}
	}

#if USE_LINE_3D
	selected_item_index_ = -1;

	QOpenGLFramebufferObject fbo(viewport_size.width(), viewport_size.height(), QOpenGLFramebufferObject::Depth);
	fbo.bind();
	glViewport(0, 0, viewport_size.width(), viewport_size.height());

	glUseProgram(program);

#if 0
	for (int i = 0; i < lines_.size(); ++i)
	{
		lines_.at(i)->SetSelected(false);
	}
#endif

	for (int i = 0; i < lines_.size(); ++i)
	{
		bool each_update = false;
		CW3SurfaceDistanceItem* line = lines_.at(i);
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
#endif
}

void MeasureLength3D::ClearVAOVBO()
{
#if USE_LINE_3D
	for (int i = 0; i < lines_.size(); ++i)
	{
		lines_.at(i)->clearVAOVBO();
	}
#endif
}

void MeasureLength3D::End()
{
#if !USE_LINE_3D
	lines_.at(current_index_)->endEdit();
#endif

	started_ = false;
}

void MeasureLength3D::Clear()
{
	started_ = false;
	for (int i = 0; i < lines_.size(); ++i)
	{
		CW3SurfaceDistanceItem* line = lines_.at(i);
		line->clear();
		SAFE_DELETE_OBJECT(line);
	}
	lines_.clear();
}

void MeasureLength3D::DeleteSelectedItem()
{
	started_ = false;
	if (lines_.size() <= selected_item_index_)
	{
		return;
	}

#if USE_LINE_3D
	CW3SurfaceDistanceItem* line = lines_.at(selected_item_index_);
	line->clear();
	line->clearVAOVBO();
#else
	CW3PathItem_anno* line = lines_.at(selected_item_index_);
#endif
	SAFE_DELETE_OBJECT(line);

	lines_.erase(lines_.begin() + selected_item_index_);

	selected_item_index_ = -1;
}

void MeasureLength3D::DeleteLastItem()
{
	selected_item_index_ = lines_.size() - 1;

	DeleteSelectedItem();
}

void MeasureLength3D::SetVisible(const bool visible)
{
	for (int i = 0; i < lines_.size(); ++i)
	{
#if USE_LINE_3D
		lines_.at(i)->SetVisible(visible);
#else
#endif
	}
}

const int MeasureLength3D::GetPointCount(const int index)
{
	if (lines_.size() > index)
	{
#if USE_LINE_3D
		return lines_.at(index)->points().size();
#else
#endif
	}
	else
	{
		return -1;
	}
}

void MeasureLength3D::Init(CW3SurfaceDistanceItem* line)
{
	line->set_line_width(0.008f);
	line->set_volume_range(volume_range_);
	line->set_pixel_spacing(pixel_spacing_);
	line->set_slice_thickness(slice_thickness_);
	line->setSceneSizeInView(scene_size_in_view_.width(), scene_size_in_view_.height());
}

void MeasureLength3D::GenerateItem()
{
#if USE_LINE_3D
	CW3SurfaceDistanceItem* line = new CW3SurfaceDistanceItem(scene_, CW3SurfaceDistanceItem::SHAPE::SIMPLE_LINE);
	Init(line);

	line->ApplyPreferences();
#else
	CW3PathItem_anno* line = new CW3PathItem_anno(scene_);
	line->setLabel(QString("0 [mm]"));
#endif

	lines_.push_back(line);
	current_index_ = lines_.size() - 1;
}

const bool MeasureLength3D::IsSelected()
{
	for (int i = 0; i < lines_.size(); ++i)
	{
		CW3SurfaceDistanceItem* line = lines_.at(i);
		if (line->label_selected() || 
			line->getPickIndex() > -1 || 
			line->line_selected())
		{
			return true;
		}
	}

	return false;
}

const bool MeasureLength3D::IsLineSelected()
{
	for (int i = 0; i < lines_.size(); ++i)
	{
		CW3SurfaceDistanceItem* line = lines_.at(i);
		if (line->line_selected())
		{
			return true;
		}
	}

	return false;
}

void MeasureLength3D::ApplyPreferences()
{
	for (int i = 0; i < lines_.size(); ++i)
	{
		lines_.at(i)->ApplyPreferences();
	}
}

void MeasureLength3D::SetNodeVisible(const bool visible)
{
	for (int i = 0; i < lines_.size(); ++i)
	{
		lines_.at(i)->SetNodeVisible(visible);
	}
}
