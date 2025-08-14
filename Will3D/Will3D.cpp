#include "Will3D.h"
/*=========================================================================

File:			class CWill3D
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-11-21
Last modify:	2015-12-18

=========================================================================*/
#include <algorithm>

#include <QElapsedTimer>
#include <QDebug>
#include <qdesktopwidget.h>
#include <qgridlayout.h>
#include <QTimer>
#include <QApplication>
#include <QImageWriter>
#include <QProcess>
#include <QFileInfo>
#include <QMouseEvent>

#include <Engine/Common/Common/sw_info.h>
#include <Engine/Common/Common/W3Cursor.h>
#include <Engine/Common/Common/W3Define.h>
#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/W3Memory.h>
#include <Engine/Common/Common/W3ProgressDialog.h>
#include <Engine/Common/Common/define_otf.h>
#include <Engine/Common/Common/event_handle_common.h>
#include <Engine/Common/Common/event_handler.h>
#include <Engine/Common/Common/global_preferences.h>
#include <Engine/Common/Common/large_address_aware_win32.h>

#include <Engine/Resource/ResContainer/W3ResourceContainer.h>
#include <Engine/Resource/ResContainer/resource_container.h>
#include <Engine/Resource/Resource/W3Image3D.h>
#include <Engine/Resource/Resource/W3TF.h>
#include <Engine/Resource/Resource/pano_resource.h>

#include <Engine/Module/VREngine/W3VREngine.h>
#include <Engine/Module/Will3DEngine/render_engine.h>
#include <Engine/Module/Will3DEngine/renderer_manager.h>

#include <Engine/UIModule/UITools/tool_mgr.h>

#include <Engine/UIModule/UIComponent/W3OTFView.h>
#include <Engine/UIModule/UIFrame/W3TitleBar.h>
#include <Engine/UIModule/UIFrame/W3ToolHBar.h>
#include <Engine/UIModule/UIFrame/W3ToolVBar.h>
#include <Engine/UIModule/UIFrame/tab_contents_widget.h>
#include <Engine/UIModule/UIFrame/tab_slot_layout.h>
#include <Engine/UIModule/UIFrame/W3OTFPresetDlg.h>
#include <Engine/UIModule/UIFrame/W3SizeGrip.h>
#include <Engine/UIModule/UIFrame/global_preferences_dialog.h>
#include <Engine/UIModule/UIFrame/about_dialog.h>
#ifndef WILL3D_VIEWER
#include <Engine/UIModule/UIFrame/login_dialog.h>
#endif
#ifndef WILL3D_VIEWER
#include <Managers/DBManager/W3DBM.h>
#endif
#include <Managers/TabMgr/W3Tabmgr.h>

using common::Logger;

bool Will3DEventFilter::eventFilter(QObject *obj, QEvent *event)
{
	if (event)
	{
		if (event->type() == QEvent::GraphicsSceneMouseRelease)
		{
			if (!event->spontaneous())
			{
				emit sigGraphicsSceneMouseReleaseEvent(static_cast<QMouseEvent *>(event));
			}
			return false;
		}
		else if (event->type() == QEvent::GraphicsSceneMousePress)
		{
			if (!event->spontaneous())
			{
				emit sigGraphicsSceneMousePressEvent(static_cast<QMouseEvent *>(event));
			}
			return false;
		}
	}
	return QObject::eventFilter(obj, event);
}

