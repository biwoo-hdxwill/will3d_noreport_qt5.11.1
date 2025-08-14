#include "W3PolygonItem_anno.h"

#include <QWidget>
#include <QGraphicsScene>
#include <QApplication>

#include "../../Common/Common/global_preferences.h"
#include <Engine/Common/Common/W3Memory.h>
#include <Engine/Common/Common/W3Math.h>
#include <Engine/Common/Common/define_ui.h>

#include "W3RectItem_anno.h"
#include "W3EllipseItem_anno.h"
#include "W3EllipseItem.h"
#include "W3PathItem.h"
#include "W3LabelItem_anno.h"

using namespace	common;

CW3PolygonItem_anno::CW3PolygonItem_anno(QGraphicsScene* pScene, measure::Shape eShapeType)
	: m_pgScene(pScene), m_eShapeType(eShapeType) {
}

CW3PolygonItem_anno::~CW3PolygonItem_anno(void) {
	for (auto &i : node_list_)
		SAFE_DELETE_OBJECT(i);
	node_list_.clear();
	SAFE_DELETE_OBJECT(ui_rect_);
	SAFE_DELETE_OBJECT(ui_circle_);
	SAFE_DELETE_OBJECT(label_);
	SAFE_DELETE_OBJECT(ui_label_guide_);
}

int CW3PolygonItem_anno::getSelectedNode() const {
	for (int i = 0; i < node_list_.size(); i++) {
		if (node_list_[i]->isSelected())
			return i;
	}
	return -1;
}

bool CW3PolygonItem_anno::isLineSelected() const {
	if (m_eShapeType == measure::Shape::RECT) {
		return ui_rect_->isSelected();
	} else if (m_eShapeType == measure::Shape::CIRCLE) {
		return ui_circle_->isSelected();
	}
	return false;
}

bool CW3PolygonItem_anno::isLabelSelected() const {
	return label_ ? label_->isSelected() : false;
}

void CW3PolygonItem_anno::AddPoint(const QPointF& curr_pt) {
	if ((m_eShapeType == measure::Shape::RECT && ui_rect_ == nullptr) ||
		(m_eShapeType == measure::Shape::CIRCLE && ui_circle_ == nullptr)) {
		start_pt_ = curr_pt;
		m_bStartEdit = true;
		if (m_eShapeType == measure::Shape::RECT) {
			ui_rect_ = new CW3RectItem_anno(curr_pt);
			ui_rect_->setZValue(51.0f);
			ui_rect_->setRect(curr_pt.x(), curr_pt.y(), 100, 100);
			m_pgScene->addItem(ui_rect_);
		} else if (m_eShapeType == measure::Shape::CIRCLE) {
			ui_circle_ = new CW3EllipseItem_anno(curr_pt);
			ui_circle_->setZValue(51.0f);
			ui_circle_->setRect(curr_pt.x(), curr_pt.y(), 100, 100);
			m_pgScene->addItem(ui_circle_);
		}

		ApplyLineColor();

		for (int i = 0; i < 4; i++) {
			CW3EllipseItem *pNode = new CW3EllipseItem(curr_pt);
			pNode->setZValue(52.0f);
			pNode->setVisible(m_bShowNodes);
			pNode->setPen(QPen(Qt::green, 2, Qt::SolidLine));
			pNode->setBrush(Qt::red);
			node_list_.push_back(pNode);
			node_list_[i]->setVisible(true);
			m_pgScene->addItem(node_list_[i]);
		}

		ApplyNodeColor();
	} else {
		QPointF end_pt = curr_pt;
		if (m_eShapeType == measure::Shape::RECT || m_eShapeType == measure::Shape::CIRCLE) {
			float fTemp = 0.0f;
			if (start_pt_.x() > end_pt.x()) {
				fTemp = start_pt_.x();
				start_pt_.setX(end_pt.x());
				end_pt.setX(fTemp);
			}

			if (start_pt_.y() > end_pt.y()) {
				fTemp = start_pt_.y();
				start_pt_.setY(end_pt.y());
				end_pt.setY(fTemp);
			}
		}
		if (m_eShapeType == measure::Shape::RECT)
			ui_rect_->setRect(start_pt_.x(), start_pt_.y(), end_pt.x() - start_pt_.x(), end_pt.y() - start_pt_.y());
		if (m_eShapeType == measure::Shape::CIRCLE)
			ui_circle_->setRect(start_pt_.x(), start_pt_.y(), end_pt.x() - start_pt_.x(), end_pt.y() - start_pt_.y());

		m_bStartEdit = false;

		node_list_[0]->setPos(start_pt_.rx(), start_pt_.ry());
		node_list_[1]->setPos(end_pt.rx(), start_pt_.ry());
		node_list_[2]->setPos(end_pt.rx(), end_pt.ry());
		node_list_[3]->setPos(start_pt_.rx(), end_pt.ry());

		displayNode(true);

		DrawLabelUI();
	}
}

