#include "VpSegCanalEx.h"
#include <QPointF>

#if defined(__APPLE__)
#include <glm/gtx/rotate_vector.hpp>
#else
#include <GL/glm/gtx/rotate_vector.hpp>
#endif

#include "../../../Core/ImageProcessing/Inc/VpThreshold.h"
#include "../../../Core/ImageProcessing/Inc/VpImageAPI.inl"
#include "../OpenCVheader.h"
#include "VpPathTrackingEx.h"

#define PI 3.14159265358979323846

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVpSegCanalEx::CVpSegCanalEx() {
	memset(&m_pntOrigSeed1, 0, sizeof(TPoint3D<int>));
	memset(&m_pntOrigSeed2, 0, sizeof(TPoint3D<int>));
	memset(&m_pntSeed1, 0, sizeof(TPoint3D<int>));
	memset(&m_pntSeed2, 0, sizeof(TPoint3D<int>));

	m_nOrigWidth = 0;
	m_nOrigHeight = 0;
	m_nOrigDepth = 0;
	m_nOrigWH = 0;
	m_ppOrigVolData = NULL;

	m_nScale = 1;
	m_shVolThresh = 0;

	m_nOrigKernelSize = 0;
	m_nKernelSize = 0;

	m_fPixelSize = .0f;
	m_thdBone = 0;

	m_nPartWidth = m_nPartHeight = m_nPartDepth = 0;
	m_nPartWH = 0;
	m_ppPartVolData = nullptr;
	m_nPartSeed1 = m_nPartSeed2 = TPoint3D<int>();

	m_ppPartCanalFeature = nullptr;
}

CVpSegCanalEx::~CVpSegCanalEx() {}
// release
void CVpSegCanalEx::Release() {
	SAFE_DELETE_VOLUME(m_ppPartVolData, m_nPartDepth);
	SAFE_DELETE_VOLUME(m_ppPartCanalFeature, m_nPartDepth);
}

void CVpSegCanalEx::Init(int w, int h, int d, usvoltype ** ppVolData, float pixelsize, usvoltype thdBone) {
	// 오리지날 볼륨 정보 저장
	m_nOrigWidth = w;
	m_nOrigHeight = h;
	m_nOrigDepth = d;
	m_nOrigWH = w * h;
	m_ppOrigVolData = ppVolData;

	m_fPixelSize = pixelsize;
	m_thdBone = thdBone;

	// pixel size를 통해 canal의 크기 설정하고 kernel사이즈로 설정함
	m_nKernelSize = Max(3, (int)(2.0f / m_fPixelSize)); //2mm

	// kernal 사이즈는 홀수로 셋팅
	if ((m_nKernelSize % 2) == 0) m_nKernelSize++;

	m_nOrigKernelSize = m_nKernelSize;
}
void CVpSegCanalEx::SetSeeds3D(int x1, int y1, int z1, int x2, int y2, int z2) {
	// 3차원 복셀값 시드를 설정
	m_pntOrigSeed1.x = x1;
	m_pntOrigSeed1.y = y1;
	m_pntOrigSeed1.z = z1;

	m_pntOrigSeed2.x = x2;
	m_pntOrigSeed2.y = y2;
	m_pntOrigSeed2.z = z2;

	// 1. (seed1 - 6.5mm, seed2 + 6.5mm)를 크기로 하는 부분 볼륨을 얻는다.

	int nMargin = (int)(6.5f / m_fPixelSize);

	if (x1 > x2) {
		m_nPartSeed1.x = x2 - nMargin;
		m_nPartSeed2.x = x1 + nMargin;

		m_pntSeed1.x = x1 - x2 + nMargin;
		m_pntSeed2.x = nMargin;

		if (m_nPartSeed1.x < 0)
			m_nPartSeed1.x = 0;

		if (m_nPartSeed2.x >= m_nOrigWidth)
			m_nPartSeed2.x = m_nOrigWidth - 1;
	} else {
		m_nPartSeed1.x = x1 - nMargin;
		m_nPartSeed2.x = x2 + nMargin;

		m_pntSeed1.x = nMargin;
		m_pntSeed2.x = x2 - x1 + nMargin;

		if (m_nPartSeed1.x < 0)
			m_nPartSeed1.x = 0;

		if (m_nPartSeed2.x >= m_nOrigWidth)
			m_nPartSeed2.x = m_nOrigWidth - 1;
	}

	if (y1 > y2) {
		m_pntSeed1.y = y1 - y2 + nMargin;
		m_pntSeed2.y = nMargin;

		m_nPartSeed1.y = y2 - nMargin;
		m_nPartSeed2.y = y1 + nMargin;

		if (m_nPartSeed1.y < 0)
			m_nPartSeed1.y = 0;

		if (m_nPartSeed2.y >= m_nOrigHeight)
			m_nPartSeed2.y = m_nOrigHeight - 1;
	} else {
		m_pntSeed1.y = nMargin;
		m_pntSeed2.y = y2 - y1 + nMargin;

		m_nPartSeed1.y = y1 - nMargin;
		m_nPartSeed2.y = y2 + nMargin;

		if (m_nPartSeed1.y < 0)
			m_nPartSeed1.y = 0;

		if (m_nPartSeed2.y >= m_nOrigHeight)
			m_nPartSeed2.y = m_nOrigHeight - 1;
	}

	if (z1 > z2) {
		m_pntSeed1.z = z1 - z2 + nMargin;
		m_pntSeed2.z = nMargin;

		m_nPartSeed1.z = z2 - nMargin;
		m_nPartSeed2.z = z1 + nMargin;

		if (m_nPartSeed1.z < 0)
			m_nPartSeed1.z = m_pntSeed2.z = 0;

		if (m_nPartSeed2.z >= m_nOrigDepth)
			m_nPartSeed2.z = m_nOrigDepth - 1;
	} else {
		m_pntSeed1.z = nMargin;
		m_pntSeed2.z = z2 - z1 + nMargin;

		m_nPartSeed1.z = z1 - nMargin;
		m_nPartSeed2.z = z2 + nMargin;

		if (m_nPartSeed1.z < 0)
			m_nPartSeed1.z = 0;

		if (m_nPartSeed2.z >= m_nOrigDepth)
			m_nPartSeed2.z = m_nOrigDepth - 1;
	}

	SAFE_DELETE_VOLUME(m_ppPartVolData, m_nPartDepth);

	m_nPartWidth = m_nPartSeed2.x - m_nPartSeed1.x + 1;
	m_nPartHeight = m_nPartSeed2.y - m_nPartSeed1.y + 1;
	m_nPartDepth = m_nPartSeed2.z - m_nPartSeed1.z + 1;

	m_nPartWH = m_nPartWidth * m_nPartHeight;

	SafeNew2D(m_ppPartVolData, m_nPartWH, m_nPartDepth);

	for (int z = m_nPartSeed1.z; z <= m_nPartSeed2.z; z++) {
		for (int y = m_nPartSeed1.y; y <= m_nPartSeed2.y; y++) {
			for (int x = m_nPartSeed1.x; x <= m_nPartSeed2.x; x++) {
				m_ppPartVolData[z - m_nPartSeed1.z][(y - m_nPartSeed1.y)*m_nPartWidth + (x - m_nPartSeed1.x)]
					= m_ppOrigVolData[z][y*m_nOrigWidth + x];
			}
		}
	}
}
void CVpSegCanalEx::SetSeeds2D(int x1, int y1, int x2, int y2) {
	int z1, z2;

	this->detectSeedCoordZ(x1, y1, x2, y2, &z1, &z2);

	this->SetSeeds3D(x1, y1, z1, x2, y2, z2);
}

