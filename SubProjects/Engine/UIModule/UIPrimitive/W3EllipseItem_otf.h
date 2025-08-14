#pragma once
/*=========================================================================

File:		class CW3EllipseItem_otf
Language:	C++11
Library:	Qt 5.2.0

=========================================================================*/
#include "W3EllipseItem.h"
#include "uiprimitive_global.h"

/*
	class CW3EllipseItem_otf
	 : Re-implementation of CW3EllipseItem.
	 : OTF polygon's points representation.
*/
class UIPRIMITIVE_EXPORT CW3EllipseItem_otf : public CW3EllipseItem {
public:
	CW3EllipseItem_otf(const QPointF& point);
	~CW3EllipseItem_otf(void) {}
	/**************************
		public interface.
	***************************/
	void activate(void);
	void deactivate(void);
	bool contains(const QPointF& point);

protected:
	// overriding functions.
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
	
private:
	bool m_bIsHover;
};

