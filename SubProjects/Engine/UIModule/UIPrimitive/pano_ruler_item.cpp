#include "pano_ruler_item.h"

#include "../../Common/Common/color_will3d.h"

#include <QVector2D>

#include "line_list_item.h"
#include "W3TextItem.h"

namespace
{
	const int kInvalid = -1;
	const float kRulerLineLengthLarge = 10.0f;
	const float kRulerLineLengthMedium = 6.5f;
	const float kRulerLineLengthSmall = 2.0f;
	const QVector2D kLineNormal01(0.0, -1.0);
	const QVector2D kLineNormal10(1.0, 0.0);
	const QColor kFontColor = QColor(Qt::yellow);
	const int kFontPointSize = 10;
}

PanoRulerItem::PanoRulerItem()
{
	this->setFlag(QGraphicsItem::ItemIsSelectable, false);
	this->setFlag(QGraphicsItem::ItemIsMovable, false);
	this->setZValue(0);
}

PanoRulerItem::~PanoRulerItem()
{
}

void PanoRulerItem::SetRuler(int idx_min, int idx_max, int idx_arch_front,
	int idx_min_in_scene, int idx_max_in_scene, int idx_arch_front_in_scene,
	const std::vector<int>& medium_gradation,
	const std::vector<int>& small_gradation,
	const std::vector<double>& medium_gradation_in_scene,
	const std::vector<double>& small_gradation_in_scene,
	const QPointF& position)
{
	if (ruler_lines_[0])
	{
		DeleteRulerItem();
	}

	InitRulerItem();

	ruler_lines_[RLT_L]->set_length(kRulerLineLengthLarge);
	ruler_lines_[RLT_M]->set_length(kRulerLineLengthMedium);
	ruler_lines_[RLT_S]->set_length(kRulerLineLengthSmall);
	ruler_lines_[RLT_V]->set_length((float)(idx_max_in_scene - idx_min_in_scene));

	for (int i = 0; i < medium_gradation.size(); ++i)
	{
		double elem = medium_gradation_in_scene.at(i);

		ruler_lines_[RLT_M]->AddLine(QPointF(elem, -kRulerLineLengthMedium * 0.5f) + position, kLineNormal01);
#if 1
		float length = qAbs((medium_gradation.at(i) - idx_arch_front) * pixel_spacing_);
		CW3TextItem* length_text = CreateTextItem();
		length_text->setPlainText(QString("%1").arg(length));
		length_text->setPos(
			QPointF(elem, -kRulerLineLengthMedium - 8.0f) -
			QPointF(length_text->boundingRect().right() * 0.5f, length_text->boundingRect().bottom() * 0.5f) +
			position
		);
		length_texts_.push_back(length_text);
#endif
	}

	double prev_gradation = 0.0;
	int small_length = 0;
	for (int i = 0; i < small_gradation.size(); ++i)
	{
		double elem = small_gradation_in_scene.at(i);

		if (elem < prev_gradation)
		{
			small_length = 0;
			prev_gradation = 0.0;
		}
		else
		{
			prev_gradation = elem;
		}

		++small_length;
		if (small_length % 5 == 0)
		{
			++small_length;
		}

		ruler_lines_[RLT_S]->AddLine(QPointF(elem, -kRulerLineLengthSmall * 0.5f) + position, kLineNormal01);
	}

	ruler_lines_[RLT_L]->AddLine(QPointF(idx_min_in_scene, -kRulerLineLengthLarge * 0.5f) + position, kLineNormal01);
	ruler_lines_[RLT_L]->AddLine(QPointF(idx_max_in_scene, -kRulerLineLengthLarge * 0.5f) + position, kLineNormal01);
	ruler_lines_[RLT_L]->AddLine(QPointF(idx_arch_front_in_scene, -kRulerLineLengthLarge * 0.5f) + position, kLineNormal01);

	ruler_lines_[RLT_V]->AddLine(QPointF((float)(idx_max_in_scene - idx_min_in_scene)*0.5f + idx_min_in_scene, 0.0) + position, kLineNormal10);

	int slice_index_text_pos_y = slice_index_texts_[RTT_CENTER]->boundingRect().height() * 0.5f;

	int front_index = idx_min - idx_arch_front;
	int back_index = idx_max - idx_arch_front;

	slice_index_texts_[RTT_FRONT]->setPlainText(QString("%1").arg(front_index));
	slice_index_texts_[RTT_FRONT]->setPos(
		QPointF(idx_min_in_scene, slice_index_text_pos_y) - 
		QPointF(0.0, slice_index_texts_[RTT_FRONT]->boundingRect().height() * 0.5) +
		position
	);

	slice_index_texts_[RTT_BACK]->setPlainText(QString("%1").arg(back_index));
	slice_index_texts_[RTT_BACK]->setPos(
		QPointF(idx_max_in_scene, slice_index_text_pos_y) - 
		QPointF(slice_index_texts_[RTT_BACK]->boundingRect().width(), slice_index_texts_[RTT_BACK]->boundingRect().height() * 0.5) +
		position
	);

	slice_index_texts_[RTT_CENTER]->setPlainText(QString("%1 idx").arg(0));
	slice_index_texts_[RTT_CENTER]->setPos(
		QPointF(idx_arch_front_in_scene, slice_index_text_pos_y) -
		slice_index_texts_[RTT_CENTER]->boundingRect().bottomRight() * 0.5 + 
		position
	);


	int length_text_pos_y = -kRulerLineLengthLarge - 8.0f;

	CW3TextItem* front_length_text = CreateTextItem();
	front_length_text->setPlainText(QString("%1").arg(qAbs(front_index * pixel_spacing_)));
	front_length_text->setPos(
		QPointF(idx_min_in_scene, length_text_pos_y) -
		QPointF(front_length_text->boundingRect().width(), front_length_text->boundingRect().height() * 0.5) +
		position
	);
	length_texts_.push_back(front_length_text);

	CW3TextItem* back_length_text = CreateTextItem();
	back_length_text->setPlainText(QString("%1").arg(qAbs(back_index * pixel_spacing_)));
	back_length_text->setPos(
		QPointF(idx_max_in_scene, length_text_pos_y) -
		QPointF(0.0f, back_length_text->boundingRect().height() * 0.5) +
		position
	);
	length_texts_.push_back(back_length_text);

	CW3TextItem* center_length_text = CreateTextItem();
	center_length_text->setPlainText(QString("%1 mm").arg(0));
	center_length_text->setPos(
		QPointF(idx_arch_front_in_scene, length_text_pos_y) -
		center_length_text->boundingRect().bottomRight() * 0.5 +
		position
	);
	length_texts_.push_back(center_length_text);

	if (back_index == 0 || front_index == back_index)
	{
		slice_index_texts_[RTT_CENTER]->setVisible(false);
	}
}

