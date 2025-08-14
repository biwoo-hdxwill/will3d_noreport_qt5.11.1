#include "pano_nerve_item.h"

#include "../../Common/Common/color_will3d.h"
#include <assert.h>
#include <QGraphicsSceneMouseEvent>

#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3MessageBox.h"
namespace {
	const int kMaxNerveCount = 20;
	const int kLinePenWidth = 1;
	const int kEllipsePenWidth = 1;

	const float kVisibleOffOpacity = 0.002f;
	const float kVisibleOnOpacity = 1.0f;

	const int kInvalid = -1;
}
PanoNerveItem::PanoNerveItem() {
	id_curr_nerve_ = kInvalid;
	id_selected_nerve_ = kInvalid;
	idx_saved_hovered_pt_ = kInvalid;
	id_hovered_nerve_ = kInvalid;
	idx_hovered_pt_ = kInvalid;
}

PanoNerveItem::~PanoNerveItem() {
}

/**=================================================================================================
public functions
*===============================================================================================**/

void PanoNerveItem::SetHover(int id, bool is_hover) {
	if (nerves_.find(id) == nerves_.end()) {
		auto logger = common::Logger::instance();
		logger->Print(common::LogType::ERR, "PanoNerveItem::SetHover: invalid id.");
		assert(false);
	}
	if (status_focus_ell_.focus && status_focus_ell_.nerve_index == id)
		return;

	if (is_hover)
		nerves_[id]->setOpacity(kVisibleOnOpacity);
	else
		nerves_[id]->setOpacity(kVisibleOffOpacity);
}

void PanoNerveItem::SetVisible(int id, bool is_visible) {
	if (nerves_.find(id) == nerves_.end()) {
		auto logger = common::Logger::instance();
		logger->Print(common::LogType::ERR, "PanoNerveItem::SetVisible: invalid id.");
		assert(false);
	}
	nerves_[id]->setVisible(is_visible);
}
void PanoNerveItem::SetVisibleAll(bool is_visible) {
	for (const auto& elem : nerves_) {
		elem.second->setVisible(is_visible);
	}
}

void PanoNerveItem::ReleaseSelectedNerve() {
	if (id_selected_nerve_ == kInvalid)
		return;

	if (nerves_.find(id_selected_nerve_) != nerves_.end()) {
		nerves_[id_selected_nerve_]->setOpacity(kVisibleOffOpacity);
	}

	if (status_focus_ell_.focus) {
		status_focus_ell_.focus = false;
		this->SetFocusEllipse(status_focus_ell_);
	}
	id_selected_nerve_ = kInvalid;

	emit sigPressedNerveEllipse(-1, -1, false);
}

void PanoNerveItem::SetHighlight(const bool& is_highlight) {
	is_hightlight_ = is_highlight;
	for (auto& elem : nerves_)
		elem.second->setHighligtedEllipse(is_highlight);
}

void PanoNerveItem::TransformItem(const QTransform& transform) {
	for (const auto& elem : nerves_) {
		elem.second->TransformItems(transform);
	}
}

