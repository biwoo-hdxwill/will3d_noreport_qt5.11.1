#pragma once
/*=========================================================================

File:		Tab's frame 
Language:	C++11
Library:	Qt Library
Author:		Seokman Seo
Date:		2014-11-07
Version:	1.0
Mail:		tjsm@cglab.snu.ac.kr

	Copyright (c) 2014 All rights reserved by PentaMed.

=========================================================================*/

#include "uiframe_global.h"
#include "../../Common/Common/AzTypes.h"
#include "../../Common/Common/AzEnum.h"
#include "../../Common/Common/AzDefine.h"

#define VIEW_CNT_MPR		4
#define VIEW_CNT_SINGLETON	1
#define VIEW_CNT_MAIN		4

#define MPR_GRID_ROW		2
#define MPR_GRID_COL		2
#define DEFAULT_GRID_ROW	3
#define DEFAULT_GRID_COL	3
#define SIDEVIEW_SIZE		3

#define VIEW_WIDTH_MIN		300
#define VIEW_HEIGHT_MIN		300

#define LAYOUT_SPACING		1

/*=========================================================================
	Qt libraries
=========================================================================*/
#include <QWidget>
#include <qlist.h>
#include <qgridlayout.h>

class CAzView;

class UIFRAME_EXPORT CAzTabFrame : public QWidget
{
	Q_OBJECT

public:
	CAzTabFrame(EVIEW_LAYOUT_TYPE eType = EVIEW_LAYOUT_TYPE::MPR, QWidget *parent = 0);
	~CAzTabFrame();

public:
	void		addView(CAzView* view);
	CAzView*	getView(AzINT index);
	CAzView*	removeView(AzINT index);

	inline void setViewCount(AzINT cnt) { if(cnt >= 0 || cnt <= VIEW_CNT_MAX) m_nViewCount = cnt; }
	void setGridSize(AzINT row, AzINT col);

	inline QList<CAzView*> getViewList(void) { return m_viewList; }

public slots:
	void slotSetGridSize(AzINT row, AzINT col);
	void slotChangeLayout(EVIEW_LAYOUT_TYPE eType, AzINT selectedIdx = 0, AzINT row = 4, AzINT col = 4);

private:
	//private functions
	void mprLayout(void);
	void singletonLayout(AzINT selectedIdx);
	void segmentationLayout(AzINT selectedIdx);
	void gridLayout(AzINT selectedIdx, AzINT row, AzINT col);
	
	void lMainLayout(AzINT selectedIdx = 3);
	void rMainLayout(AzINT selectedIdx = 3);
	void tMainLayout(AzINT selectedIdx = 3);
	void bMainLayout(AzINT selectedIdx = 3);

	void setViewLayout(QLayout* layout);

private:
	AzINT				m_nViewCount;
	EVIEW_LAYOUT_TYPE	m_eViewType;
	QList<CAzView*>		m_viewList;
	QLayout*			m_viewLayout;
	AzINT				m_nSelectedIndex;

	AzINT	m_nGridRow;
	AzINT	m_nGridCol;

};
