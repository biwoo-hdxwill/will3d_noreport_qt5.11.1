#include "W3PathItem_anno.h"

#include <QGraphicsScene>
#include <qmath.h>
#include <qwidget.h>
#include <QApplication>

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/Common.h"
#include <Engine/Common/Common/W3Math.h>
#include "../../Common/Common/define_ui.h"

#include "W3EllipseItem.h"
#include "W3PathItem.h"
#include "W3LabelItem_anno.h"

using namespace common;
using namespace common::ui_define;

CW3PathItem_anno::CW3PathItem_anno(
	QGraphicsScene* pScene,
	measure::PathType eLineType,
	bool bClosedPoly
) : scene_(pScene),
m_bClosedPolygon(bClosedPoly),
m_eLineType(eLineType),
m_eGuideType(measure::GuideType::LINE)
{
	line_ = new CW3PathItem();
	line_->setZValue(kZValueLine);
	line_->setMovable(true);
	scene_->addItem(line_);

	ApplyLineColor();
}

CW3PathItem_anno::~CW3PathItem_anno()
{
	clear();
}

void CW3PathItem_anno::AddPoint(const QPointF& pointf)
{
	if (m_listNodePt.size() == 0)
	{
		m_bStartEdit = true;
	}

	if (m_bStartEdit)
	{
		m_listNodePt.push_back(pointf);

		CW3EllipseItem* pEllipse = new CW3EllipseItem(pointf);
		pEllipse->setZValue(kZValuePoint);
		pEllipse->setPen(QPen(Qt::green, 2, Qt::SolidLine));
		pEllipse->setBrush(Qt::red);
		pEllipse->SetFlagHighlight(true);
		pEllipse->SetFlagMovable(true);
		scene_->addItem(pEllipse);
		m_listpNode.push_back(pEllipse);
		pEllipse->setVisible(m_bShowNodes);

		ApplyNodeColor();

		DrawingPath();
	}
}

void CW3PathItem_anno::drawingCurPath(const QPointF& pointf)
{
	if (m_bStartEdit)
	{
		m_listNodePt.push_back(pointf);
		m_listLabelPosOffset.push_back(QPointF(30.0f, -30.0f));
		DrawingPath();
		if (m_bUseLabel)
		{
			drawingGuide();
			drawingLabel();
		}
		m_listNodePt.pop_back();
		m_listLabelPosOffset.pop_back();
	}
}

void CW3PathItem_anno::addCurNode(const QPointF& pt)
{
	m_listNodePt.push_back(pt);
}

void CW3PathItem_anno::deleteCurNode()
{
	m_listNodePt.pop_back();
}

void CW3PathItem_anno::clear()
{
	if (line_)
		scene_->removeItem(line_);
	SAFE_DELETE_OBJECT(line_);

	for (auto &i : m_listpNode)
		SAFE_DELETE_OBJECT(i);
	m_listpNode.clear();

	for (auto &i : m_listpLabelGuideLine)
		SAFE_DELETE_OBJECT(i);
	m_listpLabelGuideLine.clear();

	for (auto &i : m_listpLabel)
		SAFE_DELETE_OBJECT(i);
	m_listpLabel.clear();

	m_listNodePt.clear();
	m_listSplinePt.clear();
	m_listLabelPosOffset.clear();

	m_bStartEdit = false;
}

bool CW3PathItem_anno::IsVisible()
{
	if (!line_)
	{
		return false;
	}

	return line_->isVisible();
}

void CW3PathItem_anno::setVisible(bool bFlag)
{
	if (line_ != nullptr)
		line_->setVisible(bFlag);

	if (m_bUseLabel)
	{
		for (auto &i : m_listpLabelGuideLine)
			i->setVisible(bFlag);

		for (auto &i : m_listpLabel)
			i->setVisible(bFlag);
	}

	if (m_bShowNodes)
		for (auto &i : m_listpNode)
			i->setVisible(bFlag);
}

bool CW3PathItem_anno::EndEdit()
{
	m_bStartEdit = false;

	if (line_ != nullptr)
		line_->setHighlighted(true);
	else
		return false;

	DrawingPath();

	if (m_listpNode.size() > 0)
	{
		for (auto &i : m_listpNode)
			i->SetFlagHighlight(true);
	}
	else
	{
		return false;
	}

	if (m_bUseLabel)
	{
		for (auto &i : m_listpLabelGuideLine)
			i->setHighlighted(true);

		drawingGuide();
		drawingLabel();
	}

	GraphicItemsConnection();

	if (m_bShowNodes)
		m_listpNode.front()->setBrushColor(QColor(125, 0, 255));

	return true;
}