CWill3D::CWill3D(RenderEngine *render_engine, CW3VREngine *VREngine,
								 const QString &readFilePath, const QString &outScriptPath,
								 QWidget *parent)
		: QFrame(parent), m_pgVREngine(VREngine)
{
	LargeAddressAwareWin32::GetInstance();

#ifndef WILL3D_VIEWER
	int min = GlobalPreferences::GetInstance()->preferences_.login.auto_logout_time;
	logout_timer_interval_ = min * 60 * 1000;

	logout_timer_ = new QTimer();
	logout_timer_->setSingleShot(true);
	logout_timer_->setInterval(logout_timer_interval_);
	connect(logout_timer_, SIGNAL(timeout()), this, SLOT(slotLogout()));
	logout_timer_->start();
#endif

	QRect screen_rect = QApplication::desktop()->screenGeometry();
	setMinimumSize(QSize(
			screen_rect.width() / 3 * 2,
			screen_rect.height() / 3 * 2));

	GlobalPreferences::GetInstance()->Restore();
	GlobalPreferences::GetInstance()->Load();

	CW3ProgressDialog::setInstance(this);
	EventHandler::SetInstance();

	ToolMgr::SetInstance();

	render_engine_.reset(render_engine);

#ifdef OVERRIDE_CURSOR
	QApplication::setOverrideCursor(CW3Cursor::ArrowCursor());
#else
	setCursor(CW3Cursor::ArrowCursor());
#endif

	setWindowTitle("Will3D - " + QString::fromStdString(sw_info::SWInfo::GetSWVersion()));

	// thyoo: Make this a borderless window which can't
	setFrameShape(Panel);
	int maximize_mode = GlobalPreferences::GetInstance()->preferences_.general.interfaces.maximize_type;
	if (maximize_mode == 0)
	{
		setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
	}

	setMouseTracking(true);

	title_bar_ = new CW3TitleBar(this);
	tab_contents_widget_ = new TabContentsWidget(this);

	m_pRcontainer = new CW3ResourceContainer();

	otf_view_.reset(new CW3OTFView());
	otf_view_->setVisible(false);
	tab_contents_widget_->SetOTFWidget((QWidget *)(otf_view_.get()));
	tab_mgr_ = new CW3TabMgr(VREngine, m_pRcontainer, this);
	horizontal_toolbar_ = new CW3ToolHBar(this);
	vertical_toolbar_ = new CW3ToolVBar(this);

#if 1
	event_filter_.reset(new Will3DEventFilter());
	QApplication::instance()->installEventFilter(event_filter_.get());
	connect(
			event_filter_.get(),
			SIGNAL(sigGraphicsSceneMouseReleaseEvent(QMouseEvent *)),
			tab_mgr_,
			SLOT(slotGraphicsSceneMouseReleaseEvent(QMouseEvent *)));
	connect(
			event_filter_.get(),
			SIGNAL(sigGraphicsSceneMousePressEvent(QMouseEvent *)),
			tab_mgr_,
			SLOT(slotGraphicsSceneMousePressEvent(QMouseEvent *)));
#else
	QApplication::instance()->installEventFilter(this);
	connect(this,
					&CWill3D::sigGraphicsSceneMouseReleaseEvent, tab_mgr_,
					&CW3TabMgr::slotGraphicsSceneMouseReleaseEvent);
	connect(this,
					&CWill3D::sigGraphicsSceneMousePressEvent, tab_mgr_,
					&CW3TabMgr::slotGraphicsSceneMousePressEvent);
#endif

	initLayout();

	connections();

	// title_bar_->showMaxRestore();

	common::Logger::instance()->Print(common::LogType::INF, std::string("CWill3D readFilePath : ") + readFilePath.toStdString());
	common::Logger::instance()->Print(common::LogType::INF, std::string("CWill3D outScriptPath : ") + outScriptPath.toStdString());
	tab_mgr_->setScriptFile(readFilePath, outScriptPath);
#ifndef WILL3D_LIGHT
	setEnableOnlyTRDMode(tab_mgr_->SetEnableOnlyTRDMode());
#endif

#ifdef WILL3D_VIEWER
	cd_viewer_load_timer_ = new QTimer();
	cd_viewer_load_timer_->setInterval(300);
	cd_viewer_load_timer_->setSingleShot(true);
	connect(cd_viewer_load_timer_, &QTimer::timeout, this, &CWill3D::slotCDViewerLoad);

	cd_viewer_load_timer_->start();
#endif

	tf_adjust_timer_ = new QTimer();
	tf_adjust_timer_->setInterval(300);
	tf_adjust_timer_->setSingleShot(true);
	connect(tf_adjust_timer_, &QTimer::timeout, this,
					&CWill3D::slotTimeoutAdjustOTF);

	LoadWindowGeometry();
}

CWill3D::~CWill3D()
{
	SaveWindowGeometry();

#ifndef WILL3D_VIEWER
	CW3DBM::getInstance()->destroy();
#endif
	SAFE_DELETE_OBJECT(tab_mgr_);
	SAFE_DELETE_OBJECT(horizontal_toolbar_);
	SAFE_DELETE_OBJECT(vertical_toolbar_);
	SAFE_DELETE_OBJECT(title_bar_);
	SAFE_DELETE_OBJECT(m_menuLayout);
	SAFE_DELETE_OBJECT(m_mainLayout);
	SAFE_DELETE_OBJECT(m_pRcontainer);
	SAFE_DELETE_OBJECT(tab_contents_widget_);
	SAFE_DELETE_OBJECT(tf_adjust_timer_);
#ifdef WILL3D_VIEWER
	SAFE_DELETE_OBJECT(cd_viewer_load_timer_);
#endif

#if 0
	SAFE_DELETE_OBJECT(m_pgVREngine);
#else
	CW3VREngine::Destroy();
#endif
	Logger::instance()->PrintDebugMode("CWill3D::~CWill3D", "Destroy : m_pgVREngine");
}