float CVpSegCanalEx::sagittalCanalFeature(const glm::vec3 & point, const int & Nangle, const int & detectRadius, const int & volMaskWidth, const std::vector<std::vector<unsigned char>>& volMask) {
	float angleStep = PI / Nangle;

	int volMaskHeight = volMask[0].size() / volMaskWidth;
	int volMaskDepth = volMask.size();

	float feature = 0.0f;

	//Nangle 수만큼 360도 조사한다.
	for (int i = 0; i < Nangle; i++) {
		float theta = angleStep * i;
		glm::vec3 vec = glm::vec3(0.0f, sin(theta), cos(theta));

		glm::vec3 cur = point;

		float pf, nf; //positive 방향 feature, negative 방향 feature.
		pf = nf = 0.0f;

		//point에서 detectRadius크기만큼 positive 방향으로 진행한다.
		for (int v = 0; v <= detectRadius; v++) {
			if ((cur.z < 0) || (cur.z > volMaskDepth - 1) ||
				(cur.y < 0) || (cur.y > volMaskHeight - 1)) {
				cur += vec;
				continue;
			}

			//마스크 값이 있으면
			if (volMask[(int)(cur.z)][int(cur.y)*volMaskWidth + cur.x]) {
				//point에서 진행한 위치까지의 길이를 positive feature로 설정.
				pf = glm::length(point - cur);
				break;
			}

			cur += vec;
		}

		//point에서 detectRadius크기만큼 nagative 방향으로 진행한다.
		cur = point;
		for (int v = 0; v <= detectRadius; v++) {
			if ((cur.z < 0) || (cur.z > volMaskDepth - 1) ||
				(cur.y < 0) || (cur.y > volMaskHeight - 1)) {
				cur -= vec;
				continue;
			}

			//마스크 값이 있으면
			if (volMask[(int)(cur.z)][int(cur.y)*volMaskWidth + cur.x]) {
				//point에서 진행한 위치까지의 길이를 nagative feature로 설정.
				nf = glm::length(point - cur);
				break;
			}

			cur -= vec;
		}

		//positive방향과 nagative방향 모두 발견되었다면 feature에 누적.
		if (pf != 0.0f && nf != 0.0f) {
			feature += nf + pf;
		}
	}

	//누적한 feature를 리턴
	return feature;
}

