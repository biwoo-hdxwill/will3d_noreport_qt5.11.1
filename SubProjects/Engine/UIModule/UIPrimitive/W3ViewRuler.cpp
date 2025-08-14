#include "W3ViewRuler.h"

#include <QApplication>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/define_ui.h"
#include "W3TextItem.h"

using namespace common;

namespace {
const int kMarginH = ui_define::kViewMarginHeight - 2;
const int kMarginW = ui_define::kViewMarginWidth - 2;
const int kMarginOrg = ui_define::kViewMarginWidth - 8;
const float kDefaultGradationLength = 12.0f;
const float kLevel1Limit = 100.0f;
const float kLevel2Limit = 300.0f;
enum class UnitLength { CM, MM };
} // end of namespace

CW3ViewRuler::CW3ViewRuler(const QColor& color, QObject* parent) :
	org_text_(new CW3TextItem(false, this)),
	h_length_text_(new CW3TextItem(false, this)),
	v_length_text_(new CW3TextItem(false, this)),
	QObject(parent) {
	this->setFlag(QGraphicsItem::ItemIsSelectable, false);
	this->setFlag(QGraphicsItem::ItemIsMovable, false);
	this->setFlag(QGraphicsItem::ItemIgnoresParentOpacity, true);
	this->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	SetItemColor(color);
	InitUI();
}

CW3ViewRuler::CW3ViewRuler(QObject* parent) :
	org_text_(new CW3TextItem(false, this)),
	h_length_text_(new CW3TextItem(false, this)),
	v_length_text_(new CW3TextItem(false, this)),
	QObject(parent) {
	this->setFlag(QGraphicsItem::ItemIsSelectable, false);
	this->setFlag(QGraphicsItem::ItemIsMovable, false);
	this->setFlag(QGraphicsItem::ItemIgnoresParentOpacity, true);
	this->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	InitUI();
}

void CW3ViewRuler::InitUI() {
	this->setZValue(ui_define::kZValueRuler);

	org_text_->setZValue(ui_define::kZValueRuler);
	org_text_->setPlainText("0mm");

	h_length_text_->setZValue(ui_define::kZValueRuler);
	h_length_text_->setPlainText("0mm");

	v_length_text_->setZValue(ui_define::kZValueRuler);
	v_length_text_->setRotation(-90.0);
	v_length_text_->setPlainText("0mm");

	QFont font = QApplication::font();
	font.setPixelSize(font.pixelSize() - 1);
	h_length_text_->setFont(font);
	v_length_text_->setFont(font);
}

void CW3ViewRuler::ClearLines() {
	for (auto &i : w_lines_)
		SAFE_DELETE_OBJECT(i);

	for (auto &i : h_lines_)
		SAFE_DELETE_OBJECT(i);

	w_lines_.clear();
	h_lines_.clear();
}

CW3ViewRuler::~CW3ViewRuler() {
}

void CW3ViewRuler::Disable() {
	ruler_enable_ = false;
	QGraphicsItem::setVisible(false);
	setVisibleRulerText(false);
	setVisibleRulerWidth(false);
	setVisibleRulerHeight(false);
}

void CW3ViewRuler::SetColor(const QColor& color) {
	SetItemColor(color);
	setRulerLine();
}

void CW3ViewRuler::setViewRuler(float viewWidth, float viewHeight,
								float widthLength, float heightLength,
								const QPointF& viewCenterInScene, bool show_detail_length) {
	if ((int)widthLength == 0 ||
		(int)heightLength == 0)
		return;

	show_detail_length_ = show_detail_length;

	view_width_length_ = widthLength;
	view_height_length_ = heightLength;
	view_width_in_scene_ = viewWidth;
	view_height_in_scene_ = viewHeight;
	view_center_in_scene_ = viewCenterInScene;

	setRulerLine();

	org_text_->setPos(kMarginOrg, kMarginH);
	h_length_text_->setPos(viewWidth - kMarginW - 60, kMarginH);
	v_length_text_->setPos(kMarginW, viewHeight - kMarginH - 2);
}

void CW3ViewRuler::setWidthLabelPos(float x, float y) {
	h_length_text_->setPos(x - h_length_text_->sceneBoundingRect().width(), y);
}

void CW3ViewRuler::setHeightLabelPos(float x, float y) {
	v_length_text_->setPos(x - v_length_text_->sceneBoundingRect().width(), y);
}

void CW3ViewRuler::SetItemColor(const QColor & color) {
	color_ = color;
	org_text_->setTextColor(color);
	h_length_text_->setTextColor(color);
	v_length_text_->setTextColor(color);

	for (int i = 0; i < h_detail_length_text_.size(); ++i)
	{
		h_detail_length_text_.at(i)->setTextColor(color);
	}
}

