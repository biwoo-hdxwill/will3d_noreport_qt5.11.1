#include "simple_text_item.h"
#include <QPen>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>

namespace {
const QColor kOutlineColor(Qt::black);
const QColor kDefaultTextColor(Qt::white);
const QString kTokenSplit("\n");
const QPen kPenOutline(kOutlineColor, 2.5f);
} // end of namespace

SimpleTextItem::SimpleTextItem(QGraphicsItem *parent)
	: QGraphicsTextItem(parent) {
	this->setFont(QApplication::font());
	this->setDefaultTextColor(kDefaultTextColor);
	this->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	this->setFlag(QGraphicsTextItem::ItemIsSelectable, false);
}

SimpleTextItem::~SimpleTextItem() {}

void SimpleTextItem::setPosCenter(const QPointF& scene_pos) {
	QRectF size = boundingRect();
	float width = size.right() - size.left();
	float height = size.bottom() - size.top();
	QGraphicsTextItem::setPos(scene_pos.x() - width / 2,
							  scene_pos.y() - height / 2);
}

void SimpleTextItem::SetText(const QString & text) {
	shadow_texts_ = text.split(kTokenSplit);
	setPlainText(text);
}

void SimpleTextItem::setTextColor(const QColor & c) {
	setDefaultTextColor(c);
}

void SimpleTextItem::paint(QPainter * pPainter,
						   const QStyleOptionGraphicsItem * pOption,
						   QWidget * pWidget) {
	if (!shadow_texts_.empty()) {
		QFont f = font();
		QFontMetrics fm(f);
		QRect rect = fm.boundingRect(this->boundingRect().toRect(),
									 Qt::AlignCenter, toPlainText());

		QPainterPath path;
		int y_pos = rect.top() + fm.ascent();
		for (int idx = 0; idx < shadow_texts_.size(); ++idx) {
			path.addText(rect.left(), y_pos, f, shadow_texts_[idx]);
			y_pos += f.pixelSize() + 2;
		}
		pPainter->setPen(kPenOutline);
		pPainter->drawPath(path);
	}

	QGraphicsTextItem::paint(pPainter, pOption, pWidget);
}
