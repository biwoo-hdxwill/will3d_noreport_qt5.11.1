#pragma once
/*=========================================================================

File:		class CW3RectItem_contour
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-11-23
Last modify:	2015-11-23

=========================================================================*/
#include <qobject.h>
#include <qpoint.h>
#include <QGraphicsRectItem>

#include "../../Common/Common/W3Enum.h"

#include "uiprimitive_global.h"

class UIPRIMITIVE_EXPORT CW3RectItem_rangeMPR : public QObject, public QGraphicsRectItem {
	Q_OBJECT

public:
	CW3RectItem_rangeMPR(UILineType eType, MPRViewType eViewType, float fThickness);
	~CW3RectItem_rangeMPR();

	void setRectThickness(float fThickness, float fBoxLength);
	void setPosition(float pX, float pY);

	inline void setThickness(float fThickness) { m_fThickness = fThickness; }
	inline float thickness(void) const { return m_fThickness; }
	inline float length(void) const { return m_fBoxLength; }

	void transformItems(const QTransform& transform);
private:
	void SetRect();

private:
	UILineType	m_eLineType;
	float		m_fThickness;
	float		m_fBoxLength;
	float scale_ = 1.0f;
};
