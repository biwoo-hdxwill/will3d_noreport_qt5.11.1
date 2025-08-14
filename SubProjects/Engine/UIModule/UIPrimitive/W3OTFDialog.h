#pragma once
/*=========================================================================

File:		class CW3OTFDialog
Language:	C++11
Library:	Qt 5.2.1

=========================================================================*/
#include <qstandarditemmodel.h>
#include <qdialog.h>

#include "uiprimitive_global.h"

class QListView;
/*
	class CW3OTFDialog.
	 - Re-implements QDialog.
	 - Preset viewer & selector dialog.
*/
class UIPRIMITIVE_EXPORT CW3OTFDialog : public QDialog
{
	Q_OBJECT
public:
	CW3OTFDialog(QStringList listPath, QPointF pos);
	~CW3OTFDialog(void);

signals:
	void sigSelected(int);

public slots:
	void imgClicked(QModelIndex);

private:
	// private member fields.
	QStringList				m_listNames;
	QListView				*m_pImgListView;
	QStandardItemModel		*m_pStdModel;
};

