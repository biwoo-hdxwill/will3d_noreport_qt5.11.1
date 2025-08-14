#include "W3VolumeRenderParam.h"

#include <QDebug>
#include <qopenglwidget.h>
#include <QTextCodec>

#include "../../Common/Common/global_preferences.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Resource/Resource/W3Image3D.h"

CW3VolumeRenderParam::CW3VolumeRenderParam(CW3Image3D *vol, QOpenGLWidget *GLWidget)
	: m_pgVol(vol), m_pgGLWidget(GLWidget) {

	setRenderParams();
}

CW3VolumeRenderParam::~CW3VolumeRenderParam() {
	m_Nindices = 0;
	m_texHandlerVol = 0;
	m_matFirstHitToDepth = nullptr;
	m_mvp = nullptr;
	m_invModel = nullptr;
	m_invVolTexScale = glm::vec3(1.0f);

	m_pgVol = nullptr;
	m_MaxValueForMIP = 0.0f;

	m_volTexelSize = glm::vec3(1.0f);
	m_voltexScale = glm::vec3(1.0f);
	m_stepSize = 0.0f;
	m_MinValue = 0.0f;

	m_MaxIntensity = 0.0f;
	m_AttenDistance = 0.0f;
	m_size_vol = glm::vec3(0);

	m_VolScale = glm::vec3(1.0f);
	m_VolScaleIso = glm::vec3(1.0f);

	m_basePixelSizeMM = 0.0f;

	m_isLowRes = false;
	m_isShown = false;
	down_factor_ = 0.0f;

	m_volTexBias = glm::mat4(1.0f);

	m_pgGLWidget = nullptr;
	m_thdAirTissue = 0;
	m_thdTissueBone = 0;
	m_thdBoneTeeth = 0;
}

void CW3VolumeRenderParam::setRenderParams() {
	m_MinValue = m_pgVol->getMin();
	m_MaxIntensity = m_pgVol->getMax() - m_MinValue;

	int nVolWidth = m_pgVol->width();
	int nVolHeight = m_pgVol->height();
	int nVolDepth = m_pgVol->depth();

	m_VolScale.x = nVolWidth;// -1.0f;
	m_VolScale.y = nVolHeight;// -1.0f;
	m_VolScale.z = nVolDepth;// -1.0f;

	m_volTexelSize.x = 1.0f / nVolWidth;
	m_volTexelSize.y = 1.0f / nVolHeight;
	m_volTexelSize.z = 1.0f / nVolDepth;

	float xsize = (nVolWidth)*m_pgVol->pixelSpacing();
	float ysize = (nVolHeight)*m_pgVol->pixelSpacing();
	float zsize = (nVolDepth)*m_pgVol->sliceSpacing();

	m_basePixelSizeMM = std::min(m_pgVol->pixelSpacing(), m_pgVol->sliceSpacing());

	float maxAxisSize = std::max(std::max(xsize, ysize), zsize);

	m_voltexScale = glm::vec3(xsize / maxAxisSize, ysize / maxAxisSize, zsize / maxAxisSize);

	m_stepSize = ((m_basePixelSizeMM * down_factor_) / (maxAxisSize*high_res_step_size_factor_));

	m_invVolTexScale.x = 1.0f / m_voltexScale.x;
	m_invVolTexScale.y = 1.0f / m_voltexScale.y;
	m_invVolTexScale.z = 1.0f / m_voltexScale.z;

	m_AttenDistance = 5.0f*m_basePixelSizeMM / 0.2f; // 0.2mm 당 5 의 weight

	m_size_vol.x = nVolWidth;
	m_size_vol.y = nVolHeight;
	m_size_vol.z = nVolDepth;

	m_volTexBias = mat4(vec4(-0.5f, 0.0f, 0.0f, 0.0f),
						vec4(0.0f, 0.5f, 0.0f, 0.0f),
						vec4(0.0f, 0.0f, 0.5f, 0.0f),
						vec4(0.5f, 0.5f, 0.5f, 1.0f));

	ApplyPreferences();
}

void CW3VolumeRenderParam::updateStepSize() {
  int nVolWidth = m_pgVol->width();
  int nVolHeight = m_pgVol->height();
  int nVolDepth = m_pgVol->depth();
  float xsize = (nVolWidth)*m_pgVol->pixelSpacing();
  float ysize = (nVolHeight)*m_pgVol->pixelSpacing();
  float zsize = (nVolDepth)*m_pgVol->sliceSpacing();
  float maxAxisSize = std::max(std::max(xsize, ysize), zsize);
  m_basePixelSizeMM = std::min(m_pgVol->pixelSpacing(), m_pgVol->sliceSpacing());
  m_stepSize = ((m_basePixelSizeMM * down_factor_) / (maxAxisSize*high_res_step_size_factor_));
}