float CVpSegCanalEx::sphereCanalFeature(const glm::vec3 & point, const int & Nangle, const int & outerRadius, const int& innerRadius,
										const int & volMaskWidth, const std::vector<std::vector<unsigned char>>& volMask) {
	int volMaskHeight = volMask[0].size() / volMaskWidth;
	int volMaskDepth = volMask.size();

	float feature = 0.0f;
	//int cntFeature = 0;

	for (int i = 0; i < 3; i++) {
		float V = 0.5f - (float)i / 2;
		float phi = V * (PI / 6.0f);

		//Nangle 수만큼 360도 조사한다.
		for (int j = 0; j < Nangle; j++) {
			float U = (float)j / Nangle;
			float theta = U * PI;

			glm::vec3 vec = glm::vec3(0.0f, sin(theta), cos(theta));

			//Z축 회전
			vec = glm::rotateZ(vec, phi);

			//glm::vec3 vec = glm::vec3(
			//	sin(phi),
			//	sin(theta)*cos(phi),
			//	cos(theta)*cos(phi));

			glm::vec3 cur = point;

			float pf, nf; //positive 방향 feature, negative 방향 feature.
			pf = nf = 0.0f;

			//point에서 detectRadius크기만큼 positive 방향으로 진행한다.
			for (int v = 0; v <= outerRadius; v++) {
				if ((cur.z < 0) || (cur.z > volMaskDepth - 1) ||
					(cur.y < 0) || (cur.y > volMaskHeight - 1) ||
					(cur.x < 0) || (cur.x > volMaskWidth - 1)) {
					cur += vec;
					continue;
				}

				//마스크 값이 있으면
				if (volMask[(int)(cur.z)][int(cur.y)*volMaskWidth + (int)cur.x]) {
					float len = glm::length(point - cur);

					//point에서 진행한 위치까지의 길이가 innerRadius보다 크면 positive feature로 설정.
					if (len > innerRadius) {
						pf = len;
					}
					break;
				}

				cur += vec;
			}

			cur = point;
			for (int v = 0; v <= outerRadius; v++) {
				if ((cur.z < 0) || (cur.z > volMaskDepth - 1) ||
					(cur.y < 0) || (cur.y > volMaskHeight - 1) ||
					(cur.x < 0) || (cur.x > volMaskWidth - 1)) {
					cur -= vec;
					continue;
				}

				if (volMask[(int)(cur.z)][int(cur.y)*volMaskWidth + (int)cur.x]) {
					float len = glm::length(point - cur);

					//point에서 진행한 위치까지의 길이가 innerRadius보다 크면 negative feature로 설정.
					if (len > innerRadius) {
						nf = len;
					}
					break;
				}

				cur -= vec;
			}
			if (pf != 0.0f && nf != 0.0f) {
				{
					feature += nf + pf;
				}
			}
		}
	}

	return feature;
}

