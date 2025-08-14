#pragma once

/*=========================================================================

File:			class CW3TextItem_switch
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-12-22
Modify Date:	2015-12-22

=========================================================================*/

#include "uiprimitive_global.h"

#include "W3TextItem.h"

class UIPRIMITIVE_EXPORT CW3TextItem_switch : public CW3TextItem
{
	Q_OBJECT
public:
	CW3TextItem_switch(const QString& str);	
	~CW3TextItem_switch(void);

	inline W3BOOL getCurrentState() { return m_bState; }

protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	
signals:
	void sigState(const W3BOOL bState);

private:

	W3BOOL m_bState;
	QString m_strSwitch;
};