void CW3PolygonItem_anno::drawingCurPath(const QPointF& curr_pt) {
	if (m_bStartEdit) {
		node_list_[0]->setPos(start_pt_.rx(), start_pt_.ry());
		node_list_[1]->setPos(curr_pt.x(), start_pt_.ry());
		node_list_[2]->setPos(curr_pt.x(), curr_pt.y());
		node_list_[3]->setPos(start_pt_.rx(), curr_pt.y());

		if (m_eShapeType == measure::Shape::RECT) {
			ui_rect_->setRect(start_pt_.rx(), start_pt_.ry(),
							 curr_pt.x() - start_pt_.rx(),
							 curr_pt.y() - start_pt_.ry());
		} else if (m_eShapeType == measure::Shape::CIRCLE) {
			ui_circle_->setRect(start_pt_.rx(), start_pt_.ry(),
							   curr_pt.x() - start_pt_.rx(),
							   curr_pt.y() - start_pt_.ry());
		}

		DrawLabelUI();
	}
}

void CW3PolygonItem_anno::slotTranslateRect(const QPointF& ptTrans) {
	node_list_[0]->setPos(node_list_[0]->pos() + ptTrans);
	node_list_[1]->setPos(node_list_[1]->pos() + ptTrans);
	node_list_[2]->setPos(node_list_[2]->pos() + ptTrans);
	node_list_[3]->setPos(node_list_[3]->pos() + ptTrans);

	DrawLabelUI();
}

void CW3PolygonItem_anno::slotTranslateEllipse(const QPointF& ptTrans) {
	// smseo : 2번이 selected status 상태가 계속 유지되어 
	// selected status 를 update 하는 작업을 진행함.
	QObject* sender = QObject::sender();
	int selected_idx = -1;
	for (int i = 0; i < node_list_.size(); i++) {
		if (node_list_[i] == sender) {
			selected_idx = i;
			node_list_[i]->setSelected(true);
		} else {
			node_list_[i]->setSelected(false);
		}
	}
	if (selected_idx == -1)
		return;

	switch (selected_idx) {
	case 0:
		node_list_[1]->setY(node_list_[0]->pos().y());
		node_list_[3]->setX(node_list_[0]->pos().x());
		break;
	case 1:
		node_list_[0]->setY(node_list_[1]->pos().y());
		node_list_[2]->setX(node_list_[1]->pos().x());
		break;
	case 2:
		node_list_[1]->setX(node_list_[2]->pos().x());
		node_list_[3]->setY(node_list_[2]->pos().y());
		break;
	case 3:
		node_list_[0]->setX(node_list_[3]->pos().x());
		node_list_[2]->setY(node_list_[3]->pos().y());
		break;
	}
	start_pt_ = node_list_[0]->pos();
	QPointF end_pt = node_list_[2]->pos();

	if (m_eShapeType == measure::Shape::RECT) {
		ui_rect_->setPos(QPointF(0, 0));
		ui_rect_->setRect(start_pt_.rx(), start_pt_.ry(),
						 end_pt.rx() - start_pt_.rx(),
						 end_pt.ry() - start_pt_.ry());
	} else if (m_eShapeType == measure::Shape::CIRCLE) {
		ui_circle_->setPos(QPointF(0, 0));
		ui_circle_->setRect(start_pt_.rx(), start_pt_.ry(),
						   end_pt.rx() - start_pt_.rx(),
						   end_pt.ry() - start_pt_.ry());
	}
}

void CW3PolygonItem_anno::slotTranslateLabel(const QPointF& ptTrans) {
	if (label_) {
		m_ptLabelPosOffset += ptTrans;
		ui_label_guide_->setPos(0, 0);
		drawingGuide();
	}
}

void CW3PolygonItem_anno::slotTranslateGuide(const QPointF& ptTrans) {
	QPointF curr_pos = node_list_[0]->pos() + ptTrans;

	if (m_eShapeType == measure::Shape::RECT)
		ui_rect_->setPos(curr_pos.x(), curr_pos.y());
	else if (m_eShapeType == measure::Shape::CIRCLE)
		ui_circle_->setPos(curr_pos.x(), curr_pos.y());
	drawingLabel();
}

