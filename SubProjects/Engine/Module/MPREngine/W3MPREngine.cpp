#include "W3MPREngine.h"
/*=========================================================================

File:			class CW3MPREngine
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-11-24
Last modify:	2015-12-20

=========================================================================*/
#if defined(__APPLE__)
#include </usr/local/Cellar/llvm/5.0.0/lib/clang/5.0.0/include/omp.h>
#else
#include <omp.h>
#endif
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc.hpp"

#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/W3Math.h"

#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/W3ViewPlane.h"
#include "../../Resource/Resource/nerve_resource.h"
#ifndef WILL3D_VIEWER
#include "../../Core/W3ProjectIO/project_io_mpr_engine.h"
#endif

#define USE_FLOAT

CW3MPREngine::CW3MPREngine() { init(); }

CW3MPREngine::~CW3MPREngine() {}
#ifndef WILL3D_VIEWER
void CW3MPREngine::ExportProject(ProjectIOMPREngine& out)
{
	for (int i = 0; i < 2; ++i)
	{
		out.SaveCenterInVol(i, m_vMPRrotCenterInVolorig[i], m_vMPRrotCenterInVol[i], m_vSIMPRrotCenterInVol[i]);
	}
}

void CW3MPREngine::ImportProject(ProjectIOMPREngine& in)
{
	for (int i = 0; i < 2; ++i)
	{
		in.LoadCenterInVol(i, m_vMPRrotCenterInVolorig[i], m_vMPRrotCenterInVol[i], m_vSIMPRrotCenterInVol[i]);
	}
}
#endif
void CW3MPREngine::init()
{
	for (int idx = 0; idx < 2; ++idx)
	{
		m_pgVol[idx] = nullptr;
		m_fSpacingZ[idx] = 0.0f;
		m_basePixelSize[idx] = 0.0f;
		m_nMaxAxisSize[idx] = 0;
	}

	glm::vec3 zeroVec = glm::vec3(0.0f, 0.0f, 0.0f);

	for (int idx = 0; idx < 8; ++idx) m_VolVertex[0][idx] = zeroVec;

	m_vMPRrotCenterInVol[0] = zeroVec;
	m_vSIMPRrotCenterInVol[0] = zeroVec;
	m_vVolRange[0] = zeroVec;

	for (int idx = 0; idx < 3; ++idx) m_pxyznxyz[0][idx] = 0.0f;

	m_pgParam = nullptr;
}

void CW3MPREngine::reset() { init(); }

void CW3MPREngine::setVolume(CW3Image3D* vol, int id)
{
	m_pgVol[id] = vol;
	const unsigned int nWidth = vol->width();
	const unsigned int nHeight = vol->height();
	const unsigned int nDepth = vol->depth();
	m_fSpacingZ[id] = vol->sliceSpacing() / vol->pixelSpacing();

	if (m_fSpacingZ[id] >= 1.0f)
	{
		m_nMaxAxisSize[id] = static_cast<int>(
			std::sqrt(nWidth * nWidth + nHeight * nHeight +
				nDepth * nDepth * m_fSpacingZ[id] * m_fSpacingZ[id]));
#ifdef USE_FLOAT
		m_vMPRrotCenterInVol[id] =
			glm::vec3((nWidth - 1) * 0.5f, (nHeight - 1) * 0.5f,
			(nDepth - 1) * m_fSpacingZ[id] * 0.5f);
#else
		m_vMPRrotCenterInVol[id] = glm::vec3((nWidth - 1) / 2, (nHeight - 1) / 2,
			(nDepth - 1) * m_fSpacingZ[id] / 2);
#endif
		m_VolVertex[id][0] = glm::vec3(0.0f, 0.0f, 0.0f);
		m_VolVertex[id][1] = glm::vec3(nWidth, 0.0f, 0.0f);
		m_VolVertex[id][2] = glm::vec3(0.0f, nHeight, 0.0f);
		m_VolVertex[id][3] = glm::vec3(nWidth, nHeight, 0.0f);
		m_VolVertex[id][4] = glm::vec3(0.0f, 0.0f, nDepth * m_fSpacingZ[id]);
		m_VolVertex[id][5] = glm::vec3(nWidth, 0.0f, nDepth * m_fSpacingZ[id]);
		m_VolVertex[id][6] = glm::vec3(0.0f, nHeight, nDepth * m_fSpacingZ[id]);
		m_VolVertex[id][7] = glm::vec3(nWidth, nHeight, nDepth * m_fSpacingZ[id]);

		m_vVolRange[id] = glm::vec3(nWidth, nHeight, nDepth * m_fSpacingZ[id]);

		m_basePixelSize[id] = vol->pixelSpacing();
	}
	else
	{
		m_nMaxAxisSize[id] = static_cast<int>(
			std::sqrt(nWidth * nWidth / (m_fSpacingZ[id] * m_fSpacingZ[id]) +
				nHeight * nHeight / (m_fSpacingZ[id] * m_fSpacingZ[id]) +
				nDepth * nDepth));
#ifdef USE_FLOAT
		m_vMPRrotCenterInVol[id] =
			glm::vec3((nWidth - 1) / m_fSpacingZ[id] * 0.5f,
			(nHeight - 1) / m_fSpacingZ[id] * 0.5f, (nDepth - 1) * 0.5f);
#else
		m_vMPRrotCenterInVol[id] =
			glm::vec3((nWidth - 1) / m_fSpacingZ[id] / 2,
			(nHeight - 1) / m_fSpacingZ[id] / 2, (nDepth - 1) / 2);
#endif
		m_VolVertex[id][0] = glm::vec3(0.0f, 0.0f, 0.0f);
		m_VolVertex[id][1] = glm::vec3(nWidth / m_fSpacingZ[id], 0.0f, 0.0f);
		m_VolVertex[id][2] = glm::vec3(0.0f, nHeight / m_fSpacingZ[id], 0.0f);
		m_VolVertex[id][3] =
			glm::vec3(nWidth / m_fSpacingZ[id], nHeight / m_fSpacingZ[id], 0.0f);
		m_VolVertex[id][4] = glm::vec3(0.0f, 0.0f, nDepth);
		m_VolVertex[id][5] = glm::vec3(nWidth / m_fSpacingZ[id], 0.0f, nDepth);
		m_VolVertex[id][6] = glm::vec3(0.0f, nHeight / m_fSpacingZ[id], nDepth);
		m_VolVertex[id][7] =
			glm::vec3(nWidth / m_fSpacingZ[id], nHeight / m_fSpacingZ[id], nDepth);

		m_vVolRange[id] =
			glm::vec3(nWidth / m_fSpacingZ[id], nHeight / m_fSpacingZ[id], nDepth);

		m_basePixelSize[id] = vol->sliceSpacing();
	}

	m_vMPRrotCenterInVolorig[id] = m_vMPRrotCenterInVol[id];
	m_vSIMPRrotCenterInVol[id] = m_vMPRrotCenterInVol[id];

	m_pxyznxyz[id][0] = m_vMPRrotCenterInVol[id].x;
	m_pxyznxyz[id][1] = m_vMPRrotCenterInVol[id].y;
	m_pxyznxyz[id][2] = m_vMPRrotCenterInVol[id].z;
}

