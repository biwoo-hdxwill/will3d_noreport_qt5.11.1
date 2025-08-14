#include "AzTabFrame.h"

#include "../../UIPrimitive/AzView.h"

#include <stdio.h>

CAzTabFrame::CAzTabFrame(EVIEW_LAYOUT_TYPE eType, QWidget *parent)
	: QWidget(parent),
	m_eViewType(eType),
	m_nViewCount(VIEW_CNT_MPR),
	m_nGridRow(DEFAULT_GRID_ROW), m_nGridCol(DEFAULT_GRID_COL)
{
	m_nSelectedIndex = 0;
	m_viewLayout = nullptr;

	QVBoxLayout *layout = new QVBoxLayout;
	this->setLayout(layout);

	setMouseTracking(true);
}

CAzTabFrame::~CAzTabFrame()
{
	delete this->layout();
}

//////////////////////////////////////////////////////////////////////////
//	public functions
//////////////////////////////////////////////////////////////////////////
void CAzTabFrame::addView(CAzView* view)
{
	if(m_viewList.size() >= m_nViewCount)
		return;
	m_viewList.push_back(view);
}

CAzView* CAzTabFrame::getView(AzINT index)
{
	if(index < 0 || index >= m_viewList.size())
		return nullptr;
	return m_viewList.at(index);
}

CAzView* CAzTabFrame::removeView(AzINT index)
{
	if(index < 0 || index >= m_viewList.size())
		return nullptr;
	CAzView* view = m_viewList.at(index);
	m_viewList.removeAt(index);
	return view;
}

void CAzTabFrame::setGridSize(AzINT row, AzINT col)
{
	if( row <= 0 || row > 4 || col <= 0 || col > 4 )
		return;
	
	m_nGridRow = row;
	m_nGridCol = col;
}

//////////////////////////////////////////////////////////////////////////
//	public slots
//////////////////////////////////////////////////////////////////////////
void CAzTabFrame::slotSetGridSize(AzINT row, AzINT col)
{
	if( row <= 0 || row > 4 || col <= 0 || col > 4 )
		return;
	
	m_nGridRow = row;
	m_nGridCol = col;
}