bool CW3PathItem_anno::isNodeSelected()
{
	bool bHasSelectedItem = false;
	for (int i = 0; i < m_listpNode.size(); i++)
	{
		if (m_listpNode[i]->isSelected())
		{
			m_nCurveSelectedIdx = i;
			bHasSelectedItem = true;
		}
	}
	return bHasSelectedItem;
}

bool CW3PathItem_anno::IsLineSelected() const
{
	if (m_bUseLabel && line_->isSelected() == false)
	{
		for (auto &i : m_listpLabelGuideLine)
			if (i->isSelected())
				return true;
	}

	if (line_->isSelected())
		return true;
	return false;
}

int CW3PathItem_anno::getSelectedLabel() const
{
	if (m_listpLabel.size() == 0)
		return -1;

	if (m_bUseLabel)
	{
		int dIndex = 0;
		for (auto &i : m_listpLabel)
		{
			if (i->isSelected())
				return dIndex;
			++dIndex;
		}
	}
	return -1;
}

bool CW3PathItem_anno::isSelected()
{
	if (isNodeSelected() || IsLineSelected() || getSelectedLabel() != -1)
		return true;
	return false;
}

QPolygonF CW3PathItem_anno::getSimplipiedPolygon(const QPointF& pt)
{
	if (pt != QPointF(-1.0f, -1.0f))
		m_listNodePt.push_back(pt);

	QPainterPath p;
	QPolygonF pf;
	if (m_listNodePt.size() > 1)
	{
		if (m_eLineType == measure::PathType::LINE)
		{
			p.moveTo(m_listNodePt[m_listNodePt.size() - 1]);
			for (int i = m_listNodePt.size() - 1; i >= 0; i--)
				p.lineTo(m_listNodePt[i].x(), m_listNodePt[i].y());
			if (m_bClosedPolygon && m_bStartEdit == false)
				p.lineTo(m_listNodePt[m_listNodePt.size() - 1].x(),
					m_listNodePt[m_listNodePt.size() - 1].y());
		}
		else
		{
			m_listSplinePt.clear();
			Common::generateCubicSpline(m_listNodePt, m_listSplinePt);

			p.moveTo(m_listSplinePt[m_listSplinePt.size() - 1]);
			for (int i = m_listSplinePt.size() - 1; i >= 0; i--)
				p.lineTo(m_listSplinePt[i].x(), m_listSplinePt[i].y());
			if (m_bClosedPolygon && m_bStartEdit == false)
				p.lineTo(m_listSplinePt[m_listSplinePt.size() - 1].x(),
					m_listSplinePt[m_listSplinePt.size() - 1].y());
		}

		p.setFillRule(Qt::FillRule::WindingFill);
		QPainterPath newp = p.simplified();
		pf = newp.toFillPolygon();
	}

	if (pt != QPointF(-1.0f, -1.0f))
		m_listNodePt.pop_back();

	return pf;
}

void CW3PathItem_anno::DrawingPath()
{
	if (m_listNodePt.size() <= 1)
		return;

	if (m_eLineType == measure::PathType::LINE)
	{
		QPainterPath p(m_listNodePt[m_listNodePt.size() - 1]);
		for (int i = m_listNodePt.size() - 1; i >= 0; i--)
			p.lineTo(m_listNodePt[i].x(), m_listNodePt[i].y());
		if (m_bClosedPolygon && m_bStartEdit == false)
			p.lineTo(m_listNodePt[m_listNodePt.size() - 1].x(), m_listNodePt[m_listNodePt.size() - 1].y());

		line_->setPath(p);
	}
	else
	{
		m_listSplinePt.clear();
		Common::generateCubicSpline(m_listNodePt, m_listSplinePt);

		QPainterPath p(m_listSplinePt[m_listSplinePt.size() - 1]);
		for (int i = m_listSplinePt.size() - 1; i >= 0; i--)
		{ 
			p.lineTo(m_listSplinePt[i].x(), m_listSplinePt[i].y());
		}
		if (m_bClosedPolygon && m_bStartEdit == false)
		{
			p.lineTo(m_listSplinePt[m_listSplinePt.size() - 1].x(), m_listSplinePt[m_listSplinePt.size() - 1].y());
		}

		line_->setPath(p);
	}
}

