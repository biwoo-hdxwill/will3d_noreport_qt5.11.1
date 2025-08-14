#pragma once
/*=========================================================================

File:			class CW3VREngine
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2015-12-01
Last date:		2016-04-21

=========================================================================*/

#include <QMutex>
#include <qwidget.h>
#include <qvector3d.h>

#include "W3VolumeRenderParam.h"

#include "vrengine_global.h"

class CW3Image3D;
class CW3Image2D;
class CW3TF;
class CW3OTFScene;
class CW3OTFView;
class CW3ViewPlane;
class CW3MPREngine;
class CW3Render3DParam;
class QOpenGLWidget;
class CWTF;
class VolumeRenderer;


/// 새코드
#include <memory>
class SliceRenderer;
class ImageRenderer;
class PlaneResource;


class VRENGINE_EXPORT CW3VREngine : public QWidget
{
	Q_OBJECT

public:
	static CW3VREngine* NewInstance(QOpenGLWidget* gl_widget)
	{
		static QMutex mutex;
		mutex.lock();
		if (!instance_)
		{
			instance_ = new CW3VREngine(gl_widget);
			atexit(Destroy);
		}
		mutex.unlock();
		return instance_;
	}

	static CW3VREngine* GetInstance()
	{
		return instance_;
	}

	static void Destroy()
	{
		static QMutex mutex;
		mutex.lock();
		if (instance_)
		{
			delete instance_;
			instance_ = nullptr;
		}
		mutex.unlock();
	}

	~CW3VREngine();

	void makeCurrent();
	void doneCurrent();

	void SetVRreconType(const QString& otf_name);
	/////////////////////////// For VR Quality Test
	//unsigned int buildandcompile();
	void setRCuniforms(unsigned int prog);
	//void setVolTFTextureUniforms(unsigned int prog);
	int m_MinValuetest;
	float m_AttenDistancetest;
	glm::vec3 m_texelSizetest;
	float	m_maxValuetest;
	float m_TFxincreasetest;
	float m_TFyincreasetest;
	/////////////////////////// For VR Quality Test

	/////////////////////////// For CPU VR
	//QImage* RenderCPU();
	//void setModelCPU(glm::mat4 &model);

	/////////////////////////// For GCPU VR
	//void RenderGCPU(glm::vec4 *&Result, glm::vec4 *Front, glm::vec4 *Back, int w, int h, bool isChanging);

	/////////////////////////// For Collision Detection using GPU.
	//void checkCollide(CW3Render3DParam* param, const glm::mat4& projviewMat);

	/////////////////////////////////////////////////////////////////
	///////////////////Rendering Functions///////////////////////////
	/////////////////////////////////////////////////////////////////
	void Render3Dboth(CW3Render3DParam *param, int VolId, bool &isReconSwitched, bool isSecondVolume = false);
	void Render3Dfinal(CW3Render3DParam *param, bool clear = true, bool isSecondVolume = false);
	void RenderSlice(CW3Render3DParam *param, int VolId, const bool &isXray);
	void Render3DEndo(CW3Render3DParam *param, int VolId, bool &isReconSwitched);
	void RenderSliceEndo(CW3Render3DParam *param, int VolId, bool &isReconSwitched);
	void RenderSurface(CW3Render3DParam *param, int VolId, bool isTexture, bool &isReconSwitched);
	void RenderMPR(CW3Render3DParam *param, int VolId, bool isDrawBoth, bool &isReconSwitched);
	void RenderImplantMPR(CW3Render3DParam *param, int VolId, bool &isReconSwitched);
	void RenderImplantMPRonlySelected(CW3Render3DParam *param, int VolId, int implantID);
	void RenderImplant3D(CW3Render3DParam *param, int implantID);
	void RenderMeshMPR(CW3Render3DParam *param, int VolId, bool &isReconSwitched);
	void Render3DTeeth(CW3Render3DParam *param);

	void Render3DOTFPreset(CW3Render3DParam *param, QString filePath);

	void RenderVolumeFirstHit(CW3Render3DParam *param, GLuint *frameBuffer);

	void init();

	inline const bool IsMIP() const noexcept { return m_bIsMIP; }
	inline const bool IsXRAY() const noexcept { return m_bIsXRAY; }
	inline void setMIP(const bool mip) { m_bIsMIP = mip; }
	inline void setXRAY(const bool xray) { m_bIsXRAY = xray; }

