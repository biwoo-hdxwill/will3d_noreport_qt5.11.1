#include "pano_arch_item.h"

#include <QVector2D>
#include "../../Common/Common/color_will3d.h"
#include "../../Common/Common/common.h"

#include "W3TextItem.h"
#include "line_list_item.h"
#include "pannig_handle_item.h"

namespace {
const int kInvalid = -1;
const float kRulerLineLengthLarge = 40.0f;
const float kRulerLineLengthMedium = 30.0f;
const float kRulerLineLengthSmall = 10.0f;

auto FuncSetText = [](CW3TextItem* text_item,
                      const std::vector<QPointF>& points, float index,
                      int origin_index, int text_num) {
  int idx = (int)index;
  auto FuncTextPos = [](QPointF pos, QVector2D normal, float dist) -> QPointF {
    QVector2D v = QVector2D(pos) + normal * dist;
    return QPointF(v.x(), v.y());
  };
  QVector2D normal = QVector2D(points[idx + 1] - points[idx]).normalized();
  normal = QVector2D(normal.y(), -normal.x());

  text_item->setPlainText(QString("%1").arg(text_num));

  QPointF text_bot_right = text_item->boundingRect().bottomRight();

  if (text_num < 0) {
    text_item->setPos(
        FuncTextPos(points[idx], normal, kRulerLineLengthLarge / 2.0f) -
        text_bot_right);
  } else {
    text_item->setPos(
        FuncTextPos(points[idx], normal, kRulerLineLengthLarge / 2.0f) +
        QPointF(0.0, -text_bot_right.y()));
  }
};

auto FuncAddLine = [](LineListItem* line_item,
                      const std::vector<QPointF>& points,
                      const std::vector<float>& gradation) {
  for (const auto& elem : gradation) {
    int index = (int)elem;
    if (points.size() > index && index >= 0) {
      QVector2D normal =
          QVector2D(points[index + 1] - points[index]).normalized();
      normal = QVector2D(normal.y(), -normal.x());
      line_item->AddLine(points[index], normal);
    }
  }
};

auto FuncNormalize = [](int min, int max, int value) -> float {
  if (max - min == 0)
    return 0.0f;
  else
    return (float)(value - min) / (max - min);
};
}  // namespace

PanoArchItem::PanoArchItem() { this->InitAdjustSplineItem(); }

PanoArchItem::~PanoArchItem() {}
/*=============================================================================================
public functions
===============================================================================================*/
void PanoArchItem::SetHighlightCrossSection(int cross_id, bool is_highlight) {
  if (cross_line_.get()) {
    if (is_highlight) {
      cross_line_->SetColorSpecificLine(ColorView::kCrossSectionSelected,
                                        cross_id);
      if (cross_number_text_[cross_id])
        cross_number_text_[cross_id]->setVisible(true);
      id_highlighted_cross_line_ = cross_id;
    } else {
      cross_line_->SetColorSpecificLine(ColorCrossSectionItem::kNormal,
                                        cross_id);
      if (cross_number_text_[cross_id])
        cross_number_text_[cross_id]->setVisible(false);
      id_highlighted_cross_line_ = -1;
    }
  }
}

