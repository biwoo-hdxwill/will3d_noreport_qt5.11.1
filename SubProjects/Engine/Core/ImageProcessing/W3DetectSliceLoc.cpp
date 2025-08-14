#include "W3DetectSliceLoc.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QString>

#if defined(__APPLE__)
#include </usr/local/Cellar/llvm/5.0.0/lib/clang/5.0.0/include/omp.h>
#else
#include <omp.h>
#endif

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "../../Common/Common/W3Logger.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "Inc/VpConnComponent.h"

#define FIND_CHIN_RADIUS 10
#define SCALE 4
#define PXIEL_SPACING_LIMIT 0.30
#define DEBUG 0

int CW3DetectSliceLoc::FindChinSliceLocation(CW3Image3D* volume)
{
	QElapsedTimer timer;
	timer.start();

	float slice_spacing = volume->sliceSpacing();
	int bone_threshold = volume->getTissueBoneThreshold();

	int scale = (slice_spacing < PXIEL_SPACING_LIMIT) ? SCALE : 1;
	int bone_size = (int)((float)volume->width() * volume->height()) / (float)scale;
	int depth = (int)((float)volume->depth() / scale + 0.5f);

	std::vector<std::vector<uchar>> bone_mask;
	bone_mask.resize(depth);
	
	int num_thread = omp_get_max_threads();
	omp_set_num_threads(num_thread);

#pragma omp parallel for
	for (int z = 0; z < volume->depth(); z += scale)
	{
		int mz = static_cast<int>((float)z / scale);
		if (mz >= depth)
		{
			break;
		}

		bone_mask[mz].reserve(bone_size);

		for (int y = 0; y < volume->height(); y += scale)
		{
			ushort* slice = &(volume->getData())[z][y * volume->width()];
			for (int x = 0; x < volume->width(); x += scale)
			{

				if (*slice > bone_threshold)
				{
					bone_mask[mz].push_back(1);
				}
				else
				{
					bone_mask[mz].push_back(0);
				}

				slice += scale;
			}
		}
	}

	int width = (int)((float)volume->width() / scale + 0.5f);
	int chin_slice_location = findChinSlice(bone_mask, width, volume->pixelSpacing(), slice_spacing, scale);

	if (chin_slice_location > depth - 1 || chin_slice_location < 0)
	{
		chin_slice_location = depth - 1;
	}

	chin_slice_location *= scale;

	common::Logger::instance()->Print(common::LogType::INF, "FindChinSliceLocation : " + QString::number(timer.elapsed()).toStdString() + " ms");

	return chin_slice_location;
}

void CW3DetectSliceLoc::run(const CW3Image3D* vol, int thdTissue, int thdBone, SliceLoc* sliceLoc)
{
	int method = 0;

	FindSliceLocation(vol, thdTissue, thdBone, sliceLoc, method);
}