void PanoNerveItem::AddPoint(const QPointF& point) {
	if (id_selected_nerve_ != kInvalid ||
		!IsEventAddNerve()) {
		return;
	}

	if (!IsEdit()) {
		if (!IsMaximumNerve()) {
			InsertNerve();
		} else {
			CW3MessageBox msg_box("Will3D",
								  QString("You can not create curve more than %1.")
								  .arg(kMaxNerveCount).toStdString().c_str(),
								  CW3MessageBox::MessageType::Information);
			msg_box.exec();
			return;
		}
	}

	bool is_added = CurrentNerve().addPoint(point);
	this->DrawCurrentNerve();
	if (is_added) {
		emit sigAddNerveEllipse(id_curr_nerve_, point);
	}
}
void PanoNerveItem::SetControlPoints(const std::map<int, std::vector<QPointF>>& control_points) {

	for (const auto& elem : control_points) {
		auto iter = nerves_.find(elem.first);
		if (iter == nerves_.end()) {

			CreateNerve(elem.first);

			for (const auto& pt : elem.second) {
				nerves_[elem.first]->addPoint(pt);
			}


			bool is_success = nerves_[elem.first]->endEdit();
			if (is_success) {
				nerves_[elem.first]->setHighligtedPath(false);
				nerves_[elem.first]->setOpacity(kVisibleOffOpacity);
				nerves_[elem.first]->updateSpline();
			} else {
				Clear(elem.first);
			}

		} else {

			qreal opacity = iter->second->opacity();
			bool is_end_edit = iter->second->is_finished();

			if (iter->second->getCurveData().size() == elem.second.size()) {

				for (int i = 0; i < elem.second.size(); i++) {
					const QPointF& pt = elem.second.at(i);
					iter->second->editPoint(i, elem.second.at(i));
				}
			} else {
				iter->second->clear();

				for (const auto& pt : elem.second) {
					iter->second->addPoint(pt);
				}
			}
			if (is_end_edit) {
				iter->second->endEdit();
				iter->second->setHighligtedPath(false);
				iter->second->setOpacity(opacity);
			}
			iter->second->updateSpline();
		}
	}

	this->SetFocusEllipse(status_focus_ell_);
}
const std::vector<QPointF>& PanoNerveItem::GetControlPoints(int id) const {
	if (nerves_.find(id) == nerves_.end()) {
		auto logger = common::Logger::instance();
		logger->Print(common::LogType::ERR, "PanoNerveItem::GetControlPoints: invalid id.");
		assert(false);
	}

	return nerves_.at(id)->getCurveData();
}
void PanoNerveItem::GetNerveIDs(std::vector<int>* ids) const {
	ids->clear();
	ids->reserve(nerves_.size());
	for (const auto& elem : nerves_)
		ids->push_back(elem.first);
}
void PanoNerveItem::DrawCurrentNerve() {
	if (!IsEdit())
		return;

	CurrentNerve().drawingCurPath();
}
void PanoNerveItem::DrawCurrentNerve(const QPointF& point) {
	if (!IsEdit())
		return;

	CurrentNerve().drawingCurPath(point);
}
void PanoNerveItem::CancelCurrentNerve() {
	if (!IsEdit())
		return;
	Clear(id_curr_nerve_);
}
void PanoNerveItem::Clear(int id) {
	auto iter = nerves_.find(id);
	if (iter != nerves_.end()) {
		nerves_[id].reset(nullptr);
		nerves_.erase(iter);

		if(id == id_curr_nerve_)
			id_curr_nerve_ = kInvalid;

		if (id == id_selected_nerve_)
			id_selected_nerve_ = kInvalid;

		is_hovered_point_nerve_ = is_hovered_nerve_ = false;
		emit sigClearNerve(id);
	}
}
void PanoNerveItem::ClearAll() {
	while (nerves_.size()) {
		auto iter = nerves_.begin();
		iter->second.reset(nullptr);
		nerves_.erase(iter);
	}

	id_curr_nerve_ = kInvalid;
	is_hovered_point_nerve_ = is_hovered_nerve_ = false;
}
void PanoNerveItem::CancelLastPoint(int id) {
	if (!IsEdit() || nerves_.find(id) == nerves_.end())
		return;

	int last_index = nerves_[id]->getCurveData().size() - 1;

	if (last_index < 0)
		return;

	if (last_index == 0)
		Clear(id);
	else {
		if (idx_hovered_pt_ == last_index) {
			id_hovered_nerve_ = kInvalid;
			idx_hovered_pt_ = kInvalid;
		}

		nerves_[id]->removePoint(last_index);
		emit sigCancelLastNerveEllipse(id, last_index);
	}
}
void PanoNerveItem::RemoveSelectedPoint(int id) {
	if (idx_saved_hovered_pt_ == kInvalid)
		return;

	nerves_[id]->removePoint(idx_saved_hovered_pt_);

	if (nerves_[id]->getEllipseCount() > 1) {
		emit sigRemoveNerveEllipse(id, idx_saved_hovered_pt_);
	} else {
		Clear(id);
	}

	idx_saved_hovered_pt_ = kInvalid;
}
void PanoNerveItem::InsertCloserPoint(int id, const QPointF& pt_scene) {
	auto inserted_info = nerves_[id]->insertCloserPoint(pt_scene);
	nerves_[id]->setOpacity(nerves_[id]->opacity());

	if (id_selected_nerve_ != id) {
		nerves_[id]->setOpacity(kVisibleOffOpacity);
		id_selected_nerve_ = id;

		nerves_[id_selected_nerve_]->setOpacity(kVisibleOnOpacity);
	}

	int ell_index = inserted_info.first;

	if (status_focus_ell_.focus &&
		(status_focus_ell_.nerve_index != id_selected_nerve_ ||
		status_focus_ell_.ellipse_inex != ell_index)) {
		status_focus_ell_.focus = false;
		this->SetFocusEllipse(status_focus_ell_);
	}

	status_focus_ell_.nerve_index = id_selected_nerve_;
	status_focus_ell_.ellipse_inex = ell_index;
	status_focus_ell_.focus = true;
	this->SetFocusEllipse(status_focus_ell_);

	emit sigInserteNerveEllipse(id_selected_nerve_, inserted_info.first, pt_scene);
	emit sigPressedNerveEllipse(id_selected_nerve_, ell_index, true);
}

