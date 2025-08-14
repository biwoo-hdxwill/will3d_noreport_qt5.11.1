#include "W3OTFPolygon.h"
#include <qgraphicsscene.h>

#include "../../Common/Common/W3Memory.h"

#include "W3TextItem_otf.h"
#include "W3LineItem_otf.h"
#include "W3EllipseItem_otf.h"
#include "W3PolygonItem_otf.h"

CW3OTFPolygon::CW3OTFPolygon(void)
	: m_polygon(nullptr) {
	m_polygon = new CW3PolygonItem_otf();
	m_iIndex = 0;
}

CW3OTFPolygon::CW3OTFPolygon(int index)
	: m_polygon(nullptr), m_iIndex(index) {
	m_polygon = new CW3PolygonItem_otf();
}

CW3OTFPolygon::~CW3OTFPolygon(void) {
	SAFE_DELETE_OBJECT(m_polygon);
	m_listPoint.clear();
	m_listLine.clear();
	m_listText.clear();
}

void CW3OTFPolygon::addToScene(QGraphicsScene *scene) {
	for (const auto &i : m_listPoint)	scene->addItem(i);
	for (const auto &i : m_listLine)	scene->addItem(i);
	for (const auto &i : m_listText)	scene->addItem(i);
	scene->addItem(m_polygon);
}

void CW3OTFPolygon::removeFromScene(QGraphicsScene *scene) {
	scene->removeItem(m_polygon);
	for (const auto &i : m_listPoint)	scene->removeItem(i);
	for (const auto &i : m_listLine)	scene->removeItem(i);
	for (const auto &i : m_listText)	scene->removeItem(i);
	this->clearUp();
}

void CW3OTFPolygon::clearUp(void) {
	SAFE_DELETE_OBJECT(m_polygon);
	m_listPoint.clear();
	m_listLine.clear();
	m_listText.clear();
}

void CW3OTFPolygon::setDefaultPolygon(const QPointF& point, const int offset) {
	clearUp();
	int position = static_cast<int>(point.x());

	// set points, lines, texts.
	QPolygon poly;
	QPoint pt1(position - 100, kTFHeight - kTFMargin);
	QPoint pt2(position - 50, kTFMargin);
	QPoint pt3(position + 50, kTFMargin);
	QPoint pt4(position + 100, kTFHeight - kTFMargin);

	poly.append(pt1);
	poly.append(pt2);
	poly.append(pt3);
	poly.append(pt4);

	m_polygon = new CW3PolygonItem_otf();
	m_polygon->setPolygon(poly);

	// add points.
	m_listPoint.append(new CW3EllipseItem_otf(pt1));	//m_listPoint.at(0)->setIdx(0);
	m_listPoint.append(new CW3EllipseItem_otf(pt2));	//m_listPoint.at(1)->setIdx(1);
	m_listPoint.append(new CW3EllipseItem_otf(pt3));	//m_listPoint.at(2)->setIdx(2);
	m_listPoint.append(new CW3EllipseItem_otf(pt4));	//m_listPoint.at(3)->setIdx(3);

	// add lines.
	m_listLine.append(new CW3LineItem_otf(pt2, pt3));

	m_listText.append(new CW3TextItem_otf(QPointF(pt1.x(), pt1.y() - 25),
										  static_cast<float>(pt1.x() + offset)));
	m_listText.append(new CW3TextItem_otf(QPointF(pt2.x(), pt2.y() - 25),
										  static_cast<float>(pt2.x() + offset)));
	m_listText.append(new CW3TextItem_otf(QPointF(pt3.x(), pt3.y() - 25),
										  static_cast<float>(pt3.x() + offset)));
	m_listText.append(new CW3TextItem_otf(QPointF(pt4.x(), pt4.y() - 25),
										  static_cast<float>(pt4.x() + offset)));
}

void CW3OTFPolygon::setDefaultColor() {
	QGraphicsEllipseItem *color = new QGraphicsEllipseItem(-5, -5, 10, 10);
	color->setFlag(QGraphicsEllipseItem::ItemIgnoresTransformations, true);
	color->setPos(0, kTFHeight - kTFMargin / 2 + 5);
	color->setPen(QPen(Qt::black));
	color->setBrush(QBrush(Qt::white));

	m_colorObjectList.append(color);
}