void CW3DetectSliceLoc::FindSliceLocation(const CW3Image3D* vol, int tissue_threshold, int bone_threshold, SliceLoc* slice_location, int method)
{
	QElapsedTimer timer;
	timer.start();

	float sliceSpacing = vol->sliceSpacing();

	int nScale = (sliceSpacing < PXIEL_SPACING_LIMIT) ? SCALE : 1;
	int nBoneSize = (int)((float)vol->width()*vol->height()) / (float)nScale;
	int nDepth = (int)((float)vol->depth() / nScale + 0.5f);

	// 1. 두개골 영역을 마스크로 만듬
	std::vector<std::vector<uchar>> boneMask;
	boneMask.resize(nDepth);

	int num_thread = omp_get_max_threads();
	omp_set_num_threads(num_thread);

#pragma omp parallel for
	for (int z = 0; z < vol->depth(); z += nScale)
	{
		int mz = static_cast<int>((float)z / nScale);
		if (mz >= nDepth)
		{
			break;
		}
		boneMask[mz].reserve(nBoneSize);

		for (int y = 0; y < vol->height(); y += nScale)
		{
			int yIdx = y * vol->width();
			ushort* slice = &(vol->getData())[z][y*vol->width()];
			for (int x = 0; x < vol->width(); x += nScale)
			{

				if (*slice > bone_threshold)
					boneMask[mz].push_back(1);
				else
					boneMask[mz].push_back(0);

				slice += nScale;
			}
		}
	}

	// 2. 턱 끝 및 코 끝이 있는 슬라이스 찾기

	// 2.1 턱 끝 슬라이스 찾기
	int nWidth = (int)((float)vol->width() / nScale + 0.5f);
	int chinendz = findChinSlice(boneMask, nWidth, vol->pixelSpacing(), sliceSpacing, nScale);

	// 2.2 코 끝 슬라이스 검출
	// threshold를 이용하여 사람 피부영역 검출 - sagittal plane으로 projection. 코 끝을 검출하기 위함
	// 턱 끝에서 100mm 위까지만 피부를 구함. 코는 이 범위 안에 있음.
	int nHeight = (int)((float)vol->height() / nScale + 0.5f);
	cv::Mat projTissueMask = cv::Mat(nDepth, nHeight, CV_8U, cv::Scalar(0));

	//int nosestartz = chinendz*nScale;
	int nosestartz = std::max(0, (int)(chinendz*nScale - (50.0f / sliceSpacing)));
	int noseendz = std::max(0, (int)(chinendz*nScale - (100.0f / sliceSpacing)));

	//nose가 0이면 볼륨에 코 끝이 없다고 하고 코 끝을 0으로 잡는다.
	if (noseendz)
	{
#pragma omp parallel for
		for (int z = nosestartz; z >= noseendz; z -= nScale)
		{
			ushort* slice = (vol->getData())[z];
			int mz = (int)((float)z / nScale);

			uchar* ptr_tissue_mask = projTissueMask.ptr<uchar>(mz);

			for (int y = 0; y < vol->height(); y += nScale)
			{
				int yIdx = (vol->height() - 1 - y)*vol->width();
				int my = (int)((float)y / nScale);
				for (int x = 0; x < vol->width(); x++)
				{
					if (slice[yIdx + x] > (int)tissue_threshold)
					{
						uchar* ptm = ptr_tissue_mask + my;
						*ptm = 255;
						break;
					}
				}
			}
		}

		noseendz = findNoseSlice(projTissueMask);
	}

	// 3. 뼈 마스크에서 width/2 위치의 sagittal plane을 mask로 이용하여 턱 끝과 코 끝 사이에 있는 인중 위치 찾기
	// 턱 끝에서 코 끝 사이만 구함.
	// 
	cv::Mat boneSag = cv::Mat(nDepth, nHeight, CV_8U, cv::Scalar(0));

	int sagittal_start_x = std::max(0, (int)(((float)nWidth * 0.5f - (3.0f / sliceSpacing) / nScale)));
	int sagittal_end_x = std::min(nWidth - 1, (int)(((float)nWidth * 0.5f + (3.0f / sliceSpacing) / nScale)));

	int z_start = noseendz;
	int z_end = chinendz;
	
	if (method == 1)
	{
		z_end = nDepth;
	}

#pragma omp parallel for
	for (int z = z_start; z < z_end; z++)
	{
		for (int y = nHeight / 2; y < nHeight; y++)
		{
			int yidx = nHeight - 1 - y;
			for (int x = sagittal_start_x; x <= sagittal_end_x; x++)
			{
				if (boneMask[z][yidx*nWidth + x])
				{
					uchar* ptr = boneSag.ptr<uchar>(z) + y;
					*ptr = 255;
				}
			}
		}
	}

#if DEBUG
	FILE* file_;
	fopen_s(&file_, QString("%1_%2_midChinNose0.raw").arg(nHeight).arg(nDepth).toStdString().c_str(), "wb");
	fwrite(boneSag.ptr<uchar>(0), sizeof(uchar), nHeight*nDepth, file_);
	fclose(file_);
#endif

	int morph_size;
	cv::Mat element;

	morph_size = 1;
	element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2 * morph_size + 1, 2 * morph_size + 1), cv::Point(morph_size, morph_size));

	if (method == 1)
	{
		cv::morphologyEx(boneSag, boneSag, cv::MORPH_OPEN, element, cv::Point(-1, -1), 1);
#if DEBUG
		fopen_s(&file_, QString("%1_%2_midChinNose0_opened.raw").arg(nHeight).arg(nDepth).toStdString().c_str(), "wb");
		fwrite(boneSag.ptr<uchar>(0), sizeof(uchar), nHeight*nDepth, file_);
		fclose(file_);
#endif
	}

	cv::morphologyEx(boneSag, boneSag, cv::MORPH_ERODE, element, cv::Point(-1, -1), 1);