void CW3PolygonItem_anno::slotHighlightRect(bool bHover) {
	for (auto &i : node_list_)
		i->SetHighlight(bHover);
}

bool CW3PolygonItem_anno::endEdit() {
	m_bStartEdit = false;

	if (node_list_.size() > 1) {
		for (auto &i : node_list_){
			i->SetFlagHighlight(true);
			i->SetFlagMovable(true);
		}

		graphicItemsConnection();
		return true;
	} else {
		return false;
	}
}

void CW3PolygonItem_anno::setLabel(const QString& str) {
	m_bUseLabel = true;

	if (!label_) {
		label_ = new CW3LabelItem_anno();
		label_->setZValue(53.0f);
		
		ApplyTextColor();
		ApplyTextSize();

		ui_label_guide_ = new CW3PathItem();
		ui_label_guide_->setZValue(50.0f);
		ui_label_guide_->setLineStatic(true);
		ui_label_guide_->setMovable(false);
		ui_label_guide_->setPen(QPen(Qt::yellow, common::ui_define::kToolLineWidth, Qt::DotLine, Qt::FlatCap));
		m_pgScene->addItem(ui_label_guide_);
		m_pgScene->addItem(label_);
		connect(label_, SIGNAL(sigTranslateLabel(QPointF)), this, SLOT(slotTranslateLabel(QPointF)));
		connect(label_, SIGNAL(sigHoverLabel(bool)), this, SLOT(slotHighlightRect(bool)));
	}

	label_->setPlainText(str);

	drawingLabel();
	drawingGuide();
}

QString CW3PolygonItem_anno::getLabel() {
	return label_ ? label_->toPlainText() : QString();
}

bool CW3PolygonItem_anno::isSelected() const {
	if (getSelectedNode() != -1 || isLineSelected() || isLabelSelected())
		return true;
	return false;
}

QRectF CW3PolygonItem_anno::getRect() {
	QPointF start = node_list_[0]->pos();
	QPointF end = node_list_[2]->pos();
	return QRectF(start.x(), start.y(),
				  end.x() - start.x(),
				  end.y() - start.y());
}

std::vector<QPointF> CW3PolygonItem_anno::getPoints() {
	std::vector <QPointF> arr;
	arr.reserve(node_list_.size());
	for (auto &i : node_list_)
		arr.push_back(i->pos());
	return arr;
}

std::vector<QPointF> CW3PolygonItem_anno::GetTwoPoints()
{
	std::vector<QPointF> arr;
	arr.push_back(node_list_[0]->pos());
	arr.push_back(node_list_[2]->pos());
	return arr;
}

bool CW3PolygonItem_anno::isInRect(const QPointF& pt) const {
	if (m_eShapeType == measure::Shape::RECT && ui_rect_->contains(pt))
		return true;
	else if (m_eShapeType == measure::Shape::CIRCLE && ui_circle_->contains(pt))
		return true;
	return false;
}

void CW3PolygonItem_anno::DrawLabelUI() {
	if (m_bUseLabel) {
		drawingGuide();
		drawingLabel();
	}
}

void CW3PolygonItem_anno::drawingGuide() {
	int dIndex = m_nGuideSelectedIdx;
	float min = std::numeric_limits<float>::max();
	QPointF LabelPos = node_list_[m_nGuideSelectedIdx]->pos() + m_ptLabelPosOffset;

	float srcLen;
	for (int i = 0; i < node_list_.size(); i++) {
		srcLen = W3::getLength(LabelPos, node_list_[i]->pos(), 1.0f);
		if (srcLen < min) {
			m_ptLabelPosOffset = LabelPos - node_list_[i]->pos();
			dIndex = i;
			min = srcLen;
		}
	}

	m_nGuideSelectedIdx = dIndex;

	QPointF st = node_list_[dIndex]->pos();
	QPointF et = st + m_ptLabelPosOffset;

	QWidget* parent = static_cast<QWidget*>(m_pgScene->parent());
	if (et.x() < 0)
		et.setX(0);
	else if (parent->width() < (et.x() + label_->boundingRect().width()))
		et.setX(parent->width() - label_->boundingRect().width());

	if (et.y() < label_->boundingRect().height())
		et.setY(label_->boundingRect().height());
	else if (parent->height() < et.y())
		et.setY(parent->height());

	QPainterPath p(st);
	p.lineTo(et);
	ui_label_guide_->setPath(p);
}

