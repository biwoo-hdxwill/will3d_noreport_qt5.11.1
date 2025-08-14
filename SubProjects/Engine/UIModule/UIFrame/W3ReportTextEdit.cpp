#include "W3ReportTextEdit.h"

#include <stdlib.h>
#include <iostream>
#include <exception>

#include <QTextCursor>
#include <QImage>
#include <QByteArray>
#include <QBuffer>
#include <QDropEvent>
#include <QTextStream>
#include <QMimeData>
#include <QSignalMapper>
//=======
//20250124 LIN
#include <QDebug>
#include <QDir>
#include <QRegularExpression>
#include "../../Resource/Resource/W3ImageHeader.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/ResContainer/resource_container.h"
//=======

using std::runtime_error;
using std::cout;
using std::endl;

CW3ReportTextEdit::CW3ReportTextEdit(QWidget *parent)
	: QTextEdit(parent) {
	setAcceptDrops(true);
	setAcceptRichText(true);

  selected_layout_url_ = ":/html/2by2_Images.htm";
	QFile file(selected_layout_url_);
	file.open(QFile::ReadOnly | QFile::Text);

	QTextStream readHTML(&file);
	QString html = readHTML.readAll();
	//this->setText(html);
	this->setHtml(html);
  file.close();

	QPalette palette;
	palette.setColor(QPalette::Text, QColor("#FF000000"));
	this->setPalette(palette);
	this->document()->setDefaultStyleSheet(
		"body {"
		"background: #d0d0d0;"
		"}");

	//20250124 LIN
	//footer 추가 page표시
	//currentPageIndex 초기화 0이라서 -> pages에 첫번째 내용, 따로 처리 필요없음 
	pages.append(html);
	int totalPages = pages.size();
	QString footer = QString(
		"<p style='margin-top:0px; text-align: center;'>"
		"<span style='font-size:12px; color:#000000;'>Report %1 of %2</span>"
		"</p>"
	).arg(pages.size()).arg(totalPages);
	pages[currentPageIndex] = html + footer;
	showPage(currentPageIndex, false);
	//=====
}

void CW3ReportTextEdit::setHtmlPage()
{
  QFile file(selected_layout_url_);
  file.open(QFile::ReadOnly | QFile::Text);

  QTextStream readHTML(&file);
  QString html = readHTML.readAll();
  //this->setText(html);
  this->setHtml(html);
  file.close();

  QPalette palette;
  palette.setColor(QPalette::Text, QColor("#FF000000"));
  this->setPalette(palette);
  this->document()->setDefaultStyleSheet(
    "body {"
    "background: #d0d0d0;"
    "}");

  int totalPages = pages.size();
  QString footer = QString(
    "<p style='margin-top:0px; text-align: center;'>"
    "<span style='font-size:12px; color:#000000;'>Report %1 of %2</span>"
    "</p>"
  ).arg(pages.size()).arg(totalPages);
  pages[currentPageIndex] = html + footer;
  showPage(currentPageIndex, true);


}

void CW3ReportTextEdit::dragEnterEvent(QDragEnterEvent *event) {
	if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
		if (event->source() == this) {
			event->setDropAction(Qt::MoveAction);
			event->accept();
		} else {
			event->acceptProposedAction();
		}
	} else {
		event->ignore();
	}
}

void CW3ReportTextEdit::dragMoveEvent(QDragMoveEvent *event) {
	if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
		QTextCursor cursor = cursorForPosition(event->pos());
		cursor.position();
		this->setTextCursor(cursor);

		if (event->source() == this) {
			event->setDropAction(Qt::MoveAction);
			event->accept();
		} else {
			event->acceptProposedAction();
		}
	} else {
		event->ignore();
	}
}
void CW3ReportTextEdit::dropEvent(QDropEvent * event) {
	try {
		if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
			QByteArray itemData = event->mimeData()->data("application/x-dnditemdata");
			QDataStream dataStream(&itemData, QIODevice::ReadOnly);

			QPixmap pixmap;
			QPoint offset;
			QString imgPath;

			dataStream >> pixmap >> offset >> imgPath;

			QImage img;
			if (!img.load(imgPath))
				throw runtime_error("failed to load image.");

			QString format;
			format = (imgPath.right(3)).toUpper();
			if (format != "BMP" &&
				format != "PNG" &&
				format != "JPG")
				throw runtime_error("image format isn't supported.");

			//img = img.scaled(340, img.height(), Qt::KeepAspectRatio);

			dropImage(imgPath, img.width(), img.height(), format);

			//QLabel *newIcon = new QLabel(this);
			//newIcon->setPixmap(pixmap);
			//newIcon->move(event->pos() - offset);
			//newIcon->show();
			//newIcon->setAttribute(Qt::WA_DeleteOnClose);

			if (event->source() == this) {
				event->setDropAction(Qt::MoveAction);
				event->accept();
			} else {
				event->acceptProposedAction();
			}
		} else {
			event->ignore();
		}
	} catch (const runtime_error& e) {
		cout << "CW3ReportTextEdit::dropEvent: " << e.what() << endl;
	}
}

