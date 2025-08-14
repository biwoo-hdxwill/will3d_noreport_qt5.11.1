#pragma once
/*=========================================================================

File:			class CW3VolumeRenderParam
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2016-04-22
Last date:		2016-04-22

=========================================================================*/
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#include <GL/glew.h>
#endif

#include "vrengine_global.h"

class QOpenGLWidget;
class CW3Image3D;

class VRENGINE_EXPORT CW3VolumeRenderParam {
public:
	CW3VolumeRenderParam(CW3Image3D *vol, QOpenGLWidget *GLWidget);
	~CW3VolumeRenderParam();

	void updateStepSize();

	void setRayCastingParams(unsigned int prog);
	void setRayFirstHitFrontFaceParams(unsigned int prog);
	void setRayFirstHitParams(unsigned int prog);
	void setSliceParams(unsigned int prog);
	void setSliceCanalParams(unsigned int prog);
	void setEndoPlaneParams(unsigned int prog);

	void setLowRes(bool isEnable) noexcept { m_isLowRes = isEnable; }
	void set_down_factor(const float& down_factor) { down_factor_ = down_factor; }
	void setThresholds(int thdAirTissue, int thdTissueBone, int thdBoneTeeth) noexcept {
		m_thdAirTissue = thdAirTissue; m_thdTissueBone = thdTissueBone; m_thdBoneTeeth = thdBoneTeeth;
	}

	inline const int getAirTissueThreshold() const noexcept { return m_thdAirTissue; }
	inline const int getTissueBoneThreshold() const noexcept { return m_thdTissueBone; }
	inline const int getBoneTeethThreshold() const noexcept { return m_thdBoneTeeth; }

	void ApplyPreferences();

private:
	void setRenderParams();


public:
	unsigned int m_Nindices = 0;
	unsigned int m_texHandlerVol = 0;
	glm::mat4* m_matFirstHitToDepth = nullptr;
	glm::mat4* m_mvp = nullptr;
	glm::mat4* m_invModel = nullptr;
	glm::vec3 m_invVolTexScale;

	CW3Image3D* m_pgVol;
	float m_MaxValueForMIP = 0.0f;

	glm::vec3 m_volTexelSize;
	glm::vec3 m_voltexScale;
	float m_stepSize = 0.0f;
	int m_MinValue = 0.0f;

	int m_MaxIntensity = 0.0f;
	float m_AttenDistance = 0.0f;
	glm::i32vec3 m_size_vol;

	glm::vec3 m_VolScale;
	glm::vec3 m_VolScaleIso;

	float m_basePixelSizeMM = 0.0f;

	bool m_isLowRes = false;
	bool m_isShown = false;
	float down_factor_ = 0.0f;

	// thyoo 160810. front&back face를 랜더링 할 때
	// vertex(-1 ~ 1)좌표계를 texture(0 ~ 1)좌표계로 변환하는 메트릭스.
	glm::mat4 m_volTexBias;

private:
	QOpenGLWidget* m_pgGLWidget = nullptr;
	int m_thdAirTissue = 0;
	int m_thdTissueBone = 0;
	int m_thdBoneTeeth = 0;

	float high_res_step_size_factor_ = 6.0f;
	float low_res_step_size_factor_ = 8.0f;
};