#if DEBUG
	fopen_s(&file_, QString("%1_%2_midChinNose0_eroded.raw").arg(nHeight).arg(nDepth).toStdString().c_str(), "wb");
	fwrite(boneSag.ptr<uchar>(0), sizeof(uchar), nHeight*nDepth, file_);
	fclose(file_);
#endif

	int ccl_min = 0;
	if (method == 0)
	{
		ccl_min = nHeight;
	}
	else if (method == 1)
	{
		ccl_min = std::min(nHeight, nDepth) / 2;
	}

	//CCL ����.
	CVpConnComponent ccl;
	ccl.Create(nHeight, nDepth);
	ccl.Do(boneSag.ptr<uchar>(0), ccl_min);
	ccl.Sort(CVpConnComponent::XCOORD);

	cv::Mat maxillaMask = cv::Mat(nDepth, nHeight, CV_8U, cv::Scalar(0));
	cv::Mat mandibleMask = cv::Mat(nDepth, nHeight, CV_8U, cv::Scalar(0));

	auto ccl_info = ccl.GetComponentInfo();

	int maxilla_index = 0, mandible_index = 0;
	if (ccl_info.empty())
	{
		return;
	}

	if (ccl_info.size() > 1)
	{
		if (method == 0)
		{
			float d = abs((float)(ccl_info[0]._pntCenter.x - ccl_info[1]._pntCenter.x));
			if (d < (15.0f / sliceSpacing))
			{
				if (ccl_info[1]._pntCenter.y < ccl_info[0]._pntCenter.y)
				{
					maxilla_index = 1;
					mandible_index = 0;
				}
				else
				{
					maxilla_index = 0;
					mandible_index = 1;
				}
				slice_location->segment_maxilla_mandible = true;
			}
		}
		else if (method == 1)
		{
			if (ccl_info[1]._pntCenter.y < ccl_info[0]._pntCenter.y)
			{
				maxilla_index = 1;
				mandible_index = 0;
			}
			else
			{
				maxilla_index = 0;
				mandible_index = 1;
			}
		}
	}

	unsigned char* tmp = ccl.ExportComponent2D(maxilla_index);
	memcpy(maxillaMask.ptr<uchar>(0), tmp, sizeof(uchar)*nDepth*nHeight);
	tmp = ccl.ExportComponent2D(mandible_index);
	memcpy(mandibleMask.ptr<uchar>(0), tmp, sizeof(uchar)*nDepth*nHeight);

#if DEBUG
	for (int i = 0; i < ccl_info.size(); ++i)
	{
		fopen_s(&file_, QString("%1_%2_ccl_info_%3.raw").arg(nHeight).arg(nDepth).arg(i).toStdString().c_str(), "wb");
		fwrite(ccl.ExportComponent2D(i), sizeof(uchar), nHeight * nDepth, file_);
		fclose(file_);
	}

	fopen_s(&file_, QString("%1_%2_midChinNose1.raw").arg(nHeight).arg(nDepth).toStdString().c_str(), "wb");
	fwrite(maxillaMask.ptr<uchar>(0), sizeof(uchar), nHeight*nDepth, file_);
	fclose(file_);
	fopen_s(&file_, QString("%1_%2_midChinNose2.raw").arg(nHeight).arg(nDepth).toStdString().c_str(), "wb");
	fwrite(mandibleMask.ptr<uchar>(0), sizeof(uchar), nHeight*nDepth, file_);
	fclose(file_);
