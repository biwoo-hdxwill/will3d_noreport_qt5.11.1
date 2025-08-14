#pragma once

/**=================================================================================================

Project: 			GLfunctions
File:				gl_helper.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-04
Last modify:		2017-08-04

 *===============================================================================================**/
#if defined(__APPLE__)
#include <glm/vec3.hpp>
#else
#include <GL/glm/vec3.hpp>
#endif
#include <QPointF>

#include "glfunctions_global.h"

namespace GLfunctions {
	static const float kSceneToGLnormalize = 2.f;
	static const float kVolToGLnormalize = 2.f;
}

class GLFUNCTIONS_EXPORT GLhelper
{
public:
	//scaledSceneToGL: Scene좌표[-0.5, 0.5]을 GL좌표[-1, 1]로 scale.
	inline static float ScaleSceneToGL(const float& arg) { return arg * GLfunctions::kSceneToGLnormalize; }
	inline static QPointF ScaleSceneToGL(const QPointF& point) { return QPointF(ScaleSceneToGL(point.x()), ScaleSceneToGL(point.y())); }
	inline static glm::vec3 ScaleSceneToGL(const glm::vec3& vec) { return glm::vec3(ScaleSceneToGL(vec.x), ScaleSceneToGL(vec.y), ScaleSceneToGL(vec.z)); }

	//scaledGLToScene: GL좌표[-1, 1]을 Scene좌표[-0.5, 0.5]로 scale.
	inline static float ScaleGLToScene(const float& arg) { return arg / GLfunctions::kSceneToGLnormalize; }
	inline static QPointF ScaleGLToScene(const QPointF& point) { return QPointF(ScaleGLToScene(point.x()), ScaleGLToScene(point.y())); }
	inline static glm::vec3 ScaleGLToScene(const glm::vec3& vec) { return glm::vec3(ScaleGLToScene(vec.x), ScaleGLToScene(vec.y), ScaleGLToScene(vec.z)); }


	//scaledGLToVol: GL좌표[-1, 1]을 Vol좌표[0.5, 0.5]로 scale.
	inline static float ScaleGLtoVol(const float& arg) { return arg / GLfunctions::kVolToGLnormalize; }
	inline static QPointF ScaleGLtoVol(const QPointF& point) { return QPointF(ScaleGLtoVol(point.x()), ScaleGLtoVol(point.y())); }
	inline static glm::vec3 ScaleGLtoVol(const glm::vec3& vec) { return glm::vec3(ScaleGLtoVol(vec.x), ScaleGLtoVol(vec.y), ScaleGLtoVol(vec.z)); }

	//scaledVolToGL: Vol좌표[0.5, 0.5]을 GL좌표[-1, 1]로 scale.
	inline static float ScaleVolToGL(const float& arg) { return arg * GLfunctions::kVolToGLnormalize; }
	inline static QPointF ScaleVolToGL(const QPointF& point) { return QPointF(ScaleVolToGL(point.x()), ScaleVolToGL(point.y())); }
	inline static glm::vec3 ScaleVolToGL(const glm::vec3& vec) { return glm::vec3(ScaleVolToGL(vec.x), ScaleVolToGL(vec.y), ScaleVolToGL(vec.z)); }


	/////////////////////////////////////////////////////////////////////////////////
	///주의사항. mapping 함수는 GL world 좌표계가 [-volRange, volRange]^3 * scale(1.0f, 1.0f, fSpaicngZ) 인 것을 가정한다.
	/////////////////////////////////////////////////////////////////////////////////

	//mapVolToGL: Vol좌표[0, volRange]를 World GL좌표[-volRange, volRange]로 mapping.
	inline static glm::vec3 MapVolToWorldGL(const glm::vec3& pt_vol, const glm::vec3& pt_vol_center, float spacing_z) { return ScaleVolToGL(pt_vol - pt_vol_center)*glm::vec3(1.0f, 1.0f, spacing_z); }

	//mapVolToGL: World GL좌표[-volRange, volRange]를 Vol좌표[0, volRange]로 mapping.
	inline static glm::vec3 MapWorldGLtoVol(const glm::vec3& pt_gl, const glm::vec3& pt_vol_center, float spacing_z) { return ScaleGLtoVol(pt_gl / glm::vec3(1.0f, 1.0f, spacing_z)) + pt_vol_center; }

	////mapVolToGL: Vol좌표[0, volRange]를 Normalized GL좌표[-1.0, 1.0]로 mapping.
	inline static glm::vec3 MapVolToNormGL(const glm::vec3& pt_vol, const glm::vec3& pt_vol_center, const glm::vec3& vol_range, float spacing_z) { return ((MapVolToWorldGL(pt_vol, pt_vol_center, spacing_z)) / vol_range); }
	
	////mapVolToGL: Normalized GL좌표[-1.0, 1.0]를 Vol좌표[0, volRange]로 mapping.
	inline static glm::vec3 MapNormGLtoVol(const glm::vec3& pt_gl, const glm::vec3& pt_vol_center, const glm::vec3& vol_range, float spacing_z) { return MapWorldGLtoVol(pt_gl*vol_range, pt_vol_center, spacing_z); }


};
