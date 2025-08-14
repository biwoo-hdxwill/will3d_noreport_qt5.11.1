#include "W3AutoArch.h"
#include <functional>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <QDebug>
#include <QVector2D>
#include <QRect>
#include <QString>

#include "Inc/ConnectedComponent.h"
#include "../../../Engine/Common/Common/common.h"
#include "../../../Engine/Resource/Resource/W3Image3D.h"

namespace
{
	const int kResolutionThreshold = 125000000;
	const int kDownScale = 2;
}

#define DEBUG 0
using namespace std;

CW3AutoArch::CW3AutoArch()
{
}

CW3AutoArch::~CW3AutoArch()
{
}

void CW3AutoArch::runMaxillaArch(const CW3Image3D* vol, std::vector<glm::vec3>& points, const int target_slice_number)
{
#if DEBUG
	FILE* file_;
#endif

	SliceLoc sliceLoc = vol->getSliceLoc();
	int noseSlice = sliceLoc.nose;

	ushort** volData = vol->getData();

	int nWidth = vol->width();
	int nHeight = vol->height();
	int nDepth = vol->depth();

	int nWH = nWidth*nHeight;

	//ushort boneThreshold = (ushort)((float)vol->windowCenter() - (float)vol->windowWidth()*0.08f);
	const ushort boneThreshold = vol->getTissueBoneThreshold();

	cv::Mat mandibleMask = cv::Mat(nHeight, nWidth, CV_16U);
	int nScale;

	int maxilla_slice_idx = (target_slice_number > -1) ? target_slice_number : sliceLoc.maxilla;
	//1. maxilla - 10mm에서 maxilla 위치까지 Axial plane에 projection한 bone mask를 구함.
	int maxilla_start = std::max(maxilla_slice_idx - (int)(15.0f / vol->sliceSpacing()), 0);

	// 속도 때문에 resolution이 큰 경우 다운스케일 한다.
	if (nWidth*nHeight*nDepth > kResolutionThreshold)
		nScale = kDownScale;
	else
		nScale = 1;

	int projWidth = (int)(((float)(nWidth) / (float)nScale) + 0.5f);
	int projHeight = (int)(((float)(nHeight) / (float)nScale) + 0.5f);

	cv::Mat projMask = cv::Mat(projHeight, projWidth, CV_16U);

	ushort* projMaskData = projMask.ptr<ushort>(0);
	memset(projMaskData, 0, sizeof(ushort)*projHeight*projWidth);

	for (int y = 0; y < projHeight; y++)
	{
		int iy = y*projWidth;
		for (int x = 0; x < projWidth; x++)
		{
			bool is_hit = false;
			for (int z = maxilla_start; z < maxilla_slice_idx; z++)
			{
				if (volData[z][(x + y*nWidth)*nScale] > boneThreshold)
				{
					is_hit = true;
					break;
				}
			}

			if (is_hit)
				*projMaskData++ = 1;
			else
				*projMaskData++ = 0;
		}
	}

	//다운 스케일 한 것을 원래 볼륨 사이즈로 되돌린다.
	if (nScale != 1)
		cv::resize(projMask, projMask, cv::Size(nWidth, nHeight));

#if DEBUG
	fopen_s(&file_, QString("%1_%2_projMask.raw").arg(nWidth).arg(nHeight).toStdString().c_str(), "wb");
	fwrite(projMask.ptr<ushort>(0), sizeof(ushort), nWidth*nHeight, file_);
	fclose(file_);
#endif

	//2. CCL을 사용해서 상악만 구함.
	int morph_size;
	cv::Mat element;

	morph_size = 1;
	element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2 * morph_size + 1, 2 * morph_size + 1), cv::Point(morph_size, morph_size));
	cv::morphologyEx(projMask, projMask, cv::MORPH_ERODE, element, cv::Point(-1, -1), 2);

#if DEBUG
	fopen_s(&file_, QString("%1_%2_projMask_eroded.raw").arg(nWidth).arg(nHeight).toStdString().c_str(), "wb");
	fwrite(projMask.ptr<ushort>(0), sizeof(ushort), nWidth*nHeight, file_);
	fclose(file_);
#endif

	cv::morphologyEx(projMask, projMask, cv::MORPH_CLOSE, element, cv::Point(-1, -1), 5);

#if DEBUG
	fopen_s(&file_, QString("%1_%2_projMask_eroded_closed.raw").arg(nWidth).arg(nHeight).toStdString().c_str(), "wb");
	fwrite(projMask.ptr<ushort>(0), sizeof(ushort), nWidth*nHeight, file_);
	fclose(file_);
