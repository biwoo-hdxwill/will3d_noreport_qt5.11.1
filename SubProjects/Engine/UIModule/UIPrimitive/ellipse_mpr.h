#pragma once
/*=========================================================================

File:		class CW3EllipseItem_MPR
Language:	C++11
Library:	Qt 5.2.1

=========================================================================*/
#include <qpoint.h>

#include "W3EllipseItem.h"
#include "uiprimitive_global.h"

class QGraphicsScene;

class UIPRIMITIVE_EXPORT EllipseMPR {
public:
	EllipseMPR(const QPointF& pt, QGraphicsItem* parent = nullptr);
	~EllipseMPR();

public:
	void SetVisible(bool visibility);
	void SetVisibleCircle(bool visibility);
	void SetVisibleCenter(bool visibility);

	void SetPos(const QPointF& pt);
	QPointF Pos();
	void AddToScene(QGraphicsScene* scene);
	void InitItems(const QPointF& center, float diameter);

private:
	CW3EllipseItem* circle_;
	CW3EllipseItem* center_;
};
