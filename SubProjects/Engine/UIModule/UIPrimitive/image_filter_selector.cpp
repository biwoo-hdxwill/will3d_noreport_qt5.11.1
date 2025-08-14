#include "image_filter_selector.h"

#include <Engine/Common/Common/language_pack.h>

#include "W3TextItem.h"
#include "W3TextItem_sequence.h"

ImageFilterSelector::ImageFilterSelector(QGraphicsItem* parent)
	: QGraphicsItem(parent)
{
	Initialize();

	AddText(lang::LanguagePack::txt_filter() + " " + lang::LanguagePack::txt_off());
	AddText(lang::LanguagePack::txt_filter() + " " + "1x");
	AddText(lang::LanguagePack::txt_filter() + " " + "2x");
	AddText(lang::LanguagePack::txt_filter() + " " + "3x");
}

ImageFilterSelector::~ImageFilterSelector() {}

void ImageFilterSelector::Clear()
{
	if (sequence_text_)
	{
		sequence_text_->clear();
	}
}

void ImageFilterSelector::AddText(const QString& text)
{
	if (sequence_text_->getCount() == 0)
	{
		text_->setPlainText(text);
	}

	sequence_text_->addText(text);
}

void ImageFilterSelector::SetTextWidth(int width)
{
	text_->setTextWidth(width);
}

void ImageFilterSelector::SetFont(const QFont& font)
{
	text_->setFont(font);
	sequence_text_->setFont(font);
}

void ImageFilterSelector::SetLevel(const int level)
{
	if (sequence_text_->getCount() <= level)
	{
		return;
	}
	QString text = sequence_text_->getTexts().at(level)->toPlainText();
	text_->setPlainText(text);

	//emit sigPressed(level);
}

void ImageFilterSelector::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	setZValue(100.0f);
	sequence_text_->setZValue(100.0f);
	sequence_text_->setVisible(true);
	QGraphicsItem::hoverEnterEvent(event);
}

void ImageFilterSelector::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	setZValue(10.0f);
	sequence_text_->setZValue(10.0f);
	sequence_text_->setVisible(false);
	QGraphicsItem::hoverLeaveEvent(event);
}

QRectF ImageFilterSelector::boundingRect() const
{
	QRectF rect = transform().mapRect(text_->boundingRect());

	if (sequence_text_->isVisible())
	{
		QRectF seqRect = transform().mapRect(sequence_text_->boundingRect());

		QPointF pos = rect.bottomRight();
		pos.setX(pos.x() - seqRect.width());
		sequence_text_->setPos(pos);

		rect.setHeight(seqRect.height() + rect.height());
		rect.setX(pos.x());
		rect.setWidth(seqRect.width() + rect.width());
	}

	text_->setPos(0, 0);

	return rect;
}

void ImageFilterSelector::Initialize()
{
	setAcceptHoverEvents(true);
	text_ = new CW3TextItem(this);
	text_->setTextHighlightColor("#FF78A3FF");

	text_->setPos(0, 0);

	sequence_text_ = new CW3TextItem_sequence(this);
	sequence_text_->setOrientation(CW3TextItem_sequence::vertical);
	sequence_text_->setVisible(false);
	sequence_text_->setZValue(10.0f);

	setZValue(10.0f);

	connect(sequence_text_, SIGNAL(sigReleased(QString)), this, SLOT(slotPressedSequenceText(QString)));
}

void ImageFilterSelector::slotPressedSequenceText(const QString& text)
{
	if (text_->toPlainText() == text)
	{
		return;
	}

	QRectF prev_rect = transform().mapRect(text_->boundingRect());

	text_->setPlainText(text);

	QRectF curr_rect = transform().mapRect(text_->boundingRect());

	QPointF pos = this->pos();
	pos.setX(pos.x() - (curr_rect.width() - prev_rect.width()));
	setPos(pos);

	sequence_text_->setHighlightEffects(false);
	sequence_text_->setVisible(false);
	
	boundingRect();

	int selected_index = 0;
	for (int i = 0; i < sequence_text_->getCount(); ++i)
	{
		if (text.compare(sequence_text_->getTexts().at(i)->toPlainText()) == 0)
		{
			selected_index = i;
		}
	}

	emit sigPressed(selected_index);
}

void ImageFilterSelector::SetText(const QString &text)
{
	text_->setPlainText(text);
}

bool ImageFilterSelector::IsHovered()
{
	if (isUnderMouse() || sequence_text_->isVisible())
	{
		return true;
	}
	else
	{
		return false;
	}
}
