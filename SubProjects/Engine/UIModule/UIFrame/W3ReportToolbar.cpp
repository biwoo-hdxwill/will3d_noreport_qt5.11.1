#include "W3ReportToolbar.h"

#include <QPrintPreviewDialog>
#include <QMainWindow>
#include <QTextEdit>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QToolBar>
#include <QApplication>
#include <QComboBox>
#include <QFontDatabase>
#include <QClipboard>
#include <QFile>
#include <QTextCodec>
#include <QByteArray>
#include <QFileDialog>
#include <QTextDocumentWriter>
#include <QPrinter>
#include <QPrintDialog>
#include <QMimeData>
#include <QTextList>

#include <Engine/Common/Common/color_dialog.h>
#include "../../Common/Common/language_pack.h"

//textedit를 만들고 전달해주면서 생성해야한다!!
CW3ReportToolbar::CW3ReportToolbar(QMainWindow *pParentWindow, QTextEdit *pTextEdit) {
	m_pParent = pParentWindow;
	m_pTextEdit = pTextEdit;
	m_strImgDir = QString(":/image/report/");

	m_pParent->setToolButtonStyle(Qt::ToolButtonFollowStyle);
	setupFileActions();
	setupEditActions();
	setupTextActions();

	m_pMenuHelp = new QMenu(lang::LanguagePack::txt_help(), m_pParent);
	m_pParent->menuBar()->addMenu(m_pMenuHelp);

	connect(m_pTextEdit, SIGNAL(currentCharFormatChanged(QTextCharFormat)), this, SLOT(currentCharFormatChanged(QTextCharFormat)));
	connect(m_pTextEdit, SIGNAL(cursorPositionChanged()), this, SLOT(cursorPositionChanged()));

	fontChanged(m_pTextEdit->font());
	colorChanged(m_pTextEdit->textColor());
	alignmentChanged(m_pTextEdit->alignment());

	connect(m_pTextEdit->document(), SIGNAL(modificationChanged(bool)), m_pActionSave, SLOT(setEnabled(bool)));
	connect(m_pTextEdit->document(), SIGNAL(modificationChanged(bool)), m_pParent, SLOT(setWindowModified(bool)));
	connect(m_pTextEdit->document(), SIGNAL(undoAvailable(bool)), m_pActionUndo, SLOT(setEnabled(bool)));
	connect(m_pTextEdit->document(), SIGNAL(redoAvailable(bool)), m_pActionRedo, SLOT(setEnabled(bool)));

	m_pParent->setWindowModified(m_pTextEdit->document()->isModified());
	m_pActionSave->setEnabled(m_pTextEdit->document()->isModified());
	m_pActionUndo->setEnabled(m_pTextEdit->document()->isUndoAvailable());
	m_pActionRedo->setEnabled(m_pTextEdit->document()->isRedoAvailable());

	connect(m_pActionUndo, SIGNAL(triggered()), m_pTextEdit, SLOT(undo()));
	connect(m_pActionRedo, SIGNAL(triggered()), m_pTextEdit, SLOT(redo()));

	m_pActionCut->setEnabled(false);
	m_pActionPaste->setEnabled(false);

	connect(m_pActionCut, SIGNAL(triggered()), m_pTextEdit, SLOT(cut()));
	connect(m_pActionCopy, SIGNAL(triggered()), m_pTextEdit, SLOT(copy()));
	connect(m_pActionPaste, SIGNAL(triggered()), m_pTextEdit, SLOT(paste()));

	connect(m_pTextEdit, SIGNAL(copyAvailable(bool)), m_pActionCut, SLOT(setEnabled(bool)));
	connect(m_pTextEdit, SIGNAL(copyAvailable(bool)), m_pActionCopy, SLOT(setEnabled(bool)));

	connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardDataChanged()));

	m_initialFile = ":/html/template_1x1.html";
	if (!load(m_initialFile))
		fileNew();
}

CW3ReportToolbar::~CW3ReportToolbar(void) {
}

