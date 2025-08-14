#include "pathitem_freedraw.h"
#include <QGraphicsScene>
#include <QPainter>
#include <QGraphicsSceneEvent>
#include <QStyleOptionGraphicsItem>
#include <QSettings>

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/define_ui.h"

UIPath::UIPath(QGraphicsItem* pParent) :
	QGraphicsPathItem(pParent)
{
	m_penLine = QPen(QColor(128, 255, 0, 255),
		common::ui_define::kToolLineWidth,
		Qt::SolidLine, Qt::FlatCap);

	this->setFlag(UIPath::ItemIsSelectable, true);
	this->setFlag(UIPath::ItemIsMovable, false);
	this->setPen(m_penLine);
	this->setAcceptHoverEvents(true);
	this->setZValue(5);
}

UIPath::~UIPath()
{
	SAFE_DELETE_OBJECT(m_path);
}

void UIPath::setOpacityColor(bool bEnable, uchar alpha)
{
	QColor color = m_penLine.color();
	if (bEnable)
	{
		m_penLine.setColor(QColor(color.red(), color.green(), color.blue(), alpha));
	}
	else
	{
		m_penLine.setColor(color);
	}
	this->setPen(m_penLine);
}

void UIPath::drawingPath(const std::vector<QPointF>& points)
{
	if (points.empty())
		return;

	SAFE_DELETE_OBJECT(m_path);
	m_path = new QPainterPath(points[points.size() - 1]);

	for (int i = points.size() - 1; i >= 0; i--)
	{
		m_path->lineTo(points[i].x(), points[i].y());
	}
	this->setPath(*m_path);
}

void UIPath::SetSelected(bool selected)
{
	selected_ = selected ? true : false;
	SetUISelected(selected_);
	emit sigSelected(selected_);
}

void UIPath::SetUISelected(bool selected)
{
	selected_ = selected;
	this->setFlag(UIPath::ItemIsMovable, selected_);
#if 0
	QPen pen = m_penLine;
	QColor color = pen.color();
	color.setAlpha(selected_ ? 255 : 128);
	pen.setColor(color);
	this->setPen(pen);
#endif
	QGraphicsPathItem::setSelected(selected_);
}

void UIPath::setPen(const QPen & pen)
{
	m_penLine = pen;
	m_penLine.setCosmetic(true);
	QGraphicsPathItem::setPen(m_penLine);
}

void UIPath::paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget)
{
#if 1 // temp : update line width
	QSettings settings(GlobalPreferences::GetInstance()->ini_path(), QSettings::IniFormat);
	float line_width = settings.value("MEASURE/free_draw_line_width", GlobalPreferences::GetInstance()->preferences_.objects.measure.free_draw_line_width).toFloat();

	QPen pen = this->pen();
	pen.setWidthF(m_bHovered ? line_width + 1.0f : line_width);
	setPen(pen);
#endif

	QStyleOptionGraphicsItem* style
		= const_cast<QStyleOptionGraphicsItem*>(pOption);

	// Remove the HasFocus style state to prevent the dotted line from being drawn.
	style->state &= ~QStyle::State_HasFocus;
	style->state &= ~QStyle::State_Selected;

	pPainter->setRenderHint(QPainter::Antialiasing, true);
	QGraphicsPathItem::paint(pPainter, pOption, pWidget);
}

QPainterPath UIPath::shape() const
{
	QPainterPath path = this->path();
	QPainterPathStroker stroker;
	//stroker.setWidth(20.0f);
	stroker.setWidth(this->pen().widthF() * 2.0f);
	stroker.setJoinStyle(Qt::PenJoinStyle::BevelJoin);
	return stroker.createStroke(path);
}

void UIPath::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	QGraphicsPathItem::hoverEnterEvent(event);

	if (!m_bHovered)
	{
		QPen hoverPen = m_penLine;
		hoverPen.setWidth(m_penLine.width() + 1);
		hoverPen.setCosmetic(true);
		this->setPen(hoverPen);
		m_bHovered = true;
	}
}

void UIPath::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	QGraphicsPathItem::hoverLeaveEvent(event);

	if (m_bHovered)
	{
		QPen pen = m_penLine;
		pen.setWidth(m_penLine.width() - 1);
		pen.setCosmetic(true);
		this->setPen(pen);
		m_bHovered = false;
	}
}

void UIPath::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
	QGraphicsPathItem::mousePressEvent(event);
#if 0
	if (!selected_)
		SetUISelected(false);
#else
	SetSelected(true);
#endif
}

void UIPath::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsPathItem::mouseMoveEvent(event);

	if (!selected_)
		return;

	if (event->buttons() == Qt::LeftButton)
		emit sigTranslatePath(event->scenePos() - event->lastScenePos());
}

void UIPath::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsPathItem::mouseReleaseEvent(event);
#if 0
	SetSelected(m_bHovered);
#else
	SetSelected(false);
#endif
}

