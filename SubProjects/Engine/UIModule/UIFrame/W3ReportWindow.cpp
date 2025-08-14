#include "W3ReportWindow.h"

#include <QApplication>
#include <QMimeData>
#include <QFontDatabase>
#include <QInputDialog>
#include <QTextList>
#include <QFileDialog>
#include <QImageReader>
#include <QSettings>
#include <QPlainTextEdit>
#include <QMenu>
#include <QDialog>
#include <QImageWriter>
#include <QDesktopWidget>
#include <qtextstream.h>
#include <qdebug.h>

#include <Engine/Common/Common/color_dialog.h>
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/global_preferences.h"
#include "../../Resource/Resource/W3ImageHeader.h"

#include <QScrollBar>

CW3ReportWindow::CW3ReportWindow(QWidget *parent) : QFrame(parent) {
	setupUi(this);
	this->setContentsMargins(0, 0, 0, 0);
	CW3Theme* theme = CW3Theme::getInstance();
	this->setStyleSheet(theme->reportWindowStyleSheet());
	f_fontsize->setStyleSheet(theme->appQComboBoxStyleSheet());
	m_lastBlockList = 0;
	f_textedit->setTabStopWidth(40);

	connect(f_textedit, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
			this, SLOT(slotCurrentCharFormatChanged(QTextCharFormat)));
	connect(f_textedit, SIGNAL(cursorPositionChanged()),
			this, SLOT(slotCursorPositionChanged()));

	fontChanged(f_textedit->font());
	bgColorChanged(f_textedit->textColor());

	f_fgcolor->setVisible(false);
	f_bgcolor->setVisible(false);

	// undo & redo
	f_undo->setShortcut(QKeySequence::Undo);
	f_redo->setShortcut(QKeySequence::Redo);

	connect(f_textedit->document(), SIGNAL(undoAvailable(bool)),
			f_undo, SLOT(setEnabled(bool)));
	connect(f_textedit->document(), SIGNAL(redoAvailable(bool)),
			f_redo, SLOT(setEnabled(bool)));

	f_undo->setEnabled(f_textedit->document()->isUndoAvailable());
	f_redo->setEnabled(f_textedit->document()->isRedoAvailable());

	connect(f_undo, SIGNAL(clicked()), f_textedit, SLOT(undo()));
	connect(f_redo, SIGNAL(clicked()), f_textedit, SLOT(redo()));

	line->setFixedSize(1, 20);
	line_2->setFixedSize(1, 20);
	line_3->setFixedSize(1, 20);
	line_4->setFixedSize(1, 20);
	line_5->setFixedSize(1, 20);
	f_fontsize->setFixedWidth(50);

	// link

	f_link->setShortcut(Qt::CTRL + Qt::Key_L);

	connect(f_link, SIGNAL(clicked(bool)), this, SLOT(textLink(bool)));

	// bold, italic & underline
	f_bold->setShortcut(Qt::CTRL + Qt::Key_B);
	f_italic->setShortcut(Qt::CTRL + Qt::Key_I);
	f_underline->setShortcut(Qt::CTRL + Qt::Key_U);

	connect(f_bold, SIGNAL(clicked()), this, SLOT(textBold()));
	connect(f_italic, SIGNAL(clicked()), this, SLOT(textItalic()));
	connect(f_underline, SIGNAL(clicked()), this, SLOT(textUnderline()));
	connect(f_strikeout, SIGNAL(clicked()), this, SLOT(textStrikeout()));

	QAction *removeFormat = new QAction(tr("Remove character formatting"), this);
	removeFormat->setShortcut(QKeySequence("CTRL+M"));
	connect(removeFormat, SIGNAL(triggered()), this, SLOT(textRemoveFormat()));
	f_textedit->addAction(removeFormat);

	QAction *removeAllFormat = new QAction(tr("Remove all formatting"), this);
	connect(removeAllFormat, SIGNAL(triggered()), this, SLOT(textRemoveAllFormat()));
	f_textedit->addAction(removeAllFormat);

	QAction *textsource = new QAction(tr("Edit document source"), this);
	textsource->setShortcut(QKeySequence("CTRL+O"));
	connect(textsource, SIGNAL(triggered()), this, SLOT(textSource()));
	f_textedit->addAction(textsource);

	// lists
	f_list_bullet->setShortcut(Qt::CTRL + Qt::Key_Minus);
	f_list_ordered->setShortcut(Qt::CTRL + Qt::Key_Equal);

	connect(f_list_bullet, SIGNAL(clicked(bool)), this, SLOT(listBullet(bool)));
	connect(f_list_ordered, SIGNAL(clicked(bool)), this, SLOT(listOrdered(bool)));

	// indentation
	f_indent_dec->setShortcut(Qt::CTRL + Qt::Key_Comma);
	f_indent_inc->setShortcut(Qt::CTRL + Qt::Key_Period);

	connect(f_indent_inc, SIGNAL(clicked()), this, SLOT(increaseIndentation()));
	connect(f_indent_dec, SIGNAL(clicked()), this, SLOT(decreaseIndentation()));

	// text foreground color
	QPixmap pix(16, 16);
	pix.fill(QColor(Qt::black));
	f_fgcolor->setIcon(pix);
	QTextCursor cursor = f_textedit->textCursor();
	QTextCharFormat fmt = cursor.charFormat();
	fmt.setForeground(QColor(Qt::black));
	cursor.setCharFormat(fmt);
	f_textedit->setCurrentCharFormat(fmt);

	connect(f_fgcolor, SIGNAL(clicked()), this, SLOT(textFgColor()));

	// text background color
	pix.fill(QColor(Qt::white));
	f_bgcolor->setIcon(pix);

	connect(f_bgcolor, SIGNAL(clicked()), this, SLOT(textBgColor()));

	// images
	connect(f_saveHTML, SIGNAL(clicked()), this, SLOT(saveHTML()));

	QApplication::processEvents();

	// font size
	QFontDatabase db;
	foreach(int size, db.standardSizes())
		f_fontsize->addItem(QString::number(size));

	connect(f_fontsize, SIGNAL(activated(QString)),
			this, SLOT(textSize(QString)));

	m_defaultFontSize = QApplication::font().pixelSize() * 72 / QApplication::desktop()->logicalDpiY();
	f_fontsize->setCurrentIndex(f_fontsize->findText(QString::number(m_defaultFontSize)));

}