#endif

	int maxilla_teethz = 0;
	int mandible_teethz = 0;
	if (method == 0 || (maxilla_index == mandible_index))
	{
		ccl.Release();

		// 3.1 projection mask에서 x위치가 가장 큰 위치인 치아 detection - 치아(앞니)가 위치하는 슬라이스 찾기위함
		auto find_teeth = [&](cv::Mat& mat) -> int
		{
			int teethstartz = std::min(nDepth - 1, (int)(noseendz + (15.0f / sliceSpacing) / nScale));
			int teethendz = std::max(0, (int)(chinendz - (15.0f / sliceSpacing) / nScale));
			int maxx = 0;
			int maxy = chinendz;
			for (int y = teethstartz; y < teethendz; y++)
			{
				uchar* ptr = mat.ptr<uchar>(y) + nHeight - 1;
				for (int x = nHeight - 1; x >= 0; x--)
				{
					if (*ptr--)
					{
						if (maxx < x)
						{
							maxx = x;
							maxy = y;
						}
						break;
					}
				}
			}

			return maxy;
		};

		maxilla_teethz = find_teeth(maxillaMask);
		mandible_teethz = find_teeth(mandibleMask);

		// 3.2 bone sagittal mask에서 x위치가 가장 작은 위치 (코 밑 인중) detection
		int minx = nHeight - 1;
		int miny = noseendz;
		int maxilla_end = noseendz;

		for (int y = maxilla_teethz; y >= noseendz; y--)
		{
			uchar* ptr = boneSag.ptr<uchar>(y) + nHeight - 1;
			bool is_set = false;
			for (int x = nHeight - 1; x >= 0; x--)
			{
				if (*ptr-- == 0)
				{
					is_set = true;
					break;
				}
			}
			if (!is_set)
			{
				maxilla_end = y;
				break;
			}
		}

		// 170530 thyoo.
		// 아래 4개의 변수가 다른 탭에서 인덱스로 사용되는 경우 있음
		// 볼륨에 따라서 아래 값들이 제대로 set 되지 않아 numeric_limit 이 그대로 있는 경우
		// indexing error 로 프로그램이 종료되는 현상이 발생하여 부적절한 값은 nDepth / 2으로 clamp함

		slice_location->maxilla = (int)((float)maxilla_teethz - (float)(maxilla_teethz - maxilla_end) * 0.20);
		slice_location->teeth = mandible_teethz;
	}
	else if (method == 1)
	{
		maxilla_teethz = ccl.GetComponentInfo(maxilla_index)->_pntMax.y;
		mandible_teethz = ccl.GetComponentInfo(mandible_index)->_pntMin.y;

		float half_of_front_teeth_length = 7.0f;

		maxilla_teethz = maxilla_teethz - static_cast<int>(half_of_front_teeth_length / (vol->sliceSpacing() * nScale));
		mandible_teethz = mandible_teethz + static_cast<int>(half_of_front_teeth_length / (vol->sliceSpacing() * nScale));

		slice_location->maxilla = maxilla_teethz;
		slice_location->teeth = mandible_teethz;

		ccl.Release();
	}

	slice_location->chin =
		(chinendz <= mandible_teethz) ?
		mandible_teethz + static_cast<int>(15.0f / (vol->sliceSpacing() * nScale)) :
		chinendz;
	slice_location->nose =
		(noseendz >= maxilla_teethz) ?
		maxilla_teethz - static_cast<int>(15.0f / (vol->sliceSpacing() * nScale)) :
		noseendz;

	if ((slice_location->maxilla >= nDepth - 1) || (slice_location->maxilla <= 0)
		|| (slice_location->teeth >= nDepth - 1) || (slice_location->teeth <= 0)
		|| (slice_location->chin > nDepth - 1) || (slice_location->chin <= 0)
		|| (slice_location->nose >= nDepth - 1) || (slice_location->nose < 0))
	{
		int hafDepth = (int)((float)nDepth * 0.5f);
		slice_location->maxilla = hafDepth;
		slice_location->teeth = hafDepth;
		slice_location->chin = nDepth - 1;
		slice_location->nose = 0;
	}

	slice_location->maxilla *= nScale; // 치아와 인중 사이.
	slice_location->teeth *= nScale; //치아의 위치
	slice_location->chin *= nScale; //턱의 위치
	slice_location->nose *= nScale; //코의 위치
}

