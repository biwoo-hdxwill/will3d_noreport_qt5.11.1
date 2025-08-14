#include "SplashScreen.h"
/*=========================================================================
File:			class CSplashScreen
Language:		C++11
Library:		Qt 5.4.0
Author:			Kim You Song
First date:		2015-12-24
Last modify:	2015-12-24

=========================================================================*/
#include <sstream>
#include <thread>

#include <QApplication>
#include <QGraphicsScene>

#include <Engine/Resource/ResContainer/resource_container.h>
#include "../SubProjects/Engine/Common/Common/W3Logger.h"
#include "../SubProjects/Engine/Common/Common/W3Memory.h"
#include "../SubProjects/Engine/Common/Common/language_pack.h"
#include "../SubProjects/Engine/Module/VREngine/W3VREngine.h"
#include "../SubProjects/Engine/Module/Will3DEngine/render_engine.h"
#include "../SubProjects/Engine/Module/Will3DEngine/renderer_manager.h"
#ifndef WILL3D_VIEWER
#include "../SubProjects/Engine/UIModule/UIFrame/login_dialog.h"
#endif

#include "Will3D.h"

using lang::LanguagePack;

CSplashScreen::CSplashScreen(QString readFilePath, QString outScriptPath)
	: m_strReadFilePath(readFilePath),
	m_strOutScriptPath(outScriptPath),
	m_pWill3D(nullptr)
{
	Q_INIT_RESOURCE(splash);
	
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
	
	m_pGLWidget = new QOpenGLWidget();
	setViewport(m_pGLWidget);

	ResourceContainer::SetInstance();
	RendererManager::SetInstance();
	render_engine_ = new RenderEngine();

	common::Logger::instance()->PrintDebugMode("CSplashScreen::CSplashScreen", "Create : render_engine_");

#if 0
	m_pVREngine = new CW3VREngine(m_pGLWidget);
#else
	m_pVREngine = CW3VREngine::NewInstance(m_pGLWidget);
#endif

	common::Logger::instance()->PrintDebugMode("CSplashScreen::CSplashScreen", "Create : m_pVREngine");

	m_pImage = new QImage(":/image/splashImage.png");
	resize(m_pImage->width(), m_pImage->height());
	setScene(new QGraphicsScene);

	m_pDelayTimer = new QTimer(this);
	m_pDelayTimer->setSingleShot(true);

#ifndef WILL3D_VIEWER
	connect(m_pDelayTimer, SIGNAL(timeout()), this, SLOT(slotLogin()));
#else
	connect(m_pDelayTimer, SIGNAL(timeout()), this, SLOT(slotShowWill3D()));
#endif
}

CSplashScreen::~CSplashScreen()
{
	using common::Logger;
#if 0
	SAFE_DELETE_OBJECT(m_pVREngine);  // thyoo 160718. will3d에서 지우기... otfview 때문
	Logger::instance()->PrintDebugMode("CSplashScreen::~CSplashScreen",
		"Destroy : m_pVREngine");
#endif
	SAFE_DELETE_OBJECT(m_pWill3D);  // thyoo 160718. will3D객체에서 종료이벤트가
									// 발생하면 자기 자신을 지운다.
	Logger::instance()->PrintDebugMode("CSplashScreen::~CSplashScreen",
		"Destroy : m_pWill3D");
	SAFE_DELETE_OBJECT(m_pImage);
	Logger::instance()->PrintDebugMode("CSplashScreen::~CSplashScreen",
		"Destroy : m_pImage");
	SAFE_DELETE_OBJECT(m_pDelayTimer);
	Logger::instance()->PrintDebugMode("CSplashScreen::~CSplashScreen",
		"Destroy : m_pDelayTimer");
	SAFE_DELETE_OBJECT(m_pGLWidget);
	Logger::instance()->PrintDebugMode("CSplashScreen::~CSplashScreen",
		"Destroy : m_pGLWidget");

	common::Logger::instance()->Print(common::LogType::INF, "Will3D - end");
	common::Logger::instance()->Print(common::LogType::INF, "  ");
}

void CSplashScreen::drawBackground(QPainter *painter, const QRectF &rect)
{
	QGraphicsView::drawBackground(painter, rect);

	static bool bIsInit = false;
	if (bIsInit == false)
	{
		bIsInit = true;

		common::Logger::instance()->Print(common::LogType::INF, "start 3D Engine intialize");
		m_pVREngine->init();
		common::Logger::instance()->Print(common::LogType::INF, "end 3D Engine intialize");

		m_pDelayTimer->start(100);
	}

	painter->drawImage(rect, *m_pImage);
}

void CSplashScreen::slotShowWill3D()
{
	this->hide();

	m_pWill3D = new CWill3D(render_engine_, m_pVREngine, m_strReadFilePath, m_strOutScriptPath);
	common::Logger::instance()->PrintDebugMode("CSplashScreen::slotShowWill3D",
		"Create : m_pWill3D");
	m_pGLWidget->doneCurrent();
	m_pWill3D->show();
}
#ifndef WILL3D_VIEWER
void CSplashScreen::slotLogin()
{
	LoginDialog *login_dialog = new LoginDialog(this);
	common::Logger::instance()->PrintDebugMode("CSplashScreen::slotLogin",
		"Create : login_dialog");

	this->hide();
	if (login_dialog->exec())
		slotShowWill3D();
	else
		QCoreApplication::exit();
}
#endif
