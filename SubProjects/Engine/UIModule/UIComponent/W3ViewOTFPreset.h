#pragma once
/*=========================================================================

File:			class CW3ViewOTFPreset
Language:		C++11
Library:		Qt 5.2.0
Author:			JUNG DAE GUN
First date:		2015-10-01
Last modify:	2015-10-01

=========================================================================*/
#include <QGraphicsView>
#include "uicomponent_global.h"

class QCheckBox;
class QToolButton;
class QGraphicsProxyWidget;

class UICOMPONENT_EXPORT CW3ViewOTFPreset : public QGraphicsView
{
	Q_OBJECT
public:
	CW3ViewOTFPreset(int id, const QString& imgPath, bool isWritable, QWidget *parent = 0);
	~CW3ViewOTFPreset();

	void setChecked(bool check);
	void setPreset(const QString& imgPath);

	inline QString getViewName() { return m_strViewName; }

signals:
	void sigSetFavorite(int id, int check);
	void sigWrite(int id);
	void sigOverwrite(int id);

private slots:
	void slotSetFavorite(int check);
	void slotWrite();
	void slotOverwrite();

protected:
	virtual void resizeEvent(QResizeEvent *event) override;

private:
	int m_nId;
	bool m_bIsWritable;

	QGraphicsProxyWidget *m_pProxyWidgetFavoriteChk;
	QGraphicsProxyWidget *m_pProxyWidgetWriteBtn;
	QGraphicsPixmapItem *m_pImgItem;

	QString m_strWriteBtnStyle;
	QString m_strOverwriteBtnStyle;
	QString m_strViewName;
};