int CW3DetectSliceLoc::findChinSlice(std::vector<std::vector<uchar>>& volBoneMask, int nWidth, float pixelSpacing, float slice_spacing, int nScale)
{
	//qDebug() << "start CW3DetectSliceLoc::findChinSlice";

	QElapsedTimer timer;
	timer.start();

	//볼륨의 끝 슬라이스에서 50mm 위까지 올라가며 턱 끝 검출 - 밝기 값을 체크하여 턱 끝 검출
  int nDepth = volBoneMask.size();
  int nHeight = volBoneMask[0].size() / nWidth;

  //볼륨 끝에서 50mm위가 코라고 가정함.
  float endChin = std::max(0, nDepth - 1 - (int)((50.0f / slice_spacing) / nScale));

  // 1. 볼륨 끝에서 코까지 조사하면서 처음 bone value가 있는 z(nDepth)를 저장.
  // 이 때, Noise 때문에 이웃 복셀이 모두 bone인 depth를 저장.
  // 깊이 값이 큰 부분이 턱 부분, 작은 부분이 코 부분.


  // 현재 복셀의 이웃을 참조하기 위한 오프셋
  int neiborcnt = 26;
  TPoint3D<int> neighbordist[26];

  neighbordist[0].x = -1;	neighbordist[0].y = -1;	neighbordist[0].z = -1;
  neighbordist[1].x = 0;	neighbordist[1].y = -1;	neighbordist[1].z = -1;
  neighbordist[2].x = 1;	neighbordist[2].y = -1;	neighbordist[2].z = -1;

  neighbordist[3].x = -1;	neighbordist[3].y = 0;	neighbordist[3].z = -1;
  neighbordist[4].x = 0;	neighbordist[4].y = 0;	neighbordist[4].z = -1;
  neighbordist[5].x = 1;	neighbordist[5].y = 0;	neighbordist[5].z = -1;

  neighbordist[6].x = -1;	neighbordist[6].y = 1;	neighbordist[6].z = -1;
  neighbordist[7].x = 0;	neighbordist[7].y = 1;	neighbordist[7].z = -1;
  neighbordist[8].x = 1;	neighbordist[8].y = 1;	neighbordist[8].z = -1;

  neighbordist[9].x = -1;	neighbordist[9].y = -1;	neighbordist[9].z = 0;
  neighbordist[10].x = 0;	neighbordist[10].y = -1; neighbordist[10].z = 0;
  neighbordist[11].x = 1;	neighbordist[11].y = -1; neighbordist[11].z = 0;

  neighbordist[12].x = -1; neighbordist[12].y = 0;	neighbordist[12].z = 0;
  neighbordist[13].x = 1;	neighbordist[13].y = 0;	neighbordist[13].z = 0;

  neighbordist[14].x = -1; neighbordist[14].y = 1;	neighbordist[14].z = 0;
  neighbordist[15].x = 0;	neighbordist[15].y = 1;	neighbordist[15].z = 0;
  neighbordist[16].x = 1;	neighbordist[16].y = 1;	neighbordist[16].z = 0;

  neighbordist[17].x = -1; neighbordist[17].y = -1; neighbordist[17].z = 1;
  neighbordist[18].x = 0;	neighbordist[18].y = -1; neighbordist[18].z = 1;
  neighbordist[19].x = 1;	neighbordist[19].y = -1; neighbordist[19].z = 1;

  neighbordist[20].x = -1; neighbordist[20].y = 0;	neighbordist[20].z = 1;
  neighbordist[21].x = 0; neighbordist[21].y = 0;	neighbordist[21].z = 1;
  neighbordist[22].x = 1;	neighbordist[22].y = 0;	neighbordist[22].z = 1;

  neighbordist[23].x = -1; neighbordist[23].y = 1;	neighbordist[23].z = 1;
  neighbordist[24].x = 0;	neighbordist[24].y = 1;	neighbordist[24].z = 1;
  neighbordist[25].x = 1;	neighbordist[25].y = 1;	neighbordist[25].z = 1;

  cv::Mat depthmap(nHeight, nWidth, CV_16U, cv::Scalar(0));

  for (int y = 1; y < nHeight - 1; y++) {
	int yIdx = y * nWidth;
	for (int x = 1; x < nWidth - 1; x++) {
	  for (int z = nDepth - 2; z > endChin; z--) {
		if (volBoneMask[z][yIdx + x]) {
		  bool isClosed = true;
		  for (int n = 0; n < neiborcnt; n++) {
			int nz = z + neighbordist[n].z;
			int ny = y + neighbordist[n].y;
			int nx = x + neighbordist[n].x;

			if (!volBoneMask[nz][ny*nWidth + nx]) {
			  isClosed = false;
			  break;
			}
		  }

		  if (isClosed) {
			depthmap.at<ushort>(y, x) = z;
			break;
		  }
		}
	  }
	}
  }

  int morph_size = 1;
  cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2 * morph_size + 1, 2 * morph_size + 1), cv::Point(morph_size, morph_size));
  cv::morphologyEx(depthmap, depthmap, cv::MORPH_CLOSE, element, cv::Point(-1, -1), 4);