void CW3ReportToolbar::setupFileActions() {
	m_pToolbarFile = new QToolBar(m_pParent);
	m_pToolbarFile->setWindowTitle(tr("File Actions"));
	m_pParent->addToolBar(m_pToolbarFile);

	m_pMenuFile = new QMenu(tr("&File"), m_pParent);
	m_pParent->menuBar()->addMenu(m_pMenuFile);

	QAction *a = new QAction(QIcon::fromTheme("document-new", QIcon(m_strImgDir + "filenew.png")), tr("&New"), m_pParent);
	a->setPriority(QAction::LowPriority);
	a->setShortcut(QKeySequence::New);
	connect(a, SIGNAL(triggered()), this, SLOT(fileNew()));
	m_pToolbarFile->addAction(a);
	m_pMenuFile->addAction(a);

	a = new QAction(QIcon::fromTheme("document-open", QIcon(m_strImgDir + "fileopen.png")), tr("&Open..."), m_pParent);
	a->setShortcut(QKeySequence::Open);
	connect(a, SIGNAL(triggered()), this, SLOT(fileOpen()));
	m_pToolbarFile->addAction(a);
	m_pMenuFile->addAction(a);

	m_pActionSave = a = new QAction(QIcon::fromTheme("document-save", QIcon(m_strImgDir + "filesave.png")), tr("&Save"), m_pParent);
	a->setShortcut(QKeySequence::Save);
	connect(a, SIGNAL(triggered()), this, SLOT(fileSave()));
	m_pToolbarFile->addAction(a);
	m_pMenuFile->addAction(a);

	a = new QAction(tr("Save &As..."), this);
	a->setShortcut(QKeySequence::SaveAs);
	connect(a, SIGNAL(triggered()), this, SLOT(fileSaveAs()));
	m_pMenuFile->addAction(a);
	m_pMenuFile->addSeparator();

	a = new QAction(QIcon::fromTheme("document-print", QIcon(m_strImgDir + "fileprint.png")), tr("Print..."), m_pParent);
	a->setPriority(QAction::LowPriority);
	a->setShortcut(QKeySequence::Print);
	connect(a, SIGNAL(triggered()), this, SLOT(filePrint()));
	m_pToolbarFile->addAction(a);
	m_pMenuFile->addAction(a);

	a = new QAction(QIcon::fromTheme("fileprint", QIcon(m_strImgDir + "fileprint.png")), tr("Print Preview..."), m_pParent);
	connect(a, SIGNAL(triggered()), this, SLOT(filePrintPreview()));
	m_pMenuFile->addAction(a);

	a = new QAction(QIcon::fromTheme("exportpdf", QIcon(m_strImgDir + "exportpdf.png")), tr("&Exprot PDF..."), m_pParent);
	a->setPriority(QAction::LowPriority);
	a->setShortcut(QKeySequence::Print);
	connect(a, SIGNAL(triggered()), this, SLOT(filePrintPdf()));
	m_pToolbarFile->addAction(a);
	m_pMenuFile->addAction(a);
	m_pMenuFile->addSeparator();

	a = new QAction(QIcon::fromTheme("Test", QIcon(m_strImgDir + "exportpdf.png")), tr("&Test..."), m_pParent);
	a->setPriority(QAction::LowPriority);
	a->setShortcut(QKeySequence::Print);
	connect(a, SIGNAL(triggered()), this, SLOT(test()));
	m_pToolbarFile->addAction(a);
}

