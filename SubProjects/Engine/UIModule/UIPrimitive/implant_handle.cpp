#include "implant_handle.h"

#include <qmath.h>
#include <QGraphicsSceneEvent>
#include <QPainter>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/global_preferences.h"

#include "W3EllipseItem.h"
#include "W3TextItem.h"

namespace
{
	const QPointF kPtOrg(0.0, 0.0);
	const qreal kArrowSize = 20;
	
	const QColor kColor_Normal = QColor(255, 153, 51);
	const QColor kColor_Hover = QColor(0, 255, 255);

	const QPen kPenNormal(kColor_Normal, 3, Qt::SolidLine);
	const QPen kPenHover(kColor_Hover, 4, Qt::SolidLine);
	const QRectF kBoundingRect(-30, -30, 60, 60);

	float handle_multiple[7] = { 0.5f, 0.75f, 1.f, 1.25f, 1.5f, 1.75f, 2.f };
	int handle_width[5] = { 1, 2, 3, 4 ,5 };

}  // end of namespace

ImplantCrossHandle::ImplantCrossHandle(QGraphicsItem *parent)
	: QGraphicsItem(parent), rect_(-12, -12, 24, 24), pen_(kPenNormal)
{
	int size_index = GlobalPreferences::GetInstance()->preferences_.advanced.implant_view.implant_handle_size_index;
	if (size_index != 2)
	{
		float size = 12.f * handle_multiple[size_index];
		float length = size * 2.f;
		rect_ = QRectF(-size, -size, length, length);
	}

	int width_index = GlobalPreferences::GetInstance()->preferences_.advanced.implant_view.implant_handle_width_index;
	if (width_index != 2)
	{
		pen_.setWidth(handle_width[width_index]);
	}

	this->setFlag(QGraphicsItem::ItemIsSelectable, true);
	this->setFlag(QGraphicsItem::ItemIsMovable, false);
	this->setAcceptedMouseButtons(Qt::MouseButton::LeftButton);
	this->setAcceptHoverEvents(true);
	setZValue(14);
}

void ImplantCrossHandle::setHighlightEffect(const bool bFlag)
{
#if 0
	pen_ = bFlag ? kPenHover : kPenNormal;
#else
	int width_index = GlobalPreferences::GetInstance()->preferences_.advanced.implant_view.implant_handle_width_index;
	int width = bFlag ? handle_width[width_index] + 1 : handle_width[width_index];
	QColor Color = bFlag ? kColor_Hover : kColor_Normal;
		
	pen_ = QPen(Color, width, Qt::SolidLine);
#endif
}

void ImplantCrossHandle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	painter->setPen(pen_);
	painter->drawLine(
		QPoint(rect_.left(), rect_.top() + rect_.height() * 0.5f),
		QPointF(rect_.right(), rect_.top() + rect_.height() * 0.5f));
	painter->drawLine(
		QPoint(rect_.left() + rect_.width() * 0.5f, rect_.top()),
		QPoint(rect_.left() + rect_.width() * 0.5f, rect_.bottom()));
}

void ImplantCrossHandle::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	QGraphicsItem::hoverEnterEvent(event);

	hovered_ = true;
	setHighlightEffect(hovered_);
	emit sigCrossHandleHovered(true);
}

void ImplantCrossHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	QGraphicsItem::hoverLeaveEvent(event);

	hovered_ = false;
	setHighlightEffect(hovered_);
	emit sigCrossHandleHovered(false);
}

void ImplantCrossHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsItem::mouseMoveEvent(event);

	QPointF trans = event->scenePos() - event->lastScenePos();
	emit sigTranslateCross(trans);
}

void ImplantCrossHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsItem::mouseReleaseEvent(event);
	emit sigMouseReleased();
}