#if DEBUG
  FILE* file_;
  fopen_s(&file_, QString("%1_%2_chinDepth.raw").arg(nWidth).arg(nHeight).toStdString().c_str(), "wb");
  fwrite(depthmap.ptr(0), sizeof(ushort), nHeight*nWidth, file_);
  fclose(file_);
#endif

  // 2. X축 센터를 기준으로 wsize만큼 window를 얻고 window를 Y축으로 슬라이딩 하면서 최대 nDepth를 구함.
  // 이 때 이전 nDepth가 현재 nDepth보다 현저하게 작은 경우 해당 window component는 더 이상 슬라이딩 하지 않는다.

  int wradius = FIND_CHIN_RADIUS;
  int wsize = wradius * 2 + 1;

  std::map<int, int> mapDepth, mapBreakD;

  int intMax = std::numeric_limits<int>::max();
  int intMin = std::numeric_limits<int>::min();

  for (int i = -wradius; i <= wradius; i++) {
	mapDepth[i] = intMin;
	mapBreakD[i] = intMax;
  }

  int centerx = nWidth / 2;

  //현재 깊이와 이전의 깊이값이 5mm이상 차이나면 해당 윈도우 컴포넌트는 더 이상 진행하지 않는다.
  int breakInterval = (int)((5.0f / pixelSpacing) / nScale); //2mm

  for (int y = 0; y < nHeight; y++) {
	int yIdx = y * nWidth;

	for (auto& md : mapDepth) {
	  int xIdx = centerx + md.first;
	  int z = depthmap.at<ushort>(y, xIdx);

	  if (z == 0)
		continue;

	  if (mapBreakD.find(md.first) == mapBreakD.end())
		continue;

	  md.second = std::max(z, md.second);

	  if (mapBreakD[md.first] < intMax) {
		if (z < mapBreakD[md.first] - breakInterval) {
		  mapBreakD.erase(md.first);
		}
	  }
	  if (mapBreakD.find(md.first) != mapBreakD.end())
		mapBreakD[md.first] = z;
	}

	if (!mapBreakD.size())
	  break;
  }

  //3. 윈도우에 저장된 nDepth의 평균을 구하고 평균과 가장 가까운 값을 slice 번호로 지정함.
  //( bone의 노이즈 때문에 윈도우를 사용함 )

  int m = 0;
  for (const auto& md : mapDepth)
	m += md.second;

  m /= mapDepth.size();

  int slice = intMax;
  int minDevi = intMax;

  for (const auto& md : mapDepth) {
	int deviation = (md.second - m)*(md.second - m);
	if (minDevi > deviation) {
	  minDevi = deviation;
	  slice = md.second;
	}
  }

  //qDebug() << "end CW3DetectSliceLoc::findChinSlice :" << timer.elapsed() << "ms";

  return slice;
}