void PanoArchItem::SetAdjustMode(bool is_adjust) {
  is_adjust_mode_ = is_adjust;

  adjust_spline_->SetAdjustMode(is_adjust_mode_);

  if (adjust_panning_handle_.get() &&
      (!is_adjust_mode_ || adjust_spline_->is_finished())) {
    adjust_panning_handle_->setVisible(is_adjust_mode_);
  }

  for (int i = 0; i < RANGE_ELLIPSE_END; i++) {
    if (adjust_range_ellipse_[i].get())
      adjust_range_ellipse_[i]->setVisible(is_adjust_mode_);
  }
  for (int i = 0; i < RANGE_PATH_END; i++) {
    if (adjust_range_path_[i].get()) {
      QPen pen_range_line;
      if (is_adjust_mode_) {
        pen_range_line = QPen(ColorArchItem::kCurvePen, 1.0, Qt::SolidLine);
      } else {
        pen_range_line =
            QPen(ColorArchItem::kCurveRangePen, 1.0, Qt::SolidLine);
      }
      adjust_range_path_[i]->setPen(pen_range_line);
    }
  }

  if (curr_spline_.get()) curr_spline_->setVisible(!is_adjust_mode_);
  if (cross_line_.get()) cross_line_->setVisible(!is_adjust_mode_);

  for (int i = 0; i < RLT_END; i++) {
    if (ruler_line_[i].get()) ruler_line_[i]->setVisible(!is_adjust_mode_);
  }
  for (int i = 0; i < RTT_END; i++) {
    if (ruler_text_[i].get()) ruler_text_[i]->setVisible(!is_adjust_mode_);
  }
}

void PanoArchItem::SetHighlight(const bool& is_highlight) {
  is_highlight_ = is_highlight;

  adjust_spline_->setHighligtedEllipse(is_highlight);
  adjust_spline_->setHighligtedPath(is_highlight);

  for (int i = 0; i < RANGE_ELLIPSE_END; i++) {
    if (adjust_range_ellipse_[i]) {
      adjust_range_ellipse_[i]->SetFlagHighlight(is_highlight);
      adjust_range_ellipse_[i]->SetFlagMovable(is_highlight);
    }
  }
}

void PanoArchItem::SetDisplayMode(DisplayMode display_mode) {
  display_mode_ = display_mode;
}

void PanoArchItem::DrawCrossSectionLine(const std::vector<QPointF>& points,
                                        const std::vector<int>& index,
                                        int length, float thickness) {
  if (cross_line_.get() == nullptr) InitCrossSectionItem();

  std::vector<int> indices;
  std::vector<float> min_dist;
  indices.resize(points.size(), 0);
  min_dist.resize(points.size(), std::numeric_limits<float>::max());

  std::vector<QPointF> tmp_spline_data, spline_data;

  if (current_spline_value_ == 0.0f)
    tmp_spline_data = adjust_spline_->getSplineData();
  else
    GetCurrentSplinePoints(tmp_spline_data);

  cross_line_->ClearLines();

  if (tmp_spline_data.size() == 0) return;

  Common::equidistanceSpline(spline_data, tmp_spline_data);
  for (int j = 0; j < spline_data.size(); j++) {
    for (int i = 0; i < points.size(); i++) {
      QPointF p = points[i];
      QPointF sd = spline_data[j];
      float dist = sqrt((p.x() - sd.x()) * (p.x() - sd.x()) +
                        (p.y() - sd.y()) * (p.y() - sd.y()));

      if (min_dist[i] > dist) {
        indices[i] = j;
        min_dist[i] = dist;
      }
    }
  }
  cross_line_->set_length(length);

  if (spline_data.size() < 3) return;

  QPen pen_cross_section_line(ColorCrossSectionItem::kNormal, 1.0,
                              Qt::SolidLine);
  if (thickness >= 1.0f) {
    pen_cross_section_line.setWidthF((qreal)thickness);
  }
  pen_cross_section_line.setCapStyle(Qt::FlatCap);
  cross_line_->set_pen(pen_cross_section_line);

  QPointF add_tail =
      spline_data.back() * 2 - spline_data.at(spline_data.size() - 2);
  spline_data.push_back(add_tail);

  cross_number_text_.clear();
  for (int i = 0; i < indices.size(); i++) {
    int idx = indices[i];
    QVector2D normal = QVector2D(spline_data[idx + 1] - spline_data[idx]);
    normal = QVector2D(normal.y(), -normal.x()).normalized();
    cross_line_->AddLine(spline_data[idx], normal);

    cross_number_text_[i].reset(new CW3TextItem(this));
    cross_number_text_[i]->setTextColor(ColorView::kCrossSectionSelected);
    cross_number_text_[i]->setPlainText(QString("%1").arg(index[i]));

    QPointF text_bot_right =
        cross_number_text_[i]->boundingRect().bottomRight();
    QVector2D nv = normal * length * 0.7f;
    QPointF text_pos =
        spline_data[idx] + QPointF(nv.x(), nv.y()) - text_bot_right * 0.5;
    cross_number_text_[i]->setPos(text_pos);
    cross_number_text_[i]->setVisible(false);
  }

  if (id_highlighted_cross_line_ >= 0) {
    cross_line_->SetColorSpecificLine(ColorView::kCrossSectionSelected,
                                      id_highlighted_cross_line_);
    if (cross_number_text_[id_highlighted_cross_line_])
      cross_number_text_[id_highlighted_cross_line_]->setVisible(true);
  }
}

