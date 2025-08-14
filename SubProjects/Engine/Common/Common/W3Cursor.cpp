#include "W3Cursor.h"

#include "define_view.h"

#include <qapplication.h>
#include <QPixmap>

void CW3Cursor::SetViewCursor(const common::CommonToolTypeOnOff& tool_type) {
	switch (tool_type) {
	case common::CommonToolTypeOnOff::M_RULER:
	case common::CommonToolTypeOnOff::M_TAPELINE:
	case common::CommonToolTypeOnOff::M_TAPECURVE:
	case common::CommonToolTypeOnOff::M_ANGLE:
	case common::CommonToolTypeOnOff::M_PROFILE:
	case common::CommonToolTypeOnOff::M_AREALINE:
	case common::CommonToolTypeOnOff::M_ROI:
	case common::CommonToolTypeOnOff::M_RECTANGLE:
	case common::CommonToolTypeOnOff::M_CIRCLE:
	case common::CommonToolTypeOnOff::M_ARROW:
	case common::CommonToolTypeOnOff::M_LINE:
		QApplication::setOverrideCursor(CW3Cursor::CrossCursor());
		break;
	case common::CommonToolTypeOnOff::M_NOTE:
		QApplication::setOverrideCursor(CW3Cursor::IBeamCursor());
		break;
	case common::CommonToolTypeOnOff::M_FREEDRAW:
		QApplication::setOverrideCursor(CW3Cursor::FreedrawCursor());
		break;
	case common::CommonToolTypeOnOff::M_DEL:
		QApplication::setOverrideCursor(CW3Cursor::PointingHandCursor());
		break;
	case common::CommonToolTypeOnOff::V_PAN:
	case common::CommonToolTypeOnOff::V_PAN_LR:
		QApplication::setOverrideCursor(CW3Cursor::OpenHandCursor());
		break;
	case common::CommonToolTypeOnOff::V_ZOOM:
	case common::CommonToolTypeOnOff::V_ZOOM_R:
		QApplication::setOverrideCursor(CW3Cursor::ZoomCursor());
		break;
	case common::CommonToolTypeOnOff::V_LIGHT:
		QApplication::setOverrideCursor(CW3Cursor::LightCursor());
		break;
	default:
		QApplication::setOverrideCursor(CW3Cursor::ArrowCursor());
		break;
	}
}

QCursor CW3Cursor::ArrowCursor() {
	return QCursor(QPixmap(":/cursor/left_prt.png"), 3, 2);
}

QCursor CW3Cursor::CrossCursor() {
	return QCursor(QPixmap(":/cursor/cross.png"));
}

QCursor CW3Cursor::FreedrawCursor() {
	return QCursor(QPixmap(":/cursor/pencil.png"));
}

QCursor CW3Cursor::LightCursor() {
	return QCursor(QPixmap(":/cursor/light.png"));
}

QCursor CW3Cursor::WaitCursor(unsigned int step) {
	return QCursor(QPixmap(QString(":/cursor/wait/wait%1.png").arg(step % 8)));
}

QCursor CW3Cursor::IBeamCursor() {
	return QCursor(QPixmap(":/cursor/ibeam.png"));
}

QCursor CW3Cursor::SizeVerCursor() {
	return QCursor(QPixmap(":/cursor/size_ver.png"));
}

QCursor CW3Cursor::SizeHorCursor() {
	return QCursor(QPixmap(":/cursor/size_hor.png"));
}

QCursor CW3Cursor::SizeBDiagCursor() {
	return QCursor(QPixmap(":/cursor/size_bdiag.png"));
}

QCursor CW3Cursor::SizeFDiagCursor() {
	return QCursor(QPixmap(":/cursor/size_fdiag.png"));
}

QCursor CW3Cursor::SizeAllCursor() {
	return QCursor(QPixmap(":/cursor/size_all.png"));
}

QCursor CW3Cursor::SplitVCursor() {
	return QCursor(QPixmap(":/cursor/split_v.png"));
}

QCursor CW3Cursor::SplitHCursor() {
	return QCursor(QPixmap(":/cursor/split_h.png"));
}

QCursor CW3Cursor::PointingHandCursor() {
	return QCursor(QPixmap(":/cursor/pointinghand.png"), 8, 2);
}

QCursor CW3Cursor::ForbiddenCursor() {
	return QCursor(QPixmap(":/cursor/forbidden.png"));
}

QCursor CW3Cursor::OpenHandCursor() {
	return QCursor(QPixmap(":/cursor/openhand.png"), 8, 5);
}

QCursor CW3Cursor::ClosedHandCursor() {
	return QCursor(QPixmap(":/cursor/closedhand.png"), 8, 8);
}

QCursor CW3Cursor::RotateCursor() {
	return QCursor(QPixmap(":/cursor/rotate.png"));
}

QCursor CW3Cursor::ZoomCursor() {
	return QCursor(QPixmap(":/cursor/zoom.png"));
}

QCursor CW3Cursor::ZoomInCursor() {
	return QCursor(QPixmap(":/cursor/zoomin.png"));
}

QCursor CW3Cursor::ZoomOutCursor() {
	return QCursor(QPixmap(":/cursor/zoomout.png"));
}

QCursor CW3Cursor::PointCursor() {
	return QCursor(QPixmap(":/cursor/point.png"));
}
