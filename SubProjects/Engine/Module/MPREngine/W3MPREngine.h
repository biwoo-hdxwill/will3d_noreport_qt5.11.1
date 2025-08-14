#pragma once
/*=========================================================================

File:			class CW3MPREngine
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-11-24
Last modify:	2015-12-20

=========================================================================*/
#include <QObject>

#include "../../Common/Common/W3Enum.h"
#include "../../Common/Common/define_measure.h"
#include "../../Common/GLfunctions/WGLHeaders.h"

#include "mprengine_global.h"

class CW3Image3D;
class CW3ViewPlane;
class NerveResource;
class NerveMaskROI;
#ifndef WILL3D_VIEWER
class ProjectIOMPREngine;
#endif

class MPRENGINE_EXPORT CW3MPREngine : public QObject
{
	Q_OBJECT

public:
	CW3MPREngine();
	~CW3MPREngine();

#ifndef WILL3D_VIEWER
	virtual void ExportProject(ProjectIOMPREngine& out);
	virtual void ImportProject(ProjectIOMPREngine& in);
#endif

	inline const bool isValid() const noexcept { return m_pgVol ? true : false; }

	void init();
	void reset();

	void setVolume(CW3Image3D* vol, int id);
	void shapenPlane(CW3ViewPlane* viewPlane, unsigned int level);
	void applyReorientation(glm::mat4& model);

	void reconPlane(CW3ViewPlane* viewPlane, int id, float fThickness,
		const NerveResource& nerve_resource);
	bool reconMPRimage(CW3ViewPlane* viewPlane, int id, float fThickness);
	bool reconMPRimage(CW3ViewPlane* viewPlane, int id, float fThickness,
		const NerveResource& nerve_resource);
	void reconNerveMask(CW3ViewPlane* viewPlane,
		const NerveResource& nerve_resource);
	bool reconPANOimage(CW3ViewPlane* viewPlane,
		std::vector<glm::vec3>* TopRightCoord,
		std::vector<glm::vec3>* NormalUpToLower, float fThickness,
		common::ReconTypeID eReconType, bool isCanalShown);
	bool reconSumAlongZaxis(CW3ViewPlane* viewPlane, CW3Image3D* vol);
	void reconCUBE(CW3Image3D* volPan3D, std::vector<glm::vec3>* TopRightCoord,
		std::vector<glm::vec3>* NormalUpToLower,
		glm::vec3& backVector);

	inline void setMPRrotCenterInVol(const glm::vec3& RotCenterInVol, int id)
	{
		m_vMPRrotCenterInVol[id] = RotCenterInVol;
	}

	inline const float getBasePixelSize(const int id) const noexcept
	{
		return m_basePixelSize[id];
	}

	inline glm::vec3* getMPRrotCenterInVol(int id)
	{
		return &m_vMPRrotCenterInVol[id];
	}
	inline glm::vec3* getSIMPRrotCenterInVol(int id)
	{
		return &m_vSIMPRrotCenterInVol[id];
	}
	inline glm::vec3* getMPRrotCenterOrigInVol(int id)
	{
		return &m_vMPRrotCenterInVolorig[id];
	}

	inline const int getMaxAxisSize(int id) const noexcept
	{
		return m_nMaxAxisSize[id];
	}
	inline glm::vec3* getVolRange(int id) { return &m_vVolRange[id]; }

	inline float* getAxisMax(int id) { return m_pxyznxyz[id]; }

	glm::vec4 GetVolumeInfo(CW3ViewPlane* view_plane, float dx, float dy);
	void SecondTransform(const glm::mat4& transform);
	void SecondRegistered(float* param);

signals:
	void sigReoriupdate(glm::mat4*);
	void sigReoriupdateFromRightHanded(glm::mat4*);
	void sigSecondUpdate(glm::mat4*);

public slots:
	void slotReoriented(std::vector<float> param);

private:
	bool setPlaneSize(CW3ViewPlane* viewPlane, int id);
	bool getSharpenParams(int level, float& fSigma, float& fWeight);

	/* primitive reconstruction functions */
	void reconValueSum(int& rSum, unsigned short** ppVolume,
		const glm::vec3& ptVol, const int& nW, const int& nH,
		const int& nD);

	void reconValue2Buf(unsigned short*& pBuf, unsigned short** ppVolume,
		const glm::vec3& ptVol, const int& nW, const int& nH,
		const int& nD);

	void reconValue2Buf(unsigned short*& pBuf, const int& idxBuf,
		unsigned short** ppVolume, const glm::vec3& ptVol,
		const int& nW, const int& nH, const int& nD);

	bool IsCoordInMaskROI(const glm::vec3& coordinate, const glm::vec3& roi_start,
		const glm::vec3& roi_end);
	bool reconValueNerve(unsigned char*& pBuf, const glm::vec3& pt_vol,
		const NerveMaskROI& mask_roi);
	bool reconValueNerve(unsigned char*& pBuf, const int& idxBuf,
		const glm::vec3& pt_vol, const NerveMaskROI& mask_roi);

	void reconPlane(CW3ViewPlane* viewPlane,
		std::vector<glm::vec3>* TopRightCoord, bool isNerveDrawing,
		float fThickness);
	void reconPlane(CW3ViewPlane* viewPlane,
		std::vector<glm::vec3>* TopRightCoord,
		std::vector<glm::vec3>* NormalUpToLower, bool isNerveDrawing,
		float fThickness);

private:
	CW3Image3D* m_pgVol[2];
	float m_fSpacingZ[2];
	int m_nMaxAxisSize[2];

	glm::vec3 m_VolVertex[2][8];
	float m_pxyznxyz[2][3];
	float m_basePixelSize[2];

	glm::vec3 m_vMPRrotCenterInVol[2] = { glm::vec3() };  // CW3ViewMPR::m_crossCenter 의 3D 위치
	glm::vec3 m_vSIMPRrotCenterInVol[2] = { glm::vec3() };  // m_vMPRrotCenterInVol의 SI tab 용
	glm::vec3 m_vMPRrotCenterInVolorig[2] = { glm::vec3() };
	glm::vec3 m_vVolRange[2];

	float* m_pgParam;
};