void CW3ReportToolbar::setupEditActions() {
	m_pToolbarEdit = new QToolBar(this);
	m_pToolbarEdit->setWindowTitle(tr("Edit Actions"));
	m_pParent->addToolBar(m_pToolbarEdit);
	m_pMenuEdit = new QMenu(tr("&Edit"), m_pParent);
	m_pParent->menuBar()->addMenu(m_pMenuEdit);

	m_pActionUndo = new QAction(QIcon::fromTheme("edit-undo", QIcon(m_strImgDir + "editundo.png")), tr("&Undo"), m_pParent);
	m_pActionUndo->setShortcut(QKeySequence::Undo);
	m_pToolbarEdit->addAction(m_pActionUndo);
	m_pMenuEdit->addAction(m_pActionUndo);

	m_pActionRedo = new QAction(QIcon::fromTheme("edit-redo", QIcon(m_strImgDir + "editredo.png")), tr("&Redo"), m_pParent);
	m_pActionRedo->setPriority(QAction::LowPriority);
	m_pActionRedo->setShortcut(QKeySequence::Redo);
	m_pToolbarEdit->addAction(m_pActionRedo);
	m_pMenuEdit->addAction(m_pActionRedo);
	m_pMenuEdit->addSeparator();

	m_pActionCut = new QAction(QIcon::fromTheme("edit-cut", QIcon(m_strImgDir + "editcut.png")), tr("Cu&t"), m_pParent);
	m_pActionCut->setPriority(QAction::LowPriority);
	m_pActionCut->setShortcut(QKeySequence::Cut);
	m_pToolbarEdit->addAction(m_pActionCut);
	m_pMenuEdit->addAction(m_pActionCut);

	m_pActionCopy = new QAction(QIcon::fromTheme("edit-copy", QIcon(m_strImgDir + "editcopy.png")), tr("&Copy"), m_pParent);
	m_pActionCopy->setPriority(QAction::LowPriority);
	m_pActionCopy->setShortcut(QKeySequence::Copy);
	m_pToolbarEdit->addAction(m_pActionCopy);
	m_pMenuEdit->addAction(m_pActionCopy);

	m_pActionPaste = new QAction(QIcon::fromTheme("edit-paste", QIcon(m_strImgDir + "editpaste.png")), tr("&PAste"), m_pParent);
	m_pActionPaste->setPriority(QAction::LowPriority);
	m_pActionPaste->setShortcut(QKeySequence::Paste);
	m_pToolbarEdit->addAction(m_pActionPaste);
	m_pMenuEdit->addAction(m_pActionPaste);

	if (const QMimeData *md = QApplication::clipboard()->mimeData())
		m_pActionPaste->setEnabled(md->hasText());
}