	inline void setGLWidget(QOpenGLWidget *GLWidget) { m_pgGLWidget = GLWidget; }
	inline QOpenGLWidget* getGLWidget() { return m_pgGLWidget; }
 
	inline CW3VolumeRenderParam* getVRparams(int id) { return m_pMainVRparams[id]; }

	int GetActiveIndices(int vol_id) const;
	inline void getVolTFTexParam(GLenum &vol, int &vol_, GLenum &tf, int &tf_)
	{
		vol = m_texNumVol3D;
		vol_ = m_texNumVol3D_;
		tf = m_texNumTF2D;
		tf_ = m_texNumTF2D_;
	}

	inline void getRCTexParam(GLenum &rc, int &rc_)
	{
		rc = m_texNumRC;
		rc_ = m_texNumRC_;
	}

	inline glm::mat4* getReorientedModel() { return &m_modelReori; }

	inline const bool isVRready() const { return m_isTFinitialized; }

	inline unsigned int* getVolVBO() { return m_vboCUBE; }

	inline unsigned int getPROGRayCasting()		{ return m_PROGraycasting; }
	inline unsigned int getPROGfinal()			{ return m_PROGfinal; }
	inline unsigned int getPROGslice()			{ return m_PROGslice; }
	inline unsigned int getPROGsliceCanal()		{ return m_PROGsliceCanal; }
	inline unsigned int getPROGSR()				{ return m_PROGSR; }
	inline unsigned int getPROGSRcoord()			{ return m_PROGSRcoord; }
	inline unsigned int getPROGanno()				{ return m_PROGanno; }
	inline unsigned int getPROGpick()				{ return m_PROGpick; }
	inline unsigned int getPROGpickWithCoord()	{ return m_PROGpickWithCoord; }
	inline unsigned int getPROGendoPlane()		{ return m_PROGendoPlane; }
	inline unsigned int getPROGsurface()			{ return m_PROGsurface; }
	inline unsigned int getPROGimplant()			{ return m_PROGimplant; }
	inline unsigned int getPROGdispSurface()		{ return m_PROGdisplacementSurface; }	

	void getCUBEvertexCoord(glm::vec4* vert);
	
	inline CW3Image3D* getVol(int id)
	{
		if (!m_pMainVRparams[id])
			return nullptr;

		return m_pMainVRparams[id]->m_pgVol;
	}

	
	void volumeDown(short **in, unsigned int downFactor, unsigned int &w, unsigned int &h, unsigned int &d);
	void setVolTextureUniform(unsigned int prog, unsigned int texHandler);
	void setTFTextureUniform();
	//void setVolTFTextureUniforms();
	
	void setVolume(CW3Image3D *volume, int id);
	void setDownFactor(VREngine::VolType id, const int & down_factor);
	void setVolume(CW3Image3D* volume, VREngine::VolType id); //TODO 인자를 const참조자로 받도록 변경.
	
	void initVAOplane(unsigned int *vao);
	void InitVAOPlaneInverseY(unsigned int *vao); //thyoo 161027
	void getFrontMiddleSliceCoord(float *vert, unsigned int *index);

	void setActiveIndex(unsigned int *vao, int id);

	void tmpSetVolTexHandler(unsigned int handler, int id);
	void tmpSetTfTexHandler(unsigned int handler);
	
	void initFrameBufferSimpleSR(unsigned int &FBhandler, unsigned int &DepthHandler, unsigned int *texHandler, unsigned int nx, unsigned int ny, int texHandlerCount);
	void initFrameBufferSlice(unsigned int &FBhandler, unsigned int *texHandler, unsigned int nx, unsigned int ny, int texHandlerCount);
	void initFrameBufferFirstHit(unsigned int &FBhandler, unsigned int &DepthHandler, unsigned int *texHandler, unsigned int nx, unsigned int ny);

	inline void setMPRengine(CW3MPREngine *pMPR) { m_pgMPRengine = pMPR; }

	glm::vec3* getVolRange(int id);

	void setProjectionEvn();

	void getMVP(glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection, glm::mat4 &mvp)
	{
		model = m_model;
		view = m_view;
		projection = m_projection;
		mvp = m_mvp;
	}