void CVpSegCanalEx::postProcessing() {
	float fEnhance = 0.8f;
	int boneThreshold = m_thdBone;
	int canalThreshold = (int)(m_thdBone*0.95f + 0.5f);

	// 2. bone 영역의 밝기값을 조절하고, canal mask를 구함.
	std::vector<std::vector<unsigned char>> mask;
	mask.resize(m_nPartDepth);

	for (int z = 0; z < m_nPartDepth; z++) {
		usvoltype* pPartVolData = m_ppPartVolData[z];

		for (int y = 0; y < m_nPartHeight; y++) {
			for (int x = 0; x < m_nPartWidth; x++) {
				usvoltype val = *pPartVolData;

				if (val > canalThreshold) {
					mask[z].push_back(1);
				} else {
					mask[z].push_back(0);
				}

				if (val > boneThreshold) {
					*pPartVolData++ = (usvoltype)(val* (1 + fEnhance) + .5f);
				} else {
					*pPartVolData++ = (usvoltype)(val* (1 - fEnhance) + .5f);
				}
			}
		}
	}

	// 3. canal Mask를 이용해서 canal feature를 구함.

	//시드의 x 좌표에서 sagittal plane을 구하고
	//시드의 (y, z)좌표를 중심으로 일정 반지름(2mm)을 갖는 원모양의 윈도우를 조사함.

	glm::vec3 ptCanalSeed1; // 시드1의 좌표
	glm::vec3 ptRef1; // 시드1를 중심으로 조사하는 시드1 조사변수

	glm::vec3 ptCanalSeed2; // 시드2의 좌표
	glm::vec3 ptRef2; // 시드2를 중심으로 조사하는 시드2 조사변수

	ptCanalSeed1 = glm::vec3(m_pntSeed1.x, m_pntSeed1.y, m_pntSeed1.z);
	ptCanalSeed2 = glm::vec3(m_pntSeed2.x, m_pntSeed2.y, m_pntSeed2.z);

	ptRef1 = ptCanalSeed1;
	ptRef2 = ptCanalSeed2;

	int outerRadius = Max(1, (int)(2.35f / m_fPixelSize)); // 조사하는 윈도우의 반지름. radius 2 mm, size 4mm
	int innerRadius = Max(1, (int)(0.85f / m_fPixelSize));

	//canal feature 메모리 생성
	SAFE_DELETE_VOLUME(m_ppPartCanalFeature, m_nPartDepth);
	SafeNew2D(m_ppPartCanalFeature, m_nPartWH, m_nPartDepth);

	for (int i = 0; i < m_nPartDepth; i++)
		memset(m_ppPartCanalFeature[i], 255, sizeof(unsigned char)*m_nPartWH);

	//시드 x좌표에서 다른 시드의 x좌표까지 1씩 증감하면서 iteration하기 위한 변수.
	glm::vec3 deltaDir = glm::vec3(m_pntSeed2.x - m_pntSeed1.x,
								   m_pntSeed2.y - m_pntSeed1.y,
								   m_pntSeed2.z - m_pntSeed1.z);
	deltaDir = glm::normalize(deltaDir);

	float fStep = abs(1.0 / deltaDir.x);
	deltaDir.x *= fStep;

	float lengthStep = abs(m_pntSeed2.x - m_pntSeed1.x);
	float accumStep = 0.0f;
	int Nangle = 8;

	//시드1은 시드1의 x에서 시드2의 x까지 1씩 증감하면서 진행하고
	//시드2는 시드2의 x에서 시드1의 x까지 1씩 증감하면서 진행한다.
	bool bStopSeed1 = false;
	bool bStopSeed2 = false;

	while (accumStep < lengthStep) {
		float maxFeature1 = 0.0f;
		float maxFeature2 = 0.0f;

		std::vector<float> dataFeature1, dataFeature2; // 조사한 feature를 임시로 저장하는 변수
		dataFeature1.resize(m_nPartDepth*m_nPartHeight, 0.0f);
		dataFeature2.resize(m_nPartDepth*m_nPartHeight, 0.0f);

		//윈도우 사이즈만큼 조사한다.
		for (int j = -outerRadius; j <= outerRadius; j++) {
			for (int i = -outerRadius; i <= outerRadius; i++) {
				if (!bStopSeed1) {
					//시드1 윈도우 참조 좌표
					int x = ptRef1.x;
					int iz = ptRef1.z + i;
					int iy = ptRef1.y + j;

					//현재 참조 위치에서 시드1까지의 길이
					float lenRefToOrigin = sqrt((iz - ptRef1.z)*(iz - ptRef1.z) + (iy - ptRef1.y)*(iy - ptRef1.y));

					//현재 참조 위치가 조사하는 원 안에 있다면
					if ((iz >= 0) && (iz < m_nPartDepth) &&
						(iy >= 0) && (iy < m_nPartHeight) &&
						(lenRefToOrigin < outerRadius)) {
						//참조 위치가 원의 중심에서 멀어질 수록 feature가 낮아지도록 weight 설정.
						float weight = exp(-lenRefToOrigin / (outerRadius*4.0f));

						//Sagittal plane에서 feature가 계산되며 점 주변이 원형 구조를 가지면 가질 수록 feature값이 크다.
						float feature = sphereCanalFeature(glm::vec3(x, iy, iz), Nangle, outerRadius, innerRadius, m_nPartWidth, mask);

						feature *= weight;

						//현재 위치에서 조사한 feature를 저장한다.
						dataFeature1[iy*m_nPartDepth + iz] = feature;

						//윈도우 내의 feature 최대값을 갖는 위치를 저장한다.
						if (maxFeature1 < feature) {
							maxFeature1 = feature;
							ptCanalSeed1 = glm::vec3(x, iy, iz);
						}
					}
				}

				if (!bStopSeed2) {
					//시드2 윈도우 참조 좌표
					int x = ptRef2.x;
					int iz = ptRef2.z + i;
					int iy = ptRef2.y + j;

					//현재 참조 위치에서 시드2까지의 길이
					float lenRefToOrigin = sqrt((iz - ptRef2.z)*(iz - ptRef2.z) + (iy - ptRef2.y)*(iy - ptRef2.y));

					//현재 참조 위치가 조사하는 원 안에 있다면
					if ((iz >= 0) && (iz < m_nPartDepth) &&
						(iy >= 0) && (iy < m_nPartHeight) &&
						(lenRefToOrigin < outerRadius)) {
						//참조 위치가 원의 중심에서 멀어질 수록 feature가 낮아지도록 weight 설정.
						float weight = exp(-lenRefToOrigin / (outerRadius*4.0f));

						//Sagittal plane에서 feature가 계산되며 점 주변이 원형 구조를 가지면 가질 수록 feature값이 크다.
						float feature = sphereCanalFeature(glm::vec3(x, iy, iz), Nangle, outerRadius, innerRadius, m_nPartWidth, mask);

						feature *= weight;

						//현재 위치에서 조사한 feature를 저장한다.
						dataFeature2[iy*m_nPartDepth + iz] = feature;

						//윈도우 내의 feature 최대값을 갖는 위치를 저장한다.
						if (maxFeature2 < feature) {
							maxFeature2 = feature;
							ptCanalSeed2 = glm::vec3(x, iy, iz);
						}
					}
				}
			}
		}

		//dijkstra에서 feature로 사용 할 수 있도록 0~255로 정규화하고 반전시킨다.
		//반전하는 이유는 dijkstra에서 cost가 낮은 path를 찾기 때문이다.
		for (int j = -outerRadius; j <= outerRadius; j++) {
			for (int i = -outerRadius; i <= outerRadius; i++) {
				if (!bStopSeed1) {
					int iz = ptRef1.z + i;
					int iy = ptRef1.y + j;
					int x = ptRef1.x;

					if ((iz >= 0) && (iz < m_nPartDepth) &&
						(iy >= 0) && (iy < m_nPartHeight) && (maxFeature1 != 0.0f)) {
						unsigned char val = m_ppPartCanalFeature[iz][iy*m_nPartWidth + x];
						if (val == 255)
							m_ppPartCanalFeature[iz][iy*m_nPartWidth + x] = 255 - (dataFeature1[iy*m_nPartDepth + iz] / maxFeature1) * 255;
						else {
							int sum = m_ppPartCanalFeature[iz][iy*m_nPartWidth + x] + 255 - (dataFeature1[iy*m_nPartDepth + iz] / maxFeature1) * 255;
							sum = (sum > 255) ? 255 : sum;
							sum = (sum < 0) ? 0 : sum;
							m_ppPartCanalFeature[iz][iy*m_nPartWidth + x] = sum;
						}
					}
				}

				if (!bStopSeed2) {
					int iz = ptRef2.z + i;
					int iy = ptRef2.y + j;
					int x = ptRef2.x;

					if ((iz >= 0) && (iz < m_nPartDepth) &&
						(iy >= 0) && (iy < m_nPartHeight) && (maxFeature2 != 0.0f)) {
						unsigned char val = m_ppPartCanalFeature[iz][iy*m_nPartWidth + x];
						if (val == 255)
							m_ppPartCanalFeature[iz][iy*m_nPartWidth + x] = 255 - (dataFeature2[iy*m_nPartDepth + iz] / maxFeature2) * 255;
						else {
							int sum = m_ppPartCanalFeature[iz][iy*m_nPartWidth + x] + 255 - (dataFeature2[iy*m_nPartDepth + iz] / maxFeature2) * 255;
							sum = (sum > 255) ? 255 : sum;
							sum = (sum < 0) ? 0 : sum;
							m_ppPartCanalFeature[iz][iy*m_nPartWidth + x] = sum;
						}
					}
				}
			}
		}
		if (maxFeature1 == 0.0f)
			bStopSeed1 = true;
		if (maxFeature2 == 0.0f)
			bStopSeed2 = true;

		if (bStopSeed1 && bStopSeed2)
			break;

		//ptRef1 = ptCanalSeed1 + deltaDir;
		//ptRef2 = ptCanalSeed2 - deltaDir;
		ptRef1 = ptCanalSeed1 + deltaDir.x;
		ptRef2 = ptCanalSeed2 - deltaDir.x;

		accumStep += fStep;
	}

	//for debug...

	//while (1)
	//{
	//	int xLen = (int)abs(m_pntSeed1.x - m_pntSeed2.x);
	//	int iter = 0;
	//	int step = (m_pntSeed2.x - m_pntSeed1.x) / xLen;
	//	int x = m_pntSeed1.x;
	//
	//	while (xLen >= iter)
	//	{
	//
	//		cv::Mat partVol(m_nPartHeight, m_nPartDepth, CV_16U);
	//		cv::Mat partFeature(m_nPartHeight, m_nPartDepth, CV_8U);
	//		cv::Mat partBinary(m_nPartHeight, m_nPartDepth, CV_8U);
	//
	//		char* binaryData = partBinary.ptr<char>(0);
	//		char* featureData = partFeature.ptr<char>(0);
	//		ushort* volData = partVol.ptr<ushort>(0);
	//		for (int y = 0; y < m_nPartHeight; y++)
	//		{
	//			for (int z = 0; z < m_nPartDepth; z++)
	//			{
	//				*volData++ = m_ppPartVolData[z][y*m_nPartWidth + x];
	//				*featureData++ = m_ppPartCanalFeature[z][y*m_nPartWidth + x];
	//				*binaryData++ = mask[z][y*m_nPartWidth + x] * 255;
	//			}
	//		}
	//
	//		QOpenCV::imshow("vol", 300 * 300, partVol);
	//		QOpenCV::imshow("binary", 300 * 300, partBinary);
	//		QOpenCV::imshow("feature", 300 * 300, partFeature, true);
	//
	//		cv::moveWindow("vol", 0, 0);
	//		cv::moveWindow("binary", 250, 0);
	//		cv::moveWindow("feature", 500, 0);
	//
	//
	//		iter++;
	//		x += step;
	//
	//		int key = cvWaitKey();
	//		if (key == 27)
	//			continue;
	//		else if (key == 32)
	//		{
	//			cv::destroyAllWindows();
	//			break;
	//		}
	//
	//
	//	}
	//
	//	cv::destroyAllWindows();
	//
	//}
}
void CVpSegCanalEx::detectSeedCoordZ(int x1, int y1, int x2, int y2, int * d_z1, int * d_z2) {
	// z1을 0, z2를 m_nOrigDepth - 1라 가정하고 m_ppPartVolData를 구함.
	SetSeeds3D(x1, y1, 0, x2, y2, m_nOrigDepth - 1);

	float fEnhance = 0.8f;
	int boneThreshold = m_thdBone;
	int canalThreshold = (int)(m_thdBone*0.95f + 0.5f);

	// canal mask를 구함.
	std::vector<std::vector<unsigned char>> mask;
	mask.resize(m_nPartDepth);

	for (int z = 0; z < m_nPartDepth; z++) {
		usvoltype* pPartVolData = m_ppPartVolData[z];

		for (int y = 0; y < m_nPartHeight; y++) {
			for (int x = 0; x < m_nPartWidth; x++) {
				if (*pPartVolData++ > canalThreshold) {
					mask[z].push_back(1);
				} else {
					mask[z].push_back(0);
				}
			}
		}
	}

	int outerRadius = Max(1, (int)(2.35f / m_fPixelSize)); //radius 2 mm, size 4mm
	int innerRadius = Max(1, (int)(1.75f / m_fPixelSize));
	int wsize = outerRadius * 2;
	int Nangle = 12;

	float featureMax1, featureMax2;
	featureMax1 = featureMax2 = 0.0f;

	cv::Mat startFeature(m_nPartHeight, m_nPartDepth, CV_32F);
	cv::Mat endFeature(m_nPartHeight, m_nPartDepth, CV_32F);
	for (int z = 0; z < m_nPartDepth; z++) {
		for (int i = -wsize; i <= wsize; i++) {
			int iy = m_pntSeed1.y + i;

			float len = sqrt((iy - m_pntSeed1.y)*(iy - m_pntSeed1.y));

			if ((iy >= 0) && (iy < m_nPartHeight) && (len < outerRadius)) {
				float feature = sphereCanalFeature(glm::vec3(m_pntSeed1.x, iy, z), Nangle, outerRadius, innerRadius,
												   m_nPartWidth, mask);

				startFeature.at<float>(iy, z) = feature;

				if (featureMax1 < feature) {
					featureMax1 = feature;
					*d_z1 = z;
				}
			}

			iy = m_pntSeed2.y + i;

			len = sqrt((iy - m_pntSeed2.y)*(iy - m_pntSeed2.y));

			if ((iy >= 0) && (iy < m_nPartHeight) && (len < outerRadius)) {
				float feature = sphereCanalFeature(glm::vec3(m_pntSeed2.x, iy, z), Nangle, outerRadius, innerRadius,
												   m_nPartWidth, mask);

				endFeature.at<float>(iy, z) = feature;

				if (featureMax2 < feature) {
					featureMax2 = feature;
					*d_z2 = z;
				}
			}
		}
	}
	//QOpenCV::imshow("startFeature", 300 * 300, startFeature);
	//QOpenCV::imshow("endFeature", 300 * 300, endFeature);
	//
	//cv::moveWindow("startFeature", 500, 0);
	//cv::moveWindow("endFeature", 500, 500);
	//
	//cv::Mat partVol(m_nPartHeight, m_nPartDepth, CV_16U);
	//cv::Mat partBinary(m_nPartHeight, m_nPartDepth, CV_8U);
	//
	//char* binaryData = partBinary.ptr<char>(0);
	//ushort* volData = partVol.ptr<ushort>(0);
	//for (int y = 0; y < m_nPartHeight; y++)
	//{
	//	for (int z = 0; z < m_nPartDepth; z++)
	//	{
	//		*volData++ = m_ppPartVolData[z][y*m_nPartWidth + m_pntSeed1.x];
	//		*binaryData++ = mask[z][y*m_nPartWidth + m_pntSeed1.x] * 255;
	//	}
	//}
	//
	//QOpenCV::imshow("startVol", 300 * 300, partVol);
	//QOpenCV::imshow("startBinary", 300 * 300, partBinary);
	//
	//cv::moveWindow("startVol", 0, 0);
	//cv::moveWindow("startBinary", 250, 0);
	//
	//
	//binaryData = partBinary.ptr<char>(0);
	//volData = partVol.ptr<ushort>(0);
	//for (int y = 0; y < m_nPartHeight; y++)
	//{
	//	for (int z = 0; z < m_nPartDepth; z++)
	//	{
	//		*volData++ = m_ppPartVolData[z][y*m_nPartWidth + m_pntSeed2.x];
	//		*binaryData++ = mask[z][y*m_nPartWidth + m_pntSeed2.x] * 255;
	//	}
	//}
	//
	//QOpenCV::imshow("endVol", 300 * 300, partVol);
	//QOpenCV::imshow("endBinary", 300 * 300, partBinary);
	//
	//cv::moveWindow("endVol", 0, 500);
	//cv::moveWindow("endBinary", 250, 500);
	//
	//int key = cvWaitKey();
	//cv::destroyAllWindows();

	//////////////////////////////////////////////////////

	//int xLen = (int)abs(m_pntSeed1.x - m_pntSeed2.x);
	//int iter = 0;
	//int step = (m_pntSeed2.x - m_pntSeed1.x) / xLen;
	//int x = m_pntSeed1.x;
	//
	//while (xLen >= iter)
	//{
	//
	//	cv::Mat partVol(m_nPartHeight, m_nPartDepth, CV_16U);
	//	cv::Mat partBinary(m_nPartHeight, m_nPartDepth, CV_8U);
	//
	//	char* binaryData = partBinary.ptr<char>(0);
	//	ushort* volData = partVol.ptr<ushort>(0);
	//	for (int y = 0; y < m_nPartHeight; y++)
	//	{
	//		for (int z = 0; z < m_nPartDepth; z++)
	//		{
	//			*volData++ = m_ppPartVolData[z][y*m_nPartWidth + x];
	//			*binaryData++ = mask[z][y*m_nPartWidth + x] * 255;
	//		}
	//	}
	//
	//	QOpenCV::imshow("vol", 300 * 300, partVol);
	//	QOpenCV::imshow("binary", 300 * 300, partBinary);
	//
	//	cv::moveWindow("vol", 0, 0);
	//	cv::moveWindow("binary", 250, 0);
	//
	//
	//	iter++;
	//	x += step;
	//
	//	int key = cvWaitKey();
	//	if (key == 27)
	//		continue;
	//	else if (key == 32)
	//	{
	//		cv::destroyAllWindows();
	//		break;
	//	}
	//
	//
	//}
	//
	//cv::destroyAllWindows();
}
// kernel size의 sphere를 저장하는 마스크
// sphere영역은 255 그 밖에 영역은 0 저장
void CVpSegCanalEx::GetKernelMask(int kernel, unsigned char** ppKernelMask) {
	// fill mask
	int nRadius = (kernel >> 1);
	int r2 = nRadius * nRadius;
	int nKernelCnt = 0;

	//#pragma omp parallel for
	for (int rz = 0; rz < kernel; rz++) {
		int z2 = (rz - nRadius)*(rz - nRadius);
		for (int ry = 0; ry < kernel; ry++) {
			int y2z2 = (ry - nRadius)*(ry - nRadius) + z2;
			for (int rx = 0; rx < kernel; rx++) {
				int x2y2z2 = (rx - nRadius)*(rx - nRadius) + y2z2;
				if (x2y2z2 <= r2) {
					ppKernelMask[rz][rx + ry * kernel] = 255;
					++nKernelCnt;
				}
			}
		}
	}
}