bool CW3MPREngine::reconMPRimage(CW3ViewPlane* viewPlane, int id,
	float fThickness)
{
	bool isSizeDifferent = setPlaneSize(viewPlane, id);

	reconPlane(viewPlane, id, fThickness, NerveResource());
	return isSizeDifferent;
}

bool CW3MPREngine::reconMPRimage(CW3ViewPlane* viewPlane, int id,
	float fThickness,
	const NerveResource& nerve_resource)
{
	bool isSizeDifferent = setPlaneSize(viewPlane, id);

	reconPlane(viewPlane, id, fThickness, nerve_resource);
	return isSizeDifferent;
}

bool CW3MPREngine::reconPANOimage(CW3ViewPlane* viewPlane,
	std::vector<glm::vec3>* TopRightCoord,
	std::vector<glm::vec3>* NormalUpToLower,
	float fThickness,
	common::ReconTypeID eReconType,
	bool isCanalShown)
{
	//// may be useful for one slice recon
	// reconPlane(viewPlane, TopRightCoord, fThickness);
	switch (eReconType)
	{
	case common::ReconTypeID::MPR:
		reconPlane(viewPlane, TopRightCoord, isCanalShown, 0.0f);
		break;
	case common::ReconTypeID::X_RAY:
		reconPlane(viewPlane, TopRightCoord, NormalUpToLower, isCanalShown,
			fThickness);
		break;
	}

	return false;
}

bool CW3MPREngine::setPlaneSize(CW3ViewPlane* viewPlane, int id)
{
	/*glm::vec3 ptCenter((int)viewPlane->getPlaneCenterInVol().x,
		(int)viewPlane->getPlaneCenterInVol().y,
		(int)viewPlane->getPlaneCenterInVol().z);*/
	glm::vec3 ptCenter(viewPlane->getPlaneCenterInVol().x,
		viewPlane->getPlaneCenterInVol().y,
		viewPlane->getPlaneCenterInVol().z);
	glm::vec3 right = glm::normalize(viewPlane->getRightVec());
	glm::vec3 back = glm::normalize(viewPlane->getBackVec());
	glm::vec3 up = glm::normalize(glm::cross(right, back));

	int maxWidth = 0, maxHeight = 0;
	int maxHafDepth = 0, maxDepth = 0;
	int tmpVal;

	for (int i = 0; i < 8; i++)
	{
		glm::vec3 vec = m_VolVertex[id][i] - ptCenter;

		tmpVal = glm::dot(vec, right);
		maxWidth = std::max(maxWidth, std::abs(tmpVal));
		tmpVal = glm::dot(vec, back);
		maxHeight = std::max(maxHeight, std::abs(tmpVal));

		tmpVal = glm::dot(vec, up);
		if (maxHafDepth <= tmpVal)
		{
			maxHafDepth = tmpVal;

			int max = 0;
			for (int j = 0; j < 8; j++)
			{
				glm::vec3 vec = m_VolVertex[id][j] - ptCenter;
				tmpVal = glm::dot(vec, -up);
				if (max < tmpVal)
				{
					max = tmpVal;
					maxDepth = std::max(maxDepth, maxHafDepth + max);
				}
			}
		}
	}

	if (maxDepth == 0)
		maxDepth = std::max(maxDepth, maxHafDepth);

	viewPlane->resize(maxWidth * 2, maxHeight * 2);
	viewPlane->setAvailableDepth(maxDepth);

	return true;
}

bool CW3MPREngine::reconSumAlongZaxis(CW3ViewPlane* viewPlane,
	CW3Image3D* vol)
{
	const int depth = vol->depth();
	const int width = vol->width();
	const int height = vol->height();
	const int sliceSize = width * height;

	unsigned short* pPlaneBuffer = viewPlane->getImage2D()->getData();
	unsigned short** ppVolumeData = vol->getData();

#pragma omp parallel for
	for (int xy = 0; xy < sliceSize; xy++)
	{
		int valSum = 0;
		for (int z = 0; z < depth; z++)
		{
			valSum += ppVolumeData[z][xy];
		}

		pPlaneBuffer[xy] = valSum / depth;
	}

	return true;
}