void CW3PathItem_anno::drawingGuide()
{
	if (m_listNodePt.size() == 0)
		return;

	QPointF st, et, mt;
	int dIndex = 0;
	if (m_eGuideType == measure::GuideType::LINE)
	{
		if (m_dFixLabelPos != -1)
		{
			if (m_listpLabel.size() != 0)
			{
				int dIndex = 0;
				float min = 100000.0f;
				float srcLen;
				QPointF LabelPos = m_listNodePt[m_nGuideSelectedIdx] + m_listLabelPosOffset[0];

				for (int i = 0; i < m_listpNode.size(); i++)
				{
					srcLen = W3::getLength(LabelPos, m_listNodePt[i], 1.0f);
					if (srcLen < min)
					{
						m_listLabelPosOffset[m_dFixLabelPos] = LabelPos - m_listNodePt[i];
						dIndex = i;
						min = srcLen;
					}
				}
				m_nGuideSelectedIdx = dIndex;

				st = m_listNodePt[dIndex];
				et = st + m_listLabelPosOffset.at(0);

#if 0
				if (et.x() < 0)
					et.setX(0);
				else if (((QWidget*)scene_->parent())->width() < (et.x() + m_listpLabel.at(0)->boundingRect().width()))
					et.setX(((QWidget*)scene_->parent())->width() - m_listpLabel.at(0)->boundingRect().width());

				if (et.y() < m_listpLabel.at(0)->boundingRect().height())
					et.setY(m_listpLabel.at(0)->boundingRect().height());
				else if (((QWidget*)scene_->parent())->height() < et.y())
					et.setY(((QWidget*)scene_->parent())->height());
#endif

				QPainterPath p(st);
				p.lineTo(et);
				m_listpLabelGuideLine.at(0)->setPath(p);
			}
		}
		else
		{
			bool is_multi_label = GlobalPreferences::GetInstance()->preferences_.objects.measure.tape_line_multi_label;
			if (m_eLineType == measure::PathType::CURVE || !is_multi_label)
			{
				if (m_listpLabelGuideLine.size() == 1)
				{
					/*if (m_listNodePt.size() == 1) {
					st = m_listNodePt[0];
					et = m_listNodePt[0];
					} else if (m_listNodePt.size() % 2 == 0) {
					st = m_listNodePt[(m_listNodePt.size() - 1) / 2];
					et = m_listNodePt[(m_listNodePt.size() - 1) / 2 + 1];
					} else {
					st = m_listNodePt[m_listNodePt.size() / 2];
					et = m_listNodePt[m_listNodePt.size() / 2];
					}*/

					if (m_listNodePt.size() == 2)
					{
						st = m_listNodePt[0];
						et = m_listNodePt[1];
					}
					else
					{
						st = m_listNodePt[0];
						et = m_listNodePt[0];
					}

					if (st.rx() < et.rx())
						mt.setX(st.rx() + fabs(st.rx() - et.rx()) / 2);
					else
						mt.setX(et.rx() + fabs(et.rx() - st.rx()) / 2);

					if (st.ry() < et.ry())
						mt.setY(st.ry() + fabs(st.ry() - et.ry()) / 2);
					else
						mt.setY(et.ry() + fabs(et.ry() - st.ry()) / 2);

					st = mt;
					et = mt + m_listLabelPosOffset.at(0);

#if 0
					if (et.x() < 0)
						et.setX(0);
					else if (((QWidget*)scene_->parent())->width() < (et.x() + m_listpLabel.at(0)->boundingRect().width()))
						et.setX(((QWidget*)scene_->parent())->width() - m_listpLabel.at(0)->boundingRect().width());

					if (et.y() < m_listpLabel.at(0)->boundingRect().height())
						et.setY(m_listpLabel.at(0)->boundingRect().height());
					else if (((QWidget*)scene_->parent())->height() < et.y())
						et.setY(((QWidget*)scene_->parent())->height());
#endif
					QPainterPath p(st);
					p.lineTo(et);
					m_listpLabelGuideLine.at(0)->setPath(p);

				}
				else if (m_listpLabelGuideLine.size() > 1)
				{
					for (int i = 0; i < m_listNodePt.size() - 1; ++i)
					{
						if (i > (int)m_listpLabelGuideLine.size() - 1)
							break;

						st = m_listNodePt[i + 1];
						et = st + m_listLabelPosOffset.at(i);

#if 0
						if (et.x() < 0)
							et.setX(0);
						else if (((QWidget*)scene_->parent())->width() < (et.x() + m_listpLabel.at(i)->boundingRect().width()))
							et.setX(((QWidget*)scene_->parent())->width() - m_listpLabel.at(i)->boundingRect().width());

						if (et.y() < m_listpLabel.at(i)->boundingRect().height())
							et.setY(m_listpLabel.at(i)->boundingRect().height());
						else if (((QWidget*)scene_->parent())->height() < et.y())
							et.setY(((QWidget*)scene_->parent())->height());
#endif
						m_listpLabelGuideLine.at(i)->setPos(0, 0);
						QPainterPath p(st);
						p.lineTo(et);
						m_listpLabelGuideLine.at(i)->setPath(p);
					}
				}
			}
			else if (m_eLineType == measure::PathType::LINE && is_multi_label)
			{
				int node_size = m_listNodePt.size();
				int lable_size = m_listpLabelGuideLine.size();
				if (node_size > lable_size)
				{
					for (int i = 0; i < lable_size; ++i)
					{
						st = m_listNodePt[i];
						et = m_listNodePt[i + 1];

						if (st.rx() < et.rx())
							mt.setX(st.rx() + fabs(st.rx() - et.rx()) / 2);
						else
							mt.setX(et.rx() + fabs(et.rx() - st.rx()) / 2);

						if (st.ry() < et.ry())
							mt.setY(st.ry() + fabs(st.ry() - et.ry()) / 2);
						else
							mt.setY(et.ry() + fabs(et.ry() - st.ry()) / 2);

						st = mt;
						et = mt + m_listLabelPosOffset.at(i);

						QPainterPath p(st);
						p.lineTo(et);
						m_listpLabelGuideLine.at(i)->setPath(p);
					}
				}
			}
		}
	}
	else if (m_eGuideType == measure::GuideType::ARC)
	{
		if (m_listNodePt.size() == 3)
		{
			for (auto &i : m_listpLabelGuideLine)
			{
				st = m_listNodePt[0 + dIndex];
				mt = m_listNodePt[1 + dIndex];
				et = m_listNodePt[2 + dIndex];

				float smc = W3::getAngle(st, mt, QPointF(mt.rx() + 10, mt.ry()));
				float emc = W3::getAngle(et, mt, QPointF(mt.rx() + 10, mt.ry()));
				float sme = W3::getAngle(st, mt, et);

				float m = (st.ry() - mt.ry()) / (st.rx() - mt.rx());
				bool c = (st.ry() - mt.ry()) / (st.rx() - mt.rx()) * (et.rx() - mt.rx()) + mt.ry() < et.ry() ? true : false;

				float StartAngle;
				if (st.rx() < mt.rx() && st.ry() < mt.ry())
				{
					if (m > 0)
					{
						if (c)
							StartAngle = smc;
						else
							StartAngle = (et.ry() > mt.ry()) ? -emc : emc;
					}
				}
				else if (st.rx() < mt.rx() && st.ry() > mt.ry())
				{
					if (c)
						StartAngle = -smc;
					else
						StartAngle = (et.ry() > mt.ry()) ? -emc : emc;
				}
				else
				{
					if (m > 0)
					{
						StartAngle = c ? (360 - smc - sme) : (360 - smc);
					}
					else
					{
						StartAngle = c ? (smc - sme) : smc;
					}
				}

				QPainterPath p(mt);
				p.arcTo(QRect(mt.rx() - 25, mt.ry() - 25, 50, 50), StartAngle, sme);
				p.moveTo(mt);

				QPointF guideEnd(m_listNodePt[1] + m_listLabelPosOffset[0]);

#if 0
				if (guideEnd.x() < 0)
					guideEnd.setX(0);
				else if (((QWidget*)scene_->parent())->width() < (guideEnd.x() + m_listpLabel.at(0)->boundingRect().width()))
					guideEnd.setX(((QWidget*)scene_->parent())->width() - m_listpLabel.at(0)->boundingRect().width());

				if (guideEnd.y() < m_listpLabel.at(0)->boundingRect().height())
					guideEnd.setY(m_listpLabel.at(0)->boundingRect().height());
				else if (((QWidget*)scene_->parent())->height() < guideEnd.y())
					guideEnd.setY(((QWidget*)scene_->parent())->height());
#endif

				p.lineTo(guideEnd);
				i->setPos(0, 0);
				i->setPath(p);
				++dIndex;
			}
		}
	}
}