void CW3PolygonItem_anno::drawingLabel() {
	QPointF st = node_list_[m_nGuideSelectedIdx]->pos();
	QPointF et = st + m_ptLabelPosOffset;

	QPointF pos(et.rx(), et.ry() - label_->boundingRect().height());

	QWidget* parent = static_cast<QWidget*>(m_pgScene->parent());
	if (!parent)
	{
		return;
	}

	if (pos.x() < 0)
	{
		pos.setX(0);
	}
	else if (parent->width() < (pos.x() + label_->boundingRect().width()))
	{
		pos.setX(parent->width() - label_->boundingRect().width());
	}

	if (pos.y() < 0)
	{
		pos.setY(0);
	}
	else if (parent->height() < (pos.y() + label_->boundingRect().height()))
	{
		pos.setY(parent->height() - label_->boundingRect().height());
	}

	label_->setPos(pos);
	m_ptLabelPosOffset = QPointF((pos - st).x(), (pos - st).y() + label_->boundingRect().height());
}

void CW3PolygonItem_anno::graphicItemsConnection() {
	if (ui_rect_) {
		connect(ui_rect_, SIGNAL(sigTranslateRect(QPointF)), this, SLOT(slotTranslateRect(QPointF)));
		connect(ui_rect_, SIGNAL(sigHoverRect(bool)), this, SLOT(slotHighlightRect(bool)));
	}

	if (ui_circle_)
		connect(ui_circle_, SIGNAL(sigTranslateCircle(QPointF)), this, SLOT(slotTranslateRect(QPointF)));

	for (auto &i : node_list_)
		connect(i, SIGNAL(sigTranslateEllipse(QPointF)), this, SLOT(slotTranslateEllipse(QPointF)));
}

void CW3PolygonItem_anno::displayNode(bool bVisible) {
	m_bShowNodes = bVisible;
	for (auto &i : node_list_)
		i->setVisible(m_bShowNodes);
}

void CW3PolygonItem_anno::setVisible(bool bShow) {
	switch (m_eShapeType) {
	case measure::Shape::RECT:
		ui_rect_->setVisible(bShow);
		break;
	case measure::Shape::CIRCLE:
		ui_circle_->setVisible(bShow);
		break;
	default:
		break;
	}

	if (label_) {
		label_->setVisible(bShow);
		ui_label_guide_->setVisible(bShow);
	}

	if (m_bShowNodes)
		for (auto &i : node_list_)
			i->setVisible(bShow);
}

void CW3PolygonItem_anno::transformItems(const QTransform & transform) {
	for (int i = 0; i < node_list_.size(); i++)
		node_list_[i]->setPos(transform.map(node_list_[i]->pos()));

	QPointF start = node_list_[0]->pos();
	QPointF end = node_list_[2]->pos();

	if (m_eShapeType == measure::Shape::RECT) {
		ui_rect_->setPos(0.0f, 0.0f);
		ui_rect_->setRect(start.x(), start.y(), (end - start).x(), (end - start).y());
	} else if (m_eShapeType == measure::Shape::CIRCLE) {
		ui_circle_->setPos(0.0f, 0.0f);
		ui_circle_->setRect(start.x(), start.y(), (end - start).x(), (end - start).y());
	}

	DrawLabelUI();
}
void CW3PolygonItem_anno::ApplyPreferences() {
	ApplyNodeColor();
	ApplyLineColor();
	ApplyTextColor();
	ApplyTextSize();
}

void CW3PolygonItem_anno::ApplyNodeColor() {
	if (node_list_.empty())
		return;

	QColor line_color = GlobalPreferences::GetInstance()->preferences_.objects.measure.line_color;
	for (int i = 0; i < node_list_.size(); i++)
		node_list_[i]->setPenColor(line_color);
}

void CW3PolygonItem_anno::ApplyLineColor() {
	QColor line_color = GlobalPreferences::GetInstance()->preferences_.objects.measure.line_color;
	if (m_eShapeType == measure::Shape::RECT && ui_rect_)
		ui_rect_->setPenColor(line_color);
	else if (m_eShapeType == measure::Shape::CIRCLE && ui_circle_)
		ui_circle_->setPenColor(line_color);
}

void CW3PolygonItem_anno::ApplyTextColor() {
	if (!label_)
		return;

	QColor text_color = GlobalPreferences::GetInstance()->preferences_.objects.measure.text_color;
	label_->SetTextColor(text_color);
}

void CW3PolygonItem_anno::ApplyTextSize() {
	if (!label_)
		return;

	GlobalPreferences::Size text_size = GlobalPreferences::GetInstance()->preferences_.objects.measure.text_size;
	QFont font = QApplication::font();
	font.setPixelSize(font.pixelSize() + (static_cast<int>(text_size) * 2) + 1);
	label_->setFont(font);
}