#endif

	//CCL 실행.
	int dims[3] = { nWidth, nHeight, 1 };
	CW3ConnectedComponent ccl;

	cv::Mat maxillaMask = cv::Mat(nHeight, nWidth, CV_16U);

	// 상위 두개의 덩어리만 남김. (목 뼈와 하악 뼈)
	ccl.CountConnectedComponentsWithAllParams(projMask.ptr<ushort>(0), maxillaMask.ptr<ushort>(0), dims, 1.0, 8, 1, 2);

	// 상위 두개의 덩어리 중 y좌표가 최소가 되는 것을 상악으로 설정.
	ushort* maxillaMaskData = maxillaMask.ptr<ushort>(0);

	int minY_1, minY_2;
	minY_1 = minY_2 = nHeight;
	for (int y = 0; y < nHeight; y++)
	{
		for (int x = 0; x < nWidth; x++)
		{
			if (*maxillaMaskData == 1 && minY_1 > y)
			{
				minY_1 = y;
			}
			else if (*maxillaMaskData == 2 && minY_2 > y)
			{
				minY_2 = y;
			}

			*maxillaMaskData++;
		}
	}
	int selectedLabel = (minY_1 <= minY_2) ? 1 : 2;

	maxillaMaskData = maxillaMask.ptr<ushort>(0);
	for (int xy = 0; xy < nWH; xy++)
	{
		if (*maxillaMaskData != selectedLabel)
			*maxillaMaskData = 0;
		*maxillaMaskData++;
	}

#if DEBUG
	fopen_s(&file_, QString("%1_%2_maxillaMask.raw").arg(nWidth).arg(nHeight).toStdString().c_str(), "wb");
	fwrite(maxillaMask.ptr<ushort>(0), sizeof(ushort), nWidth*nHeight, file_);
	fclose(file_);
#endif

	//3. maxilla slice 위치의 bone mask를 구함.
	cv::Mat archMask = cv::Mat(nHeight, nWidth, CV_16U);

	// 구해진 슬라이스 위치의 bone mask를 구하고 (2)에서 구한 상악 마스크와 곱하여 Arch Mask로 만든다.
	ushort* archMaskData = archMask.ptr<ushort>(0);
	memset(archMaskData, 0, sizeof(ushort)*nWH);

	maxillaMaskData = maxillaMask.ptr<ushort>(0);

	ushort* pData = volData[maxilla_slice_idx];
	for (int i = 0; i < nWH; i++)
	{
		ushort intensity = *pData++;

		if (*maxillaMaskData++ && (intensity > boneThreshold))
			*archMaskData = 1;

		*archMaskData++;
	}

#if DEBUG
	fopen_s(&file_, QString("%1_%2_archMask.raw").arg(nWidth).arg(nHeight).toStdString().c_str(), "wb");
	fwrite(archMask.ptr<ushort>(0), sizeof(ushort), nWidth*nHeight, file_);
	fclose(file_);
#endif

	//4. close 연산으로 arch 형태로 만듬.
	morph_size = (nScale != 1) ? 2 : 1;

	element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2 * morph_size + 1, 2 * morph_size + 1), cv::Point(morph_size, morph_size));
	cv::morphologyEx(archMask, archMask, cv::MORPH_CLOSE, element, cv::Point(-1, -1), 12);

#if DEBUG
	fopen_s(&file_, QString("%1_%2_archMask_closed.raw").arg(nWidth).arg(nHeight).toStdString().c_str(), "wb");
	fwrite(archMask.ptr<ushort>(0), sizeof(ushort), nWidth*nHeight, file_);
	fclose(file_);
#endif

	//6. arch 마스크를 이용해서 point를 구함.
	std::vector<QPointF> pts;
	setArchPoints(archMask.ptr<ushort>(0), nWidth, nHeight, vol->pixelSpacing(), pts, 5.0f, 2);

	std::vector<QPointF> archPoints;
	correctPoints(volData, maxilla_slice_idx, noseSlice, nWidth, nHeight, boneThreshold, vol->sliceSpacing(), pts);

	for (const auto& pt : pts)
		archPoints.push_back(pt);

	//7. spline을 15mm씩 spampling해서 point를 구함.
	samplingPoints(15.0f / vol->pixelSpacing(), archPoints);

	for (const auto& pt : archPoints)
		points.push_back(glm::vec3(pt.x(), pt.y(), maxilla_slice_idx));
}