// path를 tracking하며 kernel mask를 이용하여 ppExpandMask에 sphere를 그려줌
void CVpSegCanalEx::ExpandPath(int w, int kernel, unsigned char** ppKernelMask, std::vector<int>* vecpathxy, std::vector<int>* vecpathz, unsigned char** ppExpandMask) {
	int nRadius = (kernel >> 1);
	int nSize = (int)vecpathxy->size();
	for (int n = 0; n < nSize; n++) {
		int x = 0, y = 0, z = 0;
		GetIdx2DTo3D(vecpathxy->at(n), vecpathz->at(n), w, &x, &y, &z);

		//#pragma omp parallel for
		for (int n = -nRadius; n <= nRadius; n++) {
			int nTempZ = z + n;
			int nMaskZ = n + nRadius;
			for (int m = -nRadius; m <= nRadius; m++) {
				int nTempY = y + m;
				int nMaskY = m + nRadius;
				for (int l = -nRadius; l <= nRadius; l++) {
					int nTempX = x + l;
					int nMaskX = l + nRadius;
					if (ppKernelMask[nMaskZ][nMaskX + nMaskY * kernel]) {
						ppExpandMask[nTempZ][nTempX + nTempY * w] = 255;
					}
				}
			}
		}
	}
}

void CVpSegCanalEx::Do(unsigned char ** ppOrigOutCanal, bool bSmooth) {
	// dijkstra에 필요한 정보를 볼륨과 두 시드를 가지고 구함
	this->postProcessing();

	// dijkstra 를 이용하여 두 시드 사이의 path를 구함
	std::vector<int> vecpathxy;
	std::vector<int> vecpathz;
	CVpPathTrackingEx dijks;
	dijks.Init(m_nPartWidth, m_nPartHeight, m_nPartDepth, m_ppPartVolData, m_ppPartCanalFeature, CVpPathTrackingEx::DIJKSTRA, m_nKernelSize);
	dijks.SetPoints(m_pntSeed1.x, m_pntSeed1.y, m_pntSeed1.z, m_pntSeed2.x, m_pntSeed2.y, m_pntSeed2.z);
	dijks.Do(&vecpathxy, &vecpathz);
	dijks.Release();

	// 다운 샘플링이 되었을 경우 path의 원래 좌표를 구함
	int n = (int)vecpathxy.size();
	for (int i = 0; i < n; i++) {
		int xy = vecpathxy[i];
		int z = vecpathz[i];
		TPoint3D<int> tmppnt;
		GetIdx2DTo3D(xy, z, m_nPartWidth, &tmppnt.x, &tmppnt.y, &tmppnt.z);
		vecpathxy[i] = ((tmppnt.x + m_nPartSeed1.x) + (tmppnt.y + m_nPartSeed1.y)*m_nOrigWidth);
		vecpathz[i] = (z + m_nPartSeed1.z);
	}

	// curve로 smoothing함
	if (bSmooth) {
		float offsett = m_fPixelSize / 5.0f;
		int sampling = (int)(1.0f / offsett + .5f);
		CVpPathTrackingEx curvesmoothing;
		curvesmoothing.Curve(m_nOrigWidth, &vecpathxy, &vecpathz, sampling, offsett, CVpPathTrackingEx::CRCURVE);
	}

	// path를 tracking하며 sphere를 그려줌
	unsigned char** ppKernelMask = NULL;
	SafeNew2D(ppKernelMask, m_nOrigKernelSize*m_nOrigKernelSize, m_nOrigKernelSize);
	GetKernelMask(m_nOrigKernelSize, ppKernelMask);
	ExpandPath(m_nOrigWidth, m_nOrigKernelSize, ppKernelMask, &vecpathxy, &vecpathz, ppOrigOutCanal);
	SAFE_DELETE_VOLUME(ppKernelMask, m_nOrigKernelSize);
}

