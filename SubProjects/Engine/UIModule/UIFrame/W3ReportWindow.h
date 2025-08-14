#pragma once

#include <QPointer>
#include <QFrame>

#include "uiframe_global.h"
#include "GeneratedFiles/ui_W3ReportWindow.h"

class CW3ImageHeader;

class UIFRAME_EXPORT CW3ReportWindow : public QFrame, protected Ui::CW3ReportWindow {
    Q_OBJECT

public:
	CW3ReportWindow(QWidget *parent = 0);

public:
	QString toPlainText() const { return f_textedit->toPlainText(); }
	QString toHtml() const;
	QTextDocument *document() { return f_textedit->document(); }
	QTextCursor    textCursor() const { return f_textedit->textCursor(); }
	void           setTextCursor(const QTextCursor& cursor) { f_textedit->setTextCursor(cursor); }
	
	void		   initReport(CW3ImageHeader* hd);
	void		   addThumbnail(const QString& path);
	
	void reset();
	//void wheelEvent(QWheelEvent *event) override;
//protected:
//	void wheelEvent(QWheelEvent *event) override;


public slots:
	void setText(const QString &text);

 protected slots:
	void setPlainText(const QString &text) { f_textedit->setPlainText(text); }
	void setHtml(const QString &text)      { f_textedit->setHtml(text); }
	void textRemoveFormat();
	void textRemoveAllFormat();
	void textBold();
	void textUnderline();
	void textStrikeout();
	void textItalic();
	void textSize(const QString &p);
	void textLink(bool checked);
	void textStyle(int index);
	void textFgColor();
	void textBgColor();
	void listBullet(bool checked);
	void listOrdered(bool checked);
	void slotCurrentCharFormatChanged(const QTextCharFormat &format);
	void slotCursorPositionChanged();
	//void slotClipboardDataChanged();
	void increaseIndentation();
	void decreaseIndentation();
	void saveHTML();
	void textSource();
	
private:
	void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
	void fontChanged(const QFont &f);
	void fgColorChanged(const QColor &c);
	void bgColorChanged(const QColor &c);
	void list(bool checked, QTextListFormat::Style style);
	void indent(int delta);
	void focusInEvent(QFocusEvent *event);

private:
	int m_fontsize_h1 = 18;
	int m_fontsize_h2 = 16;
	int m_fontsize_h3 = 14;
	int m_fontsize_h4 = 12;

	int m_defaultFontSize;
	
	enum ParagraphItems { ParagraphStandard = 0,
	                      ParagraphHeading1,
	                      ParagraphHeading2,
	                      ParagraphHeading3,
	                      ParagraphHeading4,
	                      ParagraphMonospace };
	
	QPointer<QTextList> m_lastBlockList;
	
	QString m_reportFileName;
};