void CW3ReportToolbar::setupTextActions() {
	m_pToolBarText = new QToolBar(this);
	m_pToolBarText->setWindowTitle(tr("Format Actions"));
	m_pParent->addToolBar(m_pToolBarText);

	m_pMenuText = new QMenu(tr("F&ormat"), m_pParent);
	m_pParent->menuBar()->addMenu(m_pMenuText);

	QFont bold;
	bold.setBold(true);

	QFont italic;
	italic.setItalic(true);

	QFont underline;
	underline.setUnderline(true);

	m_pActionTextBold = new QAction(QIcon::fromTheme("format-text-bold", QIcon(m_strImgDir + "textbold.png")), tr("&Bold"), m_pParent);
	m_pActionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
	m_pActionTextBold->setPriority(QAction::LowPriority);
	m_pActionTextBold->setFont(bold);
	connect(m_pActionTextBold, SIGNAL(triggered()), this, SLOT(textBold()));
	m_pToolBarText->addAction(m_pActionTextBold);
	m_pMenuText->addAction(m_pActionTextBold);
	m_pActionTextBold->setCheckable(true);

	m_pActionTextItalic = new QAction(QIcon::fromTheme("format-text-italic", QIcon(m_strImgDir + "textitalic.png")), tr("&Italic"), m_pParent);
	m_pActionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
	m_pActionTextItalic->setPriority(QAction::LowPriority);
	m_pActionTextItalic->setFont(italic);
	connect(m_pActionTextItalic, SIGNAL(triggered()), this, SLOT(textItalic()));
	m_pToolBarText->addAction(m_pActionTextItalic);
	m_pMenuText->addAction(m_pActionTextItalic);
	m_pActionTextItalic->setCheckable(true);

	m_pActionTextUnderline = new QAction(QIcon::fromTheme("format-text-underline", QIcon(m_strImgDir + "textunder.png")), tr("&Underline"), m_pParent);
	m_pActionTextUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
	m_pActionTextUnderline->setPriority(QAction::LowPriority);
	m_pActionTextUnderline->setFont(underline);
	connect(m_pActionTextUnderline, SIGNAL(triggered()), this, SLOT(textUnderline()));
	m_pToolBarText->addAction(m_pActionTextUnderline);
	m_pMenuText->addAction(m_pActionTextUnderline);
	m_pActionTextUnderline->setCheckable(true);

	m_pMenuText->addSeparator();
	//////////////////////////////////////////////////////////////////////////

	QActionGroup *grp = new QActionGroup(m_pParent);
	connect(grp, SIGNAL(triggered(QAction*)), this, SLOT(textAlign(QAction*)));

	m_pActionAlignLeft = new QAction(QIcon::fromTheme("format-justify-left", QIcon(m_strImgDir + "textleft.png")), tr("&Left"), grp);
	m_pActionAlignRight = new QAction(QIcon::fromTheme("format-justify-right", QIcon(m_strImgDir + "textright.png")), tr("&Right"), grp);
	m_pActionAlignCenter = new QAction(QIcon::fromTheme("format-justify-middle", QIcon(m_strImgDir + "textcenter.png")), tr("C&enter"), grp);
	m_pActionAlignJustify = new QAction(QIcon::fromTheme("format-justify-fill", QIcon(m_strImgDir + "textjustify.png")), tr("&Justify"), grp);
	m_pActionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
	m_pActionAlignRight->setShortcut(Qt::CTRL + Qt::Key_R);
	m_pActionAlignCenter->setShortcut(Qt::CTRL + Qt::Key_E);
	m_pActionAlignJustify->setShortcut(Qt::CTRL + Qt::Key_J);
	m_pActionAlignLeft->setCheckable(true);
	m_pActionAlignRight->setCheckable(true);
	m_pActionAlignCenter->setCheckable(true);
	m_pActionAlignJustify->setCheckable(true);
	m_pActionAlignLeft->setPriority(QAction::LowPriority);
	m_pActionAlignRight->setPriority(QAction::LowPriority);
	m_pActionAlignCenter->setPriority(QAction::LowPriority);
	m_pActionAlignJustify->setPriority(QAction::LowPriority);

	m_pToolBarText->addActions(grp->actions());
	m_pMenuText->addActions(grp->actions());

	m_pMenuText->addSeparator();
	//////////////////////////////////////////////////////////////////////////

	QPixmap pix(16, 16);
	pix.fill(Qt::black);
	m_pActionTextColor = new QAction(pix, tr("&Color..."), this);
	connect(m_pActionTextColor, SIGNAL(triggered()), this, SLOT(textColor()));
	m_pToolBarText->addAction(m_pActionTextColor);
	m_pMenuText->addAction(m_pActionTextColor);

	//////////////////////////////////////////////////////////////////////////
	m_pToolbarTextFormat = new QToolBar(m_pParent);
	m_pToolbarTextFormat->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
	m_pToolbarTextFormat->setWindowTitle(tr("Format Actions"));
	m_pParent->addToolBarBreak(Qt::TopToolBarArea);
	m_pParent->addToolBar(m_pToolbarTextFormat);

	m_pComboStyle = new QComboBox(m_pToolbarTextFormat);
	m_pToolbarTextFormat->addWidget(m_pComboStyle);
	m_pComboStyle->addItem("Standard");
	m_pComboStyle->addItem("Bullet List(Disc)");
	m_pComboStyle->addItem("Bullet List(Circle)");
	m_pComboStyle->addItem("Bullet List(Square)");
	m_pComboStyle->addItem("Ordered List (Decimal)");
	m_pComboStyle->addItem("Ordered List (Alpha lower)");
	m_pComboStyle->addItem("Ordered List (Alpha upper)");
	m_pComboStyle->addItem("Ordered List (Roman lower)");
	m_pComboStyle->addItem("Ordered List (Roman upper)");
	connect(m_pComboStyle, SIGNAL(activated(int)), this, SLOT(textStyle(int)));

	m_pComboImageLayout = new QComboBox(m_pToolbarTextFormat);
	m_pToolbarTextFormat->addWidget(m_pComboImageLayout);
	m_pComboImageLayout->addItem("1x1 Layout");
	m_pComboImageLayout->addItem("1x2 Layout");
	m_pComboImageLayout->addItem("1x3 Layout");
	m_pComboImageLayout->addItem("2x1 Layout");
	m_pComboImageLayout->addItem("2x2 Layout");
	m_pComboImageLayout->addItem("2x3 Layout");
	m_pComboImageLayout->addItem("3x1 Layout");
	m_pComboImageLayout->addItem("3x2 Layout");
	m_pComboImageLayout->addItem("3x3 Layout");
	connect(m_pComboImageLayout, SIGNAL(activated(int)), this, SLOT(ImageCellLayout(int)));

	m_pComboFont = new QComboBox(m_pToolbarTextFormat);
	m_pToolbarTextFormat->addWidget(m_pComboFont);
	connect(m_pComboFont, SIGNAL(activated(QString)), this, SLOT(textFamily(QString)));

	m_pComboSize = new QComboBox(m_pToolbarTextFormat);
	m_pComboSize->setObjectName("comboSize");
	m_pToolbarTextFormat->addWidget(m_pComboSize);
	m_pComboSize->setEditable(true);

	QFontDatabase db;
	foreach(int size, db.standardSizes())
		m_pComboSize->addItem(QString::number(size));

	connect(m_pComboSize, SIGNAL(activated(QString)), this, SLOT(textSize(QString)));
	m_pComboSize->setCurrentIndex(m_pComboSize->findText(QString::number(QApplication::font().pointSize())));
}

