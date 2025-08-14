#pragma once

#include <QWidget>
#include "uiframe_global.h"

class QMainWindow;
class QTextEdit;
class QTextCharFormat;
class QMenu;
class QPrinter;
class QMenuBar;
class QAction;
class QToolBar;
class QColor;
class QFont;
class QComboBox;

class UIFRAME_EXPORT CW3ReportToolbar : public QWidget
{
	Q_OBJECT
public:
	CW3ReportToolbar(QMainWindow *pParentWindow, QTextEdit *pTextEdit);
	~CW3ReportToolbar(void);
	void SetToolBarVisible(bool bShow);

private:
	QMainWindow *m_pParent;
	QTextEdit *m_pTextEdit;
	QString m_strImgDir;

	void setupFileActions();
	void setupEditActions();
	void setupTextActions();

private slots:
	void fileNew();
	void fileOpen();
	bool fileSave();
	bool fileSaveAs();
	
	void filePrint();
	void filePrintPreview();
	void filePrintPdf();

	void textBold();
	void textUnderline();
	void textItalic();
	void textFamily(const QString &f);
	void textSize(const QString &p);
	void textStyle(int styleIndex);
	void ImageCellLayout(int LayoutIdx);
	void textColor();
	void textAlign(QAction *a);

	void clipboardDataChanged();
	void printPreview(QPrinter *);

	void cursorPositionChanged();
	void currentCharFormatChanged(const QTextCharFormat &format);
	void test();

private:
	bool load(const QString &f);
	void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
	void fontChanged(const QFont &f);
	void colorChanged(const QColor &c);
	void alignmentChanged(Qt::Alignment a);	

	QString m_initialFile;

	QComboBox *m_pComboStyle;
	QComboBox *m_pComboFont;
	QComboBox *m_pComboSize;
	QComboBox *m_pComboImageLayout;

	QAction *m_pActionSave;
	QAction *m_pActionUndo;
	QAction *m_pActionRedo;
 	QAction *m_pActionCut;
 	QAction *m_pActionCopy;
 	QAction *m_pActionPaste;
 	QAction *m_pActionTextBold;
 	QAction *m_pActionTextUnderline;
 	QAction *m_pActionTextItalic;
	QAction *m_pActionAlignLeft;
	QAction *m_pActionAlignRight;
	QAction *m_pActionAlignCenter;
	QAction *m_pActionAlignJustify;
	QAction *m_pActionTextColor;

	QToolBar *m_pToolbarFile;
	QToolBar *m_pToolbarEdit;
	QToolBar *m_pToolBarText;
	QToolBar *m_pToolbarTextFormat;

	QMenu *m_pMenuHelp;
	QMenu *m_pMenuFile;
	QMenu *m_pMenuEdit;
	QMenu *m_pMenuText;	
};