glm::vec4 CW3MPREngine::GetVolumeInfo(CW3ViewPlane* view_plane, float dx,
	float dy)
{
	const float plane_w = static_cast<float>(view_plane->getWidth());
	const float plane_h = static_cast<float>(view_plane->getHeight());
	const glm::vec3 vR = view_plane->getRightVec();
	const glm::vec3 vB = view_plane->getBackVec();

	glm::vec3 volume_coord = view_plane->getPlaneCenterInVol() +
		vR * (dx - (plane_w - 1.0f) * 0.5f) +
		vB * (dy - (plane_h - 1.0f) * 0.5f);

	return m_pgVol[0]->GetVolumeInfo(volume_coord);
}

void CW3MPREngine::reconPlane(CW3ViewPlane* viewPlane, int volid,
	float fThickness,
	const NerveResource& nerve_resource)
{
	if (!viewPlane->getImage2D()) return;

	unsigned short* pPlaneBuffer = viewPlane->getImage2D()->getData();
	unsigned char* pMaskBuffer = viewPlane->getMask();
	unsigned short** ppVol = m_pgVol[volid]->getData();
	const unsigned short nIntensityMin = m_pgVol[0]->getMin();

	const int nWidth = m_pgVol[volid]->width();
	const int nHeight = m_pgVol[volid]->height();
	const int nDepth = m_pgVol[volid]->depth();
	const int nWPre = nWidth - 1;
	const int nHPre = nHeight - 1;
	const int nDPre = nDepth - 1;

	const int iPlaneWidth = viewPlane->getWidth();
	const int iPlaneHeight = viewPlane->getHeight();
	const float fPlaneWidth = static_cast<float>(iPlaneWidth);
	const float fPlaneHeight = static_cast<float>(iPlaneHeight);

	const glm::vec3& vR = viewPlane->getRightVec();
	const glm::vec3& vB = viewPlane->getBackVec();

	const glm::vec3 ptPlaneCenterInVol = viewPlane->getPlaneCenterInVol();
	glm::vec3 ptPlane = -(fPlaneWidth - 1.0f) * vR * 0.5f +
		-(fPlaneHeight - 1.0f) * vB * 0.5f + ptPlaneCenterInVol;

	bool is_set_nerve_mask = nerve_resource.IsSetNerveMask();

	if (is_set_nerve_mask)
		memset(pMaskBuffer, 0, sizeof(unsigned char) * iPlaneWidth * iPlaneHeight);

	const int numThread = omp_get_max_threads();
	omp_set_num_threads(numThread);

	if (fThickness < 2.0f)
	{
#pragma omp parallel for
		for (int v = 0; v < iPlaneHeight; v++)
		{
			unsigned short* pBufferTemp = pPlaneBuffer + v * iPlaneWidth;
			unsigned char* pMaskTemp = pMaskBuffer + v * iPlaneWidth;

			const glm::vec3 pt1 = ptPlane + float(v) * vB;

			for (int u = 0; u < iPlaneWidth; u++)
			{
				glm::vec3 ptVol = pt1 + float(u) * vR;
				if (m_fSpacingZ[volid] >= 1.0f)
				{
					ptVol.z = ptVol.z / m_fSpacingZ[volid];
				}
				else
				{
					ptVol.x *= m_fSpacingZ[volid];
					ptVol.y *= m_fSpacingZ[volid];
				}

				if ((ptVol.x >= 0 && ptVol.x <= nWPre) &&
					(ptVol.y >= 0 && ptVol.y <= nHPre) &&
					(ptVol.z >= 0 && ptVol.z <= nDPre))
				{
					reconValue2Buf(pBufferTemp, u, ppVol, ptVol, nWidth, nHeight, nDepth);

					if (!is_set_nerve_mask) continue;

					const NerveMaskROI& mask_roi = nerve_resource.mask_roi();
					const int x1 = static_cast<int>(ptVol.x);
					const int y1 = static_cast<int>(ptVol.y);
					const int z1 = static_cast<int>(ptVol.z);
					const int id = y1 * nWidth + x1;
					if (!mask_roi.IsTrueBitMaskROI(id, z1)) continue;

					reconValueNerve(pMaskTemp, u, ptVol, mask_roi);
				}
				else
				{
					pBufferTemp[u] = nIntensityMin;
				}
			}
		}
	}
	else
	{
		const glm::vec3& vUp = viewPlane->getUpVec();
		const int nThickness = static_cast<int>(fThickness);
		ptPlane = ptPlane - vUp * fThickness * 0.5f;

#pragma omp parallel for
		for (int v = 0; v < iPlaneHeight; v++)
		{
			bool isMasked = false;
			unsigned short* pBufferTemp = pPlaneBuffer + v * iPlaneWidth;
			unsigned char* pMaskTemp = pMaskBuffer + v * iPlaneWidth;
			const glm::vec3 pt1 = ptPlane + float(v) * vB;

			for (int u = 0; u < iPlaneWidth; u++)
			{
				const glm::vec3 pt2 = pt1 + float(u) * vR;

				int tmpVal = 0;
				isMasked = false;
				for (int d = 0; d < nThickness; d++)
				{
					glm::vec3 ptVol = pt2 + float(d) * vUp;
					if (m_fSpacingZ[volid] >= 1.0f)
					{
						ptVol.z = ptVol.z / m_fSpacingZ[volid];
					}
					else
					{
						ptVol.x *= m_fSpacingZ[volid];
						ptVol.y *= m_fSpacingZ[volid];
					}

					if ((ptVol.x >= 0 && ptVol.x <= nWPre) &&
						(ptVol.y >= 0 && ptVol.y <= nHPre) &&
						(ptVol.z >= 0 && ptVol.z <= nDPre))
					{
						reconValueSum(tmpVal, ppVol, ptVol, nWidth, nHeight, nDepth);

						if (is_set_nerve_mask && !isMasked)
						{
							const int x1 = static_cast<int>(ptVol.x);
							const int y1 = static_cast<int>(ptVol.y);
							const int z1 = static_cast<int>(ptVol.z);
							const int id = y1 * nWidth + x1;

							const NerveMaskROI& mask_roi = nerve_resource.mask_roi();

							if (!mask_roi.IsTrueBitMaskROI(id, z1)) continue;

							isMasked = reconValueNerve(pMaskTemp, ptVol, mask_roi);
						}
					}
					else
						tmpVal += nIntensityMin;
				}

				*(pBufferTemp++) = tmpVal / nThickness;
				pMaskTemp++;
			}
		}
	}
}