void CW3ReportWindow::reset() {
}

void CW3ReportWindow::textSource() {
	QDialog *dialog = new QDialog(this);
	QPlainTextEdit *pte = new QPlainTextEdit(dialog);
	pte->setPlainText(f_textedit->toHtml());
	QGridLayout *gl = new QGridLayout(dialog);
	gl->addWidget(pte, 0, 0, 1, 1);
	dialog->setWindowTitle(tr("Document source"));
	dialog->setMinimumWidth(400);
	dialog->setMinimumHeight(600);
	dialog->exec();

	f_textedit->setHtml(pte->toPlainText());

	delete dialog;
}

void CW3ReportWindow::textRemoveFormat() {
	QTextCharFormat fmt;
	fmt.setFontWeight(QFont::Normal);
	fmt.setFontUnderline(false);
	fmt.setFontStrikeOut(false);
	fmt.setFontItalic(false);
	fmt.setFontPointSize(9);
	//  fmt.setFontFamily     ("Helvetica");
	//  fmt.setFontStyleHint  (QFont::SansSerif);
	//  fmt.setFontFixedPitch (true);

	f_bold->setChecked(false);
	f_underline->setChecked(false);
	f_italic->setChecked(false);
	f_strikeout->setChecked(false);
	f_fontsize->setCurrentIndex(f_fontsize->findText("9"));

	//  QTextBlockFormat bfmt = cursor.blockFormat();
	//  bfmt->setIndent(0);

	fmt.clearBackground();

	mergeFormatOnWordOrSelection(fmt);
}