void CW3AutoArch::runMandibleArch(const CW3Image3D* vol, std::vector<glm::vec3>& points, const int target_slice_number)
{
#if DEBUG
	FILE* file_;
#endif

	int nWidth = vol->width();
	int nHeight = vol->height();
	int nDepth = vol->depth();
	int mandible_slice_idx = 0;
	ushort** volData = vol->getData();
	const ushort boneThreshold = vol->getTissueBoneThreshold();

	cv::Mat mandibleMask = cv::Mat(nHeight, nWidth, CV_16U);
	const int nWH = nWidth * nHeight;

	cv::Mat archMask = cv::Mat(nHeight, nWidth, CV_16U);
	if (target_slice_number > -1)
	{
		mandible_slice_idx = target_slice_number;

		findMandible(vol, &mandible_slice_idx, &mandibleMask, true);
	}
	else
	{
		mandible_slice_idx = vol->getSliceLoc().teeth;

		findMandible(vol, &mandible_slice_idx, &mandibleMask, false);

#if DEBUG
		fopen_s(&file_, QString("%1_%2_mandibleMask.raw").arg(nWidth).arg(nHeight).toStdString().c_str(), "wb");
		fwrite(mandibleMask.ptr(0), sizeof(ushort), nWidth * nHeight, file_);
		fclose(file_);
#endif

		SliceLoc sliceLoc = vol->getSliceLoc();
		if (sliceLoc.segment_maxilla_mandible)
			mandible_slice_idx = std::min(nDepth - 1, mandible_slice_idx + (int)(2.5f / vol->sliceSpacing()));
		else
			mandible_slice_idx = std::min(nDepth - 1, mandible_slice_idx + (int)(5.0f / vol->sliceSpacing()));
	}

	// 구해진 슬라이스 위치의 bone mask를 구하고 (2)에서 구한 하악 마스크와 곱하여 Arch Mask로 만든다.
	ushort* archMaskData = archMask.ptr<ushort>(0);
	memset(archMaskData, 0, sizeof(ushort)*nWH);

	ushort* mandibleMaskData = mandibleMask.ptr<ushort>(0);
	ushort* pData = volData[mandible_slice_idx];
	for (int i = 0; i < nWH; ++i)
	{
		ushort intensity = *pData++;

		if (*mandibleMaskData++ && (intensity > boneThreshold))
		{
			*archMaskData = 1;
		}

		*archMaskData++;
	}

#if DEBUG
	fopen_s(&file_, QString("%1_%2_archMask.raw").arg(nWidth).arg(nHeight).toStdString().c_str(), "wb");
	fwrite(archMask.ptr(0), sizeof(ushort), nWidth*nHeight, file_);
	fclose(file_);
#endif

	//4. close 연산으로 arch 형태로 만듬.
	int morph_size;
	cv::Mat element;
	const int nScale = (nWidth*nHeight*nDepth > kResolutionThreshold) ? kDownScale : 1;
	morph_size = (nScale != 1) ? 2 : 1;

	element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2 * morph_size + 1, 2 * morph_size + 1), cv::Point(morph_size, morph_size));
	cv::morphologyEx(archMask, archMask, cv::MORPH_CLOSE, element, cv::Point(-1, -1), 12);

#if DEBUG
	fopen_s(&file_, QString("%1_%2_archMask_closed.raw").arg(nWidth).arg(nHeight).toStdString().c_str(), "wb");
	fwrite(archMask.ptr(0), sizeof(ushort), nWidth*nHeight, file_);
	fclose(file_);