void CW3PathItem_anno::drawingLabel()
{
	if (m_listNodePt.size() == 0)
		return;

	QPointF st, et, mt;
	if (m_dFixLabelPos != -1)
	{
		if (m_eGuideType == measure::GuideType::ARC)
			st = m_listNodePt[m_dFixLabelPos];
		else
			st = m_listNodePt[m_nGuideSelectedIdx];
		et = st + m_listLabelPosOffset[0];
		if (m_listpLabel.size() != 0)
		{
			//m_listpLabel.at(0)->setPos(et.rx(), et.ry() - 20);
			QPointF pos(et.rx(), et.ry() - m_listpLabel.at(0)->boundingRect().height());

#if 0
			if (pos.x() < 0)
				pos.setX(0);
			else if (((QWidget*)scene_->parent())->width() < (pos.x() + m_listpLabel.at(0)->boundingRect().width()))
				pos.setX(((QWidget*)scene_->parent())->width() - m_listpLabel.at(0)->boundingRect().width());

			if (pos.y() < 0)
				pos.setY(0);
			else if (((QWidget*)scene_->parent())->height() < (pos.y() + m_listpLabel.at(0)->boundingRect().height()))
				pos.setY(((QWidget*)scene_->parent())->height() - m_listpLabel.at(0)->boundingRect().height());
#endif

			m_listpLabel.at(0)->setPos(pos);
			m_listLabelPosOffset.at(0) = QPointF((pos - st).x(), (pos - st).y() + m_listpLabel.at(0)->boundingRect().height());
		}
	}
	else
	{
		bool is_multi_label = GlobalPreferences::GetInstance()->preferences_.objects.measure.tape_line_multi_label;
		if (m_eLineType == measure::PathType::CURVE || !is_multi_label)
		{
			if (m_listpLabel.size() == 1)
			{
				/*if (m_listNodePt.size() == 1) {
				st = m_listNodePt[0];
				et = m_listNodePt[0];
				} else if (m_listNodePt.size() % 2 == 0) {
				st = m_listNodePt[(m_listNodePt.size() - 1) / 2];
				et = m_listNodePt[(m_listNodePt.size() - 1) / 2 + 1];
				} else {
				st = m_listNodePt[m_listNodePt.size() / 2];
				et = m_listNodePt[m_listNodePt.size() / 2];
				}*/

				if (m_listNodePt.size() == 2)
				{
					st = m_listNodePt[0];
					et = m_listNodePt[1];
				}
				else
				{
					st = m_listNodePt[0];
					et = m_listNodePt[0];
				}

				if (st.rx() < et.rx())
					mt.setX(st.rx() + fabs(st.rx() - et.rx()) / 2);
				else
					mt.setX(et.rx() + fabs(et.rx() - st.rx()) / 2);

				if (st.ry() < et.ry())
					mt.setY(st.ry() + fabs(st.ry() - et.ry()) / 2);
				else
					mt.setY(et.ry() + fabs(et.ry() - st.ry()) / 2);

				st = mt;
				et = st + m_listLabelPosOffset.at(0);

				//m_listpLabel.at(0)->setPos(et.rx(), et.ry() - 20);
				QPointF pos(et.rx(), et.ry() - m_listpLabel.at(0)->boundingRect().height());

#if 0
				if (pos.x() < 0)
					pos.setX(0);
				else if (((QWidget*)scene_->parent())->width() < (pos.x() + m_listpLabel.at(0)->boundingRect().width()))
					pos.setX(((QWidget*)scene_->parent())->width() - m_listpLabel.at(0)->boundingRect().width());

				if (pos.y() < 0)
					pos.setY(0);
				else if (((QWidget*)scene_->parent())->height() < (pos.y() + m_listpLabel.at(0)->boundingRect().height()))
					pos.setY(((QWidget*)scene_->parent())->height() - m_listpLabel.at(0)->boundingRect().height());
#endif
				m_listpLabel.at(0)->setPos(pos);
				m_listLabelPosOffset.at(0) = QPointF((pos - st).x(), (pos - st).y() + m_listpLabel.at(0)->boundingRect().height());
			}
			else if (m_listpLabel.size() > 1)
			{
				for (int i = 0; i < m_listpLabel.size(); i++)
				{
					if (i + 1 > m_listNodePt.size() - 1)
						break;
					st = m_listNodePt[i + 1];
					et = st + m_listLabelPosOffset.at(i);
					//m_listpLabel.at(0)->setPos(et.rx(), et.ry() - 20);
					QPointF pos(et.rx(), et.ry() - m_listpLabel.at(i)->boundingRect().height());

#if 0
					if (pos.x() < 0)
						pos.setX(0);
					else if (((QWidget*)scene_->parent())->width() < (pos.x() + m_listpLabel.at(i)->boundingRect().width()))
						pos.setX(((QWidget*)scene_->parent())->width() - m_listpLabel.at(i)->boundingRect().width());

					if (pos.y() < 0)
						pos.setY(0);
					else if (((QWidget*)scene_->parent())->height() < (pos.y() + m_listpLabel.at(i)->boundingRect().height()))
						pos.setY(((QWidget*)scene_->parent())->height() - m_listpLabel.at(i)->boundingRect().height());
#endif
					m_listpLabel.at(i)->setPos(pos);
					m_listLabelPosOffset.at(i) = QPointF((pos - st).x(), (pos - st).y() + m_listpLabel.at(i)->boundingRect().height());
				}
			}
		}
		else if (m_eLineType == measure::PathType::LINE && is_multi_label)
		{
			for (int i = 0; i < m_listpLabel.size(); ++i)
			{
				if (i + 1 > m_listNodePt.size() - 1)
					break;

				st = m_listNodePt[i];
				et = m_listNodePt[i + 1];

				if (st.rx() < et.rx())
					mt.setX(st.rx() + fabs(st.rx() - et.rx()) / 2);
				else
					mt.setX(et.rx() + fabs(et.rx() - st.rx()) / 2);

				if (st.ry() < et.ry())
					mt.setY(st.ry() + fabs(st.ry() - et.ry()) / 2);
				else
					mt.setY(et.ry() + fabs(et.ry() - st.ry()) / 2);

				st = mt;
				et = st + m_listLabelPosOffset.at(i);

				QPointF pos(et.rx(), et.ry() - m_listpLabel.at(i)->boundingRect().height());

				m_listpLabel.at(i)->setPos(pos);
				m_listLabelPosOffset.at(i) = QPointF((pos - st).x(), (pos - st).y() + m_listpLabel.at(i)->boundingRect().height());
			}
		}		
	}
}