void PanoArchItem::DrawRuler(int idx_min, int idx_max, int idx_arch_front,
                             const std::vector<int>& medium_gradation,
                             const std::vector<int>& small_gradation) {
  if (ruler_line_[0]) DeleteRulerItem();

  std::vector<QPointF> tmp_current_spline_points, current_spline_points;
  GetCurrentSplinePoints(tmp_current_spline_points);

  if (tmp_current_spline_points.size() == 0) return;

  Common::equidistanceSpline(current_spline_points, tmp_current_spline_points);

  InitRulerItem();

  if (display_mode_ == DisplayMode::PANORAMA) {
    ruler_line_[RLT_M]->set_length(kRulerLineLengthMedium * zoom_scale_);
    ruler_line_[RLT_S]->set_length(kRulerLineLengthSmall * zoom_scale_);
  }
  ruler_line_[RLT_L]->set_length(kRulerLineLengthLarge * zoom_scale_);

  std::vector<float> large_gradation_in_spline;

  float idx_spline_max = current_spline_points.size() - 1;
  int idx_origin =
      (int)(FuncNormalize(idx_min, idx_max, idx_arch_front) * idx_spline_max);
  large_gradation_in_spline.push_back(FuncNormalize(idx_min, idx_max, idx_min) *
                                      idx_spline_max);
  large_gradation_in_spline.push_back(FuncNormalize(idx_min, idx_max, idx_max) *
                                      idx_spline_max);
  large_gradation_in_spline.push_back(idx_origin);

  QPointF add_tail = current_spline_points.back() * 2 -
                     current_spline_points.at(current_spline_points.size() - 2);
  current_spline_points.push_back(add_tail);

  FuncAddLine(ruler_line_[RLT_L].get(), current_spline_points,
              large_gradation_in_spline);
  if (display_mode_ == DisplayMode::PANORAMA) {
    std::vector<float> medium_gradation_in_spline;
    for (const auto& elem : medium_gradation)
      medium_gradation_in_spline.push_back(
          FuncNormalize(idx_min, idx_max, elem) * idx_spline_max);

    std::vector<float> small_gradation_in_spline;
    for (const auto& elem : small_gradation)
      small_gradation_in_spline.push_back(
          FuncNormalize(idx_min, idx_max, elem) * idx_spline_max);

    FuncAddLine(ruler_line_[RLT_M].get(), current_spline_points,
                medium_gradation_in_spline);
    FuncAddLine(ruler_line_[RLT_S].get(), current_spline_points,
                small_gradation_in_spline);

    FuncSetText(ruler_text_[RTT_FRONT].get(), current_spline_points,
                large_gradation_in_spline[0], idx_origin,
                idx_min - idx_arch_front);
    FuncSetText(ruler_text_[RTT_BACK].get(), current_spline_points,
                large_gradation_in_spline[1], idx_origin,
                idx_max - idx_arch_front);
    FuncSetText(ruler_text_[RTT_CENTER].get(), current_spline_points,
                large_gradation_in_spline[2], idx_origin, 0);
  }
}

