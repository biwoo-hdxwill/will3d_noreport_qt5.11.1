#include "surface_axes_item_geometry.h"

#include <QThread>

#include <Engine/Common/Common/W3Memory.h>

#include "W3VBOs.h"

SurfaceAxesItemGeometry* SurfaceAxesItemGeometry::instance_ = nullptr;

SurfaceAxesItemGeometry::SurfaceAxesItemGeometry()
{
	if (arrow_ || torus_)
		return;

	arrow_ = new CW3VBOSTL(QString(":/stl/Axes/arrow.stl"));

	QThread::msleep(100);

	torus_ = new CW3VBOSTL(QString(":/stl/Axes/torus.stl"));
}

SurfaceAxesItemGeometry::~SurfaceAxesItemGeometry()
{
	SAFE_DELETE_OBJECT(arrow_);
	SAFE_DELETE_OBJECT(torus_);
}