#endif

	//6. arch 마스크를 이용해서 point를 구함.
	std::vector<QPointF> pts;
	setArchPoints(archMask.ptr<ushort>(0), nWidth, nHeight, vol->pixelSpacing(), pts, 10.0f);

	int teethEnd = std::min(nDepth - 1, mandible_slice_idx + (int)(20.0f / vol->sliceSpacing()));
	correctPoints(volData, mandible_slice_idx, teethEnd, nWidth, nHeight, boneThreshold, vol->sliceSpacing(), pts);

	std::vector<QPointF> archPoints;
	for (const auto& pt : pts)
		archPoints.push_back(pt);

	//7. spline을 15mm씩 spampling해서 point를 구함.
	samplingPoints(15.0f / vol->pixelSpacing(), archPoints);

	for (const auto& pt : archPoints)
	{
		points.push_back(glm::vec3(pt.x(), pt.y(), mandible_slice_idx));
	}
}
/*
void CW3AutoArch::runFreeFOVArch(const CW3Image3D* vol, std::vector<glm::vec3>& points)
{
	//int nWidth = vol->width();
	//int nHeight = vol->height();
	//int nDepth = vol->depth();
	//int slice_idx = (int)((float)vol->depth()*0.5f);
	//ushort** volData = vol->getData();
	//const ushort boneThreshold = vol->getTissueBoneThreshold();

	//cv::Mat mandibleMask = cv::Mat(nHeight, nWidth, CV_16U);
	//const int nWH = nWidth * nHeight;

	//// 구해진 슬라이스 위치의 bone mask를 구하고 (2)에서 구한 하악 마스크와 곱하여 Arch Mask로 만든다.
	//cv::Mat archMask = cv::Mat(nHeight, nWidth, CV_16U);
	//ushort* archMaskData = archMask.ptr<ushort>(0);
	//memset(archMaskData, 0, sizeof(ushort)*nWH);

	//ushort* mandibleMaskData = mandibleMask.ptr<ushort>(0);
	//ushort* pData = volData[slice_idx];
	//for (int i = 0; i < nWH; i++) {
	//	ushort intensity = *pData++;

	//	if (*mandibleMaskData++ && (intensity > boneThreshold))
	//		*archMaskData = 1;

	//	*archMaskData++;
	//}

	//int morph_size;
	//cv::Mat element;

	//morph_size = 1;
	//element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2 * morph_size + 1, 2 * morph_size + 1), cv::Point(morph_size, morph_size));
	//cv::morphologyEx(archMask, archMask, cv::MORPH_CLOSE, element, cv::Point(-1, -1), 12);
}
float CW3AutoArch::getTeethSliceNum(const float * depthMapTeeth, const int width, const int height,
	const float teethInterval)
{
	int centerx = width / 2;
	std::map<int, float> depth;

	int roi = 5;
	int roi_rad = roi / 2;

	for (int i = 0; i < roi; i++)
	{
		depth[i] = std::numeric_limits<float>::max();
	}

	for (int y = 0; y < height; y++)
	{
		for (auto& elem : depth)
		{
			float dt = depthMapTeeth[y*width + centerx + elem.first - roi_rad];
			if (dt > 0.0001f)
			{
				if (dt < elem.second)
					elem.second = dt;
			}
		}
	}

	float depthLoc = depth[0];
	float avgDepth = 0.0;
	for (const auto& elem : depth) {
		avgDepth += elem.second;
	}
	avgDepth /= depth.size();

	for (const auto& elem : depth)
	{
		if (std::abs(elem.second - avgDepth) < teethInterval)
			depthLoc = (elem.second > depthLoc) ? depthLoc : elem.second;
	}

	return depthLoc + teethInterval;
}*/
void CW3AutoArch::findMandible(const CW3Image3D* vol, int* mandible_slice_idx, cv::Mat* mandible_mask, const bool use_fixed_target_slice)
{
	int nWidth = vol->width();
	int nHeight = vol->height();
	int nDepth = vol->depth();

	//1. 볼륨 끝에서 teeth 위치까지 Axial plane에 projection한 bone mask를 구함.

	// 속도 때문에 resolution이 큰 경우 다운스케일 한다.
	const int nScale = (nWidth*nHeight*nDepth > kResolutionThreshold) ? kDownScale : 1;

	int projWidth = (int)(((float)(nWidth) / (float)nScale) + 0.5f);
	int projHeight = (int)(((float)(nHeight) / (float)nScale) + 0.5f);

	cv::Mat projMask = cv::Mat(projHeight, projWidth, CV_16U);
	ushort* projMaskData = projMask.ptr<ushort>(0);
	memset(projMaskData, 0, sizeof(ushort)*projHeight*projWidth);

	ushort** volData = vol->getData();
	const ushort boneThreshold = vol->getTissueBoneThreshold();
	const int teethSlice = vol->getSliceLoc().teeth;
	for (int y = 0; y < projHeight; y++)
	{
		int iy = y*projWidth;
		for (int x = 0; x < projWidth; x++)
		{
			bool is_hit = false;
			for (int z = nDepth - 1; z >= teethSlice; z--)
			{
				if (volData[z][(x + y*nWidth)*nScale] > boneThreshold)
				{
					is_hit = true;
					break;
				}
			}

			if (is_hit)
				*projMaskData++ = 1;
			else
				*projMaskData++ = 0;
		}
	}

	//다운 스케일 한 것을 원래 볼륨 사이즈로 되돌린다.
	if (nScale != 1)
		cv::resize(projMask, projMask, cv::Size(nWidth, nHeight));

	//2. CCL을 사용해서 하악만 구함.

#if DEBUG
	FILE* file_;
	fopen_s(&file_, QString("%1_%2_0.raw").arg(nWidth).arg(nHeight).toStdString().c_str(), "wb");
	fwrite(projMask.ptr<ushort>(0), sizeof(ushort), nWidth*nHeight, file_);
	fclose(file_);
#endif

	//CCL 실행.
	int dims[3] = { nWidth, nHeight, 1 };

	// 상위 두개의 덩어리만 남김. (목 뼈와 하악 뼈)
	CW3ConnectedComponent ccl;
	ccl.CountConnectedComponentsWithAllParams(projMask.ptr<ushort>(0), mandible_mask->ptr<ushort>(0), dims, 1.0, 8, 1, 2);

	// 상위 두개의 덩어리 중 y좌표가 최소가 되는 것을 하악으로 설정.
	ushort* mandibleMaskData = mandible_mask->ptr<ushort>(0);

	int minY_1 = nHeight, minY_2 = nHeight;
	for (int y = 0; y < nHeight; y++)
	{
		for (int x = 0; x < nWidth; x++)
		{
			if (*mandibleMaskData == 1 && minY_1 > y)
			{
				minY_1 = y;
			}
			else if (*mandibleMaskData == 2 && minY_2 > y)
			{
				minY_2 = y;
			}

			*mandibleMaskData++;
		}
	}
	int selectedLabel = (minY_1 <= minY_2) ? 1 : 2;

	const int nWH = nWidth * nHeight;
	mandibleMaskData = mandible_mask->ptr<ushort>(0);
	for (int xy = 0; xy < nWH; xy++)
	{
		if (*mandibleMaskData != selectedLabel)
			*mandibleMaskData = 0;
		*mandibleMaskData++;
	}

#if DEBUG
	fopen_s(&file_, QString("%1_%2_1.raw").arg(nWidth).arg(nHeight).toStdString().c_str(), "wb");
	fwrite(mandible_mask->ptr<ushort>(0), sizeof(ushort), nWidth*nHeight, file_);
	fclose(file_);
#endif

	//3. mandible slice 위치의 bone mask를 구함.
	cv::Mat archMask = cv::Mat(nHeight, nWidth, CV_16U);

	//mandible slice 위치를 찾기 위해서 teethSlice에서 20mm 아래까지 조사한다.
	//조사는 (2)에서 구한 하악 마스크와 조사 대상인 slice를 곱해서 가장 많은 intensity를 가진 slice를 선택한다.
	int target_slice = use_fixed_target_slice ? *mandible_slice_idx : teethSlice;
	int teethEnd = std::min(nDepth - 1, target_slice + (int)(20.0f / vol->sliceSpacing()));
	int teethStart = std::min(nDepth - 1, target_slice + (int)(3.5f / vol->sliceSpacing()));

	std::vector<float> sliceSumIntensity;
	ushort* pData;
	float maxIntensity = ((float)vol->windowCenter() + (float)vol->windowWidth()*0.5f);
	ushort teethThreshold = (ushort)((float)vol->getTissueBoneThreshold() + (maxIntensity - (float)vol->getTissueBoneThreshold())*0.15f);

	for (int z = teethStart; z < teethEnd; z++)
	{
		mandibleMaskData = mandible_mask->ptr<ushort>(0);
		pData = volData[z];
		float bones = 0.0f;
		for (int i = 0; i < nWH; i++)
		{
			float intensity = *pData++;
			intensity = (maxIntensity < intensity) ? maxIntensity : intensity;
			intensity = exp((float)(intensity - teethThreshold) / (maxIntensity - teethThreshold));

			if (*mandibleMaskData++)
				bones += intensity;
		}
		sliceSumIntensity.push_back(bones);
	}

	if (!use_fixed_target_slice)
	{
		float sliceMaxIntensity = 0.0f;
		for (int i = 0; i < sliceSumIntensity.size(); i++)
		{
			if (sliceMaxIntensity < sliceSumIntensity[i])
			{
				sliceMaxIntensity = sliceSumIntensity[i];
				*mandible_slice_idx = teethStart + i;
			}
		}
	}
}
void CW3AutoArch::setLinePoints(const ushort * line_mask, const int width, const int height, const float pixelSpacing,
	std::vector<QPointF>& points, float margin_mm, int sample_iter)
{
	try
	{
		//1. ROI rect 설정.
		QPoint leftTop = QPoint(width - 1, height - 1);
		QPoint rightBot = QPoint(0, 0);

		for (int j = 0; j < height; j++)
		{
			for (int i = 0; i < width; i++)
			{
				if (line_mask[j*width + i])
				{
					leftTop.setX(std::min(i, leftTop.x()));
					leftTop.setY(std::min(j, leftTop.y()));

					rightBot.setX(std::max(i, rightBot.x()));
					rightBot.setY(std::max(j, rightBot.y()));
				}
			}
		}

		QRect rect = QRect(leftTop, rightBot);

		int h = (int)(sqrt(rect.width()*rect.width() + rect.height()*rect.height()) * 0.25f);

		std::vector<QPointF> erasePoints;

		//2. 두 점을 받고 두 점의 ortho 방향으로 mask의 중앙 point를 리턴.
		auto findCenterOfMass = [&](const QPointF& p1, const QPointF& p2) -> QPointF
		{
			QVector2D vec1 = QVector2D(p2) - QVector2D(p1);

			QVector2D ortho = QVector2D(vec1.y(), -vec1.x());
			ortho.normalize();

			QVector2D cur = vec1*0.5f + QVector2D(p1) - ortho*h*0.5f;
			QPointF center = QPointF(0, 0);
			int count = 0;
			for (int i = 0; i < h; i++)
			{
				int x = (int)cur.x();
				int y = (int)cur.y();

				x = (x < 0) ? 0 : (x > width - 1) ? (width - 1) : x;
				y = (y < 0) ? 0 : (y > height - 1) ? (height - 1) : y;

				int idx = width*y + x;
				if (line_mask[idx])
				{
					center += QPointF(x, y);
					++count;
				}
				cur += ortho;
			}

			if (count == 0)
			{
				QPointF p = (p1 + p2) / 2.0f;
				erasePoints.push_back(p);
				return p;

				//throw exception("Incorrect generated arch."); //  mask is not connected or empty
			}

			center /= count;

			return QPointF(center.x(), center.y());
		};

		//포인트 초기화. ROI의 좌하단 포인트, 중앙상단 포인트, 우하단의 포인트를 가지고 두개의 Pair Point를 만듬.
		//Pair 1은 ( 좌하단, 중앙상단 )이고 Pair 2는 ( 중앙상단, 좌하단)
		std::vector<PairPoint> pp;
		pp.push_back(PairPoint(QPointF(leftTop.x(), rightBot.y()), QPointF(leftTop.x() + rect.width()*0.5f, leftTop.y())));
		pp.push_back(PairPoint(QPointF(leftTop.x() + rect.width()*0.5f, leftTop.y()), QPointF(rightBot.x(), rightBot.y())));

		erasePoints.push_back(pp.front().p1);
		erasePoints.push_back(pp.back().p2);

		//iter를 돌면서 pair point를 나눔.
		for (int i = 0; i < sample_iter; i++)
		{
			std::vector<PairPoint> ap;
			for (const auto& elem : pp)
			{
				QPointF findPoint = findCenterOfMass(elem.p1, elem.p2);
				ap.push_back(PairPoint(elem.p1, findPoint));
				ap.push_back(PairPoint(findPoint, elem.p2));
			}
			pp.assign(ap.begin(), ap.end());
		}

		//초기화 할 때 주어졌던 중앙상단 포인트를 mask의 중앙으로 이동.
		int idxC = (int)(pp.size() / 2);

		QPointF ptCenter = pp[idxC].p1;
		QPointF ptCorrectCenter = findCenterOfMass(ptCenter - QPointF(1.0, 0.0), ptCenter + QPointF(1.0, 0.0));

		pp[idxC].p1 = ptCorrectCenter;
		pp[idxC - 1].p2 = ptCorrectCenter;

		//pp.erase(pp.begin());
		//pp.pop_back();
		//marginPoint는 구해진 arch point에서 10mm만큼 연장한 점을 할당.
		auto marginPoint = [](const QPointF& extreamPoint, const QPointF& prevPoint, const float& margin) -> QPointF
		{
			QVector2D vec = QVector2D(extreamPoint) - QVector2D(prevPoint);
			vec.normalize();
			vec *= margin;

			QVector2D extream = QVector2D(extreamPoint) + vec;

			return QPointF(extream.x(), extream.y());
		};

		if (margin_mm != 0.0f)
		{
			float margin_in_vol = margin_mm / pixelSpacing; //10mm

			QPointF firstMarginPoint = marginPoint(pp.front().p1, pp.front().p2, margin_in_vol);
			pp.insert(pp.begin(), PairPoint(firstMarginPoint, pp.front().p1));

			QPointF lastMarginPoint = marginPoint(pp.back().p2, pp.back().p1, margin_in_vol);
			pp.insert(pp.end(), PairPoint(pp.back().p2, lastMarginPoint));
		}

		for (auto & elem : pp)
		{
			if (std::find(points.begin(), points.end(), elem.p1) == points.end() &&
				std::find(erasePoints.begin(), erasePoints.end(), elem.p1) == erasePoints.end())
				points.push_back(elem.p1);

			if (std::find(points.begin(), points.end(), elem.p2) == points.end() &&
				std::find(erasePoints.begin(), erasePoints.end(), elem.p2) == erasePoints.end())
				points.push_back(elem.p2);
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "AutoArch::setLinePoints: " << e.what() << std::endl;
		throw e;
	}
}
void CW3AutoArch::setArchPoints(const ushort * archMask, const int width, const int height, const float pixelSpacing,
	std::vector<QPointF>& points, float margin_mm, int sample_iter)
{
	try
	{
		//1. ROI rect 설정.
		QPoint leftTop = QPoint(width - 1, height - 1);
		QPoint rightBot = QPoint(0, 0);

		for (int j = 0; j < height; j++)
		{
			for (int i = 0; i < width; i++)
			{
				if (archMask[j*width + i])
				{
					leftTop.setX(std::min(i, leftTop.x()));
					leftTop.setY(std::min(j, leftTop.y()));

					rightBot.setX(std::max(i, rightBot.x()));
					rightBot.setY(std::max(j, rightBot.y()));
				}
			}
		}

		QRect rect = QRect(leftTop, rightBot);

		int h = (int)(rect.width()*0.25f);
		std::vector<QPointF> erasePoints;

		//2. 두 점을 받고 두 점의 ortho 방향으로 mask의 중앙 point를 리턴.
		auto findCenterOfMass = [&](const QPointF& p1, const QPointF& p2) -> QPointF
		{
			QVector2D vec1 = QVector2D(p2) - QVector2D(p1);

			QVector2D ortho = QVector2D(vec1.y(), -vec1.x());
			ortho.normalize();

			QVector2D cur = vec1*0.5f + QVector2D(p1) - ortho*h*0.5f;
			QPointF center = QPointF(0, 0);
			int count = 0;
			for (int i = 0; i < h; i++)
			{
				int x = (int)cur.x();
				int y = (int)cur.y();

				x = (x < 0) ? 0 : (x > width - 1) ? (width - 1) : x;
				y = (y < 0) ? 0 : (y > height - 1) ? (height - 1) : y;

				int idx = width*y + x;
				if (archMask[idx])
				{
					center += QPointF(x, y);
					++count;
				}
				cur += ortho;
			}

			if (count == 0)
			{
				QPointF p = (p1 + p2) / 2.0f;
				erasePoints.push_back(p);
				return p;

				//throw exception("Incorrect generated arch."); //  mask is not connected or empty
			}

			center /= count;

			return QPointF(center.x(), center.y());
		};

		//포인트 초기화. ROI의 좌하단 포인트, 중앙상단 포인트, 우하단의 포인트를 가지고 두개의 Pair Point를 만듬.
		//Pair 1은 ( 좌하단, 중앙상단 )이고 Pair 2는 ( 중앙상단, 좌하단)
		std::vector<PairPoint> pp;
		pp.push_back(PairPoint(QPointF(leftTop.x(), rightBot.y()), QPointF(leftTop.x() + rect.width()*0.5f, leftTop.y())));
		pp.push_back(PairPoint(QPointF(leftTop.x() + rect.width()*0.5f, leftTop.y()), QPointF(rightBot.x(), rightBot.y())));

		erasePoints.push_back(pp.front().p1);
		erasePoints.push_back(pp.back().p2);

		//iter를 돌면서 pair point를 나눔.
		for (int i = 0; i < sample_iter; i++)
		{
			std::vector<PairPoint> ap;
			for (const auto& elem : pp)
			{
				QPointF findPoint = findCenterOfMass(elem.p1, elem.p2);
				ap.push_back(PairPoint(elem.p1, findPoint));
				ap.push_back(PairPoint(findPoint, elem.p2));
			}
			pp.assign(ap.begin(), ap.end());
		}

		//초기화 할 때 주어졌던 중앙상단 포인트를 mask의 중앙으로 이동.
		int idxC = (int)(pp.size() / 2);

		QPointF ptCenter = pp[idxC].p1;
		QPointF ptCorrectCenter = findCenterOfMass(ptCenter - QPointF(1.0, 0.0), ptCenter + QPointF(1.0, 0.0));

		pp[idxC].p1 = ptCorrectCenter;
		pp[idxC - 1].p2 = ptCorrectCenter;

		//pp.erase(pp.begin());
		//pp.pop_back();
		//marginPoint는 구해진 arch point에서 10mm만큼 연장한 점을 할당.
		auto marginPoint = [](const QPointF& extreamPoint, const QPointF& prevPoint, const float& margin) -> QPointF
		{
			QVector2D vec = QVector2D(extreamPoint) - QVector2D(prevPoint);
			vec.normalize();
			vec *= margin;

			QVector2D extream = QVector2D(extreamPoint) + vec;

			return QPointF(extream.x(), extream.y());
		};

		if (margin_mm != 0.0f)
		{
			float margin_in_vol = margin_mm / pixelSpacing; //10mm

			QPointF firstMarginPoint = marginPoint(pp.front().p1, pp.front().p2, margin_in_vol);
			pp.insert(pp.begin(), PairPoint(firstMarginPoint, pp.front().p1));

			QPointF lastMarginPoint = marginPoint(pp.back().p2, pp.back().p1, margin_in_vol);
			pp.insert(pp.end(), PairPoint(pp.back().p2, lastMarginPoint));
		}

		for (auto & elem : pp)
		{
			if (std::find(points.begin(), points.end(), elem.p1) == points.end() &&
				std::find(erasePoints.begin(), erasePoints.end(), elem.p1) == erasePoints.end())
				points.push_back(elem.p1);

			if (std::find(points.begin(), points.end(), elem.p2) == points.end() &&
				std::find(erasePoints.begin(), erasePoints.end(), elem.p2) == erasePoints.end())
				points.push_back(elem.p2);
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "AutoArch::setArchPoints: " << e.what() << std::endl;
		throw e;
	}
}

bool CW3AutoArch::isTwistedInsertPoint(const std::vector<QPointF>& points, int insert_idx, QPointF insert_point)
{
	for (int i = 0; i < insert_idx; i++)
	{
		double sign = points[i].x() - insert_point.x();
		if (sign < 0.0f)
			return true;
	}
	return false;
}

void CW3AutoArch::correctPoints(ushort** volData, const int startZ, const int endZ, const int width, const int height, const int boneThreshold, const float sliceSpacing, std::vector<QPointF>& points)
{
	// 점과 점 사이 길이가 15mm 이상 된다면 Arch Mask가 끊어져 있다고 판정.(SetPoint에서 충분히 많은 점을 뽑았기 때문에 끊어진 컴포넌트)
	// startZ 슬라이스부터 endZ 슬라이스까지 조사하는데, 현재 슬라이스에서 점과 점의 Ortho방향으로 bone을 조사. bone이 있으면 그 위치에 point를 추가한다.

	float length;
	float pointGap = 15.0f / sliceSpacing;
	float h = 40.0f / sliceSpacing;
	int maxIter = 20;

	auto comp = [](const int&a, const int&b) { return a < b; };

	for (int k = 0; k < maxIter; k++)
	{
		auto mapPairPoint = std::map<int, PairPoint, std::function<bool(const int&, const int&)>>{
			[](const int& a, const int& b)
 {
return a > b;
}
		};

		for (int i = 0; i < (int)points.size() - 1; i++)
		{
			QVector2D v = QVector2D(points[i + 1]) - QVector2D(points[i]);
			length = sqrt(v.x()*v.x() + v.y()*v.y());

			if (length > pointGap)
			{
				mapPairPoint[i + 1] = (PairPoint(points[i], points[i + 1]));
			}
		}

		if (mapPairPoint.size() == 0)
			break;

		bool isInsert = false;
		int insert_cnt = 0;

		auto func_insert = [&](const QVector2D& v, const PairPoint& pp, const int& z, const int& idx) -> bool
		{
			QVector2D ortho = QVector2D(v.y(), -v.x());
			ortho.normalize();

			QVector2D cur = (QVector2D(pp.p2) + QVector2D(pp.p1))*0.5f - ortho*h*0.5f;

			int count = 0;
			QPointF center = QPointF(0, 0);
			for (int i = 0; i < h; i++)
			{
				int x = (int)cur.x();
				int y = (int)cur.y();

				x = (x < 0) ? 0 : (x > width - 1) ? (width - 1) : x;
				y = (y < 0) ? 0 : (y > height - 1) ? (height - 1) : y;

				int xy = width*y + x;
				if (volData[z][xy] > boneThreshold)
				{
					center += QPointF(x, y);
					++count;
				}
				cur += ortho;
			}

			if (count != 0 && !isTwistedInsertPoint(points, idx, center))
			{
				center /= count;
				points.insert(points.begin() + idx, center);
				isInsert = true;
				return true;
			}

			return false;
		};

		for (auto iter = mapPairPoint.rbegin(); iter != mapPairPoint.rend(); iter++)
		{
			int idx = iter->first + insert_cnt;
			PairPoint pp = iter->second;

			QVector2D v = QVector2D(pp.p2) - QVector2D(pp.p1);
			if (startZ < endZ)
			{
				for (int z = startZ; z < endZ; z++)
				{
					if (func_insert(v, pp, z, idx))
					{
						insert_cnt++;
						break;
					}
				}
			}
			else
			{
				for (int z = startZ; z > endZ; z--)
				{
					if (func_insert(v, pp, z, idx))
					{
						insert_cnt++;
						break;
					}
				}
			}
		}

		if (!isInsert)
			break;
	}
}

void CW3AutoArch::samplingPoints(const float divideLen, std::vector<QPointF>& points)
{
	//std::vector<QPointF> splinePoints;
	//
	//Common::generateCubicSpline(points, splinePoints);
	//
	//std::vector<QPointF> equiSpliPoints;
	//
	//equiSpliPoints.push_back(splinePoints.front());
	//
	//for (int i = 1; i < splinePoints.size(); i++)
	//{
	//	QPointF p0 = equiSpliPoints.back();
	//	QPointF p1 = splinePoints[i];
	//
	//	QPointF v = p1 - p0;
	//	int length = sqrt(v.x()*v.x() + v.y()*v.y());
	//
	//	v /= length;
	//
	//	for (int j = 0; j < length; j++)
	//		equiSpliPoints.push_back(p0 + v*((float)j + 1));
	//}

	std::vector<QPointF> equiSpliPoints;
	equiSpliPoints.push_back(points.front());
	for (int i = 1; i < points.size(); i++)
	{
		QPointF p0 = equiSpliPoints.back();
		QPointF p1 = points[i];

		QPointF v = p1 - p0;
		int length = sqrt(v.x()*v.x() + v.y()*v.y());

		v /= length;

		for (int j = 0; j < length; j++)
			equiSpliPoints.push_back(p0 + v*((float)j + 1));
	}

	if (equiSpliPoints.size() > divideLen)
	{
		float splitStep;
		splitStep = divideLen;

		points.clear();

		for (int i = 0; i < equiSpliPoints.size(); i += splitStep)
			points.push_back(equiSpliPoints[i]);

		QPointF tailVec = points.back() - equiSpliPoints.back();
		int tailLen = sqrt(tailVec.x()*tailVec.x() + tailVec.y()*tailVec.y());
		if (tailLen <= splitStep)
		{
			points.pop_back();
			points.push_back((points.back() + equiSpliPoints.back())*0.5f);
		}

		points.push_back(equiSpliPoints.back());
	}
}
