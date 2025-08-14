#include "measure_base_3d.h"

#include <QDebug>
#include <QOpenGLFramebufferObject>

#include <Engine/Common/Common/W3Memory.h>
#include <Engine/Common/GLfunctions/W3GLFunctions.h>

MeasureBase3D::MeasureBase3D(QGraphicsScene* scene)
	: scene_(scene)
{
}

MeasureBase3D::~MeasureBase3D()
{
}