void PanoArchItem::SetShfitedPath(float current_spline_value) {
  DeleteCurrentSpline();

  current_spline_value_ = current_spline_value / zoom_scale_;
  SetThicknessSplinePen();
  DrawCurrentSpline();
}
void PanoArchItem::SetPanoRange(float range_value) {
  range_value = range_value / zoom_scale_;

  if (range_value_ != range_value) {
    range_value_ = range_value;
    DrawRangeItems();
  }

  if (isVisible() && adjust_spline_->is_finished()) emit sigUpdatedFinish();
}
void PanoArchItem::SetThickness(float thickness_value) {
  thickness_value_ = thickness_value;

  SetThicknessSplinePen();
  DrawCurrentSpline();

  if (isVisible() && adjust_spline_->is_finished()) emit sigUpdated();
}
void PanoArchItem::TransformItem(const QTransform& transform) {
  if (adjust_spline_) adjust_spline_->TransformItems(transform);

  if (adjust_panning_handle_)
    adjust_panning_handle_->setPos(
        transform.map(adjust_panning_handle_->pos()));

  if (cross_line_) cross_line_->TransformItems(transform);

  for (auto& elem : cross_number_text_) {
    if (elem.second) elem.second->setPos(transform.map(elem.second->pos()));
  }

  for (int i = 0; i < RLT_END; i++) {
    if (ruler_line_[i]) ruler_line_[i]->TransformItems(transform);
  }

  for (int i = 0; i < RTT_END; i++) {
    if (ruler_text_[i])
      ruler_text_[i]->setPos(transform.map(ruler_text_[i]->pos()));
  }
  float delta_scale = (transform.m11() + transform.m22()) * 0.5f;
  if (curr_spline_thickness) {
    QPen pen = curr_spline_thickness->pen();
    pen.setWidthF(pen.widthF() * delta_scale);
    curr_spline_thickness->setPen(pen);
  }

  zoom_scale_ *= delta_scale;

  if (adjust_spline_ && adjust_spline_->is_finished()) {
	DrawCurrentSpline();
	DrawRangeItems();
  }
}
void PanoArchItem::Clear() {
  adjust_spline_->clear();
  current_spline_value_ = 0.0f;

  DeleteRangePathItem();
  DeleteRangeEllipseItem();
  DeleteCurrentSpline();
  DeleteCrossSectionItem();
  DeleteRulerItem();
  this->SetAdjustMode(false);
}
void PanoArchItem::CancelLastPoint() {
  if (!IsStartEdit()) return;

  int last_index = GetCtrlPoints().size() - 1;

  if (last_index < 0) return;

  if (last_index == 0)
    Clear();
  else {
    if (id_idx_hovered_point_spline_ == last_index) {
      id_hovered_spline_ = kInvalid;
      id_idx_hovered_point_spline_ = kInvalid;
    }

    adjust_spline_->removePoint(last_index);
  }
}
bool PanoArchItem::EndEdit() {
  bool is_edit = adjust_spline_->endEdit();

  if (is_edit) {
    adjust_spline_->sortPointsOrderX();
    adjust_spline_->updateSpline();
    adjust_spline_->SetAdjustMode(false);
    SetPositionPanningHandle();
    SetThicknessSplinePen();
    DrawCurrentSpline();
    DrawRangeItems();

    if (adjust_panning_handle_.get()) {
      adjust_panning_handle_->setVisible(is_adjust_mode_);
    }

    emit sigEndEdit();
    is_adjust_mode_ = false;
  }

  return is_edit;
}

void PanoArchItem::RemoveSelectedPoint() {
  if (id_selected_point_spline_ == kInvalid) return;

  adjust_spline_->removePoint(id_selected_point_spline_);

  if (adjust_spline_->getEllipseCount() < 2) {
    Clear();
  } else {
    DrawRangeItems();
  }

  id_selected_point_spline_ = kInvalid;

  if(adjust_spline_->getCurveData().size() > 0)
	emit sigUpdatedFinish();
}