void CW3MPREngine::shapenPlane(CW3ViewPlane * viewPlane, unsigned int level)
{
	if (!viewPlane->getImage2D())
	{
		return;
	}

	unsigned short *pPlaneBuffer = viewPlane->getImage2D()->getData();

	if (level > 3)
	{
		float sigma = 0.0f;

		switch (level)
		{
		case 4:
			sigma = 0.25f;
			break;
		case 5:
			sigma = 0.5f;
			break;
		case 6:
			sigma = 1.0f;
			break;
		default:
			assert(false);
		}

		const int width = viewPlane->getWidth();
		const int height = viewPlane->getHeight();

		cv::Mat cvBuffer = cv::Mat(height, width, CV_16U);
		memcpy(cvBuffer.ptr<unsigned short>(0), pPlaneBuffer, sizeof(unsigned short)*width*height);

		int ksize = (int)MAX(3.0, 2 * 4 * (sigma)+1.0);
		if (ksize % 2 == 0) ksize++; // 홀수

		cv::Mat blurred;
		cv::GaussianBlur(cvBuffer, blurred, cv::Size(ksize, ksize), 0.0, 0.0, cv::BORDER_REPLICATE);

		memcpy(pPlaneBuffer, blurred.ptr<unsigned short>(0), sizeof(unsigned short)*width*height);
	}
	else
	{
		float sigma = 0.0f, weight = 0.0f;
		if (!getSharpenParams(level, sigma, weight))
			return;

		const int width = viewPlane->getWidth();
		const int height = viewPlane->getHeight();

		cv::Mat cvBuffer = cv::Mat(height, width, CV_16U);
		memcpy(cvBuffer.ptr<unsigned short>(0), pPlaneBuffer, sizeof(unsigned short)*width*height);

		int ksize = (int)MAX(3.0, 2 * 4 * (sigma)+1.0);
		if (ksize % 2 == 0) ksize++; // 홀수

		cv::Mat blurred;
		cv::GaussianBlur(cvBuffer, blurred, cv::Size(ksize, ksize), 0.0, 0.0, cv::BORDER_REPLICATE);

		double max, min;
		cv::minMaxLoc(cvBuffer, &min, &max);

		unsigned short* iData = cvBuffer.ptr<unsigned short>(0);
		unsigned short* bData = blurred.ptr<unsigned short>(0);
		for (int i = 0; i < width*height; i++)
		{
			const float wsub = (*iData - (weight*(*bData++)));
			int val = wsub / (1.0f - weight);

			val = (val > max) ? max : (val < min) ? min : val;
			*iData++ = val;
		}

		memcpy(pPlaneBuffer, cvBuffer.ptr<unsigned short>(0), sizeof(unsigned short)*width*height);
	}
}

void CW3MPREngine::reconNerveMask(CW3ViewPlane* viewPlane,
	const NerveResource& nerve_resource)
{
	if (&nerve_resource == nullptr || !nerve_resource.IsSetNerveMask()) return;

	unsigned char* pMaskBuffer = viewPlane->getMask();

	const int iPlaneWidth = viewPlane->getWidth();
	const int iPlaneHeight = viewPlane->getHeight();
	const float fPlaneWidth = static_cast<float>(iPlaneWidth);
	const float fPlaneHeight = static_cast<float>(iPlaneHeight);

	const int nWidth = m_pgVol[0]->width();
	const int nHeight = m_pgVol[0]->height();
	const int nDepth = m_pgVol[0]->depth();
	const int nWPre = nWidth - 1;
	const int nHPre = nHeight - 1;
	const int nDPre = nDepth - 1;

	const glm::vec3 vR = viewPlane->getRightVec();
	const glm::vec3 vB = viewPlane->getBackVec();

	memset(pMaskBuffer, 0, sizeof(unsigned char) * iPlaneWidth * iPlaneHeight);

	const glm::vec3 ptPlaneCenterInVol = viewPlane->getPlaneCenterInVol();
	const glm::vec3 ptPlane = -(fPlaneWidth - 1.0f) * vR * 0.5f +
		-(fPlaneHeight - 1.0f) * vB * 0.5f +
		ptPlaneCenterInVol;

	const NerveMaskROI& mask_roi = nerve_resource.mask_roi();
	const int numThread = omp_get_max_threads();
	omp_set_num_threads(numThread);
#pragma omp parallel for
	for (int v = 0; v < iPlaneHeight; v++)
	{
		unsigned char* pMaskTemp = pMaskBuffer + v * iPlaneWidth;
		glm::vec3 pt1 = ptPlane + float(v) * vB;

		for (int u = 0; u < iPlaneWidth; u++)
		{
			glm::vec3 ptVol = pt1 + float(u) * vR;
			if (m_fSpacingZ[0] >= 1.0f)
			{
				ptVol.z /= m_fSpacingZ[0];
			}
			else
			{
				ptVol.x *= m_fSpacingZ[0];
				ptVol.y *= m_fSpacingZ[0];
			}

			if ((ptVol.x >= 0 && ptVol.x <= nWPre) &&
				(ptVol.y >= 0 && ptVol.y <= nHPre) &&
				(ptVol.z >= 0 && ptVol.z <= nDPre))
			{
				const int x1 = static_cast<int>(ptVol.x);
				const int y1 = static_cast<int>(ptVol.y);
				const int z1 = static_cast<int>(ptVol.z);
				const int id = y1 * nWidth + x1;
				if (!mask_roi.IsTrueBitMaskROI(id, z1)) continue;

				reconValueNerve(pMaskTemp, u, ptVol, mask_roi);
			}
		}
	}
}

