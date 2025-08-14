#include "W3ReportThumbnail.h"

#include <exception>
#include <iostream>

#include <QLabel>
#include <QMimeData>
#include <qboxlayout.h>
#include <qevent.h>
#include <qdrag.h>

#include <Engine/Common/Common/language_pack.h>

#include "../../Common/Common/W3Memory.h"

using std::exception;
using std::cout;
using std::endl;

CW3ReportThumbnail::CW3ReportThumbnail(QWidget *parent)
	: QFrame(parent)
{

	setAcceptDrops(true);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->setAlignment(Qt::AlignTop);
	this->setLayout(mainLayout);

	QLabel* label= new QLabel();
	label->setObjectName("thumbCaption");
	label->setTextFormat(Qt::RichText);
	label->setText("<img src=\":/image/report/box.png\">" + QString(" ") + lang::LanguagePack::txt_data_thumbnail());

	this->layout()->addWidget(label);
}

void CW3ReportThumbnail::addThumbnail(const QString& strPath)
{
	try
	{
		QImage img;

		if (!img.load(strPath))
            throw std::runtime_error("failed to load image.");

		QPixmap pixmap;
		pixmap = QPixmap::fromImage(img);
		pixmap = pixmap.scaled(300, 300, Qt::KeepAspectRatio);

		QLabel* label = new QLabel;
		label->setPixmap(pixmap);
		label->setObjectName(strPath);
		//label->setAttribute(Qt::WA_DeleteOnClose);

		this->layout()->addWidget(label);

	}
    catch (std::runtime_error& e)
	{
		cout << "CW3ReportThumbnail::addThumbnail: " << e.what() << endl;
	}

}

void CW3ReportThumbnail::clearThumbnail()
{
	QLayout* layout = this->layout();
	for (int i = 1; i < layout->count(); i++)
	{
		QWidget* widget = layout->itemAt(i)->widget();
		if (widget != nullptr)
		{
			SAFE_DELETE_OBJECT(widget);
		}
	}
}

void CW3ReportThumbnail::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
		if (event->source() == this) {
			event->setDropAction(Qt::MoveAction);
			event->accept();
		}
		else {
			event->acceptProposedAction();
		}
	}
	else {
		event->ignore();
	}
}

void CW3ReportThumbnail::dragMoveEvent(QDragMoveEvent *event)
{
	if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
		if (event->source() == this) {
			event->setDropAction(Qt::MoveAction);
			event->accept();
		}
		else {
			event->acceptProposedAction();
		}
	}
	else {
		event->ignore();
	}
}

void CW3ReportThumbnail::dropEvent(QDropEvent *event)
{
	if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
		QByteArray itemData = event->mimeData()->data("application/x-dnditemdata");
		QDataStream dataStream(&itemData, QIODevice::ReadOnly);

		QPixmap pixmap;
		QPoint offset;
		dataStream >> pixmap >> offset;

		//QLabel *newIcon = new QLabel(this);
		//newIcon->setPixmap(pixmap);
		//newIcon->move(event->pos() - offset);
		//newIcon->show();
		//newIcon->setAttribute(Qt::WA_DeleteOnClose);

		if (event->source() == this) {
			event->setDropAction(Qt::MoveAction);
			event->accept();
		}
		else {
			event->acceptProposedAction();
		}
	}
	else {
		event->ignore();
	}
}
#include <QDebug>
void CW3ReportThumbnail::mousePressEvent(QMouseEvent *event)
{
	QLabel *child = static_cast<QLabel*>(childAt(event->pos()));
	if (!child)
		return;

	if (!child->pixmap()) // v1.0.2 report tab에서 thumbnail title label 클릭 시 죽는 현상 수정
		return;

	QPixmap pixmap = *child->pixmap();
	QString imgPath = child->objectName();

	QByteArray itemData;
	QDataStream dataStream(&itemData, QIODevice::WriteOnly);
	dataStream << pixmap << QPoint(event->pos() - child->pos()) << imgPath;

	QMimeData *mimeData = new QMimeData;
	mimeData->setData("application/x-dnditemdata", itemData);

	QDrag *drag = new QDrag(this);
	drag->setMimeData(mimeData);
	drag->setPixmap(pixmap);
	drag->setHotSpot(event->pos() - child->pos());

	//QPixmap tempPixmap = pixmap;
	//QPainter painter;
	//painter.begin(&tempPixmap);
	//painter.fillRect(pixmap.rect(), QColor(127, 127, 127, 127));
	//painter.end();
	//
	//child->setPixmap(tempPixmap);

	if (drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction) == Qt::MoveAction) {
		//child->close();
	}
	else {
		child->show();
		child->setPixmap(pixmap);
	}
}

//void CW3ReportThumbnail::wheelEvent(QWheelEvent * event)
//{
//}
