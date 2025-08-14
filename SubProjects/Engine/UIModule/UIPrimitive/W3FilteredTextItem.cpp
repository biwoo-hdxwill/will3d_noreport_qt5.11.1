#include "W3FilteredTextItem.h"

#include "W3TextItem.h"
#include "W3TextItem_sequence.h"

CW3FilteredTextItem::CW3FilteredTextItem(const QString& text, QGraphicsItem *parent)
	: QGraphicsItem(parent) {
	Initialize();
	addText(text);
}
CW3FilteredTextItem::CW3FilteredTextItem(QGraphicsItem* parent)
	: QGraphicsItem(parent) {
	Initialize();
}
CW3FilteredTextItem::~CW3FilteredTextItem() {}

////////////////////////////////////////////////////////////////////////////////////
// public functions
////////////////////////////////////////////////////////////////////////////////////

void CW3FilteredTextItem::clear() {
	if (m_pSequenceText)
		m_pSequenceText->clear();
}

void CW3FilteredTextItem::addText(const QString& text) {
	if (m_pSequenceText->getCount() == 0)
		m_pText->setPlainText(text);

	m_pSequenceText->addText(text);
}

void CW3FilteredTextItem::changeText(const QString& text) {
	if (m_pText->toPlainText() == text)
		return;

	const auto& texts = m_pSequenceText->getTexts();
	for (const auto& txt : texts) {
		if (txt->toPlainText() == text) {
			QTransform transform = this->transform();
			QRectF prev_rect = transform.mapRect(m_pText->boundingRect());

			m_pText->setPlainText(text);

			QRectF curr_rect = transform.mapRect(m_pText->boundingRect());

			QPointF pos = this->pos();
			pos.setX(pos.x() - (curr_rect.width() - prev_rect.width()));
			this->setPos(pos);

			m_pSequenceText->setHighlightEffects(false);
			m_pSequenceText->setVisible(false);
			boundingRect();
			break;
		}
	}
}

void CW3FilteredTextItem::setTextWidth(int width) {
	m_pText->setTextWidth(width);
}

void CW3FilteredTextItem::setFont(const QFont & afont) {
	m_pText->setFont(afont);
	m_pSequenceText->setFont(afont);
}

////////////////////////////////////////////////////////////////////////////////////
// protected functions
////////////////////////////////////////////////////////////////////////////////////
void CW3FilteredTextItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
	this->setZValue(100.0f);
	m_pSequenceText->setZValue(100.0f);
	m_pSequenceText->setVisible(true);
	QGraphicsItem::hoverEnterEvent(event);
}
void CW3FilteredTextItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
	this->setZValue(10.0f);
	m_pSequenceText->setZValue(10.0f);
	m_pSequenceText->setVisible(false);
	QGraphicsItem::hoverLeaveEvent(event);
}

QRectF CW3FilteredTextItem::boundingRect() const {
	QTransform transform = this->transform();

	//QRectF debug = m_pText->boundingRect();
	QRectF rect = transform.mapRect(m_pText->boundingRect());

	if (m_pSequenceText->isVisible()) {
		QRectF seqRect = transform.mapRect(m_pSequenceText->boundingRect());

		QPointF pos = rect.bottomRight();
		pos.setX(pos.x() - seqRect.width());
		m_pSequenceText->setPos(pos);

		rect.setHeight(seqRect.height() + rect.height());
		rect.setX(pos.x());
		rect.setWidth(seqRect.width() + rect.width());
	}

	m_pText->setPos(0, 0);

	return rect;
}

////////////////////////////////////////////////////////////////////////////////////
// private functions
////////////////////////////////////////////////////////////////////////////////////

void CW3FilteredTextItem::Initialize() {
	this->setAcceptHoverEvents(true);
	m_pText = new CW3TextItem(this);
	m_pText->setTextHighlightColor("#FF78A3FF");

	m_pText->setPos(0, 0);

	m_pSequenceText = new CW3TextItem_sequence(this);
	m_pSequenceText->setOrientation(CW3TextItem_sequence::vertical);
	m_pSequenceText->setVisible(false);
	m_pSequenceText->setZValue(10.0f);
	//m_pSequenceText->setPos(QPointF(50, 0));

	this->setZValue(10.0f);

	connect(m_pSequenceText, SIGNAL(sigReleased(QString)), this, SLOT(slotPressedSequenceText(QString)));
}
////////////////////////////////////////////////////////////////////////////////////
// private slots
////////////////////////////////////////////////////////////////////////////////////

void CW3FilteredTextItem::slotPressedSequenceText(const QString& text) {
	if (m_pText->toPlainText() == text)
		return;

	QTransform transform = this->transform();
	QRectF prev_rect = transform.mapRect(m_pText->boundingRect());

	m_pText->setPlainText(text);

	QRectF curr_rect = transform.mapRect(m_pText->boundingRect());

	QPointF pos = this->pos();
	pos.setX(pos.x() - (curr_rect.width() - prev_rect.width()));
	this->setPos(pos);

	m_pSequenceText->setHighlightEffects(false);
	m_pSequenceText->setVisible(false);
	boundingRect();
	emit sigPressed(text);
}

void CW3FilteredTextItem::setText(const QString &text) {
	m_pText->setPlainText(text);
}

void CW3FilteredTextItem::setReconType(const QString &text_type) {
	setText(text_type);
	slotPressedSequenceText(text_type);
}

bool CW3FilteredTextItem::isHovered() {
	if (this->isUnderMouse() || m_pSequenceText->isVisible())
		return true;
	else
		return false;
}