void CW3MPREngine::reconCUBE(CW3Image3D* volPan3D_,
	std::vector<glm::vec3>* TopRightCoord,
	std::vector<glm::vec3>* NormalUpToLower,
	glm::vec3& vB)
{
	unsigned short** ppVolumeData = m_pgVol[0]->getData();

	const int panoWidth = volPan3D_->width();
	const int panoHeight = volPan3D_->height();
	const int panoThickness = volPan3D_->depth();
	const int nWidth = m_pgVol[0]->width();
	const int nHeight = m_pgVol[0]->height();
	const int nDepth = m_pgVol[0]->depth();
	const int nWPre = nWidth - 1;
	const int nHPre = nHeight - 1;
	const int nDPre = nDepth - 1;

	std::vector<glm::vec3> farestTopRightCoord(panoWidth);
	const float halfThickness = float(panoThickness) * 0.5f;
	for (int v = 0; v < panoWidth; v++)
		farestTopRightCoord[v] =
		TopRightCoord->at(v) - NormalUpToLower->at(v) * halfThickness;

	const int numThread = omp_get_max_threads();
	omp_set_num_threads(numThread);

#pragma omp parallel for
	for (int d = 0; d < panoThickness; d++)
	{
		const float fD = float(d);
		unsigned short* pBufferTemp = volPan3D_->getData()[d];
		glm::vec3 pt2;
		for (int v = 0; v < panoHeight; v++)
		{
			for (int u = 0; u < panoWidth; u++)
			{
				const glm::vec3 pt1 = farestTopRightCoord.at(u);
				const glm::vec3 vUp = NormalUpToLower->at(u);
				glm::vec3 ptVol = pt1 + fD * vUp + pt2;

				if (m_fSpacingZ[0] >= 1.0f)
				{
					ptVol.z /= m_fSpacingZ[0];
				}
				else
				{
					ptVol.x *= m_fSpacingZ[0];
					ptVol.y *= m_fSpacingZ[0];
				}

				if ((ptVol.x >= 0 && ptVol.x <= nWPre) &&
					(ptVol.y >= 0 && ptVol.y <= nHPre) &&
					(ptVol.z >= 0 && ptVol.z <= nDPre))
				{
					reconValue2Buf(pBufferTemp, ppVolumeData, ptVol, nWidth, nHeight,
						nDepth);
				}
				else
				{
					*pBufferTemp++ = m_pgVol[0]->getMin();
				}
			}
			pt2 += vB;
		}
	}
}

void CW3MPREngine::slotReoriented(std::vector<float> param)
{
	const int nWidth = m_pgVol[0]->width();
	const int nHeight = m_pgVol[0]->height();
	const int nDepth = m_pgVol[0]->depth();

	if (m_fSpacingZ[0] >= 1.0f)
	{
		m_vMPRrotCenterInVol[0] =
			glm::vec3((nWidth - 1) * 0.5f + param[1], (nHeight - 1) * 0.5f,
			(nDepth - 1) * m_fSpacingZ[0] * 0.5f);
	}
	else
	{
		m_vMPRrotCenterInVol[0] =
			glm::vec3(((nWidth - 1) * 0.5f + param[1]) / m_fSpacingZ[0],
			(nHeight - 1) / m_fSpacingZ[0] * 0.5f, (nDepth - 1) * 0.5f);
	}

	m_vMPRrotCenterInVolorig[0] = m_vSIMPRrotCenterInVol[0] =
		m_vMPRrotCenterInVol[0];
	glm::mat4 T = glm::rotate(param[2], glm::vec3(0.0f, -1.0f, 0.0f)) *
		glm::rotate(param[0], glm::vec3(0.0f, 0.0f, -1.0f));

	emit sigReoriupdate(&T);
}