	inline QVector3D* getVolRangeGL() { return &m_VolRangeGL; }

	inline GLenum* getSimpleSRbuffers() { return m_SimpleSR_draw_buffers; }

	void setTFtoVolume(int volid);

	void initTF(int tf_size);

	void applyReorientation(glm::mat4 &mat);

	void readyForSetVolume(CW3Image3D* volume, int id);


	inline const bool isReadyTeethTF() const { return m_texTF2DTeethHandler; }

	//thyoo
	void recompileRaycasting(void);
	void PostProcessingXray(CW3Render3DParam* param, int view_width, int view_height);
	void SetVolShade(bool is_shade);

	inline unsigned int getThyooPROGRayCasting() { return m_thyShd.m_PROGraycasting; }
	inline unsigned int getThyooPROGbackface() { return m_thyShd.m_PROGbackfaceCUBE; }
	inline unsigned int getThyooPROGfrontface() { return m_thyShd.m_PROGfrontfaceCUBE; }
	inline unsigned int getThyooPROGforntfaceFinal() { return m_thyShd.m_PROGfrontfaceFinal; }
	inline unsigned int getPROGsurfaceTexture() { return m_PROGsurfaceTexture; }

	void getVolumeIntensity(const unsigned int idVol, const glm::vec3& position, unsigned short& intensity);

	inline unsigned int getVolTFHandle() { return	m_texTF2DHandler; }
	inline int getMaxTexAxisSize() { return m_nMaxTexAxisSize; }

	inline void getVolTFTexSize(int* width, int* height)
	{
		*width = m_nTFtexwidth;
		*height = m_nTFtexheight;
	}

	//thyoo end

	// by jdk 161122 tmj
	inline unsigned int getTexSegTmjMask() { return m_texSegTmjMask; }
	inline GLenum getTexNumSegTmjMask() { return m_texNumSegTmjMask; }
	inline int getTexNumSegTmjMask_() { return m_texNumSegTmjMask_; }
	void setSegTmjTexture(unsigned char **data, const int &width, const int &height, const int &depth);
	void deleteVRparams(int id);
	void deleteVBOs();
	void deleteTextures();
	void reset();

	bool VolumeTracking(CW3Render3DParam* param, const QPointF& mouse_pos, glm::vec3& picked_volume_pos);

	inline unsigned int* vbo_plane_inverse_y() { return vbo_plane_inverse_y_; }

signals:
	void sigTFupdated(bool);
	void sigTFupdateCompleted();
	void sigReoriupdate(glm::mat4 *m);
	void sigImgWH(int, int);
	void sigNerveSet(unsigned int*, unsigned int);
	void sigMIPforAutoArch(float*, int, int);
	void sigChangeMIP(bool isEnable);
	void sigShadeOn(bool is_shade);

public slots:
	void slotUpdateTF(int tf_min, int tf_max, bool isMinMaxChanged);
	void slotReoriented(std::vector<float> param);

protected:
	CW3VREngine(QOpenGLWidget *GLwidget);

private:
	void initGLSL();
	void initGLContextValues();
	void initPlaneVertex();
	void initCUBEVertex();
	void initPrograms();
	void InitVBOPlaneInverseY();
	void initVBOplane();
	void initLookupTextures();

	void setUniformImplant(CW3Render3DParam* param, const bool wire = false);
	void setUniformSingleImplant(CW3Render3DParam* param, int implantID, const bool wire = false);

private:
	static CW3VREngine* instance_;

	CW3MPREngine*	m_pgMPRengine = nullptr;
	QOpenGLWidget*	m_pgGLWidget = nullptr;

	CW3VolumeRenderParam *m_pMainVRparams[5];

	float m_vertPlaneCoord[12];
	float m_texPlaneCoord[8];
	unsigned int	m_IndexPlane[6];

	float m_tex_inverseY_PlaneCoord[8];

	float m_vertCUBECoord[24];
	float m_texCUBECoord[24];
	unsigned int  m_IndexCUBE[36];

	int	m_nTotalGPUMemKb = 0;
	int	m_nCurGPUMemKb = 0;
	int	m_nMaxTexAxisSize = 0;
	int	m_nMax3DTexSize = 0;
	int	m_nMaxNTexture = 0;

	int	m_nTFtexwidth = 0;
	int	m_nTFtexheight = 0;

