#pragma once
/*=========================================================================

File:		class CW3OTFView
Language:	C++11
Library:	Qt 5.2.0
Author:			Hong Jung
First date:		2015-12-22
Last modify:	2015-12-22

=========================================================================*/
#include <qgraphicsview.h>

#include "../../Common/Common/W3Enum.h"
#include "../../Common/Common/W3Types.h"

#include "uicomponent_global.h"

/*
	* CW3OTFView class
	* reimplementation of QGraphicsView
	* used in main UI.
	* this class is used with CW3OTFScene class. (reimplementation of QGraphicsScene)
	* the connections of signal & slots must be implemented in the main UI module.
	* THE HEIGHT OF A CW3OTFView MUST BE FIXED VALUE : 260 (in .ui declaration file)
*/
class CW3TextItem_switch;
class CW3TextItem;
class CW3OTFScene;

class UICOMPONENT_EXPORT CW3OTFView : public QGraphicsView
{
	Q_OBJECT
public:
	CW3OTFView(/*CW3OTFScene *scene, */QWidget *parent = 0);
	~CW3OTFView(void);

public:
	void setTeethPreset();
	void SetOTFpreset(const QString& preset);
	void initOTF(int *histogram, int size, float slope, int offset);
	void setThreshold(int thdAir, int thdTissue, int thdBone);
	bool setAdjustOTF(const AdjustOTF& val);
	void Export(const QString& path);
	void setVisible(bool is_visible);

	void movePolygonAtMin(float minValue);
	void setPreset(const QString& str);
	void setXRayTF(float max, float min);
	QString getCurrentPreset() const;

	void connections();
	void disconnections();

	void SetSoftTissueMin(const float value);

public slots:
	void slotSetScrollHandDrag(void);
	void slotShadeOnSwitch(bool);

signals:
	void sigOTFSave();
	void sigChangeTFMove(bool isMinMaxChanged);
	void sigSetScrollHandDrag(void);
	void sigRenderCompleted();
	void sigShadeOnSwitch(bool isEnable);

protected:
	virtual void leaveEvent(QEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void resizeEvent(QResizeEvent *event) override;
	virtual void wheelEvent(QWheelEvent *event) override;

private:
	void setItemPos();

	float m_fFitWidth;	// by jdk 150616
	float m_fPreFitWidth;	// by jdk 151221

	CW3TextItem_switch* m_pShadeSwitch;
	CW3TextItem	*m_pTextSavePreset;

	CW3OTFScene *m_pOTFScene;

	bool is_resize_scene = false;
};