void CW3MPREngine::applyReorientation(glm::mat4& model)
{
	const glm::mat4 modelMPR = glm::inverse(model);
	const glm::vec4 translate = modelMPR[3] * 0.5f;

	const int nWidth = m_pgVol[0]->width();
	const int nHeight = m_pgVol[0]->height();
	const int nDepth = m_pgVol[0]->depth();

	// x translation 은 left hand vector 로 바꾸기 위해 -1 곱함
	if (m_fSpacingZ[0] >= 1.0f)
	{
#ifdef USE_FLOAT
		m_vMPRrotCenterInVol[0] = glm::vec3(
			(nWidth - 1) * 0.5f - translate.x, (nHeight - 1) * 0.5f + translate.y,
			(nDepth - 1) * m_fSpacingZ[0] * 0.5f + translate.z);
#else
		m_vMPRrotCenterInVol[0] = glm::vec3(
			(nWidth - 1) / 2 - translate.x, (nHeight - 1) / 2 + translate.y,
			(nDepth - 1) * m_fSpacingZ[0] / 2 + translate.z);
#endif
	}
	else
	{
#ifdef USE_FLOAT
		m_vMPRrotCenterInVol[0] =
			glm::vec3((nWidth - 1) / m_fSpacingZ[0] * 0.5f - translate.x,
			(nHeight - 1) / m_fSpacingZ[0] * 0.5f + translate.y,
				(nDepth - 1) * 0.5f + translate.z);
#else
		m_vMPRrotCenterInVol[0] =
			glm::vec3((nWidth - 1) / m_fSpacingZ[0] / 2 - translate.x,
			(nHeight - 1) / m_fSpacingZ[0] / 2 + translate.y,
				(m_depth[0] - 1) / 2 + translate.z);
#endif
	}

	m_vMPRrotCenterInVolorig[0] = m_vSIMPRrotCenterInVol[0] =
		m_vMPRrotCenterInVol[0];

	glm::mat4 modelInv(1.0f);
	modelInv[0] = modelMPR[0];
	modelInv[1] = modelMPR[1];
	modelInv[2] = modelMPR[2];

	// left hand vector 를 right hand tansform 으로 변환하기 위한 과정
	modelInv = glm::scale(glm::vec3(-1.0f, 1.0f, 1.0f)) * modelInv *
		glm::scale(glm::vec3(-1.0f, 1.0f, 1.0f));

	emit sigReoriupdate(&modelInv);
}

void CW3MPREngine::SecondRegistered(float* param)
{
	m_pgParam = param;
	const float scaleRatio = m_basePixelSize[0] / m_basePixelSize[1];
	const int nWidth = m_pgVol[0]->width();
	const int nHeight = m_pgVol[0]->height();
	const int nDepth = m_pgVol[0]->depth();

	glm::mat4 mMainToSecond =
		glm::scale(glm::vec3(scaleRatio)) *
		glm::translate(glm::vec3((nWidth - 1) * 0.5f + m_pgParam[1],
		(nHeight - 1) * 0.5f + m_pgParam[2],
			(nDepth - 1) * 0.5f + m_pgParam[4])) *
		glm::rotate(m_pgParam[3], glm::vec3(-1.0f, 0.0f, 0.0f)) *
		glm::rotate(m_pgParam[5], glm::vec3(0.0f, -1.0f, 0.0f)) *
		glm::rotate(m_pgParam[0], glm::vec3(0.0f, 0.0f, -1.0f)) *
		glm::translate(glm::vec3(-(nWidth - 1) * 0.5f, -(nHeight - 1) * 0.5f,
			-(nDepth - 1) * 0.5f));

	emit sigSecondUpdate(&mMainToSecond);
}

void CW3MPREngine::SecondTransform(const glm::mat4& transform)
{
	if (!m_pgParam) return;

	const int nWidth = m_pgVol[0]->width();
	const int nHeight = m_pgVol[0]->height();
	const int nDepth = m_pgVol[0]->depth();

	const float scaleRatio = m_basePixelSize[0] / m_basePixelSize[1];

	glm::mat4 transMat = glm::inverse(transform);
	transMat[3][0] *= 0.5f;
	transMat[3][1] *= 0.5f;
	transMat[3][2] *= 0.5f;

	glm::mat4 mMainToSecond =
		glm::scale(glm::vec3(scaleRatio)) *
		glm::translate(glm::vec3((nWidth - 1) * 0.5f + m_pgParam[1],
		(nHeight - 1) * 0.5f + m_pgParam[2],
			(nDepth - 1) * 0.5f + m_pgParam[4])) *
		transMat * glm::rotate(m_pgParam[3], glm::vec3(-1.0f, 0.0f, 0.0f)) *
		glm::rotate(m_pgParam[5], glm::vec3(0.0f, -1.0f, 0.0f)) *
		glm::rotate(m_pgParam[0], glm::vec3(0.0f, 0.0f, -1.0f)) *
		glm::translate(glm::vec3(-(nWidth - 1) * 0.5f, -(nHeight - 1) * 0.5f,
			-(nDepth - 1) * 0.5f));

	emit sigSecondUpdate(&mMainToSecond);
}

bool CW3MPREngine::getSharpenParams(int level, float& fSigma, float& fWeight)
{
	switch (level)
	{
	case 1:
		fSigma = 0.25f;
		fWeight = 0.5f;
		break;
	case 2:
		fSigma = 0.25f;
		fWeight = 0.62f;
		break;
	case 3:
		fSigma = 0.25f;
		fWeight = 0.7f;
		break;
	default:
		return false;
	}
	return true;
}

void CW3MPREngine::reconValueSum(int& rSum, unsigned short** ppVolume,
	const glm::vec3& ptVol, const int& nW,
	const int& nH, const int& nD)
{
	const int x1 = static_cast<int>(ptVol.x);
	const int y1 = static_cast<int>(ptVol.y);
	const int z1 = static_cast<int>(ptVol.z);
	const int x2 = std::min(x1 + 1, nW - 1);
	const int y2 = std::min(y1 + 1, nH - 1);
	const int z2 = std::min(z1 + 1, nD - 1);
	const int Y1 = y1 * nW;
	const int Y2 = y2 * nW;

	rSum += W3::trilerp(ppVolume[z1][Y1 + x1], ppVolume[z1][Y1 + x2],
		ppVolume[z1][Y2 + x1], ppVolume[z1][Y2 + x2],
		ppVolume[z2][Y1 + x1], ppVolume[z2][Y1 + x2],
		ppVolume[z2][Y2 + x1], ppVolume[z2][Y2 + x2],
		ptVol.x - x1, ptVol.y - y1, ptVol.z - z1);
}