int CW3DetectSliceLoc::findNoseSlice(cv::Mat& projTissueMask) {
	//qDebug() << "start CW3DetectSliceLoc::findNoseSlice";

	QElapsedTimer timer;
	timer.start();

	// 피부영역으로 부터 코끝이 있는 슬라이스를 검출하기 위해 sagittal plane에서 한 행 왼쪽부터 오른쪽 까지의 mask 길이 조사.
	// 가장 길이가 긴 것이 코. 해당 y값을 찾는다.
  int maxLen;
  int nWidth = projTissueMask.cols;
  int nHeight = projTissueMask.rows;

  //������ ����.
  int morph_size;
  cv::Mat element;

#if DEBUG
  FILE* file_;
  fopen_s(&file_, QString("%1_%2_tissueMask0.raw").arg(nWidth).arg(nHeight).toStdString().c_str(), "wb");
  fwrite(projTissueMask.ptr<uchar>(0), sizeof(uchar), nHeight*nWidth, file_);
  fclose(file_);
#endif

  morph_size = 1;
  element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2 * morph_size + 1, 2 * morph_size + 1), cv::Point(morph_size, morph_size));
  cv::morphologyEx(projTissueMask, projTissueMask, cv::MORPH_ERODE, element, cv::Point(-1, -1), 2);
  cv::morphologyEx(projTissueMask, projTissueMask, cv::MORPH_CLOSE, element, cv::Point(-1, -1), 4);


#if DEBUG
  fopen_s(&file_, QString("%1_%2_tissueMask1.raw").arg(nWidth).arg(nHeight).toStdString().c_str(), "wb");
  fwrite(projTissueMask.ptr<uchar>(0), sizeof(uchar), nHeight*nWidth, file_);
  fclose(file_);
#endif

  std::map<int, float> lens;
  for (int y = 0; y < nHeight; y++) {
	std::vector<int> SAE;

	int yIdx = nWidth * y;
	uchar* ptr_proj_tissue_mask = projTissueMask.ptr<uchar>(y);
	std::vector<float> length;
	for (int x = 0; x < nWidth; x++) {

	  if (SAE.size() == 2) {
		length.push_back(glm::abs(SAE.front() - SAE.back()));
		SAE.clear();
	  }

	  if (*ptr_proj_tissue_mask++) {
		if (SAE.size() == 0) {
		  SAE.push_back(x);
		}
	  }
	  else {
		if (SAE.size() == 1) {
		  SAE.push_back(x);
		}
	  }
	}

	if (SAE.size() == 2)
	  length.push_back(glm::abs(SAE.front() - SAE.back()));

	maxLen = 0;

	for (const auto& elem : length) {
	  if (maxLen < elem) {
		maxLen = elem;
	  }

	}
	lens[y] = maxLen;
  }

  maxLen = 0;

  int noseZ = nHeight;
  for (const auto& elem : lens) {
	if (maxLen < elem.second) {
	  maxLen = elem.second;
	  noseZ = elem.first;
	}
  }

  //qDebug() << "end CW3DetectSliceLoc::findNoseSlice :" << timer.elapsed() << "ms";

  return noseZ;
}
