#include "W3OTFDialog.h"

#include <qgridlayout.h>
#include <qlistview.h>

#include "../../Common/Common/W3Memory.h"

CW3OTFDialog::CW3OTFDialog(QStringList listPath, QPointF pos)
{
	m_listNames = listPath;
	//this->setGeometry(200, 200, 965, 640);
	this->setGeometry(pos.x(), pos.y()-640, 965, 640);
	this->setModal(true);
	
	QGridLayout *gridLayout = new QGridLayout(this);
	m_pImgListView = new QListView(this);
	gridLayout->addWidget(m_pImgListView);

	m_pImgListView->setViewMode(QListView::IconMode);
	m_pImgListView->setUniformItemSizes(true);
	m_pImgListView->setSelectionRectVisible(true);
	m_pImgListView->setMovement(QListView::Static);
	m_pImgListView->setSelectionMode(QListView::SingleSelection);
	m_pImgListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_pImgListView->setResizeMode(QListView::Adjust);

	m_pStdModel = new QStandardItemModel();
	m_pImgListView->setModel(m_pStdModel);

	for( auto &i : m_listNames )
	{
	//	std::cout<<i.toStdString()<<std::endl;
		QImage img(i);
		img.scaled(QSize(300, 300));
		//QPixmap pixmap(300, 300);

		QPixmap pixmap(i);
		//pixmap.load(i);
		//pixmap.fromImage(img);

		QStandardItem *pImgItem = new QStandardItem();
		//pImgItem->setData(pixmap, Qt::DecorationRole);
		pImgItem->setData(pixmap.scaled(300, 300), Qt::DecorationRole);
		m_pStdModel->appendRow(pImgItem);
	}

	connect(m_pImgListView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(imgClicked(QModelIndex)));
}

CW3OTFDialog::~CW3OTFDialog(void)
{
	SAFE_DELETE_OBJECT(m_pImgListView);
	SAFE_DELETE_OBJECT(m_pStdModel);
}

void CW3OTFDialog::imgClicked(QModelIndex index)
{
	// row is an item index.
	int idx = index.row();
	emit sigSelected(idx);
	close();
}
