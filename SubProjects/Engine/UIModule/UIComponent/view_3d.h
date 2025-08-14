#pragma once

#include "uicomponent_global.h"

#include "view.h"

class BaseViewController3D;

class UICOMPONENT_EXPORT View3D : public View
{
	Q_OBJECT

public:
	View3D(const common::ViewTypeID& view_type, QWidget* parent = nullptr);
	virtual ~View3D();

	virtual BaseViewController3D* controller_3d() = 0;
	virtual void UpdateVolume() {};

	void SetCliping(const std::vector<glm::vec4>& clip_plane, bool clip_enable);

protected:
	enum class RotateOneDegreeDirection
	{
		LEFT = Qt::Key_Left,
		RIGHT = Qt::Key_Right,
		UP = Qt::Key_Up,
		DOWN = Qt::Key_Down
	};

	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void SetWorldAxisDirection();
	virtual void RotateOneDegree(RotateOneDegreeDirection direction);
};