void CW3ReportWindow::textRemoveAllFormat() {
	f_bold->setChecked(false);
	f_underline->setChecked(false);
	f_italic->setChecked(false);
	f_strikeout->setChecked(false);
	f_fontsize->setCurrentIndex(f_fontsize->findText("9"));
	QString text = f_textedit->toPlainText();
	f_textedit->setPlainText(text);
}

void CW3ReportWindow::textBold() {
	QTextCharFormat fmt;
	fmt.setFontWeight(f_bold->isChecked() ? QFont::Bold : QFont::Normal);
	mergeFormatOnWordOrSelection(fmt);
}

void CW3ReportWindow::focusInEvent(QFocusEvent *) {
	f_textedit->setFocus(Qt::TabFocusReason);
}

void CW3ReportWindow::textUnderline() {
	QTextCharFormat fmt;
	fmt.setFontUnderline(f_underline->isChecked());
	mergeFormatOnWordOrSelection(fmt);
}

void CW3ReportWindow::textItalic() {
	QTextCharFormat fmt;
	fmt.setFontItalic(f_italic->isChecked());
	mergeFormatOnWordOrSelection(fmt);
}

void CW3ReportWindow::textStrikeout() {
	QTextCharFormat fmt;
	fmt.setFontStrikeOut(f_strikeout->isChecked());
	mergeFormatOnWordOrSelection(fmt);
}

void CW3ReportWindow::textSize(const QString &p) {
	QApplication::processEvents();
	qreal pointSize = p.toFloat();
	if (p.toFloat() > 0) {
		QTextCharFormat fmt;
		fmt.setFontPointSize(pointSize);
		mergeFormatOnWordOrSelection(fmt);
	}
}

void CW3ReportWindow::textLink(bool checked) {
	bool unlink = false;
	QTextCharFormat fmt;
	if (checked) {
		QString url = f_textedit->currentCharFormat().anchorHref();
		bool ok;
		QString newUrl = QInputDialog::getText(this, tr("Create a link"),
											   tr("Link URL:"), QLineEdit::Normal,
											   url,
											   &ok);
		if (ok) {
			fmt.setAnchor(true);
			fmt.setAnchorHref(newUrl);
			fmt.setForeground(QApplication::palette().color(QPalette::Link));
			fmt.setFontUnderline(true);
		} else {
			unlink = true;
		}
	} else {
		unlink = true;
	}
	if (unlink) {
		fmt.setAnchor(false);
		fmt.setForeground(QApplication::palette().color(QPalette::Text));
		fmt.setFontUnderline(false);
	}
	mergeFormatOnWordOrSelection(fmt);
}

void CW3ReportWindow::textStyle(int index) {
	QTextCursor cursor = f_textedit->textCursor();
	cursor.beginEditBlock();

	// standard
	if (!cursor.hasSelection()) {
		cursor.select(QTextCursor::BlockUnderCursor);
	}
	QTextCharFormat fmt;
	cursor.setCharFormat(fmt);
	f_textedit->setCurrentCharFormat(fmt);

	if (index == ParagraphHeading1
		|| index == ParagraphHeading2
		|| index == ParagraphHeading3
		|| index == ParagraphHeading4) {
		if (index == ParagraphHeading1) {
			fmt.setFontPointSize(m_fontsize_h1);
		}
		if (index == ParagraphHeading2) {
			fmt.setFontPointSize(m_fontsize_h2);
		}
		if (index == ParagraphHeading3) {
			fmt.setFontPointSize(m_fontsize_h3);
		}
		if (index == ParagraphHeading4) {
			fmt.setFontPointSize(m_fontsize_h4);
		}
		if (index == ParagraphHeading2 || index == ParagraphHeading4) {
			fmt.setFontItalic(true);
		}

		fmt.setFontWeight(QFont::Bold);
	}
	if (index == ParagraphMonospace) {
		fmt = cursor.charFormat();
		fmt.setFontFamily("Monospace");
		fmt.setFontStyleHint(QFont::Monospace);
		fmt.setFontFixedPitch(true);
	}
	cursor.setCharFormat(fmt);
	f_textedit->setCurrentCharFormat(fmt);

	cursor.endEditBlock();
}