void PanoArchItem::SetSelectPointCurrentHover() {
  id_selected_point_spline_ = id_idx_hovered_point_spline_;
}

void PanoArchItem::ReleaseSelectedPoint() {
	if (!is_hovered_point_spline_)
		return;

	adjust_spline_->setHighlightEffectEllipse(id_idx_hovered_point_spline_, false);
	id_hovered_spline_ = kInvalid;
	id_idx_hovered_point_spline_ = kInvalid;
	is_hovered_point_spline_ = false;
}

/*=============================================================================================
private functions
===============================================================================================*/

void PanoArchItem::InitAdjustSplineItem() {
  adjust_spline_.reset(new CW3Spline(0, this));
  adjust_spline_->SetAdjustMode(false);
  adjust_spline_->setPenEllipse(QPen(ColorArchItem::kControlPen, 2.0));
  adjust_spline_->setBrushEllipse(QBrush(ColorArchItem::kControlBrush));
  adjust_spline_->setPenPath(QPen(ColorArchItem::kCurvePen, 2.0));
  adjust_spline_->setZValue(this->zValue());

  adjust_panning_handle_.reset(new PanningHandleItem(this));
  adjust_panning_handle_->setFlag(GraphicsItemFlag::ItemIsMovable, false);
  adjust_panning_handle_->setVisible(false);

  connect(adjust_spline_.get(), SIGNAL(sigUpdateSpline()), this,
          SLOT(slotUpdateSpline()));
  connect(adjust_spline_.get(), SIGNAL(sigMouseReleased()), this,
          SIGNAL(sigUpdatedFinish()));

  connect(adjust_spline_.get(), SIGNAL(sigHighlightEllipse(bool, int, int)),
          this, SLOT(slotHoverArchPoint(bool, int, int)));
  connect(adjust_spline_.get(), SIGNAL(sigHighlightCurve(bool, int)), this,
          SLOT(slotHoverArchSpline(bool)));
  connect(adjust_spline_.get(), SIGNAL(sigTranslatePath(QPointF, int)), this,
          SLOT(slotTranslateAdjustSpline(QPointF, int)));

  connect(adjust_panning_handle_.get(), SIGNAL(sigTranslate(QPointF)), this,
          SLOT(slotTranslatePanningHandle(QPointF)));
  connect(adjust_panning_handle_.get(), SIGNAL(sigMouseReleased()), this,
          SIGNAL(sigUpdatedFinish()));
}
void PanoArchItem::InitCurrentSpline() {
  curr_spline_.reset(new CW3PathItem(this));
  curr_spline_->setPen(QPen(ColorArchItem::kCurvePen, 2.0));
  curr_spline_->setZValue(this->zValue() - 1);
  curr_spline_->setVisible(!is_adjust_mode_);
}
void PanoArchItem::InitCurrentSplineThickness() {
  curr_spline_thickness.reset(new CW3PathItem(this));
  curr_spline_thickness->setZValue(this->zValue() - 2.0);
  curr_spline_thickness->setAcceptHoverEvents(false);
  curr_spline_thickness->setFlag(QGraphicsItem::ItemIsSelectable, false);
}
void PanoArchItem::InitRulerItem() {
  QPen pen_ruler(ColorArchItem::kCurvePen, 1.0, Qt::SolidLine);

  for (int i = 0; i < RLT_END; i++) {
    if (display_mode_ == DisplayMode::IMPLANT) {
      if (i == RLT_S || i == RLT_M) continue;
    }
    ruler_line_[i].reset(new LineListItem(this));
    ruler_line_[i]->set_pen(pen_ruler);
    ruler_line_[i]->setZValue(this->zValue() - 1);
    ruler_line_[i]->setVisible(!is_adjust_mode_);
  }

  if (display_mode_ == DisplayMode::PANORAMA) {
    for (int i = 0; i < RTT_END; i++) {
      ruler_text_[i].reset(new CW3TextItem(this));
      ruler_text_[i]->setTextColor(ColorView::kCrossSection);
      ruler_text_[i]->setPixelSize(10.0f);
      ruler_text_[i]->setVisible(!is_adjust_mode_);
    }
  }
}