void CW3MPREngine::reconValue2Buf(unsigned short*& pBuf,
	unsigned short** ppVolume,
	const glm::vec3& ptVol, const int& nW,
	const int& nH, const int& nD)
{
	const int x1 = static_cast<int>(ptVol.x);
	const int y1 = static_cast<int>(ptVol.y);
	const int z1 = static_cast<int>(ptVol.z);
	const int x2 = std::min(x1 + 1, nW - 1);
	const int y2 = std::min(y1 + 1, nH - 1);
	const int z2 = std::min(z1 + 1, nD - 1);
	const int Y1 = y1 * nW;
	const int Y2 = y2 * nW;

	*pBuf++ = W3::trilerp(ppVolume[z1][Y1 + x1], ppVolume[z1][Y1 + x2],
		ppVolume[z1][Y2 + x1], ppVolume[z1][Y2 + x2],
		ppVolume[z2][Y1 + x1], ppVolume[z2][Y1 + x2],
		ppVolume[z2][Y2 + x1], ppVolume[z2][Y2 + x2],
		ptVol.x - x1, ptVol.y - y1, ptVol.z - z1);
}

void CW3MPREngine::reconValue2Buf(unsigned short*& pBuf, const int& bufIdx,
	unsigned short** ppVolume,
	const glm::vec3& ptVol, const int& nW,
	const int& nH, const int& nD)
{
	const int x1 = static_cast<int>(ptVol.x);
	const int y1 = static_cast<int>(ptVol.y);
	const int z1 = static_cast<int>(ptVol.z);
	const int x2 = std::min(x1 + 1, nW - 1);
	const int y2 = std::min(y1 + 1, nH - 1);
	const int z2 = std::min(z1 + 1, nD - 1);
	const int Y1 = y1 * nW;
	const int Y2 = y2 * nW;

	pBuf[bufIdx] = W3::trilerp(ppVolume[z1][Y1 + x1], ppVolume[z1][Y1 + x2],
		ppVolume[z1][Y2 + x1], ppVolume[z1][Y2 + x2],
		ppVolume[z2][Y1 + x1], ppVolume[z2][Y1 + x2],
		ppVolume[z2][Y2 + x1], ppVolume[z2][Y2 + x2],
		ptVol.x - x1, ptVol.y - y1, ptVol.z - z1);
}

bool CW3MPREngine::IsCoordInMaskROI(const glm::vec3& coordinate,
	const glm::vec3& roi_start,
	const glm::vec3& roi_end)
{
	if (((int)roi_start.x <= (int)coordinate.x) &&
		((int)roi_end.x >= (int)coordinate.x) &&
		((int)roi_start.y <= (int)coordinate.y) &&
		((int)roi_end.y >= (int)coordinate.y) &&
		((int)roi_start.z <= (int)coordinate.z) &&
		((int)roi_end.z >= (int)coordinate.z))
		return true;
	else
		return false;
}

bool CW3MPREngine::reconValueNerve(unsigned char*& pBuf,
	const glm::vec3& pt_vol,
	const NerveMaskROI& mask_roi)
{
	const auto& roi_dir = mask_roi.roi_direction();
	const auto& roi_start_pos = mask_roi.roi_start_pos();
	const auto& roi_end_pos = mask_roi.roi_end_pos();
	const auto& roi_radius = mask_roi.roi_radius();

	for (int i = 0; i < mask_roi.roi_list_size(); i++)
	{
		glm::vec3 roi_start = roi_start_pos[i];
		glm::vec3 roi_end = roi_end_pos[i];

		if (IsCoordInMaskROI(pt_vol, roi_start, roi_end))
		{
			vec3 roi_center = (roi_start + roi_end) * 0.5f;
			vec3 roi_center_to_point = pt_vol - roi_center;

			float len_cross =
				glm::length(glm::cross(roi_center_to_point, roi_dir[i]));
			if (len_cross <= roi_radius[i])
			{
				*pBuf = 255;
				return true;
			}
		}
	}
	return false;
}

bool CW3MPREngine::reconValueNerve(unsigned char*& pBuf, const int& idxBuf,
	const glm::vec3& pt_vol,
	const NerveMaskROI& mask_roi)
{
	const auto& roi_dir = mask_roi.roi_direction();
	const auto& roi_start_pos = mask_roi.roi_start_pos();
	const auto& roi_end_pos = mask_roi.roi_end_pos();
	const auto& roi_radius = mask_roi.roi_radius();

	for (int i = 0; i < mask_roi.roi_list_size(); i++)
	{
		glm::vec3 roi_start = roi_start_pos[i];
		glm::vec3 roi_end = roi_end_pos[i];

		if (IsCoordInMaskROI(pt_vol, roi_start, roi_end))
		{
			vec3 roi_center = (roi_start + roi_end) * 0.5f;
			vec3 roi_center_to_point = pt_vol - roi_center;

			float len_cross =
				glm::length(glm::cross(roi_center_to_point, roi_dir[i]));
			if (len_cross <= roi_radius[i])
			{
				pBuf[idxBuf] = 255;
				return true;
			}
		}
	}
	return false;
}

