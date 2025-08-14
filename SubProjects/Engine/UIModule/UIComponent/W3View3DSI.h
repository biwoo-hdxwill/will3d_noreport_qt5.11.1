#pragma once
/*=========================================================================

File:			class CW3View3DSI
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2015-04-22
Last date:		2016-04-22

=========================================================================*/
#include <memory>
#include "W3View3D.h"
#include "uicomponent_global.h"

class CW3SurfaceAxesItem;
class ViewBorderItem;
#ifndef WILL3D_VIEWER
class ProjectIOSI;
#endif

class UICOMPONENT_EXPORT CW3View3DSI : public CW3View3D {
	Q_OBJECT
public:
	CW3View3DSI(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
				CW3ResourceContainer *Rcontainer, common::ViewTypeID eType,
				QWidget *pParent = 0);

	~CW3View3DSI();

	// serialization
#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOSI& out);
	void importProject(ProjectIOSI& in);
#endif

	void UpdateVR(bool is_high_quality);

signals:
	void sigSetTranslateMatSecondVolume(glm::mat4 *);
	void sigSetRotateMatSecondVolume(glm::mat4 *);
	void sigSecondTransform(glm::mat4 *);

public:
	virtual void reset();
	void ResetMatrixToAuto();
	void SecondVolumeLoaded(const glm::mat4& secondModel);

	void VisibleMain(bool state);
	void VisibleSecond(bool state);
	void VisibleBoth(bool state);

private:
	virtual void init();
	virtual void drawBackground(QPainter *painter, const QRectF &rect);
	virtual bool mousePressEventW3(QMouseEvent *event);
	virtual bool mouseMoveEventW3(QMouseEvent *event);
	virtual bool mouseReleaseEventW3(QMouseEvent *event);
	virtual void clearGL();
	void resizeEvent(QResizeEvent* event) override;

	void RenderAndUpdate();

#ifdef WILL3D_EUROPE
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;
#endif // WILL3D_EUROPE

private:
	std::unique_ptr<ViewBorderItem> border_;

	CW3SurfaceAxesItem *m_pAxes = nullptr;

	unsigned int m_PROGsurface = 0;
	unsigned int m_PROGpick = 0;

	bool m_bShowSecondVolume = false;
	bool m_isTranslatingSecondVolume = false;
	bool m_isRotatingSecondVolume = false;

	glm::mat4 m_secondToFirstModelOrig;
	glm::mat4 m_secondTransform;
	glm::mat4 m_translateSecondForMPR;
	glm::mat4 m_rotateSecondForMPR;
};
