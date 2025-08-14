#pragma once
/*=========================================================================

File:			class ImplantListDlg
Language:		C++11
Library:		Qt 5.8, Standard C++ Library
Author:			Seo Seok Man
First date:		2018-10-19
Last modify:	2018-10-19

Copyright (c) 2018 All rights reserved by HDXWILL.

=========================================================================*/
#include <memory>

#include <QFrame>
#include <QTimer>

#include <Engine/Common/Common/W3Dialog.h>
#include <Engine/Common/Common/W3Enum.h>
#include <Engine/Common/Common/define_measure.h>
#include "uiframe_global.h"

class QHBoxLayout;
class QLabel;
class QToolButton;
class MeasureData;

class MeasureRecord : public QFrame {
  Q_OBJECT

 public:
  MeasureRecord(const unsigned int& record_id,
                const common::ViewTypeID& view_type,
                const unsigned int& measure_id,
                const common::measure::MeasureType& type, const QString& value,
                const QString& memo);
  ~MeasureRecord();
  MeasureRecord(const MeasureRecord&) = delete;
  MeasureRecord& operator=(const MeasureRecord&) = delete;

 signals:
  void sigChangeMemo(const common::ViewTypeID& view_type,
                     const unsigned int& measure_id, const QString& memo);
  void sigSelect(unsigned int record_id, const common::ViewTypeID& view_type,
                 const unsigned int& measure_id);
  void sigDelete(unsigned int record_id, const common::ViewTypeID& view_type,
                 const unsigned int& measure_id);

 public:
  void ChangeRecordID(const unsigned int& new_id);
  void SetRecordSelected(const bool& select);

 private:
  QLabel* CreateRecordLabel(const QString& text);
  QString GetMeasureTextLabelStyleSheet(bool selected);
  virtual void mousePressEvent(QMouseEvent* event) override;

 private slots:
  void slotTimerExpired();

 private:
  unsigned int id_;
  common::ViewTypeID view_id_;
  unsigned int measure_id_;

  std::unique_ptr<QHBoxLayout> layout_main_;
  std::unique_ptr<QLabel> ui_id_;
  std::unique_ptr<QLabel> ui_measure_type_;
  std::unique_ptr<QLabel> ui_position_;
  std::unique_ptr<QLabel> ui_value_;
  std::unique_ptr<QLabel> ui_memo_;
  std::unique_ptr<QToolButton> ui_delete_;

  QTimer timer_;
};

class UIFRAME_EXPORT MeasureListDialog : public CW3Dialog {
  Q_OBJECT

 public:
  MeasureListDialog(const common::measure::MeasureDataContainer& datas,
                    QWidget* parent = 0);
  virtual ~MeasureListDialog();

 signals:
  void sigMeasureChangeMemo(const common::ViewTypeID& view_type,
                            const unsigned int& measure_id,
                            const QString& memo);
  void sigMeasureDelete(const common::ViewTypeID& view_type,
                        const unsigned int& measure_id);
  void sigMeasureSelect(const common::ViewTypeID& view_type,
                        const unsigned int& measure_id);
  void sigMeasureListCapture(QImage image);

 private slots:
  void slotClose();
  void slotCapture();
  void slotDeleteRecord(unsigned int record_id,
                        const common::ViewTypeID& view_type,
                        const unsigned int& measure_id);
  void slotSelectRecord(unsigned int record_id,
                        const common::ViewTypeID& view_type,
                        const unsigned int& measure_id);

 private:
  void InitUI();
  void InitFieldUI();
  void InitRecordAreaUI();
  QHBoxLayout* InitButtonUI();

  void UpdateList(const common::measure::MeasureDataContainer& datas);
  void Connections();

  void CreateRecord(const common::ViewTypeID& view_type,
                    const std::weak_ptr<MeasureData>& measure_data);
  QLabel* CreateFieldLabel(const QString& text);

  virtual void closeEvent(QCloseEvent* e) override;

 private:
  std::unique_ptr<QVBoxLayout> layout_records_;

  std::unique_ptr<QFrame> field_area_;
  std::unique_ptr<QToolButton> capture_;
  std::unique_ptr<QToolButton> close_;

  unsigned int next_record_id_ = 1;
  std::vector<std::unique_ptr<MeasureRecord> > records_;
};
