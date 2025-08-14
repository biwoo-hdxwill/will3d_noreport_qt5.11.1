#pragma once
/*=========================================================================

File:			class CW3Render3DParam
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2016-04-20
Last date:		2016-05-24

=========================================================================*/
#include <vector>
#include <GL/glew.h>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif

#include "../../Common/Common/define_view.h"
#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/W3Enum.h"
#include "W3VRengineTextures.h"

#include "vrengine_global.h"

class CW3SurfacePlaneItem;
class CW3GLObject;
class CW3GLNerve;
class CW3VolumeRenderParam;
class QOpenGLWidget;

namespace NW3Render3DParam {
typedef struct _LightInfo {
	glm::vec4 Position;
	glm::vec3 Intensity;
} LightInfo;

// 0 For firstVolume draw
// 1 For firstVolume front
// 2 For firstVolume back
// 3 For Mesh ID
// 4 For secondVolume draw
// 5 For secondVolume front
// 6 For secondVolume back
constexpr unsigned int kNumTexHandle = 7;
constexpr unsigned int kImplantIDPreview = 0;
}

class VRENGINE_EXPORT CW3Render3DParam {
public:
	explicit CW3Render3DParam(QOpenGLWidget *curGL);
	CW3Render3DParam(const common::ViewTypeID & view_type, QOpenGLWidget * curGL,
					 bool * collision_list, bool * implant_there);
	CW3Render3DParam(QOpenGLWidget *curGL,
					 bool* collision_list, bool* implant_there);

	~CW3Render3DParam();

	void SetCurGLWidget(QOpenGLWidget* curGL);
	void ClearGLObjects();

	GLuint defaultFBO();
	void makeCurrent();
	void doneCurrent();

	const float windowing_min() const noexcept;
	const float windowing_norm() const noexcept;
	void set_windowing_min(float min) noexcept;
	void set_windowing_norm(float norm) noexcept;

public:
	unsigned int	m_mainVolume_vao[2] = { 0, 0 };
	CW3VRengineTextures		m_VRtextures;
	CW3VolumeRenderParam*	m_pgMainVolume[2] = { nullptr, nullptr };
	CW3GLNerve*				m_pNerve = nullptr;
	CW3GLObject*			m_pImplant[MAX_IMPLANT];
	CW3GLObject*			m_pAirway = nullptr;
	CW3GLObject*			m_plane = nullptr;
	CW3GLObject*			m_photo3D = nullptr;
	CW3SurfacePlaneItem*	m_pMPROverlay = nullptr;

	bool		*g_is_implant_exist_ = nullptr;
	bool		*g_is_implant_collided_ = nullptr;

	GLuint m_fbo = 0;
	GLuint m_depthMap = 0;

	/**
		@brief	The tex handler[ 7]
		// 0 For firstVolume draw
		// 1 For firstVolume front
		// 2 For firstVolume back
		// 3 For Mesh ID
		// 4 For secondVolume draw
		// 5 For secondVolume front
		// 6 For secondVolume back
	*/
	GLuint m_texHandler[NW3Render3DParam::kNumTexHandle] = { 0,0,0,0,0,0,0 };

	int	m_width = 0;
	int	m_height = 0;
	int	m_widthPre = 0;
	int	m_heightPre = 0;

	bool	m_isLowRes = false;

	bool	m_isNearClipping = false;
	bool	m_isDerivedVolume = false;
	bool	m_isShading = true;
	bool	m_isMIP = false;
	bool	m_isImplantShown = false;
	bool	m_isLowResDrawn = false;
	bool	m_isDepthFirstHit = false;
	bool	m_isPerspective = false;
	bool	m_isClipped = false;
	bool flip_clipping_ = false;

	std::vector<glm::vec4> m_clipPlanes;

	GLuint m_fboCollide = 0;
	GLuint m_depthmapCollide = 0;

	NW3Render3DParam::LightInfo m_lightInfo;

private:
	void clearFBOs();
	void clearVAOs();

private:
	QOpenGLWidget * m_pgCurGLWidget;
};