void CW3ReportWindow::textFgColor() {
	ColorDialog color_dialog;
	color_dialog.SetCurrentColor(f_textedit->textColor());
	if (!color_dialog.exec())
	{
		return;
	}

	QColor color = color_dialog.SelectedColor();

	QTextCursor cursor = f_textedit->textCursor();
	if (!cursor.hasSelection()) {
		cursor.select(QTextCursor::WordUnderCursor);
	}
	QTextCharFormat fmt = cursor.charFormat();
	if (color.isValid()) {
		fmt.setForeground(color);
	} else {
		fmt.clearForeground();
	}
	cursor.setCharFormat(fmt);
	f_textedit->setCurrentCharFormat(fmt);
	fgColorChanged(color);
}

void CW3ReportWindow::textBgColor() {
	ColorDialog color_dialog;
	color_dialog.SetCurrentColor(f_textedit->textBackgroundColor());
	if (!color_dialog.exec())
	{
		return;
	}

	QColor color = color_dialog.SelectedColor();

	QTextCursor cursor = f_textedit->textCursor();
	if (!cursor.hasSelection()) {
		cursor.select(QTextCursor::WordUnderCursor);
	}
	QTextCharFormat fmt = cursor.charFormat();
	if (color.isValid()) {
		fmt.setBackground(color);
	} else {
		fmt.clearBackground();
	}
	cursor.setCharFormat(fmt);
	f_textedit->setCurrentCharFormat(fmt);
	bgColorChanged(color);
}

void CW3ReportWindow::listBullet(bool checked) {
	if (checked) {
		f_list_ordered->setChecked(false);
	}
	list(checked, QTextListFormat::ListDisc);
}

void CW3ReportWindow::listOrdered(bool checked) {
	if (checked) {
		f_list_bullet->setChecked(false);
	}
	list(checked, QTextListFormat::ListDecimal);
}

void CW3ReportWindow::list(bool checked, QTextListFormat::Style style) {
	QTextCursor cursor = f_textedit->textCursor();
	cursor.beginEditBlock();
	if (!checked) {
		QTextBlockFormat obfmt = cursor.blockFormat();
		QTextBlockFormat bfmt;
		bfmt.setIndent(obfmt.indent());
		cursor.setBlockFormat(bfmt);
	} else {
		QTextListFormat listFmt;
		if (cursor.currentList()) {
			listFmt = cursor.currentList()->format();
		}
		listFmt.setStyle(style);
		cursor.createList(listFmt);
	}
	cursor.endEditBlock();
}
void CW3ReportWindow::mergeFormatOnWordOrSelection(const QTextCharFormat &format) {
	QTextCursor cursor = f_textedit->textCursor();
	if (!cursor.hasSelection()) {
		cursor.select(QTextCursor::WordUnderCursor);
	}
	cursor.mergeCharFormat(format);
	f_textedit->mergeCurrentCharFormat(format);
	f_textedit->setFocus(Qt::TabFocusReason);
}

void CW3ReportWindow::slotCursorPositionChanged() {
	QTextList *l = f_textedit->textCursor().currentList();
	if (m_lastBlockList && (l == m_lastBlockList || (l != 0 && m_lastBlockList != 0
		&& l->format().style() == m_lastBlockList->format().style()))) {
		return;
	}
	m_lastBlockList = l;
	if (l) {
		QTextListFormat lfmt = l->format();
		if (lfmt.style() == QTextListFormat::ListDisc) {
			f_list_bullet->setChecked(true);
			f_list_ordered->setChecked(false);
		} else if (lfmt.style() == QTextListFormat::ListDecimal) {
			f_list_bullet->setChecked(false);
			f_list_ordered->setChecked(true);
		} else {
			f_list_bullet->setChecked(false);
			f_list_ordered->setChecked(false);
		}
	} else {
		f_list_bullet->setChecked(false);
		f_list_ordered->setChecked(false);
	}
}