void CWill3D::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		dragStartPosition = event->pos();
		dragStartGeometry = geometry();
	}

	QFrame::mousePressEvent(event);
}

void CWill3D::mouseMoveEvent(QMouseEvent *event)
{
	QFrame::mouseMoveEvent(event);

	GlobalPreferences *global_preferences = GlobalPreferences::GetInstance();
	int maximize_mode = GlobalPreferences::GetInstance()->preferences_.general.interfaces.maximize_type;
	const bool is_maximized = global_preferences->preferences_.general.interfaces.is_maximize;

	if (!(event->buttons() & Qt::LeftButton) && !is_maximized && (maximize_mode == 0))
	{
		// No drag, just change the cursor and return

		if (event->x() <= 3 && event->y() <= 3)
		{
			startPos = topleft;
			QApplication::setOverrideCursor(CW3Cursor::SizeFDiagCursor());
		}
		else if (event->x() <= 14 && event->y() >= height() - 14)
		{
			startPos = bottomleft;
			QApplication::setOverrideCursor(CW3Cursor::SizeBDiagCursor());
		}
		else if (event->x() >= width() - 3 && event->y() <= 3)
		{
			startPos = topright;

			QApplication::setOverrideCursor(CW3Cursor::SizeBDiagCursor());
		}
		else if (event->x() >= width() - 14 && event->y() >= height() - 14)
		{
			startPos = bottomright;

			QApplication::setOverrideCursor(CW3Cursor::SizeFDiagCursor());
		}
		else if (event->x() <= 1)
		{
			startPos = left;

			QApplication::setOverrideCursor(CW3Cursor::SizeHorCursor());
		}
		else if (event->x() >= width() - 1)
		{
			startPos = right;

			QApplication::setOverrideCursor(CW3Cursor::SizeHorCursor());
		}
		else if (event->y() <= 1)
		{
			startPos = top;

			QApplication::setOverrideCursor(CW3Cursor::SizeVerCursor());
		}
		else if (event->y() >= height() - 14)
		{
			startPos = bottom;

			QApplication::setOverrideCursor(CW3Cursor::SizeVerCursor());
		}
		else
		{
			startPos = empty;

			QApplication::setOverrideCursor(CW3Cursor::ArrowCursor());
		}
		return;
	}

	switch (startPos)
	{
	case topleft:
		setGeometry(
				dragStartGeometry.left() - (dragStartPosition.x() - event->x()),
				dragStartGeometry.top() - (dragStartPosition.y() - event->y()),
				dragStartGeometry.width() + (dragStartPosition.x() - event->x()),
				height() + (dragStartPosition.y() - event->y()));
		dragStartGeometry = normal_geometry_ = geometry();
		break;

	case bottomleft:
		setGeometry(
				dragStartGeometry.left() - (dragStartPosition.x() - event->x()),
				dragStartGeometry.top(),
				dragStartGeometry.width() + (dragStartPosition.x() - event->x()),
				event->y());
		dragStartGeometry = normal_geometry_ = geometry();
		break;

	case topright:
		setGeometry(
				dragStartGeometry.left(),
				dragStartGeometry.top() - (dragStartPosition.y() - event->y()),
				event->x(), height() + (dragStartPosition.y() - event->y()));
		dragStartGeometry = normal_geometry_ = geometry();
		break;

	case bottomright:
		setGeometry(dragStartGeometry.left(), dragStartGeometry.top(), event->x(),
								event->y());

		dragStartGeometry = normal_geometry_ = geometry();
		break;

	case left:
		setGeometry(
				dragStartGeometry.left() - (dragStartPosition.x() - event->x()),
				dragStartGeometry.top(),
				dragStartGeometry.width() + (dragStartPosition.x() - event->x()),
				height());

		dragStartGeometry = normal_geometry_ = geometry();
		break;

	case right:
		setGeometry(dragStartGeometry.left(), dragStartGeometry.top(), event->x(),
								height());

		dragStartGeometry = normal_geometry_ = geometry();
		break;

	case top:
		setGeometry(
				dragStartGeometry.left(),
				dragStartGeometry.top() - (dragStartPosition.y() - event->y()),
				dragStartGeometry.width(),
				height() + (dragStartPosition.y() - event->y()));
		dragStartGeometry = normal_geometry_ = geometry();
		break;

	case bottom:
		setGeometry(dragStartGeometry.left(), dragStartGeometry.top(), width(),
								event->y());

		dragStartGeometry = normal_geometry_ = geometry();
		break;

	default:
		break;
	}
}

