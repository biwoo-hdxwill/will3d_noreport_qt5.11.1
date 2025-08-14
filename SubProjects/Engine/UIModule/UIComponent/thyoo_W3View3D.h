#pragma once
/*=========================================================================

File:			class CW3View3D
Language:		C++11
Library:        Qt 5.4.0
Author:			Hong Jung
First date:		2015-12-02
Last modify:	2015-12-11
				2016-04-18(Tae Hoon Yoo)

=========================================================================*/
#include "uicomponent_global.h"
#include "thyoo_W3View2D.h"

#define kRenderActiveCube 1

class CW3TextItem_switch;
class Measure3DManager;
#ifndef WILL3D_VIEWER
class ProjectIOView;
#endif

class UICOMPONENT_EXPORT CW3View3D_thyoo : public CW3View2D_thyoo {
	Q_OBJECT

public:
	CW3View3D_thyoo(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
			  common::ViewTypeID eType, bool isMPRused, QWidget *pParent = 0);
	~CW3View3D_thyoo();

#ifndef WILL3D_VIEWER
	void ExportProjectForMeasure3D(ProjectIOView& out);
	void ImportProjectForMeasure3D(ProjectIOView& in);
#endif

	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;

public:
	void initClipValues(const MPRClipID& clip_plane, const bool& is_clipping,
						const int& lower, const int& upper);
	
	void setMIP(bool isMIP);

	virtual void reset() override;

	void ApplyPreferences();

public slots:
	void slotTFupdated(bool);
	void slotRenderCompleted();

	virtual void slotReoriupdate(glm::mat4 *m);
	virtual void HideUI(bool bToggled) override;

	void slotShadeOnFromOTF(bool);

signals:
	void sigRenderCompleted();
	void sigRotateMat(const glm::mat4& mat);

protected:
	enum GL_TEXTURE_HANDLE {
		TEX_ENTRY_POSITION = 0,
		TEX_EXIT_POSITION,
		TEX_RAYCASTING,
		TEX_FINAL,
		TEX_END
	};

	enum GL_DEPTH_HANDLE {
		DEPTH_DEFAULT = 0,
		DEPTH_RAYCASTING,
		DEPTH_END
	};

	virtual void ResetView() override;
	virtual void FitView() override;
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void drawBackground(QPainter *painter, const QRectF &rect);
	virtual void wheelEvent(QWheelEvent *event);
	virtual void resizeEvent(QResizeEvent *pEvent) override;
	virtual void resizeScene();
	virtual void keyPressEvent(QKeyEvent* event) override;

	virtual void setProjection();
	virtual void drawBackFaceSurface() {}
	virtual void drawSurface() {}
	virtual void drawTransparencySurface() {}
	virtual void drawOverrideSurface() {}
	virtual void setMVP(float rotAngle, glm::vec3 rotAxis);
	virtual void setReorientation(const glm::mat4& reorienMat);
	virtual void clearGL();

	virtual void mousePanningEvent();
	virtual void mouseScaleEvent();

	void initializeGL(void);
	void renderingGL(void);
	void basicRayCasting(void);
	void blendingGL();
	void drawFinal();

	void setViewMatrix();

	void ArcBallRotate();
	glm::vec3 ArcBallVector(QPointF &v);

	bool volumeTracking(const QPointF& mousePos, glm::vec3& ptOutVolume);
	bool printGLError(unsigned int line);

	void setRotateMatrix(const glm::mat4& mat);

	virtual void HideMeasure(bool toggled) override;
	virtual void DeleteAllMeasure() override;
	virtual void DeleteUnfinishedMeasure() override;

protected slots:
	void slotVRAlignS();
	void slotVRAlignI();
	void slotVRAlignL();
	void slotVRAlignR();
	void slotVRAlignA();
	void slotVRAlignP();

protected:
	float m_camFOV = 1.0f;

	unsigned int m_PROGfrontfaceCUBE = 0;
	unsigned int m_PROGfrontfaceFinal = 0;
	unsigned int m_PROGbackfaceCUBE = 0;
	unsigned int m_PROGraycasting = 0;
	unsigned int m_PROGfinal = 0;
	unsigned int m_PROGsurface = 0;
	unsigned int m_PROGpick = 0;

	unsigned int m_vaoCUBE = 0;
	unsigned int m_vaoPlane = 0;

	unsigned int m_FBHandler = 0;
	std::vector<unsigned int> m_depthHandler;

	std::vector<GLenum> m_texBuffer;
	std::vector<unsigned int> m_texHandler;
	std::vector<GLenum> m_texNum;
	std::vector<int> m_texNum_;

	int	m_width3Dview = 0;
	int	m_height3Dview = 0;

	glm::vec3 m_rotAxis = glm::vec3(1.0f);
	float m_rotAngle = 0.0f;

	glm::vec3	m_vVolRange;

	glm::mat4 m_mvpForFinal;

	float m_stepSize;
	////////////////////////////////////
	///////////// Flag For State
	bool	m_is3Dready = false;
	bool	m_isChanging = false;

	//thyoo
	CW3TextItem_switch* m_pShadeSwitch;
	glm::mat4 m_arcMat = glm::mat4(1.0f);
	glm::mat4 m_reorienMat = glm::mat4(1.0f);

	//thyoo. MPR clipping
	struct ClippingParams {
		bool isEnable = false;
		std::vector<glm::vec4> planes;
	};
	ClippingParams m_clipParams;

	CW3TextItem *m_lpTextAlign[6];

	bool m_isMIP = false;

	Measure3DManager* measure_3d_manager_ = nullptr;

	float low_res_frame_buffer_resize_factor_ = 1.0f;
	float low_res_step_size_factor_ = 1.0f;

private:
	void setVRAlign();
};