void CW3PathItem_anno::GraphicItemsConnection()
{
	if (line_)
	{
		connect(line_, SIGNAL(sigMouseReleased()), this, SIGNAL(sigMouseReleased()));
		connect(line_, SIGNAL(sigHoverPath(bool)), this, SLOT(slotHighlightCurve(bool)));
		connect(line_, SIGNAL(sigTranslatePath(QPointF)), this, SLOT(slotTranslatePath(QPointF)));
	}

	for (auto &i : m_listpLabel)
	{
		if (i)
		{
			connect(i, SIGNAL(sigHoverLabel(bool)), this, SLOT(slotHighlightCurve(bool)));
			connect(i, SIGNAL(sigTranslateLabel(QPointF)), this, SLOT(slotTranslateLabel(QPointF)));
		}
	}

	for (auto &i : m_listpNode)
		connect(i, SIGNAL(sigTranslateEllipse(QPointF)), this, SLOT(slotTranslateEllipse(QPointF)));

	for (auto &i : m_listpLabelGuideLine)
		connect(i, SIGNAL(sigTranslatePath(QPointF)), this, SLOT(slotTranslateGuide(QPointF)));
}

void CW3PathItem_anno::slotHighlightCurve(bool highlighted)
{
	for (auto &i : m_listpNode)
	{
		i->SetHighlight(highlighted);
	}
}