void CWill3D::initLayout(void)
{
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	this->move(0, 0);
	this->setContentsMargins(0, 0, 0, 0);

	QRect screenRect = QApplication::desktop()->screenGeometry(0);
	this->setMinimumSize(
			QSize(std::max((float)screenRect.width() * 0.5f, 960.0f),
						std::max((float)screenRect.height() * 0.5f, 600.0f)));

	m_menuLayout = new QHBoxLayout;
	m_menuLayout->setSpacing(0);
	m_menuLayout->setContentsMargins(4, 2, 4, 2);
	m_menuLayout->addWidget(tab_contents_widget_);

	QVBoxLayout *top_layout = new QVBoxLayout;
	top_layout->addWidget(title_bar_);
	top_layout->addWidget(horizontal_toolbar_);
	top_layout->setSpacing(0);

	m_mainLayout = new QVBoxLayout;
	m_mainLayout->setSpacing(0);
	m_mainLayout->setMargin(0);
	m_mainLayout->addLayout(top_layout);
	m_mainLayout->addLayout(m_menuLayout);
	m_mainLayout->addWidget(new CW3SizeGrip(this), 0,
													Qt::AlignBottom | Qt::AlignRight);

	this->setLayout(m_mainLayout);

	Logger::instance()->PrintDebugMode("CWill3D::initLayout", "layout Init");
}

void CWill3D::connections()
{
	connect(tab_contents_widget_, &TabContentsWidget::sigChangeTab, this,
					&CWill3D::slotChangeTabFromTabBar);

	connect(tab_mgr_, &CW3TabMgr::sigGetTabSlotGlobalRect, this, &CWill3D::slotGetTabSlotGlobalRect);
	connect(tab_mgr_, &CW3TabMgr::sigGetTabSlotRect, this, &CWill3D::slotGetTabSlotRect);
	connect(tab_mgr_, &CW3TabMgr::sigChangeTab, this, &CWill3D::slotChangeTab);
	connect(tab_mgr_, &CW3TabMgr::sigInitProgram, this, &CWill3D::slotInitProgram);
#ifndef WILL3D_VIEWER
	connect(tab_mgr_, &CW3TabMgr::sigSyncBDViewStatus, this, &CWill3D::slotSyncBDViewStatus);
#endif

	connect(tab_mgr_, &CW3TabMgr::sigSetDicomInfo, this, &CWill3D::slotSetDicomInfo);
#ifndef WILL3D_VIEWER
	connect(tab_mgr_, &CW3TabMgr::sigActiveLoadProject, this,
					&CWill3D::slotActiveLoadProject);
	connect(tab_mgr_, &CW3TabMgr::sigDoneLoadProject, this,
					&CWill3D::slotDoneLoadProject);
#endif
	connect(tab_mgr_, &CW3TabMgr::sigOTFAdjust, this,
					&CWill3D::slotOTFAdjust);
	connect(tab_mgr_, &CW3TabMgr::sigSaveTfPreset, this,
					&CWill3D::slotSaveTfPreset);
	connect(tab_mgr_, &CW3TabMgr::sigOTFPreset, this,
					&CWill3D::slotOTFPreset);
	connect(tab_mgr_, &CW3TabMgr::sigOTFManualOnOff, this,
					&CWill3D::slotOTFManualOnOff);
	connect(tab_mgr_, &CW3TabMgr::sigOTFAuto, this,
					&CWill3D::slotOTFAuto);
	connect(tab_mgr_, &CW3TabMgr::sigSetSoftTissueMin, this, &CWill3D::slotSetSoftTissueMin);

	connect(title_bar_, &CW3TitleBar::sigClose, this, &QWidget::close);

	connect(title_bar_, &CW3TitleBar::sigShowMaxRestore, this,
					&CWill3D::slotShowMaxRestore);

	connect(otf_view_.get(), &CW3OTFView::sigOTFSave, this,
					&CWill3D::slotOTFSave);
	connect(otf_view_.get(), &CW3OTFView::sigChangeTFMove, this,
					&CWill3D::slotUpdateTF);
	connect(otf_view_.get(), &CW3OTFView::sigRenderCompleted, this,
					&CWill3D::slotUpdateDoneTF);
	connect(otf_view_.get(), &CW3OTFView::sigShadeOnSwitch, this,
					&CWill3D::slotShadeOnSwitchTF);

	EventHandler::GetInstance()->GetCommonEventHandle().ConnectSigSetMainVolume(
			this, SLOT(slotSetMainVolume()));
	EventHandler::GetInstance()->GetCommonEventHandle().ConnectSigSetSecondVolume(
			this, SLOT(slotSetSecondVolume()));
	EventHandler::GetInstance()->GetCommonEventHandle().ConnectSigSetPanoVolume(
			this, SLOT(slotSetPanoVolume()));
	EventHandler::GetInstance()->GetCommonEventHandle().ConnectSigClearPanoVolume(
			this, SLOT(slotClearPanoVolume()));

	EventHandler::GetInstance()->GetCommonEventHandle().ConnectSigSetTFpreset(
			this, SLOT(slotOTFPreset(QString)));
	EventHandler::GetInstance()->GetCommonEventHandle().ConnectSigMoveTFpolygon(
			this, SLOT(slotMoveTFpolygon(double)));
}