void CW3ReportWindow::fontChanged(const QFont &f) {
	int fidx = f_fontsize->findText(QString::number(f.pointSize()));

	if (fidx >= 0)
		f_fontsize->setCurrentIndex(fidx);
	else
		f_fontsize->setCurrentIndex(f_fontsize->findText(QString::number(m_defaultFontSize)));

	f_bold->setChecked(f.bold());
	f_italic->setChecked(f.italic());
	f_underline->setChecked(f.underline());
	f_strikeout->setChecked(f.strikeOut());
	//if (f.pointSize() == m_fontsize_h1) {
	//    f_paragraph->setCurrentIndex(ParagraphHeading1);
	//  } else if (f.pointSize() == m_fontsize_h2) {
	//    f_paragraph->setCurrentIndex(ParagraphHeading2);
	//  } else if (f.pointSize() == m_fontsize_h3) {
	//    f_paragraph->setCurrentIndex(ParagraphHeading3);
	//  } else if (f.pointSize() == m_fontsize_h4) {
	//    f_paragraph->setCurrentIndex(ParagraphHeading4);
	//  } else {
	//    if (f.fixedPitch() && f.family() == "Monospace") {
	//        f_paragraph->setCurrentIndex(ParagraphMonospace);
	//      } else {
	//        f_paragraph->setCurrentIndex(ParagraphStandard);
	//        }
	//    }
	if (f_textedit->textCursor().currentList()) {
		QTextListFormat lfmt = f_textedit->textCursor().currentList()->format();
		if (lfmt.style() == QTextListFormat::ListDisc) {
			f_list_bullet->setChecked(true);
			f_list_ordered->setChecked(false);
		} else if (lfmt.style() == QTextListFormat::ListDecimal) {
			f_list_bullet->setChecked(false);
			f_list_ordered->setChecked(true);
		} else {
			f_list_bullet->setChecked(false);
			f_list_ordered->setChecked(false);
		}
	} else {
		f_list_bullet->setChecked(false);
		f_list_ordered->setChecked(false);
	}
}

void CW3ReportWindow::fgColorChanged(const QColor &c) {
	QPixmap pix(16, 16);
	if (c.isValid()) {
		pix.fill(c);
	} else {
		pix.fill(QColor(Qt::black));
	}
	f_fgcolor->setIcon(pix);
}

void CW3ReportWindow::bgColorChanged(const QColor &c) {
	QPixmap pix(16, 16);
	if (c.isValid()) {
		pix.fill(c);
	} else {
		pix.fill(QColor(Qt::white));
	}
	f_bgcolor->setIcon(pix);
}

void CW3ReportWindow::slotCurrentCharFormatChanged(const QTextCharFormat &format) {
	fontChanged(format.font());
	bgColorChanged((format.background().isOpaque()) ? format.background().color() : QColor());
	fgColorChanged((format.foreground().isOpaque()) ? format.foreground().color() : QColor());
	f_link->setChecked(format.isAnchor());
}

//void CW3ReportWindow::slotClipboardDataChanged() {
//#ifndef QT_NO_CLIPBOARD
//    if (const QMimeData *md = QApplication::clipboard()->mimeData())
//        f_paste->setEnabled(md->hasText());
//#endif
//}

QString CW3ReportWindow::toHtml() const {
	QString s = f_textedit->toHtml();
	// convert emails to links
	s = s.replace(QRegExp("(<[^a][^>]+>(?:<span[^>]+>)?|\\s)([a-zA-Z\\d]+@[a-zA-Z\\d]+\\.[a-zA-Z]+)"), "\\1<a href=\"mailto:\\2\">\\2</a>");
	// convert links
	s = s.replace(QRegExp("(<[^a][^>]+>(?:<span[^>]+>)?|\\s)((?:https?|ftp|file)://[^\\s'\"<>]+)"), "\\1<a href=\"\\2\">\\2</a>");
	// see also: Utils::linkify()
	return s;
}

