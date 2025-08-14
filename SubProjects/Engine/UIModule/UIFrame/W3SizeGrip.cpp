#include "W3SizeGrip.h"

#include <QApplication>
#include "../../Common/Common/W3Cursor.h"

CW3SizeGrip::CW3SizeGrip(QWidget* parent)
	: QSizeGrip(parent) {}

CW3SizeGrip::~CW3SizeGrip() {}

void CW3SizeGrip::mouseMoveEvent(QMouseEvent * event) {
	QSizeGrip::mouseMoveEvent(event);
}

void CW3SizeGrip::enterEvent(QEvent * event) {
	QSizeGrip::enterEvent(event);
	QApplication::setOverrideCursor(CW3Cursor::SizeFDiagCursor());
}

void CW3SizeGrip::leaveEvent(QEvent * event) {
	QSizeGrip::leaveEvent(event);
	QApplication::setOverrideCursor(CW3Cursor::ArrowCursor());
}