void PanoArchItem::InitRangePathItem() {
  QPen pen_range_line;
  if (is_adjust_mode_) {
    pen_range_line = QPen(ColorArchItem::kCurvePen, 1.0, Qt::SolidLine);
  } else {
    pen_range_line = QPen(ColorArchItem::kCurveRangePen, 1.0, Qt::SolidLine);
  }

  for (int i = 0; i < RANGE_PATH_END; i++) {
    adjust_range_path_[i].reset(new CW3PathItem(this));
    adjust_range_path_[i]->setPen(pen_range_line);
  }
}

void PanoArchItem::InitCrossSectionItem() {
  QPen pen_cross_section_line(ColorCrossSectionItem::kNormal, 1.0,
                              Qt::SolidLine);
  pen_cross_section_line.setCapStyle(Qt::FlatCap);
  cross_line_.reset(new LineListItem(this));
  cross_line_->setZValue(this->zValue() - 1);
  cross_line_->set_pen(pen_cross_section_line);
}

void PanoArchItem::InitRangeEllipseItem() {
  for (int i = 0; i < RANGE_ELLIPSE_END; i++) {
    CW3EllipseItem* ell = new CW3EllipseItem(this);
    ell->setBrush(ColorArchItem::kControlBrush);
    ell->setPen(QPen(ColorArchItem::kControlPen, 2.0));
    ell->SetFlagHighlight(true);
    ell->SetFlagMovable(false);
    ell->setVisible(is_adjust_mode_);
    connect(ell, SIGNAL(sigTranslateEllipse(QPointF)), this,
            SLOT(slotTranslateRange(QPointF)));
    connect(ell, SIGNAL(sigMouseReleased()), this,
            SIGNAL(sigChangedArchRange()));

    adjust_range_ellipse_[i].reset(ell);
  }
}

void PanoArchItem::SetThicknessSplinePen() {
  if (thickness_value_ >= 2.0f) {
    if (curr_spline_thickness.get() == nullptr) InitCurrentSplineThickness();

    curr_spline_thickness->setPen(QPen(ColorArchItem::kCurveThicknessPen,
                                       (qreal)thickness_value_, Qt::SolidLine,
                                       Qt::FlatCap));
  } else {
    if (curr_spline_thickness.get() != nullptr) curr_spline_thickness.reset();
  }
}

void PanoArchItem::SetPositionPanningHandle() {
  std::vector<QPointF> points = adjust_spline_->getCurveData();

  double limit_max_double = std::numeric_limits<double>::max();
  double limit_min_double = std::numeric_limits<double>::min();

  QPointF minima(limit_max_double, limit_max_double);
  QPointF maxima(limit_min_double, limit_min_double);

  for (const auto& elem : points) {
    minima.setX(std::min(elem.x(), minima.x()));
    minima.setY(std::min(elem.y(), minima.y()));

    maxima.setX(std::max(elem.x(), maxima.x()));
    maxima.setY(std::max(elem.y(), maxima.y()));
  }

  adjust_panning_handle_->setPos((maxima + minima) * 0.5);
}

void PanoArchItem::DeleteRangePathItem() {
  for (int i = 0; i < RANGE_PATH_END; i++) {
    if (adjust_range_path_[i]) adjust_range_path_[i].reset();
  }
}

