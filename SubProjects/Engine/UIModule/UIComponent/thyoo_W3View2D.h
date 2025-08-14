#pragma once
/*=========================================================================

File:			class CW3View2D
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-11-21
Last modify:	2015-12-11

=========================================================================*/
#include <GL/glew.h>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#include <glm/gtx/transform2.hpp>
#else
#include <gl/glm/glm.hpp>
#include <GL/glm/gtx/transform2.hpp>
#endif

#include <qgraphicsview.h>
#include <QMouseEvent>
#include <qvector3d.h>

#include "../../Common/Common/W3Enum.h"

#include <Engine/Common/Common/define_view.h>
#include <Engine/Common/Common/define_measure.h>
#include "uicomponent_global.h"

class QOpenGLWidget;
class MeasureTools;
class ViewNavigatorItem;
class CW3TextItem;
class CW3VREngine;
class CW3MPREngine;
class CW3ViewPlane;
class CW3ViewRuler;
class GridLines;
#ifndef WILL3D_VIEWER
class ProjectIOView;
#endif

class UICOMPONENT_EXPORT CW3View2D_thyoo : public QGraphicsView {
	Q_OBJECT

public:
	CW3View2D_thyoo(
		CW3VREngine *VREngine,
		CW3MPREngine *MPRengine,
		common::ViewTypeID eType,
		bool is2Dused,
		QWidget *pParent = 0);

	~CW3View2D_thyoo();

#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOView & out);
	void importProject(ProjectIOView & in);
#endif

	virtual void SetCommonToolOnce(const common::CommonToolTypeOnce& type, bool on);
	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type);

	QPointF GetViewCenterinScene(void) { return m_pntCurViewCenterinScene; }

	virtual void reset();
	virtual void setVisible(bool state);
	void ApplyPreferences();
	void DeleteMeasureUI(const unsigned int & measure_id);

#ifdef WILL3D_EUROPE
	inline void SetSyncControlButton(bool is_on) { is_control_key_on_ = is_on; }
#endif // WILL3D_EUROPE

signals:
#ifdef WILL3D_EUROPE
	void sigSyncControlButton(bool);
	void sigShowButtonListDialog(const QPoint& window_pos, const int mpr_type = -1);
#endif // WILL3D_EUROPE

protected:
	virtual void ResetView();
	virtual void FitView();
	void HideText(bool bToggled);
	virtual void HideUI(bool bToggled);
	virtual void HideMeasure(bool bToggled);
	virtual void DeleteAllMeasure();
	virtual void DeleteUnfinishedMeasure();
	virtual void resizeEvent(QResizeEvent *pEvent) override;
	virtual void resizeScene();
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseDoubleClickEvent(QMouseEvent *event);
	virtual void mousePanningEvent();
	virtual void mouseScaleEvent();
	virtual void wheelEvent(QWheelEvent *event);
	virtual void drawBackground(QPainter *painter, const QRectF &rect);
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
	virtual void enterEvent(QEvent* event) override;

	virtual void setProjection();
	virtual void clearGL();

	inline const float scaledSceneToVol(float f) const { return f * m_scaleSceneToGL*0.5f; }
	inline const float scaledVolToScene(float f) const { return f * 2.0f / m_scaleSceneToGL; }
	inline const float scaledGLToVol(float f) const { return f * 0.5f; }
	inline const float scaledVolToGL(float f) const { return f * 2.0f; }

	void setViewRulerValue();
	void SetGridValue();
	virtual void SetVisibleItems();

	void transformItems(const QPointF& translate);
	void transformItems(const QPointF& preViewCenterInScene, const QPointF& curViewCenterInScene, float scale);

	void transformPositionMemberItems(const QTransform& transform);
	virtual void transformPositionItems(const QTransform& transform) {};

	void resetFromViewTool();
	void setViewProjection();
	
private:
	void GridOnOff(bool grid_on);

protected:
	QOpenGLWidget * m_pGLWidget = nullptr;

	CW3MPREngine *m_pgMPRengine;
	CW3VREngine *m_pgVREngine;

	CW3ViewPlane *m_pViewPlane = nullptr;

	GLenum	m_texNumPlane = GL_TEXTURE0;
	int	m_texNumPlane_ = 0;
	unsigned int	m_texHandlerPlane = 0;
	float		m_scaleSceneToGL = 1.0f;
	// Actual Coordinate = (X + size)*0.5f
	float		m_sceneWinView = 1.0f;
	float		m_sceneHinView = 1.0f;
	float		m_Wglorig = 1.0f;
	float		m_Hglorig = 1.0f;
	float		m_Wglpre = 1.0f;
	float		m_Hglpre = 1.0f;
	float		m_Dglpre = 1.0f;
	float		m_Wgl = 1.0f;
	float		m_Hgl = 1.0f;
	float		m_Dgl = 1.0f;
	float		m_WglTrans = 0.0f;
	float		m_HglTrans = 0.0f;

	// projection 의 네 변의 위치
	float proj_left_;
	float proj_right_;
	float proj_top_;
	float proj_bottom_;

	common::ViewTypeID	m_eViewType;
	common::ReconTypeID	m_eReconType = common::ReconTypeID::VR;

	QPointF m_pntCurViewCenterinScene;

	glm::mat4 m_model;
	glm::mat4 m_view;
	glm::mat4 m_projection;
	glm::mat4 m_mvp;
	float m_scale;
	// model 사이즈는 고정된 상태에서 ViewFrustum 사이즈를 늘이고 줄임

	// used for PANO recon, every recon except MPRtab
	std::vector<QVector3D> *m_TopRightCoord;
	std::vector<QVector3D> *m_NormalUpToLower;

	// view 좌표
	QPointF last_view_pos_;
	QPointF curr_view_pos_;
	// scene 좌표
	QPointF last_scene_pos_;
	QPointF curr_scene_pos_;

	float		m_scalePre = 1.0f;

	////////////////////////////////////
	///////////// Flag For State

	bool		m_isIntendedResize = false;
	bool		m_is2Dready = false;

	MeasureTools *measure_tools_;
	common::CommonToolTypeOnOff common_tool_type_ = common::CommonToolTypeOnOff::NONE;

	bool hide_all_view_ui_ = false;

	CW3TextItem *m_pTextRight;

	bool m_bLoadProject = false;

	CW3ViewRuler *ruler_ = nullptr;
	GridLines* grid_ = nullptr;
	ViewNavigatorItem *m_pWorldAxisItem = nullptr;
	bool	m_is2Dused;

	QPointF m_sceneTrans = QPointF(0.0f, 0.0f);

#ifdef WILL3D_EUROPE
	bool is_control_key_on_ = false;
#endif // WILL3D_EUROPE

private:
	bool show_rulers_ = false;
	bool grid_visible_ = false;
};
