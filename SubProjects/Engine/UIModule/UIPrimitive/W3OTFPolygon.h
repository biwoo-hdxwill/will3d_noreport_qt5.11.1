#pragma once
/*=========================================================================

File:		class CW3EllipseItem
Language:	C++11
Library:	Qt 5.2.0

=========================================================================*/
#include <qpoint.h>
#include <qlist.h>
#include "uiprimitive_global.h"

#define kTFHeight	180		// QGraphicsScene coordinate. // fixed view height : 200
#define kTFMargin	20		// QGraphicsScene coordinate.

class CW3TextItem_otf;
class CW3LineItem_otf;
class CW3EllipseItem_otf;
class CW3PolygonItem_otf;
class QGraphicsEllipseItem;
class QGraphicsScene;

/*
	class CW3OTFPolygon
	 : represents polygon / points / lines / texts.
	 : provide interface for this polygon interactions.
*/
class UIPRIMITIVE_EXPORT CW3OTFPolygon {
public:
	CW3OTFPolygon(void);
	CW3OTFPolygon(int index);
	~CW3OTFPolygon(void);
	/**************************
		public interface.
	***************************/
	void addToScene(QGraphicsScene *scene);
	void removeFromScene(QGraphicsScene *scene);

	void setDefaultPolygon(const QPointF& point, const int offset);
	void setDefaultColor();

	void movePolygon(float dx, int offset);
	void movePoint(int idx, float dx, float dy, int offset);
	void moveLine(int idx, float dx, float dy, int offset);

	void addPoint(const QPointF& point, const int offset, QGraphicsScene *scene);
	void removePoint(const int pointIdx, QGraphicsScene *scene);

	void deactivateAll(void);

	QList<QGraphicsEllipseItem*>*	getColorObject() { return &m_colorObjectList; };
	void setColorObject(QList<QGraphicsEllipseItem*> colorObject) { m_colorObjectList = colorObject; };
	int	getIndex() { return m_iIndex; };
	void	setIndex(int index) { m_iIndex = index; };

	inline CW3PolygonItem_otf*			getPolygon(void) { return m_polygon; }
	inline QList<CW3EllipseItem_otf*>&	getPointList(void) { return m_listPoint; }
	inline QList<CW3LineItem_otf*>&		getLineList(void) { return m_listLine; }
	inline QList<CW3TextItem_otf*>&		getTextList(void) { return m_listText; }

private:
	void clearUp(void);

private:
	// private member fields.
	//int						m_iOffset;
	CW3PolygonItem_otf			*m_polygon;
	QList<CW3EllipseItem_otf*>	m_listPoint;
	QList<CW3LineItem_otf*>		m_listLine;
	QList<CW3TextItem_otf*>		m_listText;

	QList<QGraphicsEllipseItem*>	m_colorObjectList;
	int	m_iIndex;
};