void PanoArchItem::DeleteRangeEllipseItem() {
  for (int i = 0; i < RANGE_ELLIPSE_END; i++) {
    if (adjust_range_ellipse_[i]) adjust_range_ellipse_[i].reset();
  }
}
void PanoArchItem::DeleteCurrentSpline() {
  curr_spline_.reset();
  curr_spline_thickness.reset();
}
void PanoArchItem::DeleteRulerItem() {
  for (int i = 0; i < RLT_END; i++) {
    ruler_line_[i].reset();
  }
  for (int i = 0; i < RTT_END; i++) {
    ruler_text_[i].reset();
  }
}
void PanoArchItem::DeleteCrossSectionItem() {
  cross_line_.reset();
  cross_number_text_.clear();
}
void PanoArchItem::DrawCurrentSpline() {
  if (curr_spline_.get() == nullptr) InitCurrentSpline();

  std::vector<QPointF> current_spline_points;
  GetCurrentSplinePoints(current_spline_points);
  curr_spline_->drawingPath(current_spline_points);

  if (thickness_value_ != 0.0f) {
    if (curr_spline_thickness.get() == nullptr) InitCurrentSplineThickness();

    curr_spline_thickness->drawingPath(current_spline_points);
  }
}
void PanoArchItem::DrawRangeItems() {
  if (display_mode_ == DisplayMode::IMPLANT) return;

  std::vector<QPointF> upper_points, lower_points;
  this->GetUpperLowerPoints(upper_points, lower_points);

  if (upper_points.size() == 0 || lower_points.size() == 0) return;

  if (adjust_range_path_[0].get() == nullptr) this->InitRangePathItem();

  if (adjust_range_ellipse_[0].get() == nullptr) this->InitRangeEllipseItem();

  adjust_range_path_[UPPER_PATH]->drawingPath(upper_points);
  adjust_range_path_[LOWER_PATH]->drawingPath(lower_points);

  adjust_range_ellipse_[UPPER_FRONT]->setPos(upper_points.front());
  adjust_range_ellipse_[UPPER_BACK]->setPos(upper_points.back());
  adjust_range_ellipse_[LOWER_FRONT]->setPos(lower_points.front());
  adjust_range_ellipse_[LOWER_BACK]->setPos(lower_points.back());
}

void PanoArchItem::GetCurrentSplinePoints(
    std::vector<QPointF>& current_spline) const {
  std::vector<QPointF> splinePoints = adjust_spline_->getData();

  if (splinePoints.empty()) return;

  QPointF addTail =
      splinePoints.back() * 2 - splinePoints.at(splinePoints.size() - 2);
  splinePoints.push_back(addTail);

  current_spline.reserve(splinePoints.size() - 1);
  float curr_value = current_spline_value_ * zoom_scale_;
  for (int i = 0; i < splinePoints.size() - 1; i++) {
    QPointF ptTemp = splinePoints.at(i) - splinePoints.at(i + 1);
    QVector2D norDir(-ptTemp.y(), ptTemp.x());
    norDir.normalize();

    QVector2D vOffset = norDir * curr_value;
    QPointF point = splinePoints.at(i);
    current_spline.push_back(
        QPointF(point.x() + vOffset.x(), point.y() + vOffset.y()));
  }
}

void PanoArchItem::GetUpperLowerPoints(
    std::vector<QPointF>& upper_points,
    std::vector<QPointF>& lower_points) const {
  std::vector<QPointF> points = adjust_spline_->getSplineData();

  if (points.size() < 3) return;

  QPointF addTail = points.back() * 2 - points.at(points.size() - 2);
  points.push_back(addTail);

  float Range = range_value_ * zoom_scale_;
  float upper_dist = Range * -0.5f + 2.0f;
  float lower_dist = Range * 0.5f - 2.0f;
  lower_points.reserve(points.size() - 1);
  upper_points.reserve(points.size() - 1);
  for (int i = 0; i < points.size() - 1; i++) {
    QPointF point = points[i];
    QPointF vec = points[i + 1] - point;
    QVector2D normal(-vec.y(), vec.x());
    normal.normalize();

    QVector2D upperVector = normal * upper_dist;
    QVector2D lowerVector = normal * lower_dist;

    upper_points.push_back(
        QPointF(point.x() + upperVector.x(), point.y() + upperVector.y()));

    lower_points.push_back(
        QPointF(point.x() + lowerVector.x(), point.y() + lowerVector.y()));
  }
}

