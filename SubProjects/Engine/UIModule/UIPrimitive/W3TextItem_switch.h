#pragma once

/*=========================================================================

File:			class CW3TextItem_switch
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-12-22
Modify Date:	2016-06-20

=========================================================================*/
#include "W3TextItem.h"
#include "uiprimitive_global.h"

class UIPRIMITIVE_EXPORT CW3TextItem_switch : public CW3TextItem
{
	Q_OBJECT
public:
	CW3TextItem_switch(const QString& str);
	~CW3TextItem_switch(void);

	void setCurrentState(bool);
	inline bool getCurrentState() { return m_bState; }

protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	
signals:
	void sigState(bool bState);

private:
	bool m_bState;
	QString m_strSwitch;
};
