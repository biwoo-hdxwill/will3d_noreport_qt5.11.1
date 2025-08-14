#pragma once
/*=========================================================================

File:		class CW3EllipseItem
Language:	C++11
Library:	Qt 5.2.0

=========================================================================*/
#include <qgraphicsitem.h>
#include "uiprimitive_global.h"

class UIPRIMITIVE_EXPORT CW3PolygonItem_otf : public QObject, public QGraphicsPolygonItem
{
	Q_OBJECT
public:
	CW3PolygonItem_otf(void);
	~CW3PolygonItem_otf(void);
	/**************************
		public interface.
	***************************/
	void activate(void);

	void select(void);
	inline bool isSelected() const { return m_bIsSelected; };

	void deactivate(void);
protected:
	//virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
	//virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	//virtual bool eventFilter(QObject*, QEvent*);

private:
	// private member fields.
	bool m_bIsSelected = false;
};