bool PanoNerveItem::EndEdit() {
	if (!IsEdit())
		return false;

	bool is_success = CurrentNerve().endEdit();

	if (is_success) {
		CurrentNerve().setHighligtedPath(false);
		CurrentNerve().updateSpline();
		CurrentNerve().setOpacity(kVisibleOffOpacity);
	} else {
		Clear(id_curr_nerve_);
	}

	return is_success;
}

bool PanoNerveItem::IsEdit() const {
	if (id_curr_nerve_ == kInvalid)
		return false;

	if (CurrentNerve().isStartEdit())
		return true;
	else
		return false;
}

bool PanoNerveItem::IsHovered() const {
	return (IsHoveredLine() | IsHoveredPoint());
}
bool PanoNerveItem::IsEventAddNerve() const {
	if (id_curr_nerve_ != kInvalid &&
		id_hovered_nerve_ == id_curr_nerve_) {
		bool is_hovered_nerve = (is_hovered_nerve_ || is_hovered_point_nerve_) ? true : false;

		return !is_hovered_nerve;
	} else {
		return true;
	}
}

bool PanoNerveItem::IsMaximumNerve() const {
	if (nerves_.size() >= kMaxNerveCount)
		return true;
	else
		return false;
}
int PanoNerveItem::GetAvailableNerveID() {
	if (!IsEdit() && !IsMaximumNerve()) {
		InsertNerve();
	}

	return id_curr_nerve_;
}

void PanoNerveItem::SaveCurrentHoveredPointIndex() {
	idx_saved_hovered_pt_ = idx_hovered_pt_;
}

/**=================================================================================================
private slots
*===============================================================================================**/

void PanoNerveItem::slotTranslatedNerveEllipse(const QPointF& pos) {
	if (!is_hightlight_)
		return;

	for (const auto& elem : nerves_) {
		if (QObject::sender() == (QObject*)elem.second.get() &&
			elem.second->is_finished()) {
			const int selected_index = elem.second->getSelectedIndex();
			emit sigTranslatedNerveEllipse(elem.first, selected_index, pos);
			break;
		}
	}
}
void PanoNerveItem::slotHoveredNerveSpline(const bool is_hovered) {
	if (!is_hightlight_)
		return;

	is_hovered_nerve_ = is_hovered;
	id_hovered_nerve_ = GetEventSenderNerveID();

	if (id_selected_nerve_ != id_hovered_nerve_)
		SetHoverEvent(is_hovered);
}
void PanoNerveItem::slotHoveredNervePoint(const bool is_hovered, const int id, const int index) {
	if (!is_hightlight_)
		return;

	is_hovered_point_nerve_ = is_hovered;

	if (is_hovered_point_nerve_) {
		id_hovered_nerve_ = id;
		idx_hovered_pt_ = index;
	} else {
		id_hovered_nerve_ = kInvalid;
		idx_hovered_pt_ = kInvalid;
	}

	if (id_selected_nerve_ != id)
		SetHoverEvent(is_hovered);
}
void PanoNerveItem::slotMousePressedNerve() {
	if (!is_hightlight_ || IsEdit())
		return;
	if (id_hovered_nerve_ == kInvalid)
		return;

	if (id_selected_nerve_ != id_hovered_nerve_) {
		if(nerves_.find(id_selected_nerve_) != nerves_.end())
			nerves_[id_selected_nerve_]->setOpacity(kVisibleOffOpacity);

		id_selected_nerve_ = id_hovered_nerve_;
		nerves_[id_selected_nerve_]->setOpacity(kVisibleOnOpacity);
	}

	if (id_selected_nerve_ == kInvalid || !nerves_[id_selected_nerve_]->isSelectPoints())
		return;

	int selected_ell_index = nerves_[id_selected_nerve_]->getSelectedIndex();

	if (selected_ell_index == kInvalid)
		return;

	if (status_focus_ell_.focus &&
		(status_focus_ell_.nerve_index != id_selected_nerve_ ||
		status_focus_ell_.ellipse_inex != selected_ell_index)) {
		status_focus_ell_.focus = false;
		this->SetFocusEllipse(status_focus_ell_);
	}

	status_focus_ell_.nerve_index = id_selected_nerve_;
	status_focus_ell_.ellipse_inex = selected_ell_index;
	status_focus_ell_.focus = true;
	this->SetFocusEllipse(status_focus_ell_);

	emit sigPressedNerveEllipse(id_selected_nerve_, selected_ell_index, true);
}