bool CW3ReportToolbar::load(const QString &f) {
	if (!QFile::exists(f))
		return false;

	QFile file(f);
	if (!file.open(QFile::ReadOnly))
		return false;

	QByteArray data = file.readAll();
	QTextCodec *codec = Qt::codecForHtml(data);
	QString str = codec->toUnicode(data);
	if (Qt::mightBeRichText(str))
		m_pTextEdit->setHtml(str);
	else {
		str = QString::fromLocal8Bit(data);
		m_pTextEdit->setPlainText(str);
	}
	return true;
}

void CW3ReportToolbar::fileNew() {
	m_pTextEdit->clear();
}

void CW3ReportToolbar::fileOpen() {
#if defined(__APPLE__)
	QString fn = QFileDialog::getOpenFileName(m_pParent,
											  tr("Open File..."),
											  "./",
											  tr("HTML-Files (*.htm *.html);;All Files (*)"),
											  nullptr,
											  QFileDialog::Option::DontUseNativeDialog);
#else
	QString fn = QFileDialog::getOpenFileName(m_pParent, tr("Open File..."), QString(), tr("HTML-Files (*.htm *.html);;All Files (*)"));
#endif

	if (!fn.isEmpty())
		load(fn);
}

bool CW3ReportToolbar::fileSave() {
	QTextDocumentWriter writer(m_initialFile);
	bool success = writer.write(m_pTextEdit->document());
	if (success)
		m_pTextEdit->document()->setModified(false);
	return success;
}

bool CW3ReportToolbar::fileSaveAs() {
#if defined(__APPLE__)
	QString fn = QFileDialog::getSaveFileName(m_pParent, tr("Save As..."),
											  "./",
											  tr("ODF files (*.odt);;HTML-Files (*.htm *.html);;All Files (*)"),
											  nullptr,
											  QFileDialog::Option::DontUseNativeDialog);
#else
	QString fn = QFileDialog::getSaveFileName(m_pParent, tr("Save As..."), QString(), tr("ODF files (*.odt);;HTML-Files (*.htm *.html);;All Files (*)"));
#endif
	if (fn.isEmpty())
		return false;
	if (!(fn.endsWith(".odt", Qt::CaseInsensitive) ||
		  fn.endsWith(".htm", Qt::CaseInsensitive) ||
		  fn.endsWith(".html", Qt::CaseInsensitive))) {
		fn += ".odt";
	}

	m_initialFile = fn;
	return fileSave();
}

void CW3ReportToolbar::filePrint() {
	QPrinter printer(QPrinter::HighResolution);
	QPrintDialog *dlg = new QPrintDialog(&printer, m_pParent);
	if (m_pTextEdit->textCursor().hasSelection())
		dlg->addEnabledOption(QAbstractPrintDialog::PrintSelection);
	dlg->setWindowTitle(lang::LanguagePack::txt_print());
	if (dlg->exec() == QDialog::Accepted)
		m_pTextEdit->print(&printer);
	delete dlg;
}

