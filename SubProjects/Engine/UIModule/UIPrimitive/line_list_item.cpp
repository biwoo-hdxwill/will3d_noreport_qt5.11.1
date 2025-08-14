#include "line_list_item.h"

#include <exception>
#include <QGraphicsScene>
#include <qvector2d.h>
#include <QtMath>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3MessageBox.h"
#include "W3LineItem.h"

using namespace std;

namespace {
const int kMaxLineCount = 300;
}

////////////////////////////////////////////////////////////////////////////////////
//LineListItem
////////////////////////////////////////////////////////////////////////////////////

LineListItem::LineListItem(QGraphicsItem* parent)
	:QGraphicsItem(parent) {
	this->setFlag(QGraphicsItem::ItemIsSelectable, true);
	this->setFlag(QGraphicsItem::ItemIsMovable, false);
	pen_ = QPen(QColor(255, 0, 0), 2.0, Qt::SolidLine);
}

LineListItem::~LineListItem() {
}
void LineListItem::SetHighlight(const bool& is_highlight) {
	flag_highlight_ = is_highlight;

	for (auto& elem : lines_)
		elem.second->SetFlagHighlight(flag_highlight_);
}
void LineListItem::SetMovable(const bool& flag_movable) {
	flag_movable_ = flag_movable;

	for (auto& elem : lines_)
		elem.second->SetFlagMovable(flag_movable_);
}
void LineListItem::ClearLines() {
	lines_.clear();
	line_positions_.clear();
	line_vectors_.clear();
}

void LineListItem::AddLine(const QPointF& pos, const QVector2D& vector) {
	int new_number = GetNewLineID();

	if (new_number < 0)
		return;

	this->InitLine(new_number, pos, vector);
}

void LineListItem::SetLine(int line_id, const QPointF& position, const QVector2D& vector) {
	if (IsInvalidLineID(line_id)) {
		this->InitLine(line_id, position, vector);
	} else {
		line_positions_[line_id] = position;
		line_vectors_[line_id] = vector.normalized();

		UpdateLine(line_id);
	}
}
void LineListItem::SetRotatePointAtLine(int line_id, const QPointF& pos) {
	if (IsInvalidLineID(line_id)) {
		auto log = common::Logger::instance();
		log->Print(common::LogType::ERR, "LineListItem::SetRotatePointAtLine: invalid id.");
		return;
	}

	line_rotate_positions_[line_id] = pos;
}
void LineListItem::UpdateLine(int line_id) {
	if (IsInvalidLineID(line_id)) {
		auto log = common::Logger::instance();
		log->Print(common::LogType::ERR, "LineListItem::UpdateLine: invalid id.");
		return;
	}

	QVector2D nv = line_vectors_[line_id] * length_*0.5f;

	QPointF p1 = line_positions_[line_id] + QPointF(nv.x(), nv.y());
	QPointF p2 = line_positions_[line_id] - QPointF(nv.x(), nv.y());

	lines_[line_id]->setLine(p1.x(), p1.y(), p2.x(), p2.y());
	lines_[line_id]->setPen(pen_);
	lines_[line_id]->SetHighlightEffect(is_highlight_);
}

void LineListItem::SetVisibleSpecificLine(const bool & is_visible, int line_id) {
	if (IsInvalidLineID(line_id)) {
		auto log = common::Logger::instance();
		log->Print(common::LogType::WRN, "LineListItem::SetVisibleSpecificLine: invalid id.");
		return;
	}
	lines_[line_id]->setVisible(is_visible);
}

void LineListItem::SetColorSpecificLine(const QColor & color, int line_id) {
	if (IsInvalidLineID(line_id)) {
		auto log = common::Logger::instance();
		log->Print(common::LogType::WRN, "LineListItem::SetColorSpecificLine: invalid id.");
		return;
	}
	QPen pen = lines_[line_id]->pen();
	pen.setColor(color);

	lines_[line_id]->setPen(pen);
}

void LineListItem::TransformItems(const QTransform & transform) {
	for (auto& elem : lines_) {
		this->TransformLine(elem.first, transform);
	}
}