	unsigned int	m_texTF2DHandler = 0;
	unsigned int  m_texTF2DTeethHandler = 0;
	
	unsigned int	m_texBsplineHandler = 0;
	unsigned int	m_texBsplinePrimeHandler = 0;

	GLenum	m_texNumVol3D;
	int	m_texNumVol3D_;
	GLenum	m_texNumTF2D;
	int	m_texNumTF2D_;

	GLenum	m_texNumTF2DTeeth;
	int	m_texNumTF2DTeeth_;

	GLenum	m_texNumPlane;
	int	m_texNumPlane_;

	GLenum	m_texNumMaskPlane;
	int	m_texNumMaskPlane_;

	GLenum	m_texNumFACE;
	int	m_texNumFACE_;

	GLenum	m_texNumBspline;
	int	m_texNumBspline_;
	GLenum	m_texNumBsplinePrime;
	int	m_texNumBsplinePrime_;

	unsigned int	m_PROGplaneDisplay = 0;
	unsigned int	m_PROGbackfaceCUBE = 0;
	unsigned int	m_PROGfrontfaceCUBE = 0;
	unsigned int	m_PROGraycasting = 0;
	unsigned int	m_PROGendoraycasting = 0;
	unsigned int	m_PROGfrontfaceFinal = 0;
	unsigned int	m_PROGvolumeFirstHitforFaceSim = 0;
	unsigned int	m_PROGfinal = 0;
	unsigned int	m_PROGslice = 0;
	unsigned int	m_PROGsliceCanal = 0;
	unsigned int	m_PROGSR = 0;
	unsigned int	m_PROGSRdepth = 0;
	unsigned int	m_PROGSRcoord = 0;
	unsigned int	m_PROGsimpleSR = 0;
	unsigned int	m_PROGanno = 0;
	unsigned int	m_PROGimplant = 0;
	unsigned int	m_PROGsurface = 0;		//3D axis surface의 프로그램
	unsigned int	m_PROGpick = 0;			// TODO : remove (PickWithCoord 에서 이 기능까지 포함)
	unsigned int	m_PROGpickWithCoord = 0;
	unsigned int	m_PROGendoPlane = 0;
	unsigned int	m_PROGdepthSetting = 0;
	unsigned int	m_PROGdepthSetting2 = 0;
	unsigned int	m_PROGsurfaceTexture = 0;
	unsigned int	m_PROGsurfaceEndo = 0;
	unsigned int	m_PROGdisplacementSurface = 0; // DisplacementSurface 에서 프로그램 이동해옴.
	unsigned int	prog_image_texture_ = 0;
	unsigned int	prog_mask_texture_ = 0;
	

	struct thyooRayShader
	{
		unsigned int	m_PROGbackfaceCUBE = 0;
		unsigned int	m_PROGfrontfaceCUBE = 0;
		unsigned int	m_PROGfrontfaceFinal = 0;
		unsigned int	m_PROGraycasting = 0;
		unsigned int	m_PROGslice = 0;
		unsigned int	m_PROGfirsthit = 0;

	};
	thyooRayShader m_thyShd;

	unsigned int  m_vboPlane[3];
	unsigned int  vbo_plane_inverse_y_[3];
	unsigned int  m_vboCUBE[3];
	unsigned int  m_vboCUBEdivided[3];

	GLenum	m_SimpleSR_draw_buffers[7];
	GLenum m_texNumSimpleSR[7];
	int	m_texNumSimpleSR_[7];
	GLenum m_texNumRC;
	int	m_texNumRC_;

	float		m_camFOV;
	glm::mat4	m_model, m_view, m_projection, m_mvp;
	glm::mat4	m_modelReori;
	QVector3D m_VolRangeGL;

	bool m_bIsMIP = false;
	bool m_bIsXRAY = false;
	bool	m_isTFinitialized = false;

	// by jdk 161122 tmj
	unsigned int m_texSegTmjMask;
	GLenum m_texNumSegTmjMask;
	int m_texNumSegTmjMask_;


	unsigned int tf_min_, tf_max_;
	bool vol_shade_ = true;



/* 
///새코드
 */
public:
	void ApplyPreferences();

	VolumeRenderer* GetVolumeRenderer(int id) const;

	float low_res_frame_buffer_resize_factor_ = 0.5f;
};
