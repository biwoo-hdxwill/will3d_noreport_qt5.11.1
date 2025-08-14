#pragma once
/*=========================================================================

File:			class CW3View3DMPR
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2015-12-02
Last modify:	2016-04-21

=========================================================================*/
#include <memory>

#include "W3View3D_forMPR.h"
#include "uicomponent_global.h"

class CW3TextItem_switch;
class ViewBorderItem;
class CW3Image3D;
class AnnotationFreedraw;
#ifndef WILL3D_VIEWER
class ProjectIOMPR;
#endif

class UICOMPONENT_EXPORT CW3View3DMPR : public CW3View3D_forMPR
{
	Q_OBJECT
public:
	CW3View3DMPR(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
		CW3ResourceContainer *Rcontainer, common::ViewTypeID eType,
		QWidget *pParent = 0);

	~CW3View3DMPR(void);

signals:
	void sigRequestClipStatus();
	void sigMPROverlayOn();
	void sigKeyPressEvent(QKeyEvent*);

#ifdef WILL3D_EUROPE
	void sigSyncControlButton(bool);
	void sigShowButtonListDialog(const QPoint& window_pos, const int mpr_type = -1);
#endif // WILL3D_EUROPE

public:

	virtual void reset();

	// serialization
#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOMPR& out);
	void importProject(ProjectIOMPR& in);
#endif

	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;

	void UpdateVR(bool is_high_quality = true);
	void setMPROverlay(const glm::vec4& equ, const glm::vec3& rightVec);
	void setClippingValueFromMPROverlay();
	void setTranslateMatSecondVolume(glm::mat4 *translate);
	void setRotateMatSecondVolume(glm::mat4 *rotate);
	void setOnlyTRDMode();
	void setMIP(bool isMIP);
	void setEnableClipMPRMode(bool isEnable);

	inline const bool isDrawMPROverlay() const noexcept { return m_bDrawMPROverlay; }

	glm::vec4 getClipPlaneNearbyCenter();

	void editDistanceMPROverlayFromClipPlane();
	void editMPROverlayFromClipPlane();

	void zoom3D(bool bToggled);
	void SetZoom3DVR(const MPRViewType eViewType, const glm::vec3 center, const float radius);

	bool IsDefaultInteractionsIn3D(Qt::MouseButton button);
	bool IsDefaultInteractionsIn3D(Qt::MouseButtons buttons);

	void Cut3D(bool bToggled, VRCutTool cut_tool);
	void Reset3DCut();
	void Undo3DCut();
	void Redo3DCut();

	void SetOblique(bool bToggled);
	virtual void ClipEnable(int) override;
	virtual void ClipRangeMove(int, int) override;
	virtual void ClipPlaneChanged(const MPRClipID& clip_plane) override;

	inline const CW3Image3D* vr_cut_mask_vol() const { return vr_cut_mask_vol_; }
	inline const int cur_vr_cut_history_step() const { return cur_vr_cut_history_step_; }
	inline const bool only_trd_mode() const { return m_bIsOnlyTRDMode; }

	virtual void VisibleFace(int state) override;

	void SetFlipClipping(int state);

public slots:
	void setMaximize(bool maximize);

protected:
	virtual void setInitScale();
	virtual bool IsControllerTextOnMousePress() override;

private:
	virtual void ResetView() override;
	virtual void FitView() override;
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseDoubleClickEvent(QMouseEvent *event);
	virtual void drawBackground(QPainter *painter, const QRectF &rect);
	virtual void wheelEvent(QWheelEvent *event);
	virtual void resizeEvent(QResizeEvent *pEvent) override;
	virtual void render3D();
	virtual void init();
	virtual void clearGL();
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;

	void connections();
	void setZoom3DCube();
	bool offMIPOverlayWhereOnMIP(bool isMIP);

	void InitMPROverlaySwitch();
	void SaveMPROverlaySwitch();

	bool IsCurrentDrawingVRCut();
	void Render3DAndUpdateIfVisible();

	void CreateVRCutUIInPress(const QPointF& curr_scene_pos);
	void Draw3DCutUI(QPolygonF& poly, const QPointF& curr_scene_pos);
	void Draw3DCutSelectAreaUI(QPolygonF& poly, const QPointF& curr_scene_pos);
	void DeleteVRCutUI();
	void EndDraw3DCutUI();

	void SelectVRCutAreaInPressNew();
	void CreateNewVRCutMaskVolume(int width, int height, int depth);
	void DeleteVRCutMaskVolume();
	void UpdateVRCutMaskVolume(int width, int height, int depth, unsigned short** data);
	void UpdateVRCutHistoryStep(int step);
	QList<QPolygonF> GetSubpathPolygons();

	virtual void SetVisibleItems() override;
	void DeleteVRCutPolygonArea();

	QPolygonF CreateCutAreaByLine(const QLineF& line);

private slots:
	void slotMPROverlayOnOff(bool isMPROverlay);
	void slotPerspectiveOnOff(bool isPerspective);
	void slotWheelTimeout();

private:
	enum class MPR3DEventType
	{
		NONE,
		VRCUT_DRAW,
		VRCUT_SELECT
	};

private:
	std::unique_ptr<ViewBorderItem> border_;
	CW3TextItem_switch *m_pMPROverlayOnOff = nullptr;
	CW3TextItem_switch *m_pPerspectiveOnOff = nullptr;

	VRCutTool cut_tool_ = VRCutTool::POLYGON;
	MPR3DEventType event_type_ = MPR3DEventType::NONE;
	QGraphicsPolygonItem *vr_cut_polygon_ = nullptr;
	AnnotationFreedraw* vr_cut_freedraw_ = nullptr;
	QGraphicsPolygonItem* vr_cut_selected_area_ = nullptr;

	float m_fObiquePlanePos = 0.0f;

	unsigned int m_vboCUBE[3];
	float m_fRadius = 0.0f;
	glm::vec3 zoom_center_;

	float m_vertCube[72];

	bool m_bIsZoom3DMode = false;
	bool m_bDrawMPROverlay = false;
	bool m_bToggledClipMPROverlay = false;
	bool m_bIsObliqueMode = false;
	bool m_bIsOnlyTRDMode = false;

	glm::vec4 m_saveClipPlanes[2];
	bool m_saveIsClipped;

	QTimer* wheel_timer_;

	unsigned int m_vboDrawCube = 0;
	unsigned int m_vaoDrawCube = 0;

	bool is_cut_inside_ = false;
	int cur_vr_cut_history_step_;
	int last_vr_cut_history_step_;
	bool is_cut_shifted_ = false;
	CW3Image3D *vr_cut_mask_vol_ = nullptr;
	GLenum gl_tex_num_vr_cut_mask_vol_ = GL_TEXTURE27;
	int	tex_num_vr_cut_mask_vol_ = 27;
	unsigned int tex_handler_vr_cut_mask_vol_ = 0;
};
