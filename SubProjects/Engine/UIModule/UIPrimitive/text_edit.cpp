#include "text_edit.h"

#include <Engine/Common/Common/language_pack.h>

TextEdit::TextEdit(QWidget *parent)
	: QTextEdit(parent)
{
	empty_state_text_ = lang::LanguagePack::txt_write_memo_here();

	setText(empty_state_text_);
	setStyleSheet("QTextEdit {background-color: #FFBEC4DC; color: black;}");
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	setMinimumHeight(150);
}

TextEdit::~TextEdit()
{
}

void TextEdit::focusInEvent(QFocusEvent * e)
{
	QTextEdit::focusInEvent(e);
	const QString& contents = toPlainText();
	if (QString::compare(contents, empty_state_text_, Qt::CaseInsensitive) == 0)
	{
		clear();
	}
}

void TextEdit::focusOutEvent(QFocusEvent * e)
{
	QTextEdit::focusOutEvent(e);
	const QString& contents = toPlainText();
	if (contents.isEmpty())
	{
		setText(empty_state_text_);
	}
}
