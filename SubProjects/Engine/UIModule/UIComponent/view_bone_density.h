#pragma once
/**=================================================================================================

Project: 			UIComponent
File:				view_bone_density.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-07-20
Last modify:		2017-07-20

Copyright (c) 2017 ~ 2018 HDXWILL. All rights reserved.

*===============================================================================================**/
#if defined(__APPLE__)
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>
#else
#include <GL/glm/detail/type_mat.hpp>
#include <GL/glm/detail/type_mat4x4.hpp>
#endif

#include "uicomponent_global.h"
#include "view.h"

class QGraphicsTextItem;
class SimpleTextItem;
class ViewControllerBoneDensity;

class UICOMPONENT_EXPORT ViewBoneDensity : public View 
{
  Q_OBJECT

 public:
  explicit ViewBoneDensity(QWidget* parent = 0);
  ~ViewBoneDensity();

  ViewBoneDensity(const ViewBoneDensity&) = delete;
  ViewBoneDensity& operator=(const ViewBoneDensity&) = delete;

 public:
  void SyncImplant3DCameraMatrix(const glm::mat4& rotate_mat,
                                 const glm::mat4& reorien_mat,
                                 const glm::mat4& view_mat);
  void UpdateBoneDensity();

  void SyncPopupStatus(bool popup);

  // Implant3D view에서 현재 보고 있는 뷰와 일치시키기 위함
  const glm::mat4& GetRotateMatrix() const;

 signals:
  void sigBoneDensityPopupMode(bool);  // temp : bone density popup
  void sigRotated();

 private:
  virtual void drawBackground(QPainter* painter, const QRectF& rect) override;
  virtual void resizeEvent(QResizeEvent* pEvent) override;

  virtual void mouseMoveEvent(QMouseEvent* event) override;
  virtual void mouseDoubleClickEvent(QMouseEvent* event) override;  // temp : bone density popup

  virtual void InitializeController() override;
  virtual bool IsReadyController() override;
  virtual void TransformItems(const QTransform& transform) override;
  virtual void ClearGL() override;
  virtual void ActiveControllerViewEvent() override;

  void RenderBoneDensity();

  void RemoveItems();
  void DrawBoneDensityBar();
  void AddGradient(float box_width);
  void AddHUText(int index, float interval, float hu_pos_x, float hu_pos_y);
  void AddDLevelText(int index, float interval, float curr_hu_pos_y);
  void AddHSpacer(int index, float interval, float spacer_length);

 private:
  std::unique_ptr<ViewControllerBoneDensity> controller_;
  std::vector<QGraphicsTextItem*> hu_level_text_;
  std::vector<QGraphicsTextItem*> d_level_text_;
  std::vector<QGraphicsLineItem*> spacer_;
  QGraphicsRectItem* color_bar_ = nullptr;

  bool popup_mode_ = false;  // temp : bone density popup
};
