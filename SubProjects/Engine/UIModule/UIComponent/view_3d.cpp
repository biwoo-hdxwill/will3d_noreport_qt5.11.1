#include "view_3d.h"

#include <QKeyEvent>

#include <Engine/UIModule/UIViewController/base_view_controller_3d.h>
#include <Engine/UIModule/UIViewController/base_transform.h>

#include "scene.h"

View3D::View3D(const common::ViewTypeID& view_type, QWidget* parent)
	: View(view_type, parent)
{
}

View3D::~View3D()
{
}

void View3D::keyPressEvent(QKeyEvent* event)
{
	View::keyPressEvent(event);

	if (!View::IsEnableGLContext())
		return;

	if (event->key() == Qt::Key_Left ||
		event->key() == Qt::Key_Up ||
		event->key() == Qt::Key_Right ||
		event->key() == Qt::Key_Down)
	{
		RotateOneDegree(static_cast<RotateOneDegreeDirection>(event->key()));
	}
}

void View3D::SetWorldAxisDirection()
{
	scene().SetWorldAxisDirection(controller_3d()->GetRotateMatrix(), controller_3d()->GetViewMatrix());
}

void View3D::RotateOneDegree(RotateOneDegreeDirection direction)
{
	glm::vec3 rotate_axis(0.0f, 1.0f, 0.0f);

	if (direction == RotateOneDegreeDirection::UP ||
		direction == RotateOneDegreeDirection::DOWN)
	{
		rotate_axis = glm::vec3(-1.0f, 0.0f, 0.0f);
	}

	if (direction == RotateOneDegreeDirection::LEFT ||
		direction == RotateOneDegreeDirection::UP)
	{
		controller_3d()->transform().Rotate(-1.0f, rotate_axis);
	}
	else if (direction == RotateOneDegreeDirection::RIGHT ||
		direction == RotateOneDegreeDirection::DOWN)
	{
		controller_3d()->transform().Rotate(1.0f, rotate_axis);
	}

	View::MakeCurrent();
	controller_3d()->RenderingVolume();
	View::DoneCurrent();

	SetWorldAxisDirection();

	SceneUpdate();
}

void View3D::SetCliping(const std::vector<glm::vec4>& clip_plane, bool clip_enable)
{
	controller_3d()->SetCliping(clip_plane, clip_enable);

	UpdateVolume();
}