bool CW3ReportTextEdit::canInsertFromMimeData(const QMimeData *source) const {
	return source->hasImage() || QTextEdit::canInsertFromMimeData(source);
}

void CW3ReportTextEdit::insertFromMimeData(const QMimeData *source) {
	if (source->hasImage()) {
		QStringList formats = source->formats();
		QString format;
		for (int i = 0; i < formats.size(); i++) {
			if (formats[i] == "image/bmp") { format = "BMP";  break; }
			if (formats[i] == "image/jpeg") { format = "JPG";  break; }
			if (formats[i] == "image/jpg") { format = "JPG";  break; }
			if (formats[i] == "image/gif") { format = "GIF";  break; }
			if (formats[i] == "image/png") { format = "PNG";  break; }
			if (formats[i] == "image/pbm") { format = "PBM";  break; }
			if (formats[i] == "image/pgm") { format = "PGM";  break; }
			if (formats[i] == "image/ppm") { format = "PPM";  break; }
			if (formats[i] == "image/tiff") { format = "TIFF"; break; }
			if (formats[i] == "image/xbm") { format = "XBM";  break; }
			if (formats[i] == "image/xpm") { format = "XPM";  break; }
		}
		if (!format.isEmpty()) {
			//          dropImage(qvariant_cast<QImage>(source->imageData()), format);
			//            dropImage(qvariant_cast<QImage>(source->imageData()), "JPG");
			return;
		}
	}
	QTextEdit::insertFromMimeData(source);
}

QMimeData *CW3ReportTextEdit::createMimeDataFromSelection() const {
	return QTextEdit::createMimeDataFromSelection();
}

void CW3ReportTextEdit::dropImage(const QString& filePath, int imgWidth, int imgHeight, const QString& format) {
	////QByteArray bytes;
	////QBuffer buffer(&bytes);
	////buffer.open(QIODevice::WriteOnly);
	////image.save(&buffer, format.toLocal8Bit().data());
	////buffer.close();
	////QByteArray base64 = bytes.toBase64();
	////QByteArray base64l;
	////for (int i=0; i<base64.size(); i++) {
	////    base64l.append(base64[i]);
	////    if (i%80 == 0) {
	////        base64l.append("\n");
	////        }
	////    }

	//float rectRatio = 340.0f / imgHeight;
	//float imgRatio = static_cast<float>(imgWidth) / (imgHeight);
	//int width, height;

	//if (imgRatio < rectRatio) {
	//	width = imgWidth;
	//	height = imgHeight;
	//} else {
	//	width = 340.0f;
	//	height = static_cast<int>(340.0f / imgRatio);
	//}

	//QTextCursor cursor = textCursor();
	//cursor.insertHtml(QString(R"(<IMG src= "%1" width=%2 height=%3>)").arg(filePath).arg(width).arg(height));
	////QTextImageFormat imageFormat;
	////imageFormat.setWidth  ( image.width() );
	////imageFormat.setHeight ( image.height() );
	////imageFormat.setName   ( QString("data:image/%1;base64,%2")
	////                            .arg(QString("%1.%2").arg(rand()).arg(format))
	////                            .arg(base64l.data())
	////                            );
	////cursor.insertImage    ( imageFormat );

	//20250124 img data html에 넣기 위해 
	QImage img(filePath);
	if (img.isNull()) {
		return;
	}

	QByteArray bytes;
	QBuffer buffer(&bytes);
	buffer.open(QIODevice::WriteOnly);
	img.save(&buffer, format.toUtf8().constData());
	QString base64Data = QString::fromLatin1(bytes.toBase64());

	float rectRatio = 340.0f / imgHeight;
	float imgRatio = static_cast<float>(imgWidth) / imgHeight;
	int width, height;

	if (imgRatio < rectRatio) {
		width = imgWidth;
		height = imgHeight;
	}
	else {
		width = 340.0f;
		height = static_cast<int>(340.0f / imgRatio);
	}

	QTextCursor cursor = textCursor();
	cursor.insertHtml(QString(
		R"(<img src="data:image/%1;base64,%2" width="%3" height="%4"/>)")
		.arg(format.toLower())
		.arg(base64Data)
		.arg(width)
		.arg(height)
	);
}

