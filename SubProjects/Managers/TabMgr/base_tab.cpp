#include "base_tab.h"

#include <assert.h>

#include <QGuiApplication>
#include <QLayout>
#include <QScreen>
#include <QWidget>
#include <QWindow>

#include "base_view_mgr.h"
int BaseTab::kLayoutSpacing = 1;

BaseTab::BaseTab(QObject* parent) : QObject(parent) {}

BaseTab::~BaseTab(void) {}

void BaseTab::SetCommonToolOnce(const common::CommonToolTypeOnce& type, bool on) {
  if (view_mgr_.lock() == nullptr) return;

  view_mgr_.lock()->SetCommonToolOnce(type, on);
}

void BaseTab::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) {
  if (view_mgr_.lock() == nullptr) return;

  view_mgr_.lock()->SetCommonToolOnOff(type);
}

void BaseTab::DeleteMeasureUI(const common::ViewTypeID& view_type,
                              const unsigned int& measure_id) {
  if (view_mgr_.lock() == nullptr) return;

  view_mgr_.lock()->DeleteMeasureUI(view_type, measure_id);
}

void BaseTab::ApplyPreferences() {
  if (view_mgr_.lock() == nullptr) return;

  view_mgr_.lock()->ApplyPreferences();
}

QLayout* BaseTab::GetTabLayout() {
  if (!initialized_) this->Initialize();

  return tab_layout_;
}

void BaseTab::SetRenderModeQuality(bool is_high_quality) {
  if (view_mgr_.lock() == nullptr) return;

  view_mgr_.lock()->SetRenderModeQuality(is_high_quality);
}

QImage BaseTab::GetScreenshot(QWidget* source) {
  QRect viewRect;
  QPointF viewGlobalRightBot;
  QPointF viewGlobalTopLeft;

  if (source) {
    viewRect = source->rect();
    viewGlobalRightBot =
        source->mapToGlobal(QPoint(viewRect.width(), viewRect.height()));
    viewGlobalTopLeft = source->mapToGlobal(QPoint(0, 0));
  } else {
    assert(false);
  }

  return GetScreenshot(viewGlobalTopLeft.x(), viewGlobalTopLeft.y(),
                       viewGlobalRightBot.x() - viewGlobalTopLeft.x(),
                       viewGlobalRightBot.y() - viewGlobalTopLeft.y());
}

QStringList BaseTab::GetViewList() { return QStringList(); }

QImage BaseTab::GetScreenshot(int view_type) { return QImage(); }

QWidget* BaseTab::GetScreenshotSource(int view_type)
{
	return nullptr;
}

void BaseTab::MoveViewsToSelectedMeasure(const common::ViewTypeID& view_type,
                                         const unsigned int& measure_id) {
  view_mgr_.lock()->MoveViewsToSelectedMeasure(view_type, measure_id);
}

QImage BaseTab::GetScreenshot(int x, int y, int w, int h) {
  QScreen* screen = QGuiApplication::primaryScreen();
  if (!screen) throw std::runtime_error("not found screen");

  QPixmap screenshot = screen->grabWindow(0, x, y, w, h);
  return screenshot.toImage();
}

void BaseTab::SetCastedViewMgr(const std::shared_ptr<BaseViewMgr>& view_mgr) {
  view_mgr_ = view_mgr;
}
