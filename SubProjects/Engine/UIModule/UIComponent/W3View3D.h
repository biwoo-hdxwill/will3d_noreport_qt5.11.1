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

#include "W3View2D.h"

#include "uicomponent_global.h"

class CW3FilteredTextItem;
class Measure3DManager;
#ifndef WILL3D_VIEWER
class ProjectIOView;
#endif

class UICOMPONENT_EXPORT CW3View3D : public CW3View2D {
	Q_OBJECT
public:
	CW3View3D(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
			  CW3ResourceContainer *Rcontainer, common::ViewTypeID eType,
			  QWidget *pParent = 0);
	~CW3View3D(void);

	// serialization
#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOView& out);
	void importProject(ProjectIOView& in);
#endif

	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;

	void setModelPhotoToMC();
	inline glm::mat4 getModelMatrix() { return m_model * glm::scale(1.0f / m_vVolRange); }
	virtual void setVisible(bool);

	virtual void reset();

	void setMIP(bool isMIP);

	inline QPointF mapVolToScene(const glm::vec3& ptVol);
	inline glm::vec3 mapSceneToVol(const QPointF& ptScene, bool isApplyBackCoord = true);

signals:
	void sigRotate(glm::mat4 *m);
	void sigScale(float);
	void sigStopModel();
	void sigInitModels(glm::mat4*, glm::mat4*, glm::mat4*, glm::mat4*, float, float);

public slots:
	void slotTFupdated(bool isMinMaxChanged);
	virtual void slotUpdateRotate(glm::mat4 *model);
	virtual void slotUpdateScale(float scale);
	void slotTFupdateCompleted();
	void slotTransformedPhotoPoints(glm::mat4 *model);
	virtual void slotReoriupdate(glm::mat4 *m);

	void slotDrawAirway(int state);
	void slotSegAirway(std::vector<tri_STL>& mesh);

protected:
	virtual void ResetView() override;
	virtual void FitView() override;
	virtual void InvertView(bool bToggled) override;
	virtual void HideUI(bool bToggled) override;
	virtual void HideMeasure(bool toggled) override;
	virtual void DeleteAllMeasure() override;

	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void wheelEvent(QWheelEvent *event);
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
	virtual void enterEvent(QEvent* event) override;

	virtual bool mousePressEventW3(QMouseEvent *event);
	virtual bool mouseMoveEventW3(QMouseEvent *event);
	virtual bool mouseReleaseEventW3(QMouseEvent *event);
	void RotateWithArcBall(const QPoint & curr_view_pos);

	virtual void drawBackground(QPainter *painter, const QRectF &rect);
	virtual void render3D();

	virtual void resizeEvent(QResizeEvent *pEvent) override;
	virtual void resizeScene();

	virtual void setInitScale();
	virtual void setProjection();
	virtual void setViewMatrix();

	virtual void setMVP();
	virtual void setModel();
	virtual void setModel(float rotAngle, glm::vec3 rotAxis);
	virtual void setDirectionFromCompass(glm::mat4 &T);

	void ArcBallRotate(const QPoint& curr_view_pos);
	glm::vec3 ArcBallVector(QPointF &v);

	virtual void init();

	void setAirwayVBO();
	void setAirwayVAO();

	void drawAirwayColorBar();

	bool isReadyRender3D() const;
	bool isRender3D() const;

	glm::vec3 getBackCoordInVol();

	void setPosReconTypeTextItem();
	void CreateReconText();

	virtual void clearGL() override;

	void DrawMeasure3D();
	void DeleteUnfinishedMeasure();

protected slots:
	void slotVRAlignS();
	void slotVRAlignI();
	void slotVRAlignL();
	void slotVRAlignR();
	void slotVRAlignA();
	void slotVRAlignP();

	void slotShadeOnFromOTF(bool isShading);
	void slotChangeMIP(bool isEnable);

	virtual void slotChangedValueSlider(int);

private:
	void connections();
	void setVRAlign();
	void RenderAndUpdate();

protected:
	CW3FilteredTextItem * recon_type_selection_ui_ = nullptr;

	float m_camFOV;
	float m_rotAngle;

	glm::vec3 m_rotAxis;
	glm::vec3 m_vVolRange;
	glm::vec3 m_VolCenter;	// Volume 의 center: (N-1)*0.5f

	glm::mat4 m_mv;
	glm::mat4 m_viewForFinal;
	glm::mat4 m_projForFinal;
	glm::mat4 m_matTextureToGL;
	glm::mat4 m_ToDepth;
	glm::mat4 m_matTexCoordToDepth;
	glm::mat4 m_matTexCoordToDepthSecond;

	glm::mat4 m_secondToFirstModel;	// second volume 을 main volume 에 일치시키기 위한 변환 행렬
	glm::mat4 m_rotateSecond;
	glm::mat4 m_scaleMatSecond;		// second volume 에 대한 scale 행렬
	glm::mat4 m_translateSecond;

	///////////// Flag For State
	bool	m_isPassive = false;
	bool	m_is3Dready = false;
	bool	m_isMinMaxChanged = false;

	unsigned int m_vboDrawCube[1];
	unsigned int m_vaoDrawCube;

	unsigned int m_vboAirway[4] = { 0, 0, 0, 0};
	unsigned int m_nCntAirwayVertex = 0;

	std::vector<glm::vec3> m_vVertices; //Vertex list
	std::vector<glm::vec3> m_vVertexNormals;
	std::vector<glm::vec3> m_vVertexColors;
	std::vector<unsigned int> m_vIndices;// index list

	glm::vec3 m_vColorMin;
	glm::vec3 m_vColorMax;
	unsigned int m_nColorMin = 0;
	unsigned int m_nColorMax = 0;

	int m_nNumColor = 0;
	std::vector<QGraphicsTextItem *> m_lpTextColorBar;
	QGraphicsRectItem *m_pAirwayColorBar = nullptr;

	float m_backScalarInGL = 0;

	glm::vec4 m_origBackVector;

	Measure3DManager* measure_3d_manager_ = nullptr;

private:
	CW3TextItem * m_lpTextAlign[6] = {
		nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr
	};
};