void CAzTabFrame::slotChangeLayout(EVIEW_LAYOUT_TYPE eType, AzINT selectedIdx, AzINT row, AzINT col)
{
	m_eViewType = eType;

	for(int i = 0; i < m_nViewCount; i++)
		getView(i)->setVisible(false);

	switch(m_eViewType)
	{
	case EVIEW_LAYOUT_TYPE::MPR:	
		mprLayout();		
		break;
	case EVIEW_LAYOUT_TYPE::SINGLE:	
		singletonLayout(selectedIdx);	
		break;
	case EVIEW_LAYOUT_TYPE::DOUBLE:	
		segmentationLayout(selectedIdx);		
		break;
	case EVIEW_LAYOUT_TYPE::LMAIN:	
		lMainLayout(selectedIdx);		
		break;
	case EVIEW_LAYOUT_TYPE::RMAIN:	
		rMainLayout(selectedIdx);		
		break;
	case EVIEW_LAYOUT_TYPE::TMAIN:	
		tMainLayout(selectedIdx);		
		break;
	case EVIEW_LAYOUT_TYPE::BMAIN:	
		bMainLayout(selectedIdx);		
		break;
	case EVIEW_LAYOUT_TYPE::GRID:	
		slotSetGridSize(row, col);
		gridLayout(selectedIdx, row, col);		
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//	private functions
//////////////////////////////////////////////////////////////////////////
void CAzTabFrame::mprLayout(void)
{
	QGridLayout *gLayout = new QGridLayout;
	gLayout->setSpacing(LAYOUT_SPACING);

	for(int c = 0 ; c < 2 ; c++)
    for(int r = 0 ; r < 2 ; r++)
    {
        getView(c*2+r)->setVisible(true);
        gLayout->setColumnMinimumWidth(c, VIEW_WIDTH_MIN);
        gLayout->setColumnStretch(c, 1);
        gLayout->setRowMinimumHeight(r, VIEW_HEIGHT_MIN);
        gLayout->setRowStretch(r, 1);
        gLayout->addWidget(getView(c*2+r), c, r);
    }

	QVBoxLayout *layout = new QVBoxLayout;
	layout->setSpacing(LAYOUT_SPACING);
	layout->addLayout(gLayout);
	layout->setStretch(0, 3);
	getView(4)->setVisible(false);
	layout->addWidget(getView(4));
	layout->setStretch(1, 1);

	setViewLayout(layout);
}
void CAzTabFrame::singletonLayout(AzINT selectedIdx)
{
	QGridLayout* layout = new  QGridLayout;
	layout->setSpacing(LAYOUT_SPACING);

	int viewIdx;
	for(int c = 0 ; c < 2 ; c++)
    for(int r = 0 ; r < 2 ; r++)
    {
		if( selectedIdx == c*2+r)
		{
			getView(c*2+r)->setVisible(true);
			layout->setColumnMinimumWidth(c, VIEW_WIDTH_MIN);
			layout->setColumnStretch(c, 1);
			layout->setRowMinimumHeight(r, VIEW_HEIGHT_MIN);
			layout->setRowStretch(r, 1);
		}
		else
		{
			getView(c*2+r)->setVisible(false);
			layout->setColumnMinimumWidth(c, 0);
            layout->setColumnStretch(c, 0);
            layout->setRowMinimumHeight(r, 0);
            layout->setRowStretch(r, 0);
		}
		layout->addWidget(getView(c*2+r), c,r);
    }
	setViewLayout(layout);
}
void CAzTabFrame::segmentationLayout(AzINT selectedIdx)
{
	QHBoxLayout *layout = new QHBoxLayout;
	layout->setSpacing(LAYOUT_SPACING);

	getView(selectedIdx)->setVisible(true);
	layout->addWidget(getView(selectedIdx));
	layout->setStretch(0, 1);
	layout->addWidget(getView(3));
	layout->setStretch(1, 1);

	setViewLayout(layout);
}

void CAzTabFrame::gridLayout(AzINT selectedIdx, AzINT row, AzINT col)
{
	QGridLayout* layout = new QGridLayout;
	layout->setSpacing(LAYOUT_SPACING);

	for(int c = 0; c < m_nGridCol; c++)
	for(int r = 0; r < m_nGridRow; r++)
	{
		getView(c + r*m_nGridCol)->setVisible(true);
		layout->setColumnStretch(c, 1);
		layout->setRowStretch(r, 1);
		layout->addWidget(getView(c + r*m_nGridCol), r, c);
	}

	setViewLayout(layout);
}

void CAzTabFrame::lMainLayout(AzINT selectedIdx)
{
	QVBoxLayout *rLayout = new QVBoxLayout;
	rLayout->setSpacing(LAYOUT_SPACING);
	
	for(int i = 0; i < VIEW_CNT_MAIN; i++)
	{
		if(i == selectedIdx)
			continue;
		getView(i)->setVisible(true);
		rLayout->addWidget(getView(i));
		rLayout->setStretch(i, 1);
	}

	getView(selectedIdx)->setVisible(true);

	QHBoxLayout *layout = new QHBoxLayout;
	layout->setSpacing(LAYOUT_SPACING);
	layout->addWidget(getView(selectedIdx));
	layout->setStretch(0, 2);
	layout->addLayout(rLayout);
	layout->setStretch(1, 1);

	setViewLayout(layout);
}
void CAzTabFrame::rMainLayout(AzINT selectedIdx)
{
	QVBoxLayout *lLayout = new QVBoxLayout;
	lLayout->setSpacing(LAYOUT_SPACING);
	for(int i = 0; i < VIEW_CNT_MAIN; i++)
	{
		if(i == selectedIdx)
			continue;
		getView(i)->setVisible(true);
		lLayout->addWidget(getView(i));
		lLayout->setStretch(i, 1);
	}

	getView(selectedIdx)->setVisible(true);
	QHBoxLayout *layout = new QHBoxLayout;
	layout->setSpacing(LAYOUT_SPACING);
	layout->addLayout(lLayout);
	layout->setStretch(0, 1);
	layout->addWidget(getView(selectedIdx));
	layout->setStretch(1, 2);

	setViewLayout(layout);
}

void CAzTabFrame::tMainLayout(AzINT selectedIdx)
{
	QHBoxLayout *bLayout = new QHBoxLayout;
	bLayout->setSpacing(LAYOUT_SPACING);
	for(int i = 0; i < VIEW_CNT_MAIN; i++)
	{
		if(i == selectedIdx)
			continue;
		getView(i)->setVisible(true);
		bLayout->addWidget(getView(i));
		bLayout->setStretch(i, 1);
	}

	getView(selectedIdx)->setVisible(true);
	QVBoxLayout *layout = new QVBoxLayout;
	layout->setSpacing(LAYOUT_SPACING);
	layout->addWidget(getView(selectedIdx));
	layout->setStretch(0, 2);
	layout->addLayout(bLayout);
	layout->setStretch(1, 1);

	setViewLayout(layout);
}

void CAzTabFrame::bMainLayout(AzINT selectedIdx)
{
	QHBoxLayout *tLayout = new QHBoxLayout;
	tLayout->setSpacing(LAYOUT_SPACING);
	for(int i = 0; i < VIEW_CNT_MAIN; i++)
	{
		if(i == selectedIdx)
			continue;
		getView(i)->setVisible(true);
		tLayout->addWidget(getView(i));
		tLayout->setStretch(i, 1);
	}

	getView(selectedIdx)->setVisible(true);
	QVBoxLayout *layout = new QVBoxLayout;
	layout->setSpacing(LAYOUT_SPACING);
	layout->addLayout(tLayout);
	layout->setStretch(0, 1);
	layout->addWidget(getView(selectedIdx));
	layout->setStretch(1, 2);

	setViewLayout(layout);
}

void CAzTabFrame::setViewLayout(QLayout* layout)
{
	QVBoxLayout* tabLayout = (QVBoxLayout*)this->layout();

    if(m_viewLayout){
        tabLayout->removeItem(m_viewLayout);
        delete m_viewLayout;
    }

	m_viewLayout = layout;
    tabLayout->addLayout(m_viewLayout);
}