void CW3ReportWindow::initReport(CW3ImageHeader * hd) {
	QString strPatientName = hd->getPatientName();
	QString strPatientID = hd->getPatientID();
	QString strPatientSex = hd->getPatientSex();
	QString strPatientAge = hd->getPatientAge();
	QString strStudyID = hd->getStudyID();
	QString strStudyDate = hd->getStudyDate();
	QString strDescription = hd->getDescription();

	GlobalPreferences* global_preferences = GlobalPreferences::GetInstance();
	QString folderPath = QString("%1/%2")
		.arg(global_preferences->preferences_.general.files.capture_path)
		.arg(strPatientID);

	QDir dirHandle(folderPath);
	dirHandle.setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Hidden);
	QStringList filters;
	filters << "*.jpg" << "*.png" << "*.bmp";
	dirHandle.setNameFilters(filters);

	QStringList fileList = dirHandle.entryList();

	f_thumnail->clearThumbnail();

	for (auto& elem : fileList) {
		f_thumnail->addThumbnail(folderPath + ("/") + elem);
	}

	f_textedit->setInformation(
		strPatientName,
		strPatientID,
		strPatientAge,
		strPatientSex,
		strStudyID,
		strStudyDate,
		strDescription);

	m_reportFileName = strPatientID + ("_") + strStudyID + (".html");
}

void CW3ReportWindow::addThumbnail(const QString & path) {
	f_thumnail->addThumbnail(path);
}

void CW3ReportWindow::increaseIndentation() {
	indent(+1);
}

void CW3ReportWindow::decreaseIndentation() {
	indent(-1);
}

void CW3ReportWindow::indent(int delta) {
	QTextCursor cursor = f_textedit->textCursor();
	cursor.beginEditBlock();
	QTextBlockFormat bfmt = cursor.blockFormat();
	int ind = bfmt.indent();
	if (ind + delta >= 0) {
		bfmt.setIndent(ind + delta);
	}
	cursor.setBlockFormat(bfmt);
	cursor.endEditBlock();
}

void CW3ReportWindow::setText(const QString& text) {
	if (text.isEmpty()) {
		setPlainText(text);
		return;
	}
	if (text[0] == '<') {
		setHtml(text);
	} else {
		setPlainText(text);
	}
}

//void CW3ReportWindow::saveHTML() {
//#if defined(__APPLE__)
//	QString htmlPath = QFileDialog::getSaveFileName(this, "Save HTML", "./" + m_reportFileName,
//													"htmls (*.html *.htm)",
//													nullptr,
//													QFileDialog::Option::DontUseNativeDialog);
//#else
//	QString htmlPath = QFileDialog::getSaveFileName(this, ("Save HTML"), "C:\\" + m_reportFileName,
//													"htmls (*.html *.htm)");
//#endif
//	QFile f(htmlPath);
//	f.open(QIODevice::WriteOnly);
//	// store data in f
//	QTextStream outputStream(&f);
//
//	QPlainTextEdit pte;
//	pte.setPlainText(f_textedit->toHtml());
//
//	QString code = pte.toPlainText();
//
//	int j = 0;
//	QStringList pathList;
//	QStringList nameList;
//	while ((j = code.indexOf("img src=\"", j)) != -1) {
//		int endIdx = code.indexOf("\"", j + 9);
//		QString imgPath = code.mid(j + 9, endIdx - j - 9);
//		qDebug() << "imgPath : " << imgPath;
//		QFileInfo fi(imgPath);
//		QString imgName = fi.fileName();
//		qDebug() << "imgName : " << imgName;
//
//		pathList.push_back(imgPath);
//		nameList.push_back(imgName);
//
//		QImage image = QImageReader(imgPath).read();
//		QString debug = htmlPath.left(htmlPath.lastIndexOf("/") + 1);
//		qDebug() << "debug : " << debug;
//		QImageWriter imgWriter(htmlPath.left(htmlPath.lastIndexOf("/") + 1) + imgName);
//		imgWriter.write(image);
//
//		++j;
//	}
//
//	for (int i = 0; i < pathList.size(); i++) {
//		code.replace(pathList[i], nameList[i]);
//	}
//
//	outputStream << code;
//	f.close();
//}



