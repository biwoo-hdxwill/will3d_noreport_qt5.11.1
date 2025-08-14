#pragma once

#include "uiframe_global.h"
#include <QFrame>

class QDragEnterEvent;
class QDropEvent;
class QLabel;
class UIFRAME_EXPORT CW3ReportThumbnail : public QFrame
{
public:
	CW3ReportThumbnail(QWidget *parent = 0);

public:
	void addThumbnail(const QString& strPath);
	void clearThumbnail();

protected:
	void dragEnterEvent(QDragEnterEvent *event) override;
	void dragMoveEvent(QDragMoveEvent *event) override;
	void dropEvent(QDropEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	//void wheelEvent(QWheelEvent *event) override;

private:
	QLabel* m_caption;
};