void CW3ViewRuler::setRulerLine() {
	ClearLines();

	float lineGap = view_width_in_scene_ / view_width_length_ * 5.0f; // 5mm
	int stepX = view_width_length_ * 0.2f;
	int stepY = view_height_length_ * 0.2f;

	UnitLength unit_length = UnitLength::MM;
	DetailLevel detail_level;
	if (view_width_length_ <= kLevel1Limit) {
		//unit_length = UnitLength::MM;
		detail_level = DetailLevel::LV_1;
		stepX = view_width_length_;
		stepY = view_height_length_;
		lineGap *= 0.2f;
	} else if (view_width_length_ <= kLevel2Limit) {
		//unit_length = UnitLength::CM;
		detail_level = DetailLevel::LV_2;
	} else {
		//unit_length = UnitLength::CM;
		detail_level = DetailLevel::LV_3;
		stepX = view_width_length_*0.1f;
		stepY = view_height_length_*0.1f;
		lineGap *= 2.0f;
	}

	QPen pen(color_, 1.0f);
	pen.setCosmetic(true);

	if (show_detail_length_)
	{
		for (int i = 0; i < h_detail_length_text_.size(); ++i)
		{
			CW3TextItem* text_item = h_detail_length_text_.at(i);
			SAFE_DELETE_OBJECT(text_item);
		}
		h_detail_length_text_.clear();
	}

	float widthX = view_center_in_scene_.x() - view_width_in_scene_ * 0.5f;
	float widthY = view_center_in_scene_.y() - view_height_in_scene_ * 0.5f + 3.0f;
	for (int i = 0; i <= stepX; i++) {
		float gradation_length = GetGradationLength(i, detail_level);
		QGraphicsLineItem *line = new QGraphicsLineItem(widthX, widthY,
														widthX, widthY + gradation_length, this);
		line->setFlag(QGraphicsLineItem::ItemIgnoresTransformations, true);
		line->setPen(pen);
		line->setZValue(ui_define::kZValueRuler);
		line->flags();
		w_lines_.push_back(line);

		widthX += lineGap;
	}

	float heightX = view_center_in_scene_.x() - view_width_in_scene_ * 0.5f + 4.0f;
	float heightY = view_center_in_scene_.y() - view_height_in_scene_ * 0.5f;
	for (int i = 0; i <= stepY; i++) {
		if (heightY < org_text_->boundingRect().height() + org_text_->boundingRect().y() + 5) {
			heightY += lineGap;
			continue;
		}

		float gradation_length = GetGradationLength(i, detail_level) - 2.0f;
		QGraphicsLineItem *line = new QGraphicsLineItem(heightX, heightY,
														heightX + gradation_length, heightY, this);
		line->setFlag(QGraphicsLineItem::ItemIgnoresTransformations, true);
		line->setPen(pen);
		line->setZValue(ui_define::kZValueRuler);
		h_lines_.push_back(line);

		heightY += lineGap;

		if (show_detail_length_ && gradation_length >= kDefaultGradationLength)
		{
			CW3TextItem* h_detail_length_text = new CW3TextItem(false, this);
			double detail_length = view_width_length_ / stepX * i;
			h_detail_length_text->setPlainText(QString("%1").arg(detail_length, 0, 'f', 1));
			h_detail_length_text->setPos(line->line().p2());
			h_detail_length_text->setFont(h_length_text_->font());
			//h_detail_length_text->setTextColor(QColor(0, 255, 0));

			h_detail_length_text_.push_back(h_detail_length_text);
		}
	}
	
	QString unit_length_text = "mm";
	float display_width = view_width_length_;
	float display_height = view_height_length_;
	if (unit_length == UnitLength::CM) {
		unit_length_text = "cm";
		display_width *= 0.1f;
		display_height *= 0.1f;
	}

	org_text_->setPlainText("0" + unit_length_text);
	h_length_text_->setPlainText(QString("%1").arg(display_width, 0, 'f', 1) + unit_length_text);
	v_length_text_->setPlainText(QString("%1").arg(display_height, 0, 'f', 1) + unit_length_text);
}

void CW3ViewRuler::setVisible(bool visible) {
	if (!ruler_enable_)
		return;

	QGraphicsItem::setVisible(visible);
	setVisibleRulerText(visible);
	setVisibleRulerWidth(visible);
	setVisibleRulerHeight(visible);
}

void CW3ViewRuler::setVisibleRulerWidth(bool visible) {
	if (w_lines_.empty())
		return;

	if (visible == w_lines_[0]->isVisible())
		return;

	for (auto &i : w_lines_)
		i->setVisible(visible);
}

void CW3ViewRuler::setVisibleRulerHeight(bool visible) {
	if (h_lines_.empty())
		return;

	if (visible == h_lines_[0]->isVisible())
		return;

	for (auto &i : h_lines_)
		i->setVisible(visible);
}

void CW3ViewRuler::setVisibleRulerText(bool visible) {
	h_length_text_->setVisible(visible);
	v_length_text_->setVisible(visible);
	for (int i = 0; i < h_detail_length_text_.size(); ++i)
	{
		h_detail_length_text_.at(i)->setVisible(visible);
	}
}

float CW3ViewRuler::GetGradationLength(int index, const DetailLevel & detail_level) {
	float gradation_length = kDefaultGradationLength;

	if (detail_level == DetailLevel::LV_2) {
		if (index % 2 != 0)
			gradation_length *= 0.5f;
	} else if (detail_level == DetailLevel::LV_1) {
		if (index % 10 == 0) {
			gradation_length *= 1.5f;
		} else if (index % 5 == 0) {
			// 의도된 공백
		} else {
			gradation_length *= 0.5f;
		}
	}

	return gradation_length;
}