void CW3PathItem_anno::updatePath()
{
	m_listNodePt.at(m_nCurveSelectedIdx) = m_listpNode.at(m_nCurveSelectedIdx)->pos();
	line_->setPos(QPointF(0, 0));
	DrawingPath();
	if (m_bUseLabel)
	{
		for (auto &i : m_listpLabelGuideLine)
			i->setPos(QPointF(0, 0));
		drawingGuide();
		drawingLabel();
	}
	emit sigTranslated();
}

void CW3PathItem_anno::updatePoints(const QPointF trans)
{
	for (int i = 0; i < m_listNodePt.size(); i++)
	{
		QPointF transPos = m_listNodePt.at(i) + trans;
		m_listNodePt.at(i) = transPos;
	}

	for (int i = 0; i < m_listpNode.size(); i++)
	{
		m_listpNode.at(i)->setPos(m_listpNode.at(i)->pos() + trans);
	}

	emit sigTranslated();
}

void CW3PathItem_anno::slotTranslateEllipse(const QPointF& pos)
{
	if (isNodeSelected() == false)
		return;

	updatePath();
	emit sigTranslated();
}

void CW3PathItem_anno::slotTranslatePath(const QPointF& ptTrans)
{
	if (isSelected() == false)
		return;

	updatePoints(ptTrans);
	line_->setPos(QPointF(0, 0));
	DrawingPath();

	if (m_bUseLabel)
	{
		//for (auto &i : m_listpLabelGuideLine)
			//i->setPos(QPointF(0, 0));
		drawingGuide();
		drawingLabel();
	}
	emit sigTranslated();
}