void CW3VolumeRenderParam::setRayCastingParams(unsigned int prog) {
	glUseProgram(prog);
	WGLSLprogram::setUniform(prog, "MinValue", (float)m_MinValue);
	WGLSLprogram::setUniform(prog, "AttenDistance", m_AttenDistance);
	WGLSLprogram::setUniform(prog, "texelSize", m_volTexelSize);
	WGLSLprogram::setUniform(prog, "size_vol", m_size_vol);
	WGLSLprogram::setUniform(prog, "MaxValue", m_MaxValueForMIP);

	if (m_isLowRes)
		WGLSLprogram::setUniform(prog, "StepSize", m_stepSize*low_res_step_size_factor_);
	else
		WGLSLprogram::setUniform(prog, "StepSize", m_stepSize);
}

void CW3VolumeRenderParam::setRayFirstHitParams(unsigned int prog) {
	glUseProgram(prog);

	//thyoo 170223.
	//m_projection = glm::ortho(m_left, m_right, m_bottom, m_top, 0.0f, m_camFOV*2.0f) 이니까 m_camFOV*2.0f이다.
	//fpn 필요 없다.

	//float fmn = m_camFOV*2.0f;
	//float fpn = fmn;

	WGLSLprogram::setUniform(prog, "MinValue", m_MinValue);
	WGLSLprogram::setUniform(prog, "MaxValue", m_MaxValueForMIP);
	WGLSLprogram::setUniform(prog, "fmn", 1.0f);
	WGLSLprogram::setUniform(prog, "fpn", m_basePixelSizeMM);

	if (m_isLowRes)
		WGLSLprogram::setUniform(prog, "StepSize", m_stepSize*low_res_step_size_factor_);
	else
		WGLSLprogram::setUniform(prog, "StepSize", m_stepSize);
}

void CW3VolumeRenderParam::setRayFirstHitFrontFaceParams(unsigned int prog) {
	glUseProgram(prog);
	//WGLSLprogram::setUniform(prog, "matFirstHitToDepth", *m_matFirstHitToDepth);
	WGLSLprogram::setUniform(prog, "MinValue", m_MinValue);

	if (m_isLowRes)
		WGLSLprogram::setUniform(prog, "StepSize", m_stepSize*low_res_step_size_factor_);
	else
		WGLSLprogram::setUniform(prog, "StepSize", m_stepSize);
}

void CW3VolumeRenderParam::setSliceParams(unsigned int prog) {
	glUseProgram(prog);
	WGLSLprogram::setUniform(prog, "MinValue", m_MinValue);
	WGLSLprogram::setUniform(prog, "MaxIntensity", m_MaxIntensity);
}

void CW3VolumeRenderParam::setSliceCanalParams(unsigned int prog) {
	glUseProgram(prog);
	WGLSLprogram::setUniform(prog, "MinValue", m_MinValue);
	WGLSLprogram::setUniform(prog, "MaxIntensity", m_MaxIntensity);
	WGLSLprogram::setUniform(prog, "VolScale", m_VolScale);
	WGLSLprogram::setUniform(prog, "MVP", *(m_mvp));
	WGLSLprogram::setUniform(prog, "invModel", *(m_invModel));
}

void CW3VolumeRenderParam::setEndoPlaneParams(unsigned int prog) {
	glUseProgram(prog);
	WGLSLprogram::setUniform(prog, "MinValue", m_MinValue);
	WGLSLprogram::setUniform(prog, "MaxIntensity", m_MaxIntensity);
	WGLSLprogram::setUniform(prog, "MVP", *(m_mvp));
	WGLSLprogram::setUniform(prog, "sliceModel", *(m_invModel));
}

void CW3VolumeRenderParam::ApplyPreferences() {
	GlobalPreferences::Quality2 volume_rendering_quality = GlobalPreferences::GetInstance()->preferences_.advanced.volume_rendering.quality;
	switch (volume_rendering_quality) {
	case GlobalPreferences::Quality2::High:
		low_res_step_size_factor_ = 2.0f;
		break;
	case GlobalPreferences::Quality2::Low:
		low_res_step_size_factor_ = 8.0f;
		break;
	}
}
