#pragma once

/*=========================================================================

File:			class CWill3D
Language:		C++11
Library:		Qt 5.4.0
Author:			Kim You Song
First date:		2015-12-24
Last modify:	2015-12-24

=========================================================================*/
#include <memory>

#include <QGraphicsView>

#include <QTimer>

#include "../SubProjects/Engine/Common/GLfunctions/WGLHeaders.h"
#include <qopenglwidget.h>

class CWill3D;
class CW3VREngine;
class RenderEngine;

class CSplashScreen : public QGraphicsView
{
	Q_OBJECT

public:
	CSplashScreen(QString readFilePath = "", QString outScriptPath = "");
	~CSplashScreen();

protected:
	virtual void drawBackground(QPainter* painter, const QRectF& rect);

private slots:
	void slotShowWill3D();
#ifndef WILL3D_VIEWER
	void slotLogin();
#endif

private:
	QOpenGLWidget* m_pGLWidget;
	RenderEngine* render_engine_;
	CW3VREngine* m_pVREngine;
	CWill3D* m_pWill3D;
	QImage* m_pImage;
	QTimer* m_pDelayTimer;
	QString m_strReadFilePath;
	QString m_strOutScriptPath;
};