void CW3ReportToolbar::filePrintPreview() {
	QPrinter printer(QPrinter::HighResolution);
	QPrintPreviewDialog preview(&printer, this);
	connect(&preview, SIGNAL(paintRequested(QPrinter*)), SLOT(printPreview(QPrinter*)));
	preview.exec();
}

void CW3ReportToolbar::printPreview(QPrinter *printer) {
	m_pTextEdit->print(printer);
}

void CW3ReportToolbar::filePrintPdf() {
#if defined(__APPLE__)
	QString fileName = QFileDialog::getSaveFileName(this, "Export PDF",
													"./",
													"*.pdf",
													nullptr,
													QFileDialog::Option::DontUseNativeDialog);
#else
	QString fileName = QFileDialog::getSaveFileName(this, "Export PDF", QString(), "*.pdf");
#endif

	if (!fileName.isEmpty()) {
		if (QFileInfo(fileName).suffix().isEmpty())
			fileName.append(".pdf");
		QPrinter printer(QPrinter::HighResolution);
		printer.setOutputFormat(QPrinter::PdfFormat);
		printer.setOutputFileName(fileName);
		m_pTextEdit->document()->print(&printer);
	}
}

void CW3ReportToolbar::textBold() {
	QTextCharFormat fmt;
	fmt.setFontWeight(m_pActionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
	mergeFormatOnWordOrSelection(fmt);
}

void CW3ReportToolbar::textUnderline() {
	QTextCharFormat fmt;
	fmt.setFontUnderline(m_pActionTextUnderline->isChecked());
	mergeFormatOnWordOrSelection(fmt);
}

void CW3ReportToolbar::textItalic() {
	QTextCharFormat fmt;
	fmt.setFontItalic(m_pActionTextItalic->isChecked());
	mergeFormatOnWordOrSelection(fmt);
}

void CW3ReportToolbar::textFamily(const QString &f) {
	QTextCharFormat fmt;
	fmt.setFontFamily(f);
	mergeFormatOnWordOrSelection(fmt);
}

void CW3ReportToolbar::textSize(const QString &p) {
	float pointSize = p.toFloat();
	if (pointSize > 0.0f) {
		QTextCharFormat fmt;
		fmt.setFontPointSize(pointSize);
		mergeFormatOnWordOrSelection(fmt);
	}
}

void CW3ReportToolbar::textStyle(int styleIndex) {
	QTextCursor cursor = m_pTextEdit->textCursor();
	if (styleIndex != 0) {
		QTextListFormat::Style style = QTextListFormat::ListDisc;

		switch (styleIndex) {
		default:
		case 1:	style = QTextListFormat::ListDisc;			break;
		case 2:	style = QTextListFormat::ListCircle;		break;
		case 3:	style = QTextListFormat::ListSquare;		break;
		case 4:	style = QTextListFormat::ListDecimal;		break;
		case 5:	style = QTextListFormat::ListLowerAlpha;	break;
		case 6:	style = QTextListFormat::ListUpperAlpha;	break;
		case 7:	style = QTextListFormat::ListLowerRoman;	break;
		case 8:	style = QTextListFormat::ListUpperRoman;	break;
		}

		cursor.beginEditBlock();

		QTextBlockFormat blockFmt = cursor.blockFormat();
		QTextListFormat listFmt;
		if (cursor.currentList()) {
			listFmt = cursor.currentList()->format();
		} else {
			listFmt.setIndent(blockFmt.indent() + 1);
			blockFmt.setIndent(0);
			cursor.setBlockFormat(blockFmt);
		}
		listFmt.setStyle(style);
		cursor.createList(listFmt);

		cursor.endEditBlock();
	} else {
		QTextBlockFormat bfmt;
		bfmt.setObjectIndex(-1);
		cursor.mergeBlockFormat(bfmt);
	}
}

void CW3ReportToolbar::ImageCellLayout(int LayoutIdx) {
	switch (LayoutIdx) {
	case 0:	m_initialFile = QString(":/html/template_1x1.html");	break;
	case 1:	m_initialFile = QString(":/html/template_1x2.html");	break;
	case 2:	m_initialFile = QString(":/html/template_1x3.html");	break;
	case 3:	m_initialFile = QString(":/html/template_2x1.html");	break;
	case 4:	m_initialFile = QString(":/html/template_2x2.html");	break;
	case 5:	m_initialFile = QString(":/html/template_2x3.html");	break;
	case 6:	m_initialFile = QString(":/html/template_3x1.html");	break;
	case 7:	m_initialFile = QString(":/html/template_3x2.html");	break;
	case 8:	m_initialFile = QString(":/html/template_3x3.html");	break;
	}
	load(m_initialFile);
}

void CW3ReportToolbar::textColor() {
	ColorDialog color_dialog;
	color_dialog.SetCurrentColor(m_pTextEdit->textColor());
	if (!color_dialog.exec())
	{
		return;
	}

	QColor color = color_dialog.SelectedColor();
	if (!color.isValid())
	{
		return;
	}

	QTextCharFormat fmt;
	fmt.setForeground(color);
	mergeFormatOnWordOrSelection(fmt);
	colorChanged(color);
}

void CW3ReportToolbar::textAlign(QAction *a) {
	if (a == m_pActionAlignLeft)	 m_pTextEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
	else if (a == m_pActionAlignRight)	 m_pTextEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
	else if (a == m_pActionAlignCenter)	 m_pTextEdit->setAlignment(Qt::AlignHCenter);
	else if (a == m_pActionAlignJustify)	 m_pTextEdit->setAlignment(Qt::AlignJustify);
}

void CW3ReportToolbar::currentCharFormatChanged(const QTextCharFormat &format) {
	fontChanged(format.font());
	colorChanged(format.foreground().color());
}

void CW3ReportToolbar::cursorPositionChanged() {
	alignmentChanged(m_pTextEdit->alignment());
}

void CW3ReportToolbar::clipboardDataChanged() {
	if (const QMimeData *md = QApplication::clipboard()->mimeData())
		m_pActionPaste->setEnabled(md->hasText());
}

void CW3ReportToolbar::mergeFormatOnWordOrSelection(const QTextCharFormat &format) {
	QTextCursor cursor = m_pTextEdit->textCursor();
	if (!cursor.hasSelection())
		cursor.select(QTextCursor::WordUnderCursor);
	cursor.mergeCharFormat(format);
	m_pTextEdit->mergeCurrentCharFormat(format);
}

void CW3ReportToolbar::fontChanged(const QFont &f) {
	m_pComboFont->setCurrentIndex(m_pComboFont->findText(QFontInfo(f).family()));
	m_pComboSize->setCurrentIndex(m_pComboSize->findText(QString::number(f.pointSize())));
	m_pActionTextBold->setChecked(f.bold());
	m_pActionTextItalic->setChecked(f.italic());
	m_pActionTextUnderline->setChecked(f.underline());
}

void CW3ReportToolbar::colorChanged(const QColor &c) {
	QPixmap pix(16, 16);
	pix.fill(c);
	m_pActionTextColor->setIcon(pix);
}

void CW3ReportToolbar::alignmentChanged(Qt::Alignment a) {
	if (a & Qt::AlignLeft)			m_pActionAlignLeft->setChecked(true);
	else if (a & Qt::AlignCenter)	m_pActionAlignCenter->setChecked(true);
	else if (a & Qt::AlignRight)		m_pActionAlignRight->setChecked(true);
	else if (a & Qt::AlignJustify)	m_pActionAlignJustify->setChecked(true);
}

void CW3ReportToolbar::SetToolBarVisible(bool bShow) {
	m_pToolbarFile->setVisible(bShow);
	m_pToolbarEdit->setVisible(bShow);
	m_pToolBarText->setVisible(bShow);
	m_pToolbarTextFormat->setVisible(bShow);
	m_pParent->menuBar()->setVisible(bShow);
}

void CW3ReportToolbar::test() {
	load(m_initialFile);
}