void CW3PathItem_anno::slotTranslateGuide(const QPointF& ptTrans)
{
	if (isSelected() == false)
		return;

	updatePoints(ptTrans);
	line_->setPos(QPointF(0, 0));
	DrawingPath();

	if (m_bUseLabel)
	{
		//if (m_listpLabel.size() > 1)
		drawingGuide();
		drawingLabel();
	}
	emit sigTranslated();
}

void CW3PathItem_anno::slotTranslateLabel(QPointF ptTrans)
{
	if (isSelected() == false)
		return;

	if (m_bUseLabel)
	{
		int dSelectedLabel = getSelectedLabel();
		QPointF pt = m_listLabelPosOffset.at(dSelectedLabel) + ptTrans;

		////
		//if (pt.x() < 0)
		//	pt.setX(0);
		//else if (((QWidget*)scene_->parent())->width() < (pt.x() + m_listpLabel.at(dSelectedLabel)->boundingRect().width()))
		//	pt.setX(((QWidget*)scene_->parent())->width() - m_listpLabel.at(dSelectedLabel)->boundingRect().width());

		//if (pt.y() < 0)
		//	pt.setY(0);
		//else if (((QWidget*)scene_->parent())->height() < (pt.y() + m_listpLabel.at(dSelectedLabel)->boundingRect().height()))
		//	pt.setY(((QWidget*)scene_->parent())->height() - m_listpLabel.at(dSelectedLabel)->boundingRect().height());
		////

		m_listLabelPosOffset.at(dSelectedLabel) = pt;
		m_listpLabelGuideLine.at(dSelectedLabel)->setPos(0, 0);
		drawingGuide();
	}
}

void CW3PathItem_anno::setLabel(const QString& str)
{
	m_bUseLabel = true;

	if (m_listpLabelGuideLine.size() == 0)
	{
		CW3PathItem *pGuide = new CW3PathItem();
		pGuide->setZValue(kZValueGuideLine);
		pGuide->setLineStatic(true);
		pGuide->setMovable(false);
		pGuide->setPen(QPen(Qt::yellow, kToolLineWidth, Qt::DotLine, Qt::FlatCap));
		m_listpLabelGuideLine.push_back(pGuide);
		scene_->addItem(m_listpLabelGuideLine.at(0));
	}

	if (m_listpLabel.size() == 0)
	{
		CW3LabelItem_anno *pLabel = new CW3LabelItem_anno;
		pLabel->setZValue(kZValueLabel);
		connect(pLabel, SIGNAL(sigPressed()), this, SIGNAL(sigLabelClicked()));	// only for profile
		m_listpLabel.push_back(pLabel);

		ApplyTextColor();
		ApplyTextSize();

		scene_->addItem(m_listpLabel.at(0));
	}

	if (m_listLabelPosOffset.size() == 0)
		m_listLabelPosOffset.push_back(QPointF(30, -30));

	m_listpLabel.at(0)->setPlainText(str);
	drawingLabel();
	drawingGuide();
}

void CW3PathItem_anno::setMultiLabel(std::vector<QString> *strArr)
{
	m_bUseLabel = true;
	if (strArr->size() != m_listpLabel.size())
	{
		for (int i = m_listpLabel.size(); i < strArr->size(); i++)
		{
			CW3PathItem *pGuide = new CW3PathItem();
			pGuide->setZValue(kZValueGuideLine);
			pGuide->setLineStatic(true);
			pGuide->setPen(QPen(Qt::yellow, kToolLineWidth, Qt::DotLine, Qt::FlatCap));
			m_listpLabelGuideLine.push_back(pGuide);
			m_listLabelPosOffset.push_back(QPointF(30, -30));

			CW3LabelItem_anno *pLabel = new CW3LabelItem_anno();
			pLabel->setZValue(kZValueLabel);
			m_listpLabel.push_back(pLabel);
			scene_->addItem(pGuide);
			scene_->addItem(pLabel);
		}
	}

	for (int i = 0; i < strArr->size(); i++)
	{
		m_listpLabel.at(i)->setPlainText(strArr->at(i));
	}

	drawingLabel();
	drawingGuide();
}

QString CW3PathItem_anno::getLabel(int dIndex)
{
	if (dIndex == -1)
		return "";
	return m_listpLabel.at(dIndex)->toPlainText();
}

QString CW3PathItem_anno::getLabel()
{
	return m_listpLabel.at(0)->toPlainText();
}