////////////////////////////////////////////////
// Arrow Class
////////////////////////////////////////////////
ArrowItem::ArrowItem(const QColor &color, const QPointF &start_pos, QGraphicsItem *parent)
	: QGraphicsLineItem(parent),
	pt_start_(start_pos),
	pt_end_(kPtOrg),
	color_(color) {
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setPen(QPen(color_, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	setAcceptHoverEvents(false);

	setZValue(13);
	setVisible(false);
}

void ArrowItem::SetPosition(float start_x, float start_y)
{
	pt_start_ = QPointF(start_x, start_y);
	pt_end_ = kPtOrg;
}

// Arrow Item draw
void ArrowItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	QPen myPen = pen();
	myPen.setColor(color_);
	painter->setPen(myPen);
	painter->setBrush(color_);

	setLine(QLineF(pt_start_, pt_end_));

	double angle = ::acos(line().dx() / line().length());
	if (line().dy() >= 0)
	{
		angle = (M_PI * 2) - angle;
	}

	QPointF arrowP1 = line().p1() + QPointF(sin(angle + M_PI / 3) * kArrowSize, cos(angle + M_PI / 3) * kArrowSize);
	QPointF arrowP2 = line().p1() + QPointF(sin(angle + M_PI - M_PI / 3) * kArrowSize,
			cos(angle + M_PI - M_PI / 3) * kArrowSize);

	arrow_head_.clear();
	arrow_head_ << line().p1() << arrowP1 << arrowP2;

	painter->drawLine(line());
	painter->drawPolygon(arrow_head_);
}

ImplantHandle::ImplantHandle(QGraphicsItem *parent)
	: QGraphicsItem(parent), rect_(kBoundingRect)
{
	this->setFlag(QGraphicsItem::ItemIsSelectable, true);
	this->setAcceptedMouseButtons(Qt::MouseButton::LeftButton);
	this->setAcceptHoverEvents(true);
	this->setZValue(10.0f);

	trans_handle_ = new ImplantCrossHandle(this);
	connect(trans_handle_, SIGNAL(sigCrossHandleHovered(bool)), this, SLOT(slotCrossHovered(bool)));
	connect(trans_handle_, SIGNAL(sigTranslateCross(QPointF)), this, SLOT(slotTranslateHandle(QPointF)));
	connect(trans_handle_, SIGNAL(sigMouseReleased()), this, SLOT(slotTranslateDone()));

	int size_index = GlobalPreferences::GetInstance()->preferences_.advanced.implant_view.implant_handle_size_index;
	if (size_index != 2)
	{
		float size = 30.f * handle_multiple[size_index];
		float length = size * 2.f;
		rect_ = QRectF(-size, -size, length, length);
	}

	int width_index = GlobalPreferences::GetInstance()->preferences_.advanced.implant_view.implant_handle_width_index;
	int width = handle_width[width_index] + 3;


	rotate_handle_ = new CW3EllipseItem(kPtOrg, rect_, rect_, this);
	rotate_handle_->SetFlagHighlight(true);
	rotate_handle_->SetFlagMovable(true);
	rotate_handle_->setFlag(CW3EllipseItem::ItemIsMovable, false);
	rotate_handle_->setPens(QPen(kColor_Normal, width, Qt::SolidLine), QPen(kColor_Hover, width + 1, Qt::SolidLine));
	rotate_handle_->setZValue(12);

	connect(rotate_handle_, SIGNAL(sigHoverEllipse(bool)), this, SLOT(slotCrossHovered(bool)));
	connect(rotate_handle_, SIGNAL(sigRotateEllipse(float)), this, SLOT(slotRotateHandle(float)));
	connect(rotate_handle_, SIGNAL(sigMouseReleased()), this, SLOT(slotRotateHandleDone()));

	h_arrow_ = new ArrowItem(QColor(255, 0, 0), QPoint(rect_.left() - 5, 0), this);
	v_arrow_ = new ArrowItem(QColor(0, 255, 0), QPoint(0, rect_.top() - 5), this);

	setOpacity(0.5f);
	setVisible(false);
}

ImplantHandle::~ImplantHandle()
{
	SAFE_DELETE_OBJECT(trans_handle_);
	SAFE_DELETE_OBJECT(rotate_handle_);
	SAFE_DELETE_OBJECT(h_arrow_);
	SAFE_DELETE_OBJECT(v_arrow_);
}

/**********************************************************************************************
Initializes this object.

selected index == -1이면 disable로 간주한다.

@author	Seo Seok Man
@date	2018-04-27

@param	selected_index	The selected index.
@param	pos			  	The position.
 **********************************************************************************************/
void ImplantHandle::Enable(int selected_id, bool visible, const QPointF &pos)
{
	selected_id_ = selected_id;

	setArrowVisible(false);
	if (selected_id >= 0)
	{
		prev_pos_ = pos;
		setPos(pos);
	}

	//setVisible(visible);
}

void ImplantHandle::Disable()
{
	selected_id_ = -1;
	setArrowVisible(false);
	setVisible(false);
}

void ImplantHandle::TransformItems(const QTransform &transform)
{
	this->setPos(transform.map(this->pos()));
}

bool ImplantHandle::isHovered()
{
	bool trans_hovered = trans_handle_->hovered();
	bool rot_hovered = rotate_handle_->isHovered();
	if (trans_hovered || rot_hovered)
	{
		return true;
	}
	return false;
}

const bool ImplantHandle::IsActive() const
{
	return event_type_ == EventType::NONE ? false : true;
}

void ImplantHandle::EndEvent()
{
	event_type_ = EventType::NONE;
}

void ImplantHandle::setArrowVisible(bool b)
{
	h_arrow_->setVisible(false);
	v_arrow_->setVisible(false);
}

void ImplantHandle::setArrowDirection(int h_dir, int v_dir)
{
	if (h_dir == 1)
	{
		h_arrow_->SetPosition(rect_.right(), 0);
	}
	else
	{
		h_arrow_->SetPosition(rect_.left(), 0);
	}

	if (v_dir == 1)
	{
		v_arrow_->SetPosition(0, rect_.top());
	}
	else
	{
		v_arrow_->SetPosition(0, rect_.bottom());
	}
}

void ImplantHandle::setArrowRotation(float angle)
{
	h_arrow_->setRotation(h_arrow_->rotation() - angle);
	v_arrow_->setRotation(v_arrow_->rotation() - angle);
}

void ImplantHandle::setArrowInit()
{
	h_arrow_->setRotation(0.0f);
	v_arrow_->setRotation(0.0f);
}

void ImplantHandle::slotCrossHovered(bool hovered)
{
	setArrowVisible(hovered);
	is_hovered_ = hovered;

	emit sigHovered(hovered);
}

void ImplantHandle::slotTranslateHandle(const QPointF &trans_pt)
{
	if (!flag_movable_)
	{
		return;
	}

	QPointF curr_pos = pos();
	curr_pos += trans_pt;

	if (prev_pos_ != curr_pos)
	{
		processing_ = true;
		setPos(curr_pos);
		event_type_ = EventType::TRANSLATE;
		emit sigTranslate();
	}

	prev_pos_ = curr_pos;
}

void ImplantHandle::slotRotateHandle(float degree_angle)
{
	if (!flag_movable_)
	{
		return;
	}

	if (prev_degree_ != degree_angle)
	{
		processing_ = true;
		setArrowRotation(degree_angle);
		event_type_ = EventType::ROTATE;
		emit sigRotate(degree_angle);
	}

	prev_degree_ = degree_angle;
}

void ImplantHandle::slotRotateHandleDone()
{
	setArrowInit();
	if (processing_)
	{
		emit sigUpdate();
		processing_ = false;
	}
}

void ImplantHandle::slotTranslateDone()
{
	if (processing_)
	{
		emit sigUpdate();
		processing_ = false;
	}
}