void LineListItem::GetLineVertex(int line_id, QPointF & p1, QPointF & p2) {
	if (IsInvalidLineID(line_id)) {
		auto log = common::Logger::instance();
		log->Print(common::LogType::WRN, "LineListItem::GetLineVertex: invalid id.");
		return;
	}

	const QLineF& line = lines_[line_id]->line();
	p1 = line.p1();
	p2 = line.p2();
}

QPointF LineListItem::GetLineCenterPosition(int line_id) {
	if (IsInvalidLineID(line_id)) {
		auto log = common::Logger::instance();
		log->Print(common::LogType::WRN, "LineListItem::GetLineCenterPosition: invalid id.");
		return QPointF();
	}

	return line_positions_[line_id];
}

QPointF LineListItem::GetLineRotatePosition(int line_id) {
	if (line_rotate_positions_.find(line_id) == line_rotate_positions_.end()) {
		return GetLineCenterPosition(line_id);
	}

	return line_rotate_positions_[line_id];
}
int LineListItem::GetHoveredLineID() const {
	for (const auto& elem : lines_) {
		if (elem.second->isHovered())
			return elem.first;
	}

	return -1;
}
void LineListItem::TransformLine(int line_id, const QTransform& transform) {
	float scale = (transform.m11() + transform.m22())*0.5f;
	QPen pen = lines_[line_id]->pen();
	pen.setWidthF(pen.widthF()*scale);
	lines_[line_id]->setPen(pen);

	QLineF old_line = lines_[line_id]->line();
	QLineF new_line = QLineF(transform.map(old_line.p1()), transform.map(old_line.p2()));
	lines_[line_id]->setLine(new_line);

	line_positions_[line_id] = (QPointF)(new_line.p1() + new_line.p2())*0.5f;
	line_vectors_[line_id] = (QVector2D)(new_line.p1() - new_line.p2());

	line_rotate_positions_[line_id] = transform.map(line_rotate_positions_[line_id]);
}

int LineListItem::GetNewLineID() const {
	int new_line_id = 0;

	for (int id = new_line_id; id < kMaxLineCount; id++) {
		if (lines_.find(id) == lines_.end())
			return id;
	}

	if (lines_.size() >= kMaxLineCount) {
		auto log = common::Logger::instance();
		log->Print(common::LogType::WRN, "create curve more than %1.");
	}

	return -1;
}

void LineListItem::InitLine(int line_id, const QPointF& pos, const QVector2D& vector) {
	if (!IsInvalidLineID(line_id)) {
		auto log = common::Logger::instance();
		log->Print(common::LogType::WRN, "LineListItem::InitLine: already line id.");
		return;
	}

	CW3LineItem* line = new CW3LineItem(line_id, this);
	connect(line, SIGNAL(sigHighLightEvent(bool)), this, SLOT(slotHighLightEventLine(bool)));
	connect(line, SIGNAL(sigHoveredEvent(bool)), this, SLOT(slotHoveredEvent(bool)));
	connect(line, SIGNAL(sigTranslateLine(QPointF, int)), this, SLOT(slotTranslateLine(QPointF, int)));
	connect(line, SIGNAL(sigMouseReleased()), this, SIGNAL(sigMouseReleased()));

	line->SetFlagHighlight(flag_highlight_);
	line->SetFlagMovable(flag_movable_);
	line->setPen(pen_);

	line_positions_[line_id] = pos;
	line_vectors_[line_id] = vector.normalized();
	lines_[line_id].reset(line);

	UpdateLine(line_id);
}

bool LineListItem::IsInvalidLineID(int line_id) {
	if (lines_.find(line_id) == lines_.end())
		return true;
	else
		return false;
}
void LineListItem::slotHighLightEventLine(bool is_highlight) {
	for (auto& elem : lines_)
		elem.second->SetHighlightEffect(is_highlight);

	is_highlight_ = is_highlight;
}
void LineListItem::slotHoveredEvent(bool is_hovered) {
	is_hovered_ = is_hovered;
}

QRectF LineListItem::boundingRect() const {
	return QRectF();
}

void LineListItem::slotTranslateLine(const QPointF& trans, int id) {
	if (flag_movable_) {
		QTransform transform;
		transform.translate(trans.x(), trans.y());

		for (auto& elem : lines_) {
			if (elem.second->id() == id)
				continue;
			
			TransformLine(elem.first, transform);
		}
	}

	emit sigTranslateLines(trans);
}