/**=================================================================================================
private functions
*===============================================================================================**/

void PanoNerveItem::SetFocusEllipse(const StatusFocusEllipse& status_focus_ell) {
	auto iter = nerves_.find(status_focus_ell.nerve_index);
	if (iter == nerves_.end())
		return;
	if (iter->second->getEllipseCount() <= status_focus_ell.ellipse_inex ||
		status_focus_ell.ellipse_inex < 0)
		return;

	iter->second->setHighlightEffectEllipse(status_focus_ell.ellipse_inex,
																		status_focus_ell.focus);
	iter->second->setHighlightFlagEllipse(status_focus_ell.ellipse_inex,
																	  !status_focus_ell.focus);
}
void PanoNerveItem::SetHoverEvent(const bool is_hovered) {
	if (IsEdit())
		return;

	int sender_id = GetEventSenderNerveID();

	const auto& nerve = nerves_[sender_id];

	if (is_hovered)
		nerve->setOpacity(kVisibleOnOpacity);
	else
		nerve->setOpacity(kVisibleOffOpacity);

	id_curr_nerve_ = sender_id;
}

int PanoNerveItem::GetEventSenderNerveID() const {
	for (const auto& elem : nerves_) {
		if (QObject::sender() == (QObject*)elem.second.get()) {
			return elem.first;
		}
	}

	return -1;
}
void PanoNerveItem::InsertNerve() {
	for (int i = 0; i < kMaxNerveCount; i++) {
		if (nerves_.find(i) == nerves_.end()) {
			this->CreateNerve(i);
			id_curr_nerve_ = i;
			break;
		}
	}
}

void PanoNerveItem::CreateNerve(int id) {
	nerves_[id] = std::unique_ptr<CW3Spline>(new CW3Spline(id, this));
	nerves_[id]->setPenEllipse(QPen(ColorNerveItem::kEllipsePenColor, kEllipsePenWidth, Qt::SolidLine));
	nerves_[id]->setBrushEllipse(QBrush(ColorNerveItem::kEllipseBrushColor));
	nerves_[id]->setPenPath(QPen(ColorNerveItem::kLinePenColor, kLinePenWidth, Qt::SolidLine));
	nerves_[id]->setHighligtedEllipse(true);
	nerves_[id]->startEdit();

	connect(nerves_[id].get(), SIGNAL(sigTranslatedEllipse(QPointF, int)), this, SLOT(slotTranslatedNerveEllipse(QPointF)));
	connect(nerves_[id].get(), SIGNAL(sigHighlightCurve(bool, int)), this, SLOT(slotHoveredNerveSpline(bool)));
	connect(nerves_[id].get(), SIGNAL(sigHighlightEllipse(bool, int, int)), this, SLOT(slotHoveredNervePoint(bool, int, int)));
	connect(nerves_[id].get(), SIGNAL(sigMousePressed()), this, SLOT(slotMousePressedNerve()));
}

CW3Spline& PanoNerveItem::CurrentNerve() const {
	if (id_curr_nerve_ == kInvalid) {
		auto logger = common::Logger::instance();
		logger->Print(common::LogType::ERR, "PanoNerveItem::CurrentNerve: unknown id.");
		assert(false);
	}

	return *(nerves_.at(id_curr_nerve_));
}