void CWill3D::slotInitProgram()
{
	common::Logger::instance()->Print(common::LogType::INF, "CWill3D::slotInitProgram");
#ifndef WILL3D_LIGHT
	setEnableOnlyTRDMode(tab_mgr_->SetEnableOnlyTRDMode());
#endif
}

void CWill3D::slotChangeTabFromTabBar(TabType tabType)
{
	if (!tab_mgr_->isVolumeLoded())
	{
#ifndef WILL3D_VIEWER
		tab_contents_widget_->setTabIdx(TabType::TAB_FILE);
#endif
		return;
	}

	if (tabType == TabType::TAB_UNKNOWN)
	{
		common::Logger::instance()->Print(
				common::LogType::ERR,
				"CWill3D::slotChangeTabFromTabBar: type is unknown.");
		return;
	}

#ifndef WILL3D_VIEWER
	if (tabType == TAB_FILE)
	{
		setmenuLayout(false);
		otf_view_->setVisible(false);
	}
	else
#endif
	{
		setmenuLayout(true);
	}

	tab_mgr_->changeTab(tabType);

  if (tabType != TAB_REPORT)
	  vertical_toolbar_->setTabTools(tabType);
}

void CWill3D::slotChangeTab(TabType tabType)
{
	if (tabType == TabType::TAB_UNKNOWN)
	{
		common::Logger::instance()->Print(
				common::LogType::ERR, "CWill3D::slotChangeTab: type is unknown.");
		return;
	}

#ifndef WILL3D_VIEWER
	if (tabType == TAB_FILE)
	{
		setmenuLayout(false);
		otf_view_->setVisible(false);
	}
	else
#endif
	{
		setmenuLayout(true);
	}

	vertical_toolbar_->setTabTools(tabType);
	tab_contents_widget_->setTabIdx(tabType);
	tab_mgr_->changeTab(tabType);
}

void CWill3D::slotSetDicomInfo()
{
	if (m_pgVREngine)
		horizontal_toolbar_->SetDicomInfo(m_pgVREngine->getVol(0)->getHeader());
}

void CWill3D::slotGetTabSlotGlobalRect(QRect &global_rect)
{
	QWidget *contents_widget = tab_contents_widget_->GetTabSlotWidget();
	const QRect &rect = contents_widget->rect();

	QPointF global_top_left = contents_widget->mapToGlobal(QPoint(0, 0));
	QPointF global_bottom_right = contents_widget->mapToGlobal(QPoint(rect.width(), rect.height()));

	qDebug() << "tab_contents_widget_ global_top_left :" << global_top_left;
	qDebug() << "tab_contents_widget_ global_bottom_right :" << global_bottom_right;

	global_rect = QRect(global_top_left.x(), global_top_left.y(),
											global_bottom_right.x() - global_top_left.x(),
											global_bottom_right.y() - global_top_left.y());
}

void CWill3D::slotGetTabSlotRect(QRect &rect)
{
	QWidget *contents_widget = tab_contents_widget_->GetTabSlotWidget();

	QPoint pos = tab_contents_widget_->pos() + contents_widget->pos();
	int rect_width = contents_widget->rect().width();
	int rect_height = contents_widget->rect().height();

	qDebug() << "tab_contents_widget_ pos :" << pos;
	qDebug() << "tab_contents_widget_ rect :" << rect_width << rect_height;

	rect = QRect(
			pos.x(),
			pos.y(),
			rect_width,
			rect_height);
}
#ifndef WILL3D_VIEWER
void CWill3D::slotActiveLoadProject()
{
	tab_contents_widget_->setEnabled(false);
}

void CWill3D::slotDoneLoadProject() { tab_contents_widget_->setEnabled(true); }
#endif
#ifndef WILL3D_VIEWER
void CWill3D::slotSyncBDViewStatus()
{
	vertical_toolbar_->setTabTools(TabType::TAB_IMPLANT);
}
#endif
void CWill3D::setmenuLayout(bool isUseToolVBar)
{
	vertical_toolbar_->setVisible(false);

	if (isUseToolVBar)
	{
		bool isExsistVBar = false;
		for (int i = 0; i < m_menuLayout->count(); i++)
		{
			if (m_menuLayout->itemAt(i)->widget() == vertical_toolbar_)
			{
				isExsistVBar = true;
				break;
			}
		}
		if (!isExsistVBar)
			m_menuLayout->addWidget(vertical_toolbar_);
	}
	else
	{
		for (int i = 0; i < m_menuLayout->count(); i++)
		{
			if (m_menuLayout->itemAt(i)->widget() == vertical_toolbar_)
			{
				m_menuLayout->removeWidget(vertical_toolbar_);
			}
		}
	}

	vertical_toolbar_->setVisible(isUseToolVBar);
}

