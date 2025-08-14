/********************************************************************************
** Form generated from reading UI file 'W3ReportWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_W3REPORTWINDOW_H
#define UI_W3REPORTWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "W3ReportTextEdit.h"
#include "W3ReportThumbnail.h"

QT_BEGIN_NAMESPACE

class Ui_CW3ReportWindow
{
public:
    QGridLayout *gridLayout;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *verticalLayout;
    CW3ReportThumbnail *f_thumnail;
    CW3ReportTextEdit *f_textedit;
    QFrame *f_toolbar;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *sp_left;
    QToolButton *f_undo;
    QFrame *line_4;
    QToolButton *f_redo;
    QFrame *line_5;
    QToolButton *f_saveHTML;
    QFrame *line;
    QToolButton *f_link;
    QToolButton *f_bold;
    QToolButton *f_italic;
    QToolButton *f_underline;
    QToolButton *f_strikeout;
    QFrame *line_3;
    QToolButton *f_list_bullet;
    QToolButton *f_list_ordered;
    QToolButton *f_indent_dec;
    QToolButton *f_indent_inc;
    QFrame *line_2;
    QToolButton *f_fgcolor;
    QToolButton *f_bgcolor;
    QComboBox *f_fontsize;
    QSpacerItem *sp_right;

    void setupUi(QFrame *CW3ReportWindow)
    {
        if (CW3ReportWindow->objectName().isEmpty())
            CW3ReportWindow->setObjectName(QStringLiteral("CW3ReportWindow"));
        CW3ReportWindow->resize(1244, 779);
        CW3ReportWindow->setWindowTitle(QStringLiteral(""));
        gridLayout = new QGridLayout(CW3ReportWindow);
        gridLayout->setSpacing(0);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        scrollArea = new QScrollArea(CW3ReportWindow);
        scrollArea->setObjectName(QStringLiteral("scrollArea"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(scrollArea->sizePolicy().hasHeightForWidth());
        scrollArea->setSizePolicy(sizePolicy);
        scrollArea->setMinimumSize(QSize(335, 335));
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        scrollArea->setWidgetResizable(true);
        scrollArea->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 333, 777));
        sizePolicy.setHeightForWidth(scrollAreaWidgetContents->sizePolicy().hasHeightForWidth());
        scrollAreaWidgetContents->setSizePolicy(sizePolicy);
        scrollAreaWidgetContents->setFocusPolicy(Qt::WheelFocus);
        verticalLayout = new QVBoxLayout(scrollAreaWidgetContents);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        f_thumnail = new CW3ReportThumbnail(scrollAreaWidgetContents);
        f_thumnail->setObjectName(QStringLiteral("f_thumnail"));
        sizePolicy.setHeightForWidth(f_thumnail->sizePolicy().hasHeightForWidth());
        f_thumnail->setSizePolicy(sizePolicy);
        f_thumnail->setMinimumSize(QSize(0, 0));
        f_thumnail->setFrameShape(QFrame::NoFrame);
        f_thumnail->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(f_thumnail);

        scrollArea->setWidget(scrollAreaWidgetContents);

        gridLayout->addWidget(scrollArea, 0, 1, 2, 1);

        f_textedit = new CW3ReportTextEdit(CW3ReportWindow);
        f_textedit->setObjectName(QStringLiteral("f_textedit"));
        f_textedit->setMidLineWidth(21);
        f_textedit->setAutoFormatting(QTextEdit::AutoNone);
        f_textedit->setTabChangesFocus(true);

        gridLayout->addWidget(f_textedit, 1, 0, 1, 1);

        f_toolbar = new QFrame(CW3ReportWindow);
        f_toolbar->setObjectName(QStringLiteral("f_toolbar"));
        horizontalLayout = new QHBoxLayout(f_toolbar);
        horizontalLayout->setSpacing(5);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(5, 5, 5, 5);
        sp_left = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(sp_left);

        f_undo = new QToolButton(f_toolbar);
        f_undo->setObjectName(QStringLiteral("f_undo"));
        f_undo->setEnabled(true);
        f_undo->setFocusPolicy(Qt::ClickFocus);
        QIcon icon;
        QString iconThemeName = QStringLiteral("edit-undo");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon = QIcon::fromTheme(iconThemeName);
        } else {
            icon.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        f_undo->setIcon(icon);
        f_undo->setIconSize(QSize(16, 16));

        horizontalLayout->addWidget(f_undo);

        line_4 = new QFrame(f_toolbar);
        line_4->setObjectName(QStringLiteral("line_4"));
        line_4->setEnabled(true);
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(line_4->sizePolicy().hasHeightForWidth());
        line_4->setSizePolicy(sizePolicy1);
        line_4->setMinimumSize(QSize(1, 5));
        line_4->setBaseSize(QSize(0, 0));
        line_4->setStyleSheet(QStringLiteral(""));
        line_4->setFrameShadow(QFrame::Plain);
        line_4->setLineWidth(0);
        line_4->setMidLineWidth(0);
        line_4->setFrameShape(QFrame::VLine);

        horizontalLayout->addWidget(line_4);

        f_redo = new QToolButton(f_toolbar);
        f_redo->setObjectName(QStringLiteral("f_redo"));
        f_redo->setEnabled(true);
        f_redo->setFocusPolicy(Qt::ClickFocus);
        QIcon icon1;
        iconThemeName = QStringLiteral("edit-redo");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon1 = QIcon::fromTheme(iconThemeName);
        } else {
            icon1.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        f_redo->setIcon(icon1);
        f_redo->setIconSize(QSize(16, 16));

        horizontalLayout->addWidget(f_redo);

        line_5 = new QFrame(f_toolbar);
        line_5->setObjectName(QStringLiteral("line_5"));
        line_5->setEnabled(true);
        sizePolicy1.setHeightForWidth(line_5->sizePolicy().hasHeightForWidth());
        line_5->setSizePolicy(sizePolicy1);
        line_5->setMinimumSize(QSize(1, 5));
        line_5->setBaseSize(QSize(0, 0));
        line_5->setStyleSheet(QStringLiteral(""));
        line_5->setFrameShadow(QFrame::Plain);
        line_5->setLineWidth(0);
        line_5->setMidLineWidth(0);
        line_5->setFrameShape(QFrame::VLine);

        horizontalLayout->addWidget(line_5);

        f_saveHTML = new QToolButton(f_toolbar);
        f_saveHTML->setObjectName(QStringLiteral("f_saveHTML"));
        f_saveHTML->setText(QStringLiteral(""));

        horizontalLayout->addWidget(f_saveHTML);

        line = new QFrame(f_toolbar);
        line->setObjectName(QStringLiteral("line"));
        line->setEnabled(true);
        sizePolicy1.setHeightForWidth(line->sizePolicy().hasHeightForWidth());
        line->setSizePolicy(sizePolicy1);
        line->setMinimumSize(QSize(1, 5));
        line->setBaseSize(QSize(0, 0));
        line->setStyleSheet(QStringLiteral(""));
        line->setFrameShadow(QFrame::Plain);
        line->setLineWidth(0);
        line->setMidLineWidth(0);
        line->setFrameShape(QFrame::VLine);

        horizontalLayout->addWidget(line);

        f_link = new QToolButton(f_toolbar);
        f_link->setObjectName(QStringLiteral("f_link"));
        f_link->setFocusPolicy(Qt::ClickFocus);
        QIcon icon2;
        iconThemeName = QStringLiteral("applications-internet");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon2 = QIcon::fromTheme(iconThemeName);
        } else {
            icon2.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        f_link->setIcon(icon2);
        f_link->setIconSize(QSize(16, 16));
        f_link->setCheckable(true);

        horizontalLayout->addWidget(f_link);

        f_bold = new QToolButton(f_toolbar);
        f_bold->setObjectName(QStringLiteral("f_bold"));
        f_bold->setFocusPolicy(Qt::ClickFocus);
#ifndef QT_NO_TOOLTIP
        f_bold->setToolTip(QStringLiteral("Bold (CTRL+B)"));
#endif // QT_NO_TOOLTIP
        QIcon icon3;
        iconThemeName = QStringLiteral("format-text-bold");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon3 = QIcon::fromTheme(iconThemeName);
        } else {
            icon3.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        f_bold->setIcon(icon3);
        f_bold->setIconSize(QSize(16, 16));
        f_bold->setCheckable(true);

        horizontalLayout->addWidget(f_bold);

        f_italic = new QToolButton(f_toolbar);
        f_italic->setObjectName(QStringLiteral("f_italic"));
        f_italic->setFocusPolicy(Qt::ClickFocus);
        QIcon icon4;
        iconThemeName = QStringLiteral("format-text-italic");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon4 = QIcon::fromTheme(iconThemeName);
        } else {
            icon4.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        f_italic->setIcon(icon4);
        f_italic->setIconSize(QSize(16, 16));
        f_italic->setCheckable(true);

        horizontalLayout->addWidget(f_italic);

        f_underline = new QToolButton(f_toolbar);
        f_underline->setObjectName(QStringLiteral("f_underline"));
        f_underline->setFocusPolicy(Qt::ClickFocus);
        QIcon icon5;
        iconThemeName = QStringLiteral("format-text-underline");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon5 = QIcon::fromTheme(iconThemeName);
        } else {
            icon5.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        f_underline->setIcon(icon5);
        f_underline->setIconSize(QSize(16, 16));
        f_underline->setCheckable(true);

        horizontalLayout->addWidget(f_underline);

        f_strikeout = new QToolButton(f_toolbar);
        f_strikeout->setObjectName(QStringLiteral("f_strikeout"));
        f_strikeout->setCheckable(true);

        horizontalLayout->addWidget(f_strikeout);

        line_3 = new QFrame(f_toolbar);
        line_3->setObjectName(QStringLiteral("line_3"));
        line_3->setEnabled(true);
        sizePolicy1.setHeightForWidth(line_3->sizePolicy().hasHeightForWidth());
        line_3->setSizePolicy(sizePolicy1);
        line_3->setMinimumSize(QSize(1, 5));
        line_3->setBaseSize(QSize(0, 0));
        line_3->setStyleSheet(QStringLiteral(""));
        line_3->setFrameShadow(QFrame::Plain);
        line_3->setLineWidth(0);
        line_3->setMidLineWidth(0);
        line_3->setFrameShape(QFrame::VLine);

        horizontalLayout->addWidget(line_3);

        f_list_bullet = new QToolButton(f_toolbar);
        f_list_bullet->setObjectName(QStringLiteral("f_list_bullet"));
        f_list_bullet->setFocusPolicy(Qt::ClickFocus);
        f_list_bullet->setIconSize(QSize(16, 16));
        f_list_bullet->setCheckable(true);

        horizontalLayout->addWidget(f_list_bullet);

        f_list_ordered = new QToolButton(f_toolbar);
        f_list_ordered->setObjectName(QStringLiteral("f_list_ordered"));
        f_list_ordered->setFocusPolicy(Qt::ClickFocus);
        f_list_ordered->setIconSize(QSize(16, 16));
        f_list_ordered->setCheckable(true);

        horizontalLayout->addWidget(f_list_ordered);

        f_indent_dec = new QToolButton(f_toolbar);
        f_indent_dec->setObjectName(QStringLiteral("f_indent_dec"));
        f_indent_dec->setFocusPolicy(Qt::ClickFocus);
        QIcon icon6;
        iconThemeName = QStringLiteral("format-indent-less");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon6 = QIcon::fromTheme(iconThemeName);
        } else {
            icon6.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        f_indent_dec->setIcon(icon6);
        f_indent_dec->setIconSize(QSize(16, 16));

        horizontalLayout->addWidget(f_indent_dec);

        f_indent_inc = new QToolButton(f_toolbar);
        f_indent_inc->setObjectName(QStringLiteral("f_indent_inc"));
        f_indent_inc->setFocusPolicy(Qt::ClickFocus);
        QIcon icon7;
        iconThemeName = QStringLiteral("format-indent-more");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon7 = QIcon::fromTheme(iconThemeName);
        } else {
            icon7.addFile(QStringLiteral("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        f_indent_inc->setIcon(icon7);
        f_indent_inc->setIconSize(QSize(16, 16));

        horizontalLayout->addWidget(f_indent_inc);

        line_2 = new QFrame(f_toolbar);
        line_2->setObjectName(QStringLiteral("line_2"));
        line_2->setEnabled(true);
        sizePolicy1.setHeightForWidth(line_2->sizePolicy().hasHeightForWidth());
        line_2->setSizePolicy(sizePolicy1);
        line_2->setMinimumSize(QSize(1, 5));
        line_2->setBaseSize(QSize(0, 0));
        line_2->setStyleSheet(QStringLiteral(""));
        line_2->setFrameShadow(QFrame::Plain);
        line_2->setLineWidth(0);
        line_2->setMidLineWidth(0);
        line_2->setFrameShape(QFrame::VLine);

        horizontalLayout->addWidget(line_2);

        f_fgcolor = new QToolButton(f_toolbar);
        f_fgcolor->setObjectName(QStringLiteral("f_fgcolor"));
        f_fgcolor->setMinimumSize(QSize(16, 16));
        f_fgcolor->setMaximumSize(QSize(16, 16));
        f_fgcolor->setFocusPolicy(Qt::ClickFocus);
        f_fgcolor->setIconSize(QSize(16, 16));

        horizontalLayout->addWidget(f_fgcolor);

        f_bgcolor = new QToolButton(f_toolbar);
        f_bgcolor->setObjectName(QStringLiteral("f_bgcolor"));
        f_bgcolor->setMinimumSize(QSize(16, 16));
        f_bgcolor->setMaximumSize(QSize(16, 16));
        f_bgcolor->setFocusPolicy(Qt::ClickFocus);
        f_bgcolor->setIconSize(QSize(16, 16));

        horizontalLayout->addWidget(f_bgcolor);

        f_fontsize = new QComboBox(f_toolbar);
        f_fontsize->setObjectName(QStringLiteral("f_fontsize"));
        sizePolicy1.setHeightForWidth(f_fontsize->sizePolicy().hasHeightForWidth());
        f_fontsize->setSizePolicy(sizePolicy1);
        f_fontsize->setMinimumSize(QSize(0, 0));
        f_fontsize->setFocusPolicy(Qt::ClickFocus);
        f_fontsize->setEditable(true);

        horizontalLayout->addWidget(f_fontsize);

        sp_right = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(sp_right);

        f_fgcolor->raise();
        f_bgcolor->raise();
        line->raise();
        f_link->raise();
        f_italic->raise();
        f_underline->raise();
        line_2->raise();
        f_fontsize->raise();
        line_3->raise();
        f_list_bullet->raise();
        f_list_ordered->raise();
        f_indent_dec->raise();
        f_indent_inc->raise();
        f_bold->raise();
        f_strikeout->raise();
        f_undo->raise();
        f_redo->raise();
        line_4->raise();
        line_5->raise();
        f_saveHTML->raise();

        gridLayout->addWidget(f_toolbar, 0, 0, 1, 1);

        QWidget::setTabOrder(f_textedit, f_strikeout);

        retranslateUi(CW3ReportWindow);

        QMetaObject::connectSlotsByName(CW3ReportWindow);
    } // setupUi

    void retranslateUi(QFrame *CW3ReportWindow)
    {
#ifndef QT_NO_TOOLTIP
        f_undo->setToolTip(QApplication::translate("CW3ReportWindow", "Undo (CTRL+Z)", nullptr));
#endif // QT_NO_TOOLTIP
        f_undo->setText(QString());
#ifndef QT_NO_TOOLTIP
        f_redo->setToolTip(QApplication::translate("CW3ReportWindow", "Redo (CTRL+Y)", nullptr));
#endif // QT_NO_TOOLTIP
        f_redo->setText(QString());
#ifndef QT_NO_TOOLTIP
        f_link->setToolTip(QApplication::translate("CW3ReportWindow", "Link (CTRL+L)", nullptr));
#endif // QT_NO_TOOLTIP
        f_link->setText(QString());
        f_bold->setText(QString());
#ifndef QT_NO_TOOLTIP
        f_italic->setToolTip(QApplication::translate("CW3ReportWindow", "Italic (CTRL+I)", nullptr));
#endif // QT_NO_TOOLTIP
        f_italic->setText(QString());
#ifndef QT_NO_TOOLTIP
        f_underline->setToolTip(QApplication::translate("CW3ReportWindow", "Underline (CTRL+U)", nullptr));
#endif // QT_NO_TOOLTIP
        f_underline->setText(QString());
        f_strikeout->setText(QString());
#ifndef QT_NO_TOOLTIP
        f_list_bullet->setToolTip(QApplication::translate("CW3ReportWindow", "Bullet list (CTRL+-)", nullptr));
#endif // QT_NO_TOOLTIP
        f_list_bullet->setText(QString());
#ifndef QT_NO_TOOLTIP
        f_list_ordered->setToolTip(QApplication::translate("CW3ReportWindow", "Ordered list (CTRL+=)", nullptr));
#endif // QT_NO_TOOLTIP
        f_list_ordered->setText(QString());
#ifndef QT_NO_TOOLTIP
        f_indent_dec->setToolTip(QApplication::translate("CW3ReportWindow", "Decrease indentation (CTRL+,)", nullptr));
#endif // QT_NO_TOOLTIP
        f_indent_dec->setText(QString());
#ifndef QT_NO_TOOLTIP
        f_indent_inc->setToolTip(QApplication::translate("CW3ReportWindow", "Increase indentation (CTRL+.)", nullptr));
#endif // QT_NO_TOOLTIP
        f_indent_inc->setText(QString());
#ifndef QT_NO_TOOLTIP
        f_fgcolor->setToolTip(QApplication::translate("CW3ReportWindow", "Text foreground color", nullptr));
#endif // QT_NO_TOOLTIP
        f_fgcolor->setText(QApplication::translate("CW3ReportWindow", ".", nullptr));
#ifndef QT_NO_TOOLTIP
        f_bgcolor->setToolTip(QApplication::translate("CW3ReportWindow", "Text background color", nullptr));
#endif // QT_NO_TOOLTIP
        f_bgcolor->setText(QApplication::translate("CW3ReportWindow", ".", nullptr));
#ifndef QT_NO_TOOLTIP
        f_fontsize->setToolTip(QApplication::translate("CW3ReportWindow", "Font size", nullptr));
#endif // QT_NO_TOOLTIP
        f_fontsize->setCurrentText(QApplication::translate("CW3ReportWindow", "7", nullptr));
        Q_UNUSED(CW3ReportWindow);
    } // retranslateUi

};

namespace Ui {
    class CW3ReportWindow: public Ui_CW3ReportWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_W3REPORTWINDOW_H