void PanoRulerItem::InitRulerItem()
{
	QPen pen_ruler(ColorArchItem::kCurvePen, 1, Qt::SolidLine);
	for (int i = 0; i < RLT_END; i++)
	{
		ruler_lines_[i].reset(new LineListItem(this));
		ruler_lines_[i]->set_pen(pen_ruler);
		ruler_lines_[i]->setZValue(this->zValue() - 1);
	}

	for (int i = 0; i < RTT_END; i++)
	{
		slice_index_texts_[i].reset(CreateTextItem());
	}
	initialized_ = true;
}

CW3TextItem* PanoRulerItem::CreateTextItem()
{
	CW3TextItem* text_item = new CW3TextItem(false, this);
	text_item->setTextColor(kFontColor);
	text_item->setPointSize(kFontPointSize);
	return text_item;
}

void PanoRulerItem::DeleteRulerItem()
{
	for (int i = 0; i < RLT_END; i++)
	{
		ruler_lines_[i].reset();
	}
	for (int i = 0; i < RTT_END; i++)
	{
		slice_index_texts_[i].reset();
	}
	for (int i = 0; i < length_texts_.size(); ++i)
	{
		CW3TextItem* ruler_text = length_texts_.at(i);
		delete ruler_text;
	}
	length_texts_.clear();

	initialized_ = false;
}

void PanoRulerItem::TransformItems(const QTransform& transform)
{
	for (int i = 0; i < RLT_END; i++)
	{
		if (ruler_lines_[i])
		{
			ruler_lines_[i]->TransformItems(transform);
		}
	}

	for (int i = 0; i < RTT_END; i++)
	{
		if (slice_index_texts_[i])
		{
			slice_index_texts_[i]->setPos(transform.map(slice_index_texts_[i]->pos()));
		}
	}

	for (auto& elem : length_texts_)
	{
		if (elem)
		{
			elem->setPos(transform.map(elem->pos()));
		}
	}
}