void CW3ReportWindow::saveHTML() {
#if defined(__APPLE__)
	QString htmlPath = QFileDialog::getSaveFileName(this, "Save HTML", "./" + m_reportFileName,
		"HTML Files (*.html *.htm)",
		nullptr,
		QFileDialog::Option::DontUseNativeDialog);
#else
	QString htmlPath = QFileDialog::getSaveFileName(this, "Save HTML", "C:\\" + m_reportFileName,
		"HTML Files (*.html *.htm)");
#endif
	if (htmlPath.isEmpty()) {
		return;
	}

	QFile f(htmlPath);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qDebug() << "Failed to open file for writing:" << htmlPath;
		return;
	}

	QTextStream outputStream(&f);
	outputStream.setCodec("UTF-8");

	QFileInfo fileInfo(htmlPath);
	QString saveDir = fileInfo.absolutePath() + "/" + m_reportFileName + "/";
	QDir dir;
	//if (!dir.exists(saveDir)) {
	//	dir.mkpath(saveDir);
	//}
	if (dir.exists()) {
		dir.removeRecursively();
	}
	dir.mkpath(saveDir);

	f_textedit->updatePages();
	QCoreApplication::processEvents();

	outputStream << "<html>\n<head>\n<meta charset='UTF-8'>\n<title>Report</title>\n";
	outputStream << "<style>\n"
		"@media print {\n"
		"  .page { page-break-after: always; }\n"
		"}\n"
		"</style>\n";
	outputStream << "</head>\n<body>\n";

	for (int pageIndex = 0; pageIndex < f_textedit->pages.size(); ++pageIndex) {
		QString pageHtml = f_textedit->pages[pageIndex];
		int j = 0;
		while ((j = pageHtml.indexOf("img src=\"", j)) != -1) {
			int endIdx = pageHtml.indexOf("\"", j + 9);
			QString imgPath = pageHtml.mid(j + 9, endIdx - j - 9);

			QFileInfo fi(imgPath);
			QString imgName = fi.fileName();
			QString imgSavePath = saveDir + imgName;

			QImage image = QImageReader(imgPath).read();
			if (image.isNull()) {
				j += 9;
				continue;
			}

			QImageWriter imgWriter(imgSavePath);
			if (!imgWriter.write(image)) {
				j += 9;
				continue;
			}
			pageHtml.replace(imgPath, "Reports/" + imgName);
			++j;
		}

		outputStream << "<div class='page'>\n" << pageHtml << "\n</div>\n";
	}

	outputStream << "</body>\n</html>";
	f.close();

	for (int pageIndex = 0; pageIndex < f_textedit->pages.size(); ++pageIndex) {
		QString pageHtml = f_textedit->pages[pageIndex];

		QString reportFilePath = saveDir + QString("report%1.html").arg(pageIndex + 1);
		QFile reportFile(reportFilePath);
		if (reportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
			QTextStream reportStream(&reportFile);
			reportStream.setCodec("UTF-8");
			reportStream << "<html>\n<head>\n<meta charset='UTF-8'>\n<title>Page " << (pageIndex + 1) << "</title>\n";
			reportStream << "<style>\n"
				"@media print {\n"
				"  .page { page-break-after: always; }\n"
				"}\n"
				"</style>\n";
			reportStream << "</head>\n<body>\n";
			reportStream << "<div class='page'>\n" << pageHtml << "\n</div>\n";
			reportStream << "</body>\n</html>";
			reportFile.close();
		}
	}
}