void CW3PathItem_anno::displayNode(bool bVisible)
{
	m_bShowNodes = bVisible;
	for (auto &i : m_listpNode)
		i->setVisible(bVisible);
}

void CW3PathItem_anno::deleteLastPoint()
{
	scene_->removeItem(m_listpNode[m_listpNode.size() - 1]);
	SAFE_DELETE_OBJECT(m_listpNode[m_listpNode.size() - 1]);
	m_listpNode.pop_back();
	m_listNodePt.pop_back();
}

void CW3PathItem_anno::shiftCurve(const QPointF &newSceneCenter,
	const QPointF &oldSceneCenter, float scale)
{
	line_->setPos(QPointF(0, 0));

	QPointF tmp;
	for (int i = 0; i < m_listNodePt.size(); i++)
	{
		tmp = newSceneCenter + (m_listNodePt.at(i) - oldSceneCenter) * scale;
		m_listNodePt.at(i) = tmp;
	}

	for (int i = 0; i < m_listpNode.size(); i++)
	{
		tmp = newSceneCenter + (m_listpNode.at(i)->pos() - oldSceneCenter) * scale;
		m_listpNode.at(i)->setPos(tmp);
	}

	for (int i = 0; i < m_listLabelPosOffset.size(); i++)
	{
		m_listLabelPosOffset[i] = m_listLabelPosOffset.at(i) * scale;
	}

	DrawingPath();

	if (m_bUseLabel)
	{
		drawingGuide();
		drawingLabel();
	}
}

void CW3PathItem_anno::shiftCurve(const QPointF &shift)
{
	line_->setPos(QPointF(0, 0));

	QPointF tmp;
	for (int i = 0; i < m_listNodePt.size(); i++)
	{
		m_listNodePt.at(i) += shift;
	}

	for (int i = 0; i < m_listpNode.size(); i++)
	{
		m_listpNode.at(i)->setPos(m_listpNode.at(i)->pos() + shift);
	}

	DrawingPath();

	if (m_bUseLabel)
	{
		drawingGuide();
		drawingLabel();
	}
}

void CW3PathItem_anno::transformItems(const QTransform & transform)
{
	line_->setPos(QPointF(0, 0));

	for (int i = 0; i < m_listNodePt.size(); i++)
	{
		m_listNodePt[i] = transform.map(m_listNodePt[i]);
	}

	for (int i = 0; i < m_listpNode.size(); i++)
	{
		m_listpNode[i]->setPos(transform.map(m_listpNode[i]->pos()));
	}

	DrawingPath();

	if (m_bUseLabel)
	{
		drawingGuide();
		drawingLabel();
	}
}

void CW3PathItem_anno::ApplyPreferences()
{
	ApplyNodeColor();
	ApplyLineColor();
	ApplyTextColor();
	ApplyTextSize();
}

void CW3PathItem_anno::ApplyNodeColor()
{
	QColor line_color = GlobalPreferences::GetInstance()->preferences_.objects.measure.line_color;
	for (int i = 0; i < m_listpNode.size(); i++)
		m_listpNode[i]->setPenColor(line_color);
}

void CW3PathItem_anno::ApplyLineColor()
{
	if (fix_line_color_)
	{
		return;
	}

	QColor line_color = GlobalPreferences::GetInstance()->preferences_.objects.measure.line_color;
	QPen pen = line_->pen();
	pen.setColor(line_color);
	line_->setPen(pen);
}

void CW3PathItem_anno::ApplyTextColor()
{
	QColor text_color = GlobalPreferences::GetInstance()->preferences_.objects.measure.text_color;
	for (int i = 0; i < m_listpLabel.size(); i++)
		m_listpLabel[i]->SetTextColor(text_color);
}

void CW3PathItem_anno::ApplyTextSize()
{
	GlobalPreferences::Size text_size = GlobalPreferences::GetInstance()->preferences_.objects.measure.text_size;
	QFont font = QApplication::font();
	font.setPixelSize(font.pixelSize() + (static_cast<int>(text_size) * 2) + 1);
	for (int i = 0; i < m_listpLabel.size(); i++)
	{
		m_listpLabel[i]->setFont(font);
	}
}

void CW3PathItem_anno::SetLineWidth(const float width)
{
	QPen pen = line_->pen();
	pen.setWidthF(width);
	line_->setPen(pen);
}

void CW3PathItem_anno::SetLineColor(const QColor& color)
{
	fix_line_color_ = true;

	QPen pen = line_->pen();
	pen.setColor(color);
	line_->setPen(pen);
}