void CW3MPREngine::reconPlane(CW3ViewPlane* viewPlane,
	std::vector<glm::vec3>* TopRightCoord,
	bool isNerveDrawing, float fThickness)
{
	unsigned short* pPlaneBuffer = viewPlane->getImage2D()->getData();
	unsigned char* pMaskBuffer = viewPlane->getMask();
	unsigned short** ppVol = m_pgVol[0]->getData();

	const int iPlaneWidth = viewPlane->getWidth();
	const int iPlaneHeight = viewPlane->getHeight();
	const unsigned short nIntensityMin = m_pgVol[0]->getMin();
	const int nWidth = m_pgVol[0]->width();
	const int nHeight = m_pgVol[0]->height();
	const int nDepth = m_pgVol[0]->depth();

	const glm::vec3 vB = viewPlane->getBackVec();

	if (isNerveDrawing)
		memset(pMaskBuffer, 0, sizeof(unsigned char) * iPlaneWidth * iPlaneHeight);

	const int numThread = omp_get_max_threads();
	omp_set_num_threads(numThread);
#pragma omp parallel for
	for (int v = 0; v < iPlaneHeight; v++)
	{
		int rescaledX, rescaledY, rescaledZ;
		unsigned short* pBufferTemp = pPlaneBuffer + v * iPlaneWidth;
		unsigned char* pMaskTemp = pMaskBuffer + v * iPlaneWidth;
		for (int u = 0; u < iPlaneWidth; u++)
		{
			glm::vec3 ptVol = TopRightCoord->at(u) + vB * float(v);
			if (m_fSpacingZ[0] >= 1.0f)
			{
				rescaledX = static_cast<int>(ptVol.x);
				rescaledY = static_cast<int>(ptVol.y);
				rescaledZ = static_cast<int>(ptVol.z / m_fSpacingZ[0]);
				ptVol.z /= m_fSpacingZ[0];
			}
			else
			{
				rescaledX = static_cast<int>(ptVol.x * m_fSpacingZ[0]);
				rescaledY = static_cast<int>(ptVol.y * m_fSpacingZ[0]);
				rescaledZ = static_cast<int>(ptVol.z);
				ptVol.x *= m_fSpacingZ[0];
				ptVol.y *= m_fSpacingZ[0];
			}

			if ((ptVol.x >= 0 && ptVol.x <= nWidth - 1) &&
				(ptVol.y >= 0 && ptVol.y <= nHeight - 1) &&
				(ptVol.z >= 0 && ptVol.z <= nDepth - 1))
			{
				reconValue2Buf(pBufferTemp, u, ppVol, ptVol, nWidth, nHeight, nDepth);
			}
			else
				pBufferTemp[u] = nIntensityMin;
		}
	}
}

void CW3MPREngine::reconPlane(CW3ViewPlane* viewPlane,
	std::vector<glm::vec3>* TopRightCoord,
	std::vector<glm::vec3>* NormalUpToLower,
	bool isNerveDrawing, float fThickness)
{
	unsigned short* pPlaneBuffer = viewPlane->getImage2D()->getData();
	unsigned char* pMaskBuffer = viewPlane->getMask();
	unsigned short** ppVol = m_pgVol[0]->getData();

	const int iPlaneWidth = viewPlane->getWidth();
	const int iPlaneHeight = viewPlane->getHeight();
	const unsigned short nIntensityMin = m_pgVol[0]->getMin();
	const int nWidth = m_pgVol[0]->width();
	const int nHeight = m_pgVol[0]->height();
	const int nDepth = m_pgVol[0]->depth();
	const int nWPre = nWidth - 1;
	const int nHPre = nHeight - 1;
	const int nDPre = nDepth - 1;

	const glm::vec3 vB = viewPlane->getBackVec();

	if (isNerveDrawing)
		memset(pMaskBuffer, 0, sizeof(unsigned char) * iPlaneWidth * iPlaneHeight);

	const int nThickness = static_cast<int>(fThickness);

	std::vector<glm::vec3> farestTopRightCoord(iPlaneWidth);
	const float halfThickness = fThickness * 0.5f;
	for (int v = 0; v < iPlaneWidth; v++)
		farestTopRightCoord[v] =
		TopRightCoord->at(v) - NormalUpToLower->at(v) * halfThickness;

	int numThread = omp_get_max_threads();
	omp_set_num_threads(numThread);

#pragma omp parallel for
	for (int u = 0; u < iPlaneWidth; u++)
	{
		unsigned short* pBufferTemp = pPlaneBuffer + u;
		unsigned char* pMaskTemp = pMaskBuffer + u;
		const glm::vec3 pt1 = farestTopRightCoord.at(u);
		const glm::vec3 vUp = NormalUpToLower->at(u);

		for (int v = 0; v < iPlaneHeight; v++)
		{
			const glm::vec3 pt2 = pt1 + float(v) * vB;

			int tmpVal = 0;
			for (int d = 0; d < nThickness; d++)
			{
				glm::vec3 ptVol = pt2 + float(d) * vUp;
				if (m_fSpacingZ[0] >= 1.0f)
				{
					ptVol.z /= m_fSpacingZ[0];
				}
				else
				{
					ptVol.x *= m_fSpacingZ[0];
					ptVol.y *= m_fSpacingZ[0];
				}

				if ((ptVol.x >= 0 && ptVol.x <= nWPre) &&
					(ptVol.y >= 0 && ptVol.y <= nHPre) &&
					(ptVol.z >= 0 && ptVol.z <= nDPre))
				{
					reconValueSum(tmpVal, ppVol, ptVol, nWidth, nHeight, nDepth);
				}
				else
					tmpVal += nIntensityMin;
			}

			*(pBufferTemp) = tmpVal / nThickness;
			pBufferTemp += iPlaneWidth;
			pMaskTemp += iPlaneWidth;
		}
	}
}
