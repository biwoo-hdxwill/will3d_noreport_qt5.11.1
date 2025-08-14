#pragma once
/*=========================================================================

File:			class CW3OTFScene.h
Language:		C++11
Library:		Qt 5.2.0
Author:			JUNG DAE GUN, Hong Jung
First date:		2015-06-22
Last modify:	2015-12-22

=========================================================================*/
#include <memory>

#include <QGroupBox>
#include <QGraphicsView>
#include <qgraphicsscene.h>
#include <qmenu.h>
#include <qevent.h>
#include <qlabel.h>
#include <QGraphicsSceneMouseEvent>

#include "../../Common/Common/W3Enum.h"
#include "../../Common/Common/W3Types.h"
#include "../../Resource/Resource/W3TF.h"

#include "../../UIModule/UIPrimitive/W3OTFPolygon.h"

/*
	* CW3OTFScene class
	* reimplementation of QGraphicsScene
	* used in main UI.
	* this class is used with CW3OTFView class. (reimplementation of QGraphicsView)
	* the connections of signal & slots must be implemented in the main UI module.
*/
class CW3OTFScene : public QGraphicsScene {
	Q_OBJECT

	/* Element of TFOBJECT */
	enum class COMPONENT {
		POLYGON, POINT, LINE, COLOR, NONE
	};

	/* Current Component id. */
	enum class CURR_COMPONENT {
		BACKGROUND,
		POINT,
		LINE,
		POLYGON,
		COLOROBJ,
		COLORTAB
	};

	// used to track Current Activated Component.
	struct ELEMENT {
		COMPONENT	_which;
		int		_iIdx = 0;
		int		_iPolyIdx = 0;
	};
	enum eThreshold { AIR_TISSUE, TISSUE_BONE, BONE_TEETH };

	struct OTFColor {
		int	_intensity = 0;	// pixel density.
		QColor	_color;	// color value according to the density.
	};

public:
	CW3OTFScene(QWidget *parent = 0);
	~CW3OTFScene();

public:
	// public functions.
	void initOTF(int *histogram, int size, float slope, int offset);
	void setThreshold(int thdAir, int thdTissue, int thdBone);
	void deactiveItems();

	void reset();

	void movePolygonAtMin(float minValue);
	void setPreset(const QString& str);
	void setXRayTF(float max, float min);
	bool setAdjustOTF(const AdjustOTF& val);

	inline QString getCurrentPreset() { return m_strCurPreset; }
	void Export(const QString& path);

	void SetSoftTissueMin(const float value);

signals:
	// signal to VREngine.
	void sigChangeTFMove(bool isMinMaxChanged);
	void sigSetScrollHandDrag(void);
	void sigRenderCompleted();
	void sigShadeOnSwitch(bool isEnable);

private slots:
	void slotAddPolygon(void);
	void slotRemovePolygon(void);
	void slotAddPoint(void);
	void slotRemovePoint(void);
	void slotAddColor(void);
	void slotRemoveColor(void);
	void slotUpdateColor(void);

protected:
	// protected overriding functions for mouse events.
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
	// private functions.
	CURR_COMPONENT	getCurrComponent(const QPointF& point);
	void setDefaultGeometry(void);
	void initLoadPresets(void);
	void loadPresets(const QString& path);
	void addColorObject(int posX, const QColor& color);
	void updateTF(const QList<QList<OTFColor>>& colorList);	// Update Color, Alpha value.
	void updateColor(void);		// Update mainColorRect.
	void selectPolygon(int index);

public:
	bool m_bShade;
	QString m_strCurPreset;
	AdjustOTF m_adjustOTF;

private:
	// TF Polygon drawing components.
	QList<CW3OTFPolygon*>	m_listOTFObj;
	QGraphicsLineItem		*m_currLine = nullptr;
	QGraphicsTextItem		*m_currText = nullptr;
	QPointF					m_currPoint;

	// mouse interaction components.
	bool					m_isPressed = false;
	bool					m_isOnPolygon = false;
	ELEMENT					m_currElem;
	int	m_selectedPolygonIdx = 0;
	int	m_activatePolygonIdx = -1;

	// TF data processing components.
	QList<OTFColor>					m_colorList;		// color list of color controls
	QList<QGraphicsEllipseItem*>*	m_pgColorObject = nullptr;	// list of color controls for a centain OTFPolygon

	// main color rect.
	QGraphicsRectItem	*m_mainColorRect = nullptr; // 제일 아래 보이는 color bar
	float slope_ = 0.0f;
	int				m_nOffset = 0;
	int				m_TFmin = 0;
	int				m_TFmax = 0;

	// TF resource handling members.
	QStringList				m_listTFPaths;
	std::vector<QLabel*>	m_listTFImg;
	QStringList				m_listTFImgPaths;

	// Menu and Actions.
	QMenu	*m_menu;
	QAction *m_addPolygonAct;
	QAction *m_removePolygonAct;
	QAction *m_addPointAct;
	QAction *m_removePointAct;
	QAction *m_addColorObjectAct;
	QAction *m_updateColorObjectAct;
	QAction *m_removeColorObjectAct;

	std::shared_ptr<std::vector<TF_BGRA>> tf_colors_;
	std::shared_ptr<CW3TF> tf_;
	//thyoo 170217: threshold Line (temp)
	std::vector<QGraphicsLineItem*>	m_thdLine;

	bool auto_mode_ = true;
	int additional_offset_ = 0;
	float new_soft_tissue_min_from_generate_face_dialog_ = -1.0f;
};