void UIPath::transformItem(const QTransform & transform)
{
	if (!m_path)
		return;

	for (int i = 0; i < m_path->elementCount(); i++)
	{
		QPointF pt = transform.map(m_path->elementAt(i));
		m_path->setElementPositionAt(i, pt.x(), pt.y());
	}

	this->setPath(*m_path);
}

QPolygonF UIPath::GetPolygon()
{
	return this->path().toFillPolygon();
}

/**********************************************************************************************
	PathitemFreedraw
 **********************************************************************************************/
PathitemFreedraw::PathitemFreedraw(QGraphicsScene* pScene) :
	m_pgScene(pScene)
{
}

PathitemFreedraw::~PathitemFreedraw()
{
	if (line_)
		m_pgScene->removeItem(line_);
	SAFE_DELETE_OBJECT(line_);
}

void PathitemFreedraw::drawingPath()
{
	if (node_pt_list_.size() <= 1)
		return;

	QPainterPath p(node_pt_list_[node_pt_list_.size() - 1]);
	for (int i = node_pt_list_.size() - 1; i >= 0; i--)
		p.lineTo(node_pt_list_[i].x(), node_pt_list_[i].y());
	line_->setPath(p);
}

void PathitemFreedraw::graphicItemsConnection()
{
	if (line_)
	{
		connect(line_, SIGNAL(sigSelected(bool)), this, SIGNAL(sigSelected(bool)));
		connect(line_, SIGNAL(sigTranslatePath(QPointF)), this, SLOT(slotTranslatePath(QPointF)));
	}
}

void PathitemFreedraw::updatePoints(const QPointF & trans)
{
	for (auto& node : node_pt_list_)
	{
		node += trans;
	}

	emit sigTranslated();
}

void PathitemFreedraw::slotTranslatePath(const QPointF & trans_pt)
{
	if (IsSelected() == false)
		return;

	updatePoints(trans_pt);
	line_->setPos(QPointF(0, 0));
	drawingPath();
}

void PathitemFreedraw::setSelected(bool)
{
	line_->SetUISelected(false);
}

void PathitemFreedraw::setVisible(const bool bFlag)
{
	if (line_)
		line_->setVisible(bFlag);
}

const bool PathitemFreedraw::IsSelected() const
{
	return line_->isSelected();
}

void PathitemFreedraw::addPoint(const QPointF & pt)
{
	if (node_pt_list_.size() == 0)
	{
		edit_start_ = true;
		line_ = new UIPath();
		line_->setZValue(52.0f);

		ApplyLineColor();

		m_pgScene->addItem(line_);
	}

	if (edit_start_)
	{
		node_pt_list_.push_back(pt);
		drawingPath();
	}
}

void PathitemFreedraw::drawingCurPath(const QPointF & pointf)
{
	if (edit_start_)
	{
		node_pt_list_.push_back(pointf);
		drawingPath();
		node_pt_list_.pop_back();
	}
}

bool PathitemFreedraw::endEdit()
{
	if (node_pt_list_.size() < 3)
		return false;
	edit_start_ = false;

	drawingPath();
	graphicItemsConnection();
	return true;
}

void PathitemFreedraw::removeItemAll()
{
	if (line_)
		m_pgScene->removeItem(line_);
}

void PathitemFreedraw::addItemAll()
{
	if (line_)
		m_pgScene->addItem(line_);
}

void PathitemFreedraw::transformItems(const QTransform & transform)
{
	line_->setPos(QPointF(0, 0));

	for (int i = 0; i < node_pt_list_.size(); i++)
	{
		node_pt_list_[i] = transform.map(node_pt_list_[i]);
	}

	drawingPath();
}

QPolygonF PathitemFreedraw::GetPolygon()
{
	if (line_)
	{
		return line_->GetPolygon();
	}
	else
		return QPolygonF();
}

void PathitemFreedraw::ApplyPreferences()
{
	ApplyLineColor();
	ApplyLineWidth();
}

void PathitemFreedraw::ApplyLineColor()
{
	QColor line_color = GlobalPreferences::GetInstance()->preferences_.objects.measure.line_color;
	QPen pen = line_->pen();
	line_color.setAlpha(pen.color().alpha());
	pen.setColor(line_color);
	line_->setPen(pen);
}

void PathitemFreedraw::ApplyLineWidth()
{
	QSettings settings(GlobalPreferences::GetInstance()->ini_path(), QSettings::IniFormat);
	float line_width = settings.value("MEASURE/free_draw_line_width", GlobalPreferences::GetInstance()->preferences_.objects.measure.free_draw_line_width).toFloat();

	QPen pen = line_->pen();
	pen.setWidthF(line_width);
	line_->setPen(pen);
}

void PathitemFreedraw::SetLineColor(const QColor& color)
{
	QPen pen = line_->pen();
	pen.setColor(color);
	line_->setPen(pen);
}

void PathitemFreedraw::SetLineWidth(const float width)
{
	QPen pen = line_->pen();
	pen.setWidthF(width);
	line_->setPen(pen);
}