void PanoArchItem::GetSelectedRangeEllipseDir(QVector2D& dir) const {
  QObject* sender = QObject::sender();

  auto points = adjust_spline_->getSplineData();

  if (sender == adjust_range_ellipse_[UPPER_FRONT].get())
    dir = this->GetDirection(adjust_range_ellipse_[UPPER_FRONT]->pos(),
                             points.front());
  else if (sender == adjust_range_ellipse_[UPPER_BACK].get())
    dir = this->GetDirection(adjust_range_ellipse_[UPPER_BACK]->pos(),
                             points.back());
  else if (sender == adjust_range_ellipse_[LOWER_FRONT].get())
    dir = this->GetDirection(adjust_range_ellipse_[LOWER_FRONT]->pos(),
                             points.front());
  else if (sender == adjust_range_ellipse_[LOWER_BACK].get())
    dir = this->GetDirection(adjust_range_ellipse_[LOWER_BACK]->pos(),
                             points.back());

  dir.normalize();
}

QVector2D PanoArchItem::GetDirection(const QPointF& point0,
                                     const QPointF& point1) const {
  QVector2D dir(point0.x() - point1.x(), point0.y() - point1.y());
  dir.normalize();
  return dir;
}

/*=============================================================================================
private slots
===============================================================================================*/
void PanoArchItem::slotTranslateRange(const QPointF& pt_trans) {
  QVector2D dir;
  this->GetSelectedRangeEllipseDir(dir);

  QVector2D vec_trans = QVector2D(pt_trans.x(), pt_trans.y());
  float delta_Range = QVector2D::dotProduct(dir, vec_trans) / zoom_scale_;
  float temp_Range = range_value_ * 0.5f + delta_Range;

  if (temp_Range > abs(adjust_spline_->getShiftedDist())) {
    range_value_ = 2.0f * temp_Range;
    this->DrawRangeItems();
  }
}

void PanoArchItem::slotUpdateSpline() {
  if (adjust_spline_->isStartEdit()) return;

  this->DrawCurrentSpline();
  this->DrawRangeItems();

  // if (!is_highlight_)
  //	return;

  emit sigUpdated();
}

void PanoArchItem::slotTranslateAdjustSpline(const QPointF& pt_trans,
                                             const int id) {
  adjust_panning_handle_->setPos(adjust_panning_handle_->pos() + pt_trans);
}

void PanoArchItem::slotHoverArchPoint(const bool is_hovered, const int id,
                                      const int index) {
  is_hovered_point_spline_ = is_hovered;

  if (is_hovered_point_spline_) {
    id_hovered_spline_ = id;
    id_idx_hovered_point_spline_ = index;
  } else {
    id_hovered_spline_ = kInvalid;
    id_idx_hovered_point_spline_ = kInvalid;
  }
}

void PanoArchItem::slotHoverArchSpline(const bool is_hovered) {
  is_hovered_spline_ = is_hovered;
}

void PanoArchItem::slotTranslatePanningHandle(const QPointF& pt_trans) {
  if (adjust_spline_->isStartEdit()) return;

  adjust_panning_handle_->setPos(adjust_panning_handle_->pos() + pt_trans);
  adjust_spline_->translatePath(pt_trans);
  adjust_spline_->updateSpline();

  this->DrawCurrentSpline();
  this->DrawRangeItems();

  if (!is_highlight_) return;

  emit sigUpdated();
}