void CW3ReportTextEdit::setInformation(
	const QString& patientName,
	const QString& patientID,
	const QString& patientAge,
	const QString& patientSex,
	const QString& studyID,
	const QString& studyDate,
	const QString& description) {
	QString html = this->toHtml();

	html.replace("cpn", patientName);
	html.replace("cpi", patientID);
	html.replace("cas", patientAge + (" / ") + patientSex);
	html.replace("csi", studyID);
	html.replace("csd", studyDate);
	html.replace("cd", description);

	this->setHtml(html);
}

#include <QMenu>
#include <QScrollBar>
void CW3ReportTextEdit::contextMenuEvent(QContextMenuEvent *event) {
	QMenu *menu = this->createStandardContextMenu();

	menu->addSeparator();
	QAction *addPageAction = menu->addAction("Add Page");
	QAction *deletePageAction = menu->addAction("Delete Page");
	QAction *previousPageAction = menu->addAction("Previous Page");
	QAction *nextPageAction = menu->addAction("Next Page");

  QMenu *menu_layout = new QMenu("Layout", this);

  QAction *layout_1by1 = menu_layout->addAction("1X1");
  QAction *layout_2by2 = menu_layout->addAction("2X2");
  QAction *layout_3by3 = menu_layout->addAction("3X3");

  menu->addMenu(menu_layout);
	connect(addPageAction, SIGNAL(triggered()), this, SLOT(addPage()));
	connect(deletePageAction, SIGNAL(triggered()),  this, SLOT(deletePage()));
	connect(previousPageAction, SIGNAL(triggered()) , this, SLOT(previousPage()));
	connect(nextPageAction, SIGNAL(triggered()),  this, SLOT(nextPage()));

  
  QSignalMapper* mapper = new QSignalMapper(this);
  QList<QAction*> layout_actions;
  layout_actions.push_back(layout_1by1);
  layout_actions.push_back(layout_2by2);
  layout_actions.push_back(layout_3by3);

  for (int i = 0; i < layout_actions.count(); i++)
  {
    connect(layout_actions[i], SIGNAL(triggered()), mapper, SLOT(map()));
    mapper->setMapping(layout_actions[i], i);
  }
  connect(mapper, SIGNAL(mapped(int)), this, SLOT(setImageLayout(int)));

	menu->exec(event->globalPos());
	delete menu;

}

void CW3ReportTextEdit::showPage(int index, bool isUpdatePatientInfo) {
	if (index >= 0 && index < pages.size()) {

		//if (currentPageIndex >= 0 && currentPageIndex < pages.size()) {
		//	pages[currentPageIndex] = this->toHtml();
		//}
		updatePageIndexes();
		this->clear();
		this->setHtml(pages[index]);


		//이미 갱신된 상태에서 다시 환자정보 update하면은 image손훼됨 -> page 추가할 때만 isUpdatePatientInfo true로 설정
		if (isUpdatePatientInfo)
		{
			//환자 정보 추가
			CW3ImageHeader* hd = ResourceContainer::GetInstance()->GetMainVolume().getHeader();
			QString strPatientName = hd->getPatientName();
			QString strPatientID = hd->getPatientID();
			QString strPatientSex = hd->getPatientSex();
			QString strPatientAge = hd->getPatientAge();
			QString strStudyID = hd->getStudyID();
			QString strStudyDate = hd->getStudyDate();
			QString strDescription = hd->getDescription();

			this->setInformation(
				strPatientName,
				strPatientID,
				strPatientAge,
				strPatientSex,
				strStudyID,
				strStudyDate,
				strDescription);

		}

		currentPageIndex = index;
	}
}