void CW3OTFPolygon::movePolygon(float dx, int offset) {
	QPolygonF tmp = m_polygon->polygon();
	tmp.translate(dx, 0);
	m_polygon->setPolygon(tmp);
	for (int i = 0; i < m_listPoint.size(); i++)
		m_listPoint.at(i)->moveBy(dx, 0.0f);
	for (const auto &i : m_listLine)
		i->setLine(i->line().x1() + dx, i->line().y1(), i->line().x2() + dx, i->line().y2());
	for (int i = 0; i < m_listText.size(); i++) {
		m_listText.at(i)->moveBy(dx, 0.0f);
		QString str;
		str.append("[" + QString().setNum(m_listText.at(i)->x() + offset, 'f', 0) + "]");
		m_listText.at(i)->setPlainText(str);
	}
	for (int i = 0; i < m_colorObjectList.size(); i++) {
		m_colorObjectList.at(i)->moveBy(dx, 0.0f);
	}
}

void CW3OTFPolygon::movePoint(int idx, float dx, float dy, int offset) {
	// move specified point & update polygon.
	m_listPoint.at(idx)->moveBy(dx, dy);
	QPolygonF tmp = m_polygon->polygon();
	QPointF point(m_listPoint.at(idx)->x(), m_listPoint.at(idx)->y());
	tmp.replace(idx, point);
	m_polygon->setPolygon(tmp);

	// move text.
	m_listText.at(idx)->moveBy(dx, dy);
	QString str("[" + QString().setNum(m_listText.at(idx)->x() + offset, 'f', 0) + "]");
	m_listText.at(idx)->setPlainText(str);

	// move line.
	if (m_listLine.empty())
		return;	// no line component.
	if (idx == 0 || idx == m_listPoint.size() - 1)
		return;	// no line editing.

	if (idx == 1) // one-end.
	{
		CW3LineItem_otf *tmp = m_listLine.at(0);
		tmp->setLine(m_listPoint.at(idx)->x(), m_listPoint.at(idx)->y(),
					 tmp->line().x2(), tmp->line().y2());
	} else if (idx == m_listPoint.size() - 2) // one-end.
	{
		CW3LineItem_otf *tmp = m_listLine.at(idx - 2);
		tmp->setLine(tmp->line().x1(), tmp->line().y1(), m_listPoint.at(idx)->x(),
					 m_listPoint.at(idx)->y());
	} else // inter-line's point.
	{
		CW3LineItem_otf *tmp1 = m_listLine.at(idx - 2);
		CW3LineItem_otf *tmp2 = m_listLine.at(idx - 1);
		tmp1->setLine(tmp1->line().x1(), tmp1->line().y1(), m_listPoint.at(idx)->x(),
					  m_listPoint.at(idx)->y());
		tmp2->setLine(m_listPoint.at(idx)->x(), m_listPoint.at(idx)->y(),
					  tmp2->line().x2(), tmp2->line().y2());
	}
}

void CW3OTFPolygon::addPoint(const QPointF& point, const int offset, QGraphicsScene *scene) {
	QPointF newPoint;
	newPoint.setX(point.x());
	QPolygonF tmpPoly = m_polygon->polygon();

	// get new point's position. ( get point index of polygon)
	int idx = -1;
	if (point.x() < m_listPoint.at(0)->x())
		idx = 0;
	else if (point.x() > m_listPoint.last()->x())
		idx = m_listPoint.size();
	else {
		for (int i = 0; i < m_listPoint.size() - 1; i++) {
			if (point.x() >= m_listPoint.at(i)->x()
				&& point.x() < m_listPoint.at(i + 1)->x()) {
				idx = i + 1;
				break;
			}
		}
	}

	if (idx == -1) // failed.
		return;

	// get new point's position. (actual position at scene)
	if (idx == 0 || idx == m_listPoint.size())
		newPoint.setY(kTFHeight - kTFMargin);
	else {
		newPoint.setY(
			(m_listPoint.at(idx)->y() - m_listPoint.at(idx - 1)->y()) *
			(newPoint.x() - m_listPoint.at(idx - 1)->x()) /
			(m_listPoint.at(idx)->x() - m_listPoint.at(idx - 1)->x()) +
			m_listPoint.at(idx - 1)->y()
		);
	}

	// add new point to polygon.
	tmpPoly.insert(idx, newPoint);
	m_polygon->setPolygon(tmpPoly);

	// add new point & other information, to lists.
	m_listPoint.insert(idx, new CW3EllipseItem_otf(newPoint));
	scene->addItem(m_listPoint.at(idx));

	m_listText.insert(idx, new CW3TextItem_otf(QPointF(newPoint.x(), newPoint.y() - 25),
											   static_cast<float>(newPoint.x() + offset)));
	scene->addItem(m_listText.at(idx));

	// remove & add line.
	if (idx == 1) { // add one line.
		m_listLine.insert(0, new CW3LineItem_otf(m_listPoint.at(1)->pos(),
												 m_listPoint.at(2)->pos()));
		scene->addItem(m_listLine.at(0));
	} else if (idx == m_listPoint.size() - 2) { // add one line.
		m_listLine.push_back(new CW3LineItem_otf(m_listPoint.at(idx - 1)->pos(),
												 m_listPoint.at(idx)->pos()));
		scene->addItem(m_listLine.last());
	} else { // remove one line & add two line.
		scene->removeItem(m_listLine.at(idx - 2));
		delete(m_listLine.at(idx - 2));
		m_listLine.replace(idx - 2, new CW3LineItem_otf(m_listPoint.at(idx - 1)->pos(),
														m_listPoint.at(idx)->pos()));
		m_listLine.insert(idx - 1, new CW3LineItem_otf(m_listPoint.at(idx)->pos(),
													   m_listPoint.at(idx + 1)->pos()));
		scene->addItem(m_listLine.at(idx - 2));
		scene->addItem(m_listLine.at(idx - 1));
	}

	// set polygon color.
	QPen pen = m_polygon->pen();
	pen.setColor(Qt::red);
	m_polygon->setPen(pen);
}

