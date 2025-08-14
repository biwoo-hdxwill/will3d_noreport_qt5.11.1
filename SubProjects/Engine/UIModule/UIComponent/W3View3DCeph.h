#pragma once
/*=========================================================================

File:			class CW3View3DCeph
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon Yoo
First date:		2016-04-14
Last modify:	2016-04-14

=========================================================================*/
#include <memory>

#include "../../Common/GLfunctions/W3GLTypes.h"
#include "../UIGLObjects/W3SurfaceItem.h"
#include "../../../Managers/DBManager/W3DBM.h"

#include "uicomponent_global.h"
#include "thyoo_W3View3D.h"

class CW3SurfaceAxesItem;
class CW3SurfaceTextEllipseItem;
class CW3SurfaceArchItem;
class CW3SurfacePlaneItem;
class CW3SurfaceAngleItem;
class CW3SurfaceDistanceItem;
class CW3TextItem;
class ViewNavigatorItem;
class CW3VTOSTO;
class CW3CephDM;
class CW3VREngine;
class QTimer;
class ViewBorderItem;
class CW3Render3DParam;
#ifndef WILL3D_VIEWER
class ProjectIOCeph;
#endif

class UICOMPONENT_EXPORT CW3View3DCeph : public CW3View3D_thyoo
{
	Q_OBJECT

public:
	CW3View3DCeph(CW3VREngine *VREngine, CW3MPREngine *MPREngine, 
		CW3VTOSTO* vtosto, CW3CephDM* DataManager,
		common::ViewTypeID eType, QWidget *pParent = 0);
	~CW3View3DCeph(void);

	// serialization
#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOCeph& out);
	void importProject(ProjectIOCeph& in);
#endif

	void UpdateVR(bool is_high_quality);
	void setVisible(bool is_visible);
	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;
	virtual void reset();
	inline void setViewType(common::ViewTypeID type) { m_eViewType = type; }

signals:
	void sigDoneTracingTask(const QString& task);
	void sigSetClipToolValues(const bool isEnable, const bool isFlip, const MPRClipID plane,
		const int lower, const int upper);
	void sigClipUpper(int upper);
	void sigClipLower(int lower);

	void sigSurgeryTrans(const surgery::CutTypeID& cut_id,
		const glm::vec3& trans);

	void sigSurgeryRotate(const surgery::CutTypeID& cut_id,
		const surgery::RotateID& trans_id,
		float trans);

	void sigSetZeroValuesSurgeryBar(const surgery::CutTypeID& cutType);
	void sigDisableAllSurgeryBar();
	void sigSetTracingGuideImage(const QString& tracing);

	void sigSyncCephIndicatorBar();

public:
	void setVTO();

	void setNoEventMode(bool isEnable);

	void forceRotateMatrix(const glm::mat4& mat);

	inline bool isFinishedTracing() { return m_isFinishTracing; }
	void TracingTasksClear();
	void TracingTasksFinished();
	void TracingTaskCancel();
	void FacePhotoEnable(int);
	void FacePhotoTransparencyChange(int value);

	void SetClipValues(const MPRClipID& clip_plane, const bool& is_clipping,
		const int& lower, const int& upper);

	void SurgeryEnable(const bool);

	public slots:
	void slotSetPhoto();

	/////////////////////////////////////////////////////////////////
	///signal: CW3CephTracingBar::m_btnTracingTasks
	///connection: CW3CephTracingBar->CW3CephViewMgr->this
	/////////////////////////////////////////////////////////////////
	void slotActiveTracingTask(const QString& task);

	/////////////////////////////////////////////////////////////////
	/// signal: CW3CephTracingBar
	/// connection: CW3CephTracingBar->CW3CephViewMgr->this
	/////////////////////////////////////////////////////////////////
	void slotSetCoordSystem(const QStringList& tasks);

	/////////////////////////////////////////////////////////////////
	/// signal: CW3CephIndicatorBar
	/// connection: CW3CephIndicatorBar->CW3CephViewMgr->this
	/////////////////////////////////////////////////////////////////
	void slotLandmarkChangeContentSwitch(const QString&, const QString&, bool);

	/////////////////////////////////////////////////////////////////
	/// signal: CW3CephIndicatorBar
	/// connection: CW3CephIndicatorBar->CW3CephViewMgr->this
	/////////////////////////////////////////////////////////////////
	void slotMeasurementChangeContentSwitch(const QString&, const QString&, bool);

	/////////////////////////////////////////////////////////////////
	/// signal: CW3CephIndicatorBar
	/// connection: CW3CephIndicatorBar->CW3CephViewMgr->this
	/////////////////////////////////////////////////////////////////
	void slotReferenceChangeContentSwitch(const QString&, const QString&, bool);

private:
	virtual void drawBackground(QPainter *painter, const QRectF &rect) override;
	virtual void drawBackFaceSurface() override;
	virtual void drawSurface() override;
	virtual void drawTransparencySurface() override;
	virtual void drawOverrideSurface() override;