void CVpSegCanalEx::Do(std::vector<glm::vec3>* pvOrigOutCanal, bool bSmooth) {
	// dijkstra에 필요한 정보를 볼륨과 두 시드를 가지고 구함
	this->postProcessing();

	// dijkstra 를 이용하여 두 시드 사이의 path를 구함
	std::vector<int> vecpathxy;
	std::vector<int> vecpathz;
	CVpPathTrackingEx dijks;
	dijks.Init(m_nPartWidth, m_nPartHeight, m_nPartDepth, m_ppPartVolData, m_ppPartCanalFeature, CVpPathTrackingEx::DIJKSTRA, m_nKernelSize);
	dijks.SetPoints(m_pntSeed1.x, m_pntSeed1.y, m_pntSeed1.z, m_pntSeed2.x, m_pntSeed2.y, m_pntSeed2.z);
	dijks.Do(&vecpathxy, &vecpathz);
	dijks.Release();

	// 다운 샘플링이 되었을 경우 path의 원래 좌표를 구함
	int n = (int)vecpathxy.size();
	int step = std::max((n - 1) / 3, 1);

	dijks.Release();

	printf("---- Release finished\n");

	for (int i = 0; i < n - step; i += step) {
		int xy = vecpathxy[i];
		int z = vecpathz[i];
		TPoint3D<int> tmppnt;
		GetIdx2DTo3D(xy, z, m_nPartWidth, &tmppnt.x, &tmppnt.y, &tmppnt.z);

		pvOrigOutCanal->push_back(glm::vec3(tmppnt.x + m_nPartSeed1.x, tmppnt.y + m_nPartSeed1.y, (z + m_nPartSeed1.z)));
	}

	int xy = vecpathxy[n - 1];
	int z = vecpathz[n - 1];
	TPoint3D<int> tmppnt;
	GetIdx2DTo3D(xy, z, m_nPartWidth, &tmppnt.x, &tmppnt.y, &tmppnt.z);

	pvOrigOutCanal->push_back(glm::vec3(tmppnt.x + m_nPartSeed1.x, tmppnt.y + m_nPartSeed1.y, (z + m_nPartSeed1.z)));
}

void CVpSegCanalEx::GetSeedSlice(int * startslice, int * endslice) {
	if (m_nScale == 1) {
		*startslice = m_pntOrigSeed1.z;
		*endslice = m_pntOrigSeed2.z;
	} else {
		*startslice = m_pntOrigSeed1.z * m_nScale;
		*endslice = m_pntOrigSeed2.z * m_nScale;
	}
}