void CW3OTFPolygon::removePoint(const int pointIdx, QGraphicsScene *scene) {
	// remove point of index(idx).
	if (pointIdx == -1)	return;
	QPolygonF tmpPoly = m_polygon->polygon();

	// remove from scene.
	scene->removeItem(m_listPoint.at(pointIdx));
	scene->removeItem(m_listText.at(pointIdx));

	// update if end-point.
	if (pointIdx == 0) {
		QPointF point(tmpPoly.at(pointIdx + 1));
		point.setY(kTFHeight - kTFMargin);
		tmpPoly.replace(pointIdx + 1, point);
		m_listPoint.at(pointIdx + 1)->setY(kTFHeight - kTFMargin);
		m_listText.at(pointIdx + 1)->setY(kTFHeight - kTFMargin - 25);
	} else if (pointIdx == m_listPoint.size() - 1) {
		QPointF point(tmpPoly.at(pointIdx - 1));
		point.setY(kTFHeight - kTFMargin);
		tmpPoly.replace(pointIdx - 1, point);
		m_listPoint.at(pointIdx - 1)->setY(kTFHeight - kTFMargin);
		m_listText.at(pointIdx - 1)->setY(kTFHeight - kTFMargin - 25);
	}

	// remove and modify line.
	if (pointIdx == 0 || pointIdx == 1) { // left-end points.
		// remove line.
		scene->removeItem(m_listLine.at(0));
		delete(m_listLine.at(0));
		m_listLine.removeAt(0);
	} else if (pointIdx == (int)m_listPoint.size() - 1 || pointIdx == (int)m_listPoint.size() - 2) { // right-end points.
		// remove line.
		scene->removeItem(m_listLine.last());
		delete(m_listLine.last());
		m_listLine.removeLast();
	} else {
		// remove 1, modify 1. (or remove 2, add 1)
		scene->removeItem(m_listLine.at(pointIdx - 1));
		delete(m_listLine.at(pointIdx - 1));
		m_listLine.removeAt(pointIdx - 1);
		m_listLine.at(pointIdx - 2)->setLine(m_listLine.at(pointIdx - 2)->line().x1(),
											 m_listLine.at(pointIdx - 2)->line().y1(),
											 m_listPoint.at(pointIdx + 1)->pos().x(),
											 m_listPoint.at(pointIdx + 1)->pos().y());
	}

	// update.
	// remove from TFPOLYGON.
	tmpPoly.removeAt(pointIdx);
	m_listPoint.removeAt(pointIdx);
	m_listText.removeAt(pointIdx);
	m_polygon->setPolygon(tmpPoly);
}

void CW3OTFPolygon::moveLine(int idx, float dx, float dy, int offset) {
	int ptIdx1 = idx + 1;
	int ptIdx2 = idx + 2;

	this->movePoint(ptIdx1, dx, dy, offset);
	this->movePoint(ptIdx2, dx, dy, offset);
}

void CW3OTFPolygon::deactivateAll(void) {
	m_polygon->deactivate();
	for (const auto& i : m_listPoint)	i->deactivate();
	for (const auto& i : m_listLine)	i->deactivate();
}