void CWill3D::setEnableOnlyTRDMode(bool enable)
{
	if (enable)
	{
		tab_contents_widget_->setOnlyTRDMode();
	}
	else
	{
		tab_contents_widget_->initTab();
	}
}

void CWill3D::keyPressEvent(QKeyEvent *event)
{
	tab_mgr_->keyPressEvent(event);
}

void CWill3D::keyReleaseEvent(QKeyEvent *event)
{
	tab_mgr_->keyReleaseEvent(event);
}

void CWill3D::moveEvent(QMoveEvent *event)
{
	QFrame::moveEvent(event);

	QRect rect = geometry();
	CW3ProgressDialog *progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);
	progress->move(rect.center() - QPoint(progress->width() * 0.5f, progress->height() * 0.5f));
}

void CWill3D::resizeEvent(QResizeEvent *event)
{
	QFrame::resizeEvent(event);

	QRect rect = geometry();
	CW3ProgressDialog *progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);
	progress->move(rect.center() - QPoint(progress->width() * 0.5f, progress->height() * 0.5f));
}

void CWill3D::closeEvent(QCloseEvent *event)
{
	if (tab_mgr_->CloseEvent())
	{
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void CWill3D::slotShowMaxRestore(bool isMax)
{
#if 0
	if (isMax)
	{
		QWidget::showMaximized();
	}
	else
	{
		QWidget::showNormal();
	}
#else
	if (isMax)
	{
		ShowMaximized();
	}
	else
	{
		ShowNormal();
	}
#endif

	GlobalPreferences *global_preferences = GlobalPreferences::GetInstance();
	global_preferences->preferences_.general.interfaces.is_maximize = isMax;
	tab_mgr_->SetApplicationUIMode(isMax);
}
#ifndef WILL3D_VIEWER
void CWill3D::slotSave()
{
	tab_mgr_->SaveProject();
}
#endif
void CWill3D::slotCapture()
{
	tab_mgr_->CaptureScreenShot();
}

void CWill3D::slotPrint()
{
	tab_mgr_->Print();
}
#ifndef WILL3D_VIEWER
void CWill3D::slotLogout()
{
#ifndef WILL3D_VIEWER
	logout_timer_->stop();
#endif

	setVisible(false);

	GlobalPreferences *global_preferences = GlobalPreferences::GetInstance();
	global_preferences->preferences_.login.auto_login = false;
	global_preferences->SaveLogin();

	LoginDialog login_dialog(this);
	common::Logger::instance()->PrintDebugMode("CWill3D::slotLogout",
																						 "Create : login_dialog");

	this->hide();
	if (login_dialog.exec())
	{
#ifndef WILL3D_VIEWER
		logout_timer_->start();
#endif
		setVisible(true);
	}
	else
	{
		QCoreApplication::exit();
	}
}
#endif
void CWill3D::slotPreferences()
{
	GlobalPreferencesDialog preferences_dialog(
			m_pgVREngine, tab_mgr_->mpr_engine(), m_pRcontainer, this);
	if (preferences_dialog.exec() && m_pgVREngine)
	{
		m_pgVREngine->ApplyPreferences();
		tab_mgr_->ApplyPreferences();
	}
}

void CWill3D::slotAbout()
{
	AboutDialog about_dialog(this);
	about_dialog.exec();
}

void CWill3D::slotSupport()
{
	QFileInfo file_info("./TeamViewerQS.exe");
	if (!file_info.exists())
	{
		return;
	}

	QProcess process;
	process.execute(file_info.absoluteFilePath());
}

void CWill3D::slotOTFSave()
{
	CW3OTFPresetDlg dlg(this);
	connect(&dlg, SIGNAL(sigSavePreset(QString)), this,
					SLOT(slotSaveTfPreset(QString)));

	if (dlg.exec())
		tab_mgr_->InitOTFPreset(otf_view_->getCurrentPreset());

	/*disconnect(&dlg, SIGNAL(sigSavePreset(QString)), this,
		SLOT(slotSavePreset(QString)));*/
}
void CWill3D::slotUpdateTF(bool changed_min_max)
{
	render_engine_->UpdateTF(changed_min_max);
	tab_mgr_->SetRenderQuality(false);

	// thyoo 180921. tmp
	const auto &tf = ResourceContainer::GetInstance()->GetTfResource();
	m_pgVREngine->slotUpdateTF(tf.min_value(), tf.max_value(), changed_min_max);
}

void CWill3D::slotUpdateDoneTF() { tab_mgr_->SetRenderQuality(true); }

void CWill3D::slotShadeOnSwitchTF(bool is_shade_on)
{
	render_engine_->SetVolumeShade(is_shade_on);
	m_pgVREngine->SetVolShade(is_shade_on);
	tab_mgr_->SetRenderQuality(true);
}

void CWill3D::slotOTFAdjust(AdjustOTF &adjust_otf)
{
	const auto &vol = ResourceContainer::GetInstance()->GetMainVolume();
	if (!&vol)
	{
		common::Logger::instance()->Print(
				common::LogType::ERR, "CWill3D::slotOTFAdjust: volume is empty");
		assert(false);
		return;
	}

	adjust_otf.bright = (adjust_otf.bright / 200.0f - 0.25f) * vol.windowCenter();
	adjust_otf.contrast = exp(adjust_otf.contrast / 50.0f - 1.0f);
	adjust_otf.opacity = adjust_otf.opacity / 50.0f;
	tf_adjust_timer_->start();
	if (!otf_view_->setAdjustOTF(adjust_otf))
	{
		common::Logger::instance()->Print(common::LogType::ERR,
																			"CWill3D::slotOTFAdjust: OTF Set failed");
		assert(false);
		return;
	}
}

void CWill3D::slotOTFPreset(QString preset)
{
	if (preset.compare("0") == 0)
		return;

	m_pgVREngine->SetVRreconType(preset);
	render_engine_->SetVRreconType(preset);

	otf_view_->SetOTFpreset(preset);
}

void CWill3D::slotOTFManualOnOff()
{
	otf_view_->setVisible(!otf_view_->isVisible());
}

void CWill3D::slotOTFAuto()
{
	this->slotOTFPreset(otf_view_->getCurrentPreset());
}

void CWill3D::slotSaveTfPreset(QString file_path)
{
	QString imgFilePath = "./tfpresets/" + file_path + ".bmp";
	render_engine_->SavePresetVolumeImage(imgFilePath);

	QString tfFilePath = "./tfpresets/" + file_path + ".tf";
	otf_view_->Export(tfFilePath);
}

void CWill3D::slotSetMainVolume()
{
	QElapsedTimer timer;
	timer.start();

	qDebug() << "start slotSetMainVolume";

	const auto &vol = ResourceContainer::GetInstance()->GetMainVolume();
	if (!&vol)
	{
		common::Logger::instance()->Print(
				common::LogType::ERR, "CWill3D::slotSetMainVolume: volume is empty");
		assert(false);
		return;
	}

	render_engine_->ClearTF();
	render_engine_->SetVolume(vol, Will3DEngine::VOL_MAIN);

	float slope = vol.slope();
	int intercept = vol.intercept();
	int *histo = vol.getHistogram();
	int histo_size = vol.getHistoSize();

	otf_view_->initOTF(histo, histo_size, slope, intercept);
	otf_view_->setThreshold(vol.getAirTissueThreshold(), vol.getTissueBoneThreshold(), vol.getBoneTeethThreshold());

	m_pgVREngine->initTF((int)ResourceContainer::GetInstance()->GetTfResource().size());
	m_pgVREngine->tmpSetVolTexHandler(render_engine_->tmpGetVolTexHandler(Will3DEngine::VOL_MAIN), 0);
	m_pgVREngine->tmpSetTfTexHandler(render_engine_->tmpGetTfTexHandler());
	m_pgVREngine->setDownFactor(VREngine::VolType::MAIN, render_engine_->tmpGetDownFactor(Will3DEngine::VOL_MAIN));

	qDebug() << "end slotSetMainVolume :" << timer.elapsed();
}

void CWill3D::slotSetSecondVolume()
{
	const auto &vol = ResourceContainer::GetInstance()->GetSecondVolume();
	if (!&vol)
	{
		common::Logger::instance()->Print(
				common::LogType::ERR, "CWill3D::slotSetSecondVolume: volume is empty");
		assert(false);
		return;
	}

	render_engine_->SetVolume(vol, Will3DEngine::VOL_SECOND);
	m_pgVREngine->tmpSetVolTexHandler(
			render_engine_->tmpGetVolTexHandler(Will3DEngine::VOL_SECOND), 1);
	m_pgVREngine->setDownFactor(VREngine::VolType::SECOND,
															render_engine_->tmpGetDownFactor(Will3DEngine::VOL_SECOND));
}

void CWill3D::slotSetPanoVolume()
{
	const auto &vol =
			ResourceContainer::GetInstance()->GetPanoResource().pano_vol();
	if (!&vol)
	{
		common::Logger::instance()->Print(common::LogType::ERR, "CWill3D::slotSetPanoVolume: volume is empty");
		return;
	}

	render_engine_->SetVolume(vol, Will3DEngine::VOL_PANORAMA);
	render_engine_->UpdateTF(true);
	// tab_mgr_->SetRenderQuality(true);
}

void CWill3D::slotClearPanoVolume()
{
	render_engine_->ClearVolume(Will3DEngine::VOL_PANORAMA);
	// tab_mgr_->SetRenderQuality(true);
}

void CWill3D::slotMoveTFpolygon(const double &value)
{
	otf_view_->movePolygonAtMin((float)value);
}

void CWill3D::slotTimeoutAdjustOTF() { tab_mgr_->SetRenderQuality(true); }

const void CWill3D::SaveWindowGeometry() const
{
	QSettings settings(GlobalPreferences::GetInstance()->ini_path(), QSettings::IniFormat);
	settings.setValue("INTERFACE/geometry", geometry());
	GlobalPreferences *global_preferences = GlobalPreferences::GetInstance();

	int maximize_mode = GlobalPreferences::GetInstance()->preferences_.general.interfaces.maximize_type;
	if (maximize_mode == 1)
	{
		global_preferences->preferences_.general.interfaces.is_maximize = isMaximized();
	}
}

const void CWill3D::LoadWindowGeometry()
{
	GlobalPreferences *global_preferences = GlobalPreferences::GetInstance();

	QSettings settings(global_preferences->ini_path(), QSettings::IniFormat);
	QRect default_geometry = QApplication::desktop()->screenGeometry();
	default_geometry.setY(default_geometry.y() - 1);
	QRect geometry = settings.value("INTERFACE/geometry", default_geometry).toRect();
	const bool maximized = global_preferences->preferences_.general.interfaces.is_maximize;

	if (maximized)
	{
		const int width_margin = geometry.width() / 10;
		const int height_margin = geometry.height() / 10;
		geometry.adjust(width_margin, height_margin, -width_margin, -height_margin);
	}

	if (geometry.isValid())
	{
		setGeometry(geometry);
	}

	int maximize_mode = GlobalPreferences::GetInstance()->preferences_.general.interfaces.maximize_type;
	if (maximize_mode == 0)
	{
		if (maximized)
		{
			title_bar_->SetMaximized(maximized);
		}
	}
	else
	{
		if (maximized)
		{
			ShowMaximized();
		}
		else
		{
			ShowNormal();
		}
	}
}

const void CWill3D::ShowMaximized()
{
#if 0
	int maximize_mode = GlobalPreferences::GetInstance()->preferences_.general.interfaces.maximize_type;
#endif

	int maximize_mode = 0;

	switch (maximize_mode)
	{
	case 0:
		showMaximized();
		break;
	case 1:
		normal_geometry_ = geometry();
		QRect screen_rect = QApplication::desktop()->screenGeometry();
		screen_rect.setY(screen_rect.y() - 1);
#if 1
		setGeometry(screen_rect);
#else
		move(screen_rect.x(), screen_rect.y());
		resize(screen_rect.width(), screen_rect.height());
#endif
		break;
	}
}

const void CWill3D::ShowNormal()
{
#if 0
	int maximize_mode = GlobalPreferences::GetInstance()->preferences_.general.interfaces.maximize_type;
#endif

	int maximize_mode = 0;

	switch (maximize_mode)
	{
	case 0:
		showNormal();
		break;
	case 1:
#if 1
		setGeometry(normal_geometry_);
#else
		move(normal_geometry_.x(), normal_geometry_.y());
		resize(normal_geometry_.width(), normal_geometry_.height());
#endif
		break;
	}
}

void CWill3D::slotSetSoftTissueMin(const float value)
{
	otf_view_->SetSoftTissueMin(value);
}

#ifdef WILL3D_VIEWER
void CWill3D::slotCDViewerLoad()
{
	cd_viewer_load_timer_->stop();
	SAFE_DELETE_OBJECT(cd_viewer_load_timer_);

	tab_mgr_->ImportCDViewerData();
}
#endif

bool CWill3D::event(QEvent *event)
{
#ifndef WILL3D_VIEWER
	if (event->type() != QEvent::Timer)
	{
		logout_timer_->setInterval(logout_timer_interval_);
	}
#endif

	return QFrame::event(event);
}