	virtual void resizeScene() override;
	virtual void setMVP(float rotAngle, glm::vec3 rotAxis) override;
	virtual void setReorientation(const glm::mat4& reorienMat) override;
	virtual void clearGL();
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void wheelEvent(QWheelEvent *event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;
	void resizeEvent(QResizeEvent *pEvent) override;

	void initializeGL(void);
	void initPhotoGL(void);
	void surgeryRayCasting(void);
	void drawSubtractedMandibleByChin();
	void renderingGL(void);

	void pointSelection();
	void editSpinBoxMoveSurgery(const glm::vec3& translate, const surgery::CutTypeID& cutType);
	void editSpinBoxRotateSurgery(const QPair<float, glm::vec3>& pairRot, const surgery::CutTypeID& cutType);

	void MeshMove();

	enum GL_SURGERY_TEXTURE_HANDLE {
		TEX_MAXILLA_ENTRY_POSITION,
		TEX_MANDIBLE_ENTRY_POSITION,
		TEX_CHIN_ENTRY_POSITION,
		TEX_MAXILLA_EXIT_POSITION,
		TEX_MANDIBLE_EXIT_POSITION,
		TEX_CHIN_EXIT_POSITION,
		TEX_END
	};

	void initItems();
	void setVisibleItems(bool isVisible);
	void initFaceItem();
	void initReferenceItem();
	void initSurgeryItems();
	void initLandmarkItem();
	void initItemModelScale();
	bool setTF(const QString& preset);

	void makeMeshMove();
	void calcDisps();
	void makeSurf();
	void makeField();

	void setIsoValueRunThread(double value);
	void SetAdjustControlPoints();

	bool isVisibleAxesItem(int index);

	public slots:
	void slotSurgeryEnableOn(const surgery::CutTypeID& cut_id, const bool isEnable);
	void slotSurgeryAdjustOn(const surgery::CutTypeID& cut_id, const bool isAdjust);
	void slotSurgeryMoveOn(const surgery::CutTypeID& cut_id, const bool isAdjust);

	void slotSurgeryCutTranslate(const surgery::CutTypeID & cut_id, const glm::mat4 & trans);
	void slotSurgeryCutRotate(const surgery::CutTypeID & cut_id, const glm::mat4 & rot);
	void slotSurgeryAxisTranslate(const surgery::CutTypeID & cut_id, const glm::mat4 & trans);
	void slotSurgeryAxisRotate(const surgery::CutTypeID& cut_id, const glm::mat4& rot);

	void slotChangeFaceAfterSurface();
	void slotSceneUpdate();

private:
	std::unique_ptr<ViewBorderItem> border_;
	unsigned int m_FBHandlerSG = 0;
	unsigned int m_depthHandlerSG = 0;

	std::vector<GLenum> m_texBufferSG;
	std::vector<unsigned int> m_texHandlerSG;
	std::vector<GLenum> m_texNumSG;
	std::vector<int> m_texNum_SG;

	CW3VTOSTO* m_pgVTOSTO;

	unsigned int m_PROGsurfaceTexture;

	TracingTaskInfo m_tracingTask;
	//thyoo. tracing point를 그려주는 아이템
	CW3SurfaceTextEllipseItem* m_pLandmarkItem = nullptr;

	//thyoo. reference plane를 그려주는 아이템
	CW3SurfacePlaneItem* m_pReferPlaneItem = nullptr;

	//thyoo. measurement를 그려주는 아이템
	std::map<QString, CW3SurfaceDistanceItem*> m_pDistanceItem;
	std::map<QString, CW3SurfaceAngleItem*> m_pAngleItem;

	//thyoo. face를 그려주는 아이템
	CW3SurfaceItem* m_pFace = nullptr;

	//thyoo. 현재 찍어야 하는 tracing text를 display 해주는 아이템
	CW3TextItem* m_pCurrTracingText;

	bool m_isFace = false;
	bool m_isSurgery = false;
	//thyoo. final만 그리게 하는 플래그(raycasting 안함)
	bool m_isDrawFinal = false;
	//thyoo. finished Landmark. tracing이 끝나고 landmark 위치가 설정됐는지 알 수 있는 상태 플래그
	bool m_isFinishTracing = false;
	bool m_isMidSagittalMod = false;
	bool m_isSetCoordSys = false;
	bool m_isNoEventMode = false;

	bool is_surgery_cut_[surgery::CutTypeID::CUT_TYPE_END] = { false, false, false };
	bool is_surgery_adjust_[surgery::CutTypeID::CUT_TYPE_END] = { false, false, false };
	bool is_surgery_move_[surgery::CutTypeID::CUT_TYPE_END] = { false, false, false };

	QStringList m_activeLandmarks;
	QString	 m_activeTracingTask;

	CW3CephDM* m_pgDataManager;

	// move 선택시 나타나는 x,y,z축 아이템
	CW3SurfaceAxesItem* m_lstAxesItem[surgery::CutTypeID::CUT_TYPE_END];
	// adjust 선택시 나타나는 영역 아이템
	CW3SurfaceArchItem* m_lstSurgeryCutItem[surgery::CutTypeID::CUT_TYPE_END];

	std::map<QString, glm::vec3> m_landmarkPos;

	std::vector<std::vector<int>> m_jointMoveGroup;
	std::vector<int> m_jointFixed;

	QTimer* render_timer_;

	glm::mat4 m_orienSave = mat4(1.0f);

	int	m_width3DviewSG = 0;
	int	m_height3DviewSG = 0;

	unsigned int m_passSurgeryRayCasting;
	unsigned int m_passBasicRayCasting;

	QList<UIGLObjects::TransformMat> m_lstTransform;
	QList<std::vector<glm::vec3>> m_lstCtrlPoint;

	bool m_bLoadProject = false;

	std::map<QString, glm::vec3> m_loadProjectLandmarks;

	CW3Render3DParam* m_pRender3DParam = nullptr;
};
