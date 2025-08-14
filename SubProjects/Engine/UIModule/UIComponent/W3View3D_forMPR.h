#pragma once
/*=========================================================================

File:			class CW3View3D
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2015-12-02
Last date:		2016-06-04

=========================================================================*/
#include "../../Common/GLfunctions/WGLHeaders.h"
#include "../../Common/GLfunctions/W3GLTypes.h"

#include "W3View2D_forMPR.h"
#include "uicomponent_global.h"

class CW3TextItem;
class CW3FilteredTextItem;
class Measure3DManager;
class QMenu;
class QAction;

class UICOMPONENT_EXPORT CW3View3D_forMPR : public CW3View2D_forMPR {
	Q_OBJECT

public:
	enum ClipToolType { LOWER, UPPER };

public:
	CW3View3D_forMPR(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
					 CW3ResourceContainer *Rcontainer, common::ViewTypeID eType,
					 QWidget *pParent = 0);
	virtual ~CW3View3D_forMPR(void);
	
	void setModelPhotoToMC();
	inline glm::mat4 getModelMatrix() { return m_model * glm::scale(1.0f / m_vVolRange); }
	virtual void setVisible(bool);

	void initClipValues(const MPRClipID clipPlane, const bool isClipping, const bool isFlip,
						const int lower, const int upper);

	virtual void reset();

	void setMIP(bool isMIP);

	virtual void VisibleNerve(int) override;
	virtual void VisibleImplant(int) override;
	virtual void VisibleSecond(int) override;
	virtual void VisibleFace(int) override;
	virtual void VisibleAirway(int) override;

	void SetAirway(std::vector<tri_STL>& mesh);
	void ChangeFaceTransparency(int value);
	virtual void ClipEnable(int);
	virtual void ClipRangeMove(int, int);
	void ClipRangeSet();
	virtual void ClipPlaneChanged(const MPRClipID& clip_plane);

	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;

#ifndef WILL3D_VIEWER
	virtual void exportProject(ProjectIOView& out) override;
	virtual void importProject(ProjectIOView& in) override;
#endif

	void ApplyPreferences();

signals:
	void sigSave3DFaceToPLYFile();
	void sigSave3DFaceToOBJFile();
	void sigVolumeClicked(const glm::vec3 gl_pos);

public slots:
	void slotTFupdated(bool isMinMaxChanged);
	virtual void slotUpdateRotate(glm::mat4 *model);
	virtual void slotUpdateScale(float scale);
	void slotTFupdateCompleted();
	void slotTransformedPhotoPoints(glm::mat4 *model);
	virtual void slotReoriupdate(glm::mat4 *m);
	virtual void slotImplantUpdated(int selectedImplantID);

protected:
	virtual void ResetView() override;
	virtual void FitView() override;
	virtual void InvertView(bool bToggled) override;
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void wheelEvent(QWheelEvent *event);
	virtual void keyPressEvent(QKeyEvent* event) override;

	virtual void drawBackground(QPainter *painter, const QRectF &rect);
	virtual void render3D();

	virtual void resizeEvent(QResizeEvent *pEvent) override;
	virtual void resizeScene();

	virtual void setInitScale() override;
	virtual void setProjection();
	virtual void setViewMatrix();

	virtual void setMVP();
	virtual void setModel();
	virtual void setModel(float rotAngle, glm::vec3 rotAxis);
	virtual void setDirectionFromCompass(glm::mat4 &T);

	void ArcBallRotate(const QPoint& curr_view_pos);
	glm::vec3 ArcBallVector(QPointF &v);

	virtual void init();

	void setNerveVAOVBO();
	void setAirwayVBO();
	void setAirwayVAO();

	void drawAirwayColorBar();

	bool isReadyRender3D() const;

	void setPosReconTypeTextItem();

	bool ToolTypeInteractions3DInMove(const QPoint& curr_view_pos);
	bool IsLabelSelected();
	bool Is3DRenderingMode();

	virtual void SetVisibleItems() override;

	virtual void clearGL();

	virtual void HideMeasure(bool toggled) override;
	virtual void DeleteAllMeasure() override;
	virtual void DeleteUnfinishedMeasure() override;

	void RenderAndUpdate();
	virtual bool IsControllerTextOnMousePress();

protected slots:
	void slotVRAlignS();
	void slotVRAlignI();
	void slotVRAlignL();
	void slotVRAlignR();
	void slotVRAlignA();
	void slotVRAlignP();

	void slotShadeOnFromOTF(bool isShading);

private:
	void connections();
	void setVRAlign();

	void RotateViewWithArcBall(const QPoint& curr_view_pos);
	void DrawImplantID(const QPoint& curr_view_pos);
	void ResizeAlignTexts();
	void ResizeAirwayUIs();

protected:
	CW3FilteredTextItem* ui_recon_type_ = nullptr;

	float m_camFOV = 1.0f;

	glm::vec3 m_rotAxis = vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 m_vVolRange;
	glm::vec3 m_VolCenter;	// Volume 의 center: (N-1)*0.5f

	glm::mat4 m_viewForFinal;
	glm::mat4 m_projForFinal;
	glm::mat4 m_matTextureToGL;
	glm::mat4 m_ToDepth;
	glm::mat4 m_matTexCoordToDepth;
	glm::mat4 m_matTexCoordToDepthSecond;

	glm::mat4 m_secondToFirstModel;	// second volume 을 main volume 에 일치시키기 위한 변환 행렬
	glm::mat4 m_rotateSecond;
	glm::mat4 m_translateSecond;

	///////////// Flag For State
	bool m_isPassive = false;
	bool m_is3Dready = false;
	bool is_update_needed_ = false;

	unsigned int m_vboAirway[4];
	std::vector<glm::vec3> m_vVertices;
	std::vector<glm::vec3> m_vVertexNormals;
	std::vector<glm::vec3> m_vVertexColors;
	std::vector<unsigned int> m_vIndices;

	glm::vec3 m_vColorMin;
	unsigned int m_nColorMin;
	glm::vec3 m_vColorMax;
	unsigned int m_nColorMax;

	std::vector<QGraphicsTextItem *> m_lpTextColorBar;
	QGraphicsRectItem *m_pAirwayColorBar = nullptr;

	float m_backScalarInGL;

	glm::vec4 m_origBackVector = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

private:
	CW3TextItem * m_lpTextAlign[6] = {
		nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr
	};

	Measure3DManager* measure_3d_manager_ = nullptr;

	bool show_export_3d_face_menu_ = false;

	std::unique_ptr<QMenu> menu_;
	std::unique_ptr<QAction> action_save_3d_face_to_ply_;
	std::unique_ptr<QAction> action_save_3d_face_to_obj_;
};