void CW3ReportTextEdit::updatePageIndexes() {
	for (int i = 0; i < pages.size(); ++i) {
		QString html = pages[i];

		// QRegularExpression 통해서 footer 형식을 matching <div>말고 <span>이다.
		QRegularExpression footerRegex(
			R"(<span\s+style\s*=\s*['"]\s*font-size:\s*12px;\s*color:\s*#000000;\s*['"]>\s*Report\s+\d+\s+of\s+\d+\s*</span>)",
			QRegularExpression::DotMatchesEverythingOption
		);

		html = html.remove(footerRegex);

		QString footer = QString(
			"<p style='margin-top:0px; text-align: center;'>"
			"<span style='font-size:12px; color:#000000;'>Report %1 of %2</span>"
			"</p>"
		).arg(i + 1).arg(pages.size());
		
		if (!footerRegex.match(html).hasMatch()) {
			pages[i] = html + footer;
		}
		else {
			pages[i] = html;
		}
	}
}

void CW3ReportTextEdit::addPage()
{
	if (currentPageIndex >= 0 && currentPageIndex < pages.size()) {
		pages[currentPageIndex] = this->toHtml();
	}

	QFile file(selected_layout_url_);
	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		return;
	}

	QTextStream readHTML(&file);
	QString newPageHtml = readHTML.readAll();

	pages.append(newPageHtml);
	currentPageIndex = pages.size() - 1 ;
	showPage(currentPageIndex, true);
}


void CW3ReportTextEdit::deletePage()
{
	if (pages.size() > 1) {
		pages.removeAt(currentPageIndex);
		//currentPageIndex = qMin(currentPageIndex, pages.size() - 1);
		//실제 showpage일 때 pagesize기준으로 다시 currentPageIndex update함
		currentPageIndex = qMax(0, qMin(currentPageIndex, pages.size() - 1));
		showPage(currentPageIndex, false);
	}
}

void CW3ReportTextEdit::previousPage()
{
	if (currentPageIndex > 0) {	
		pages[currentPageIndex] = this->toHtml();

		currentPageIndex--;
		showPage(currentPageIndex, false);
	}
}

void CW3ReportTextEdit::nextPage()
{
	if (currentPageIndex < pages.size() - 1) {
		pages[currentPageIndex] = this->toHtml();

		currentPageIndex++;
		showPage(currentPageIndex, false);
	}
}

void CW3ReportTextEdit::setImageLayout(int layout_index)
{

  QString image_layout_html_url;

  switch (layout_index)
  {
  case LAYOUT_1BY1:
    image_layout_html_url = ":/html/1by1_Images.htm";
    break;
  case LAYOUT_2BY2:
    image_layout_html_url = ":/html/2by2_Images.htm";
    break;
  case LAYOUT_3BY3:
    image_layout_html_url = ":/html/3by3_Images.htm";
    break;
  default:
    break;
  }

  selected_layout_url_ = image_layout_html_url;
  setHtmlPage();
}



void CW3ReportTextEdit::wheelEvent(QWheelEvent *event)
{
	QScrollBar *scrollBar = this->verticalScrollBar();

	bool atTop = (scrollBar->value() == scrollBar->minimum());
	bool atBottom = (scrollBar->value() == scrollBar->maximum());

	if (event->angleDelta().y() > 0) {
		if (atTop) {
			previousPage();
		}
		else {
			QTextEdit::wheelEvent(event);
		}
	}
	else if (event->angleDelta().y() < 0) { 
		if (atBottom) {
			nextPage();
		}
		else {
			QTextEdit::wheelEvent(event);
		}
	}
	else {
		QTextEdit::wheelEvent(event);
	}
}

void CW3ReportTextEdit::updatePages() {
	if (currentPageIndex >= 0 && currentPageIndex < pages.size()) {
		QString currentHtml = this->toHtml();
		pages[currentPageIndex] = currentHtml;
	}
}
