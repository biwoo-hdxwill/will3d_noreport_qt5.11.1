#include "W3LabelItem_anno.h"

#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include <QPainter>
#include <QPainterPath>

namespace {
const QColor kOutlineColor(Qt::black);
const QString kTokenSplit("\n");
const QPen kPenOutline(kOutlineColor, 1.8f);

int GetTextSpacing(int font_size) {
	switch (font_size) {
	case 14: // small
		return font_size + 3;
	case 16: // medium
		return font_size + 3;
	case 18: // large
		return font_size + 4;
	default:
		return font_size + 2;
	}
}
} // end of namespace

CW3LabelItem_anno::CW3LabelItem_anno(QGraphicsItem *parent)
	:QGraphicsTextItem(parent) {
	color_ = Qt::white;
	setDefaultTextColor(color_);
	setAcceptHoverEvents(true);

	QFont font = QApplication::font();
	font.setPixelSize(font.pixelSize() + 1);
	setFont(font);

	setZValue(10.0);

	this->setFlag(CW3LabelItem_anno::ItemIsSelectable, true);
	this->setFlag(CW3LabelItem_anno::ItemIsMovable, true);
}

CW3LabelItem_anno::~CW3LabelItem_anno(void) {}

void CW3LabelItem_anno::setPlainText(const QString & text) {
	shadow_texts_ = text.split(kTokenSplit);
	QGraphicsTextItem::setPlainText(text);
}

void CW3LabelItem_anno::paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget) {
	QStyleOptionGraphicsItem* style = const_cast<QStyleOptionGraphicsItem*>(pOption);

	// Remove the HasFocus style state to prevent the dotted line from being drawn.
	style->state &= ~QStyle::State_HasFocus;
	style->state &= ~QStyle::State_Selected;

	if (!shadow_texts_.empty()) {
		QFont f = font();
		QFontMetrics fm(f);
		QRect rect = fm.boundingRect(this->boundingRect().toRect(),
									 Qt::AlignCenter, toPlainText());
		QPainterPath path;
		int y_pos = rect.top() + fm.ascent();
		int pixel_size = GetTextSpacing(f.pixelSize());
		for (int idx = 0; idx < shadow_texts_.size(); ++idx) {
			path.addText(rect.left(), y_pos, f, shadow_texts_[idx]);
			y_pos += pixel_size;
		}
		pPainter->setPen(kPenOutline);
		pPainter->drawPath(path);
	}
	//pPainter->save();
	//pPainter->setPen(QPen(Qt::white, 1));
	//pPainter->setBrush(QBrush(QColor(50, 50, 50, 200)));
	//pPainter->drawRect(this->boundingRect());
	//pPainter->restore();
	QGraphicsTextItem::paint(pPainter, pOption, pWidget);
}

void CW3LabelItem_anno::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
	QGraphicsTextItem::hoverEnterEvent(event);
	//QFont font("Times", TOOL_LABEL_TEXT_SIZE);
	//QFont font = this->font();
	//font.setBold(true);
	//setFont(font);
	setDefaultTextColor(QColor("#FF78A3FF"));
	emit sigHoverLabel(true);
}

void CW3LabelItem_anno::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
	QGraphicsTextItem::hoverLeaveEvent(event);
	//QFont font("Times", TOOL_LABEL_TEXT_SIZE);
	//QFont font = this->font();
	//font.setBold(false);
	//setFont(font);
	setDefaultTextColor(color_);
	emit sigHoverLabel(false);
}

void CW3LabelItem_anno::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsTextItem::mouseMoveEvent(event);

	if (event->buttons() != Qt::LeftButton)
		return;

	QPointF trans = event->pos() - m_ptScene;
	emit sigTranslateLabel(trans);
}

void CW3LabelItem_anno::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsTextItem::mousePressEvent(event);
	m_ptScene = event->pos();
	emit sigPressed();
}

void CW3LabelItem_anno::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsTextItem::mouseDoubleClickEvent(event);
	emit sigDoubleClicked();
}

void CW3LabelItem_anno::SetTextColor(QColor color) {
	color_ = color;
	setDefaultTextColor(color);
}
