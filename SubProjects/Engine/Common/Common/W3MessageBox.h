#pragma once
/*=========================================================================

File:			class CW3MessageBox
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-07-12
Last modify:	2016-07-12

=========================================================================*/
#include "W3Dialog.h"
#include "common_global.h"

class QHBoxLayout;
class QToolButton;
class QSignalMapper;

class COMMON_EXPORT CW3MessageBox : public CW3Dialog {
  Q_OBJECT

 public:
  enum MessageType { Question, Information, Warning, Critical, ActionRole };

 public:
  explicit CW3MessageBox(const QString& title, const QString& text,
                         MessageType type = Information, QWidget* parent = 0,
                         CW3Dialog::Theme theme = CW3Dialog::Theme::Dark);
  ~CW3MessageBox();

 public:
  void setDetailedText(const QString& text);
  QObject* AddButton(const QString& text);

  inline QObject* clickedButton() { return m_clickedButton; }

  //20250210LIN oktbn의 text바꾸기위해 public로 변경
  QToolButton* m_btnOK;
  QToolButton* m_btnCancel;

 protected:
  void keyPressEvent(QKeyEvent* event) override;

 private slots:
  void slotClickedButton(QObject*);

 private:
  //QToolButton* m_btnOK;
  //QToolButton* m_btnCancel;
  QLabel* m_lblDetailedText;

  MessageType m_type;

  QHBoxLayout* m_commandLayout;
  QVBoxLayout* m_messageLayout;
  QSignalMapper* m_sigMapper;

  QObject* m_clickedButton;
};
