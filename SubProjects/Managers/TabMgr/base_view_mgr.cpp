#include "base_view_mgr.h"

#include "../../Engine/Common/Common/W3Logger.h"
#include "../../Engine/UIModule/UIComponent/view.h"
BaseViewMgr::BaseViewMgr(QObject* parent) : QObject(parent) {}
BaseViewMgr::~BaseViewMgr() {}

/**=================================================================================================
public functions
*===============================================================================================**/

void BaseViewMgr::SetCommonToolOnce(const common::CommonToolTypeOnce& type,
                                    bool on) {
  for (auto& elem : casted_views_) {
    elem.second.lock()->SetCommonToolOnce(type, on);
  }
}

void BaseViewMgr::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) {
  for (auto& elem : casted_views_) {
    elem.second.lock()->SetCommonToolOnOff(type);
  }
}

void BaseViewMgr::DeleteMeasureUI(const common::ViewTypeID& view_type,
                                  const unsigned int& measure_id) {
  for (auto& elem : casted_views_) {
    View* view = elem.second.lock().get();
    if (view->view_type() != view_type) continue;

    view->SyncDeleteMeasureUI(measure_id);
  }
}
void BaseViewMgr::GetMeasureParamsInView(
    const common::ViewTypeID& view_type, const unsigned int& measure_id,
    common::measure::VisibilityParams* visibility_params) {
  for (auto& elem : casted_views_) {
    View* view = elem.second.lock().get();
    if (view->view_type() == view_type) {
      view->GetMeasureParams(view_type, measure_id, visibility_params);
      return;
    }
  }
}

void BaseViewMgr::SetCastedView(unsigned int view_id,
                                const std::shared_ptr<View>& view) {
  if (casted_views_.find(view_id) != casted_views_.end()) {
    common::Logger::instance()->PrintAndAssert(
        common::LogType::ERR, "BaseViewMgr::SetCastedView : already exsist.");
  }

  casted_views_[view_id] = view;
}

void BaseViewMgr::SetRenderModeQuality(bool is_high_quality) {
  for (auto& elem : casted_views_) {
    elem.second.lock()->SetRenderModeQuality(is_high_quality);
  }
}
void BaseViewMgr::SetVisibleViews(bool visible) {
  for (auto& elem : casted_views_) {
    elem.second.lock()->setVisible(visible);
  }
}

void BaseViewMgr::ApplyPreferences() {
  for (auto& elem : casted_views_) {
    elem.second.lock()->ApplyPreferences();
  }
}

/**=================================================================================================
Private functions
*===============================================================================================**/

/**=================================================================================================
Private slots
*===============================================================================================**/
