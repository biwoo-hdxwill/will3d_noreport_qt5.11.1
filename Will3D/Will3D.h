#pragma once
/*=========================================================================

File:			class CWill3D
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-11-21
Last modify:	2015-12-18

=========================================================================*/
#include <memory>

#include <QFrame>
#include <qboxlayout.h>

#include <Engine/Common/Common/W3Enum.h>
#include <Engine/Common/Common/W3Types.h>

class QMouseEvent;
class CW3VREngine;
class TabContentsWidget;
class CW3TabMgr;
class CW3TitleBar;
class CW3ToolHBar;
class CW3ToolVBar;
class CW3ResourceContainer;
class RenderEngine;
class CW3OTFView;
class QTimer;

class Will3DEventFilter : public QObject
{
	Q_OBJECT

public:
	Will3DEventFilter(QObject *parent = 0) : QObject(parent) {}
	~Will3DEventFilter() {}

signals:
	void sigGraphicsSceneMouseReleaseEvent(QMouseEvent *event);
	void sigGraphicsSceneMousePressEvent(QMouseEvent *event);

protected:
	bool eventFilter(QObject *obj, QEvent *event);
};

class CWill3D : public QFrame
{
	Q_OBJECT

public:
	// TODO. RenderEngine을 받는 이유는 VREngine때문임. 리펙토링 해서 VREngine이
	// 없어지게 되면 이 클래스에서 RenderEngine을 생성하고 관리하도록 변경해야함.
	CWill3D(RenderEngine *render_engine, CW3VREngine *VREngine,
					const QString &readFilePath, const QString &outScriptPath,
					QWidget *parent = 0);
	~CWill3D();

protected:
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void keyReleaseEvent(QKeyEvent *event) override;
	virtual void moveEvent(QMoveEvent *event) override;
	virtual void resizeEvent(QResizeEvent *event) override;
	virtual void closeEvent(QCloseEvent *event) override;
	virtual bool event(QEvent *event) override;

signals:
	void sigSetVolume(TabType);
	void sigGoToMPR(TabType);

private:
	void initLayout();
	void connections();

	void setmenuLayout(bool isUseToolVBar);
	void setEnableOnlyTRDMode(bool enable);

	const void SaveWindowGeometry() const;
	const void LoadWindowGeometry();

	const void ShowMaximized();
	const void ShowNormal();

private slots:
	void slotShowMaxRestore(bool);
#ifndef WILL3D_VIEWER
	void slotSave();
#endif
	void slotCapture();
	void slotPrint();
#ifndef WILL3D_VIEWER
	void slotLogout();
#endif
	void slotPreferences();
	void slotAbout();
	void slotSupport();
#ifndef WILL3D_VIEWER
	void slotSyncBDViewStatus();
#endif
	void slotOTFSave();
	void slotUpdateTF(bool changed_min_max);
	void slotUpdateDoneTF();
	void slotShadeOnSwitchTF(bool is_shade_on);
	void slotOTFAdjust(AdjustOTF &adjust_otf);
	void slotOTFPreset(QString preset);
	void slotOTFManualOnOff();
	void slotOTFAuto();
	void slotSaveTfPreset(QString file_path);

	void slotInitProgram();
	void slotSetDicomInfo();
	void slotGetTabSlotGlobalRect(QRect &global_rect);
	void slotGetTabSlotRect(QRect &rect);
	void slotChangeTab(TabType tabType);
	void slotChangeTabFromTabBar(TabType);
#ifndef WILL3D_VIEWER
	void slotActiveLoadProject();
	void slotDoneLoadProject();
#endif
	void slotSetMainVolume();
	void slotSetSecondVolume();
	void slotSetPanoVolume();
	void slotClearPanoVolume();
	void slotMoveTFpolygon(const double &value);
	void slotTimeoutAdjustOTF();

	void slotSetSoftTissueMin(const float value);

#ifdef WILL3D_VIEWER
	void slotCDViewerLoad();
#endif

private:
	CW3TabMgr *tab_mgr_ = nullptr;
	CW3ResourceContainer *m_pRcontainer = nullptr;

	QHBoxLayout *m_menuLayout = nullptr;
	QVBoxLayout *m_mainLayout = nullptr;

	TabContentsWidget *tab_contents_widget_ = nullptr;
	CW3TitleBar *title_bar_ = nullptr;
	CW3ToolHBar *horizontal_toolbar_ = nullptr;
	CW3ToolVBar *vertical_toolbar_ = nullptr;
	// timer
	QTimer *tf_adjust_timer_ = nullptr;
	enum startPositions
	{
		topleft,
		left,
		bottomleft,
		bottom,
		bottomright,
		right,
		topright,
		top,
		empty,
	};
	startPositions startPos = startPositions::empty;

	QPoint dragStartPosition;
	QRect dragStartGeometry;

	CW3VREngine *m_pgVREngine = nullptr;
	std::unique_ptr<RenderEngine> render_engine_;
	std::unique_ptr<Will3DEventFilter> event_filter_;
	std::unique_ptr<CW3OTFView> otf_view_;

	QRect normal_geometry_;

#ifdef WILL3D_VIEWER
	QTimer *cd_viewer_load_timer_ = nullptr;
#endif

#ifndef WILL3D_VIEWER
	QTimer *logout_timer_ = nullptr;
	int logout_timer_interval_ = 0;
#endif
};
