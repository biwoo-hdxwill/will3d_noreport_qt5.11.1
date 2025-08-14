#include "VpSegTMJ.h"

#include "../../../Common/Common/W3Memory.h"

#include "VpThreshold.h"
#include "VpImageAPI.inl"
#include "VpConnComponent.h"
#include "GxConnectedComponentLabeling.h"
#include "VpMorph.h"

// 16.01.04
static float s_fScalePixelSize[] = { .2f, .4f, .8f };

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVpSegTMJ::CVpSegTMJ()
	: m_nOrigWidth(0), m_nOrigHeight(0), m_nOrigDepth(0), m_nOrigWH(0), m_ppOrigVolData(NULL), m_nWidth(0), m_nHeight(0), m_nDepth(0), m_nWH(0), m_ppVolData(NULL), m_fPixelSize(.0f), m_nScale(0)
{
}

CVpSegTMJ::~CVpSegTMJ()
{
}

void CVpSegTMJ::Release()
{
	SAFE_DELETE_VOLUME(m_ppVolData, m_nDepth);
}

// 16.01.04
void CVpSegTMJ::Init(int w, int h, int d, voltype** ppVolData, float pixelsize, voltype boneThreshold, voltype tissueThreshold)
{
	m_nOrigWidth = w;
	m_nOrigHeight = h;
	m_nOrigDepth = d;
	m_nOrigWH = w*h;
	m_ppOrigVolData = ppVolData;
	m_fPixelSize = pixelsize;
	bone_threshold_ = boneThreshold;
	tissue_threshold_ = tissueThreshold;

	if (pixelsize < DEF_THRESH_PIXELSIZE_L)
	{
		m_nScale = DEF_SCALE;
		CImage3D<voltype> shimg3d;
		shimg3d.Set(m_nOrigWidth, m_nOrigHeight, m_nOrigDepth, m_ppOrigVolData);
		m_ppVolData = shimg3d.ExportScale(m_nScale, m_nScale, m_nScale, &m_nWidth, &m_nHeight, &m_nDepth);
		m_nWH = m_nWidth*m_nHeight;
	}
	else
	{
		m_nScale = 1;
		m_nWidth = m_nOrigWidth;
		m_nHeight = m_nOrigHeight;
		m_nDepth = m_nOrigDepth;
		m_nWH = m_nOrigWH;

		SafeNew2D(m_ppVolData, m_nWH, m_nDepth);
		for (int z = 0; z < m_nDepth; z++)
		{
			memcpy(m_ppVolData[z], m_ppOrigVolData[z], sizeof(voltype)*m_nWH);
		}
	}
}

void CVpSegTMJ::Do(unsigned char** ppOutMask, bool bThick)
{
	// 0. scale - init 단계에서 이미 scaling

	// 1. threshold - 800 (여러 실험 결과 가장 최적화된 threshold값)
	unsigned char** ppBone = NULL;
	SafeNew2D(ppBone, m_nWH, m_nDepth);
	CVpThreshold thresh;
	thresh.Init(m_nWidth, m_nHeight, m_nDepth, m_ppVolData);
	thresh.Do(ppBone, (voltype)DEF_BONETHRESH_L, (voltype)DEF_BONETHRESH_L, CVpThreshold::GREATERTHAN);

	// 2. morphology - close 연산 실행 (dilation, erosion, 커널 사이즈 3x3x3)
	CVpMorph morph;
	morph.Init3D(m_nWidth, m_nHeight, m_nDepth, ppBone);
	morph.Close3D(3);

	// 3. slice range - 상악과 하악을 나눌 기준 슬라이스를 찾기 위한 슬라이스 범위 설정
	int startz = m_nDepth - 1;
	int endz = m_nDepth >> 1;

	// 3.1 턱끝 찾기
	unsigned char check = false;
	//#pragma omp parallel for
	for (int z = m_nDepth - 1; z >= 0; z--)
	{
		for (int y = 0; y < m_nHeight; y++)
		{
			int yidx = y*m_nWidth;
			for (int x = 0; x < m_nWidth; x++)
			{
				int xy = x + yidx;
				if ((ppBone[z][xy] != 0) && (y < (m_nHeight >> 1)))
				{
					startz = z;
					check = true;
					break;
				}
			}
			if (check) break;
		}
		if (check) break;
	}

	// 4. check within the slice range - 슬라이스 범위에서 condyle 시작 부분을 찾음
	// 4.1  scale - 1/2 (0.7mm 이하인 경우 기준, scaling 된 영상에 다시 scaling - 검색 속도 줄이기 위함)
	CImage3D< unsigned char> img3d;
	img3d.Set(m_nWidth, m_nHeight, m_nDepth, ppBone);

	unsigned char** ppScaledBone = NULL;
	int scaledw, scaledh, scaledd;
	int scaledstartz = startz;
	int scaledendz = endz;
	int scalefactor = 1;
	if (m_fPixelSize < DEF_THRESH_PIXELSIZE_H)
	{
		ppScaledBone = img3d.ExportScale(DEF_SCALE, DEF_SCALE, DEF_SCALE, &scaledw, &scaledh, &scaledd);
		scaledstartz >>= 1;
		scaledendz >>= 1;
		scalefactor = DEF_SCALE;
	}
	else
	{
		SafeNew2D(ppScaledBone, m_nWH, m_nDepth);
		scaledw = m_nWidth;
		scaledh = m_nHeight;
		scaledd = m_nDepth;
		img3d.Export(ppScaledBone);
	}

	// 4.3 목뼈부분 제거
	//#pragma omp parallel for
	for (int z = scaledstartz; z >= scaledendz; z--)
	{
		for (int y = (scaledh >> 1); y < scaledh; y++)
		{
			int yidx = y*scaledw;
			for (int x = 0; x < scaledw; x++)
			{
				int xy = x + yidx;
				ppScaledBone[z][xy] = 0;
			}
		}
	}

	//  {
	//  	CString str;
	//  	str.Format("scaledstartz = %d, scaledendz = %d, scalew = %d, scaleh = %d, scaled = %d", scaledstartz, scaledendz, scaledw, scaledh, scaledd);
	// 		AfxMessageBox(str);
	//  }

	// 4.2 ccl을 이용하여 제거해야 하는 슬라이스의 인덱스 검출
	int removedz = (scaledstartz - scaledendz) >> 1;
	//#pragma omp parallel for
	for (int z = scaledstartz; z >= scaledendz; z--)
	{
		CVpConnComponent conncomp2d;
		conncomp2d.Create(scaledw, scaledh);
		conncomp2d.Do(ppScaledBone[z], scaledw);
		if (conncomp2d.GetComponentCnt() < 3)
		{
			conncomp2d.Release();
			continue;
		}
		conncomp2d.Sort(CVpConnComponent::AREA);

		//	AfxMessageBox("candidates...");
		// 4.3 지워야 하는 슬라이스 검출을 위한 테스트
		// 4.3.1 컴포넌트들을 면적으로 정렬한 후 2번째와 3번째로 큰 면적을 소유한 컴포넌트의 정보를 가져옴 (제일 큰 면적은 치아 영역)
		SCompInfo* pComp1 = conncomp2d.GetComponentInfo(1);
		SCompInfo* pComp2 = conncomp2d.GetComponentInfo(2);
		int scaledcenterx = scaledw >> 1;

		// condition 1
		if (((pComp1->_pntCenter.x > scaledcenterx) && (pComp2->_pntCenter.x < scaledcenterx)) ||
			((pComp1->_pntCenter.x < scaledcenterx) && (pComp2->_pntCenter.x > scaledcenterx)))
		{
			removedz = z;
			conncomp2d.Release();
			break;
		}

		// condition 2
		// not yet

		conncomp2d.Release();
	}
	SAFE_DELETE_VOLUME(ppScaledBone, scaledd);

	// 	{
	// 		CString str;
	// 		str.Format("removedz = %d", removedz);
	// 		AfxMessageBox(str);
	// 	}

	// 5. remove slices on the slice of condyle - condyle 시작 부분을 포함한 슬라이스에서 위 아래로 ±3만큼 제거
	if (scalefactor == DEF_SCALE)
	{
		removedz *= DEF_SCALE;
	}

	//
#if 0
	// 6. ccl and leave only maxilla bone - ccl 돌리고 상악만 남겨놓고 나머지 제거
	for (int z = (removedz - 3); z < (removedz + 3); z++)
	{
		memset(ppBone[z], 0, sizeof( unsigned char)*m_nWH);
	}
	GxConnectedComponentLabeling gxccl;
	gxccl.Create(m_nWidth, m_nHeight, m_nDepth, ppBone);
	gxccl.Get3DConnectedComponent();
	gxccl.SortVolumeSize();
	gxccl.GetVolData(1, ppBone);
	gxccl.Release();
#else
	// 6. ccl and leave only maxilla bone - ccl 돌리고 상악만 남겨놓고 나머지 제거
	for (int z = (removedz - 3); z < (removedz + 3); z++)
	{
		memset(m_ppVolData[z], 0, sizeof(voltype)*m_nWH);
	}
	thresh.Do(ppBone, (voltype)DEF_BONETHRESH_H, (voltype)DEF_BONETHRESH_H, CVpThreshold::GREATERTHAN);

	GxConnectedComponentLabeling gxccl;
	gxccl.Create(m_nWidth, m_nHeight, m_nDepth, ppBone);
	gxccl.Get3DConnectedComponent();
	gxccl.SortVolumeSize();
	gxccl.GetVolData(1, ppBone);
	//	gxccl.GetUpVolData(ppBone);
	gxccl.Release();

	if (bThick)	morph.Dilation3D(3);
#endif
	//

	// 7. set value in the result mask - 상악 부분은 1으로 나머지는 0으로 설정. (0으로 설정된 부분만 vr을 적용하면 됨)
	for (int z = 0; z < m_nOrigDepth; z++)
	{
		for (int y = 0; y < m_nOrigHeight; y++)
		{
			int yidx = y*m_nOrigWidth;
			for (int x = 0; x < m_nOrigWidth; x++)
			{
				int scalex = Min((x / m_nScale), m_nWidth - 1);
				int scaley = Min((y / m_nScale), m_nHeight - 1);
				int scalez = Min((z / m_nScale), m_nDepth - 1);
				int scaleidxxy = scalex + scaley*m_nWidth;

				int xy = x + yidx;
				ppOutMask[z][xy] = ppBone[scalez][scaleidxxy];
			}
		}
	}
	SAFE_DELETE_VOLUME(ppBone, m_nDepth);
}

void CVpSegTMJ::DoEx(unsigned char** ppOutMask)
{
	// 0. scale - init 단계에서 이미 scaling

	// 1. threshold - 800 (여러 실험 결과 가장 최적화된 threshold값)
	unsigned char** ppBone = NULL;
	SafeNew2D(ppBone, m_nWH, m_nDepth);
	CVpThreshold thresh;
	thresh.Init(m_nWidth, m_nHeight, m_nDepth, m_ppVolData);
	thresh.Do(ppBone, (voltype)DEF_BONETHRESH_H, (voltype)DEF_BONETHRESH_H, CVpThreshold::GREATERTHAN);

	// 2. morphology - close 연산 실행 (dilation, erosion, 커널 사이즈 3x3x3)
	CVpMorph morph;
	morph.Init3D(m_nWidth, m_nHeight, m_nDepth, ppBone);
	//	morph.Close3D(3);

	// 3. slice range - 상악과 하악을 나눌 기준 슬라이스를 찾기 위한 슬라이스 범위 설정
	int startz = m_nDepth - 1;
	int endz = m_nDepth >> 1;

	// 4. 턱 끝 및 코 끝이 있는 슬라이스 찾기
	unsigned char check1 = false;

	// 4.1 턱 끝 슬라이스
	//#pragma omp parallel for
	for (int z = m_nDepth - 1; z >= 0; z--)
	{
		for (int y = 0; y < m_nHeight; y++)
		{
			int yidx = y*m_nWidth;
			for (int x = 0; x < m_nWidth; x++)
			{
				int xy = x + yidx;
				if ((!check1) && (ppBone[z][xy] != 0) && (y < (m_nHeight >> 1)))
				{
					startz = z;
					check1 = true;
					break;
				}
			}
			if (check1) break;
		}
		if (check1) break;
	}

	int nosez = endz;
	int nosey = INT_MAX;  // 코가 있는 슬라이스에서 코의 y값은 최소값이기 때문에 MAX로 설정하고 최소값을 찾는다.

	unsigned char** ppSkin = NULL;
	SafeNew2D(ppSkin, m_nWH, m_nDepth);
	thresh.Do(ppSkin, (voltype)DEF_SKINTHRESH, (voltype)DEF_SKINTHRESH, CVpThreshold::GREATERTHAN);

	// 4.2 코 끝 슬라이스
	//#pragma omp parallel for
	for (int z = startz; z >= 0; z--)
	{
		for (int y = 0; y < m_nHeight; y++)
		{
			int yidx = y*m_nWidth;
			for (int x = 0; x < m_nWidth; x++)
			{
				int xy = x + yidx;
				if (ppSkin[z][xy] != 0)
				{
					if (nosey > y)
					{
						nosey = y;
						nosez = z;
					}
				}
			}
		}
	}

	// 4.3 코가 끝나는 슬라이스
	int diff_nose = (int)(m_nWidth*DEF_NOSE_RATIO_WIDTH);
	int mid_nose_jaw = (startz + nosez) >> 1;
	int tempnosez = nosez;
	int tempnosey = INT_MAX;
	int noseendz = nosez;

	//#pragma omp parallel for
	for (int z = nosez; z < mid_nose_jaw; z++)
	{
		for (int y = 0; y < m_nHeight; y++)
		{
			int yidx = y*m_nWidth;
			for (int x = 0; x < m_nWidth; x++)
			{
				int xy = x + yidx;
				if (ppSkin[z][xy] != 0)
				{
					if (tempnosey > y)
					{
						tempnosey = y;
						tempnosez = z;
					}
				}
			}
		}

		if ((nosez - tempnosez) > diff_nose)
		{
			noseendz = z;
			break;
		}
	}
	SAFE_DELETE_VOLUME(ppSkin, m_nDepth);

	// 5. 코 부근의 뼈 영역의 밑부분 (하악) 제거
	int removedz = noseendz;
	int splitz = (startz + removedz) >> 1;
	//	for (int z = removedz ; z < m_nDepth ; z++)
	for (int z = splitz; z < m_nDepth; z++)
	{
		memset(ppBone[z], 0, sizeof(unsigned char)*m_nWH);
	}

	// 6. ccl and leave only maxilla bone - ccl 돌리고 상악만 남겨놓고 나머지 제거
	GxConnectedComponentLabeling gxccl;
	gxccl.Create(m_nWidth, m_nHeight, m_nDepth, ppBone);
	gxccl.Get3DConnectedComponent();
	gxccl.SortVolumeSize();
	gxccl.GetVolData(1, ppBone);
	//	gxccl.GetUpVolData(ppBone);
	gxccl.Release();

	if (m_fPixelSize < DEF_THRESH_PIXELSIZE_H)	morph.Dilation3D(3);

	// 7. set value in the result mask - 상악 부분은 1으로 나머지는 0으로 설정. (0으로 설정된 부분만 vr을 적용하면 됨)
	for (int z = 0; z < m_nOrigDepth; z++)
	{
		for (int y = 0; y < m_nOrigHeight; y++)
		{
			int yidx = y*m_nOrigWidth;
			for (int x = 0; x < m_nOrigWidth; x++)
			{
				int scalex = Min((x / m_nScale), m_nWidth - 1);
				int scaley = Min((y / m_nScale), m_nHeight - 1);
				int scalez = Min((z / m_nScale), m_nDepth - 1);
				int scaleidxxy = scalex + scaley*m_nWidth;

				int xy = x + yidx;
				ppOutMask[z][xy] = ppBone[scalez][scaleidxxy];
			}
		}
	}
	SAFE_DELETE_VOLUME(ppBone, m_nDepth);
}

// 하악 검출 함수
// ppOutMask : 결과 저장되는 마스크
void CVpSegTMJ::DoEx2(unsigned char** ppOutMask)
{
	// 0. volume 사이즈 조절
	// init 함수에서 이미 scaling 처리 - pixel size가 설정된 값보다 작으면 1/2로 down sampling하여 사용함

	// 1. 두개골 영역을 저장할 마스크 메모리 할당
	unsigned char** ppBone = NULL;
	SafeNew2D(ppBone, m_nWH, m_nDepth);

	// 2. 뼈 분할을 위해 threshold값을 설정하고 threshold값보다 큰 부위 모두 검출하여 대략적인 뼈 영역 추출
	CVpThreshold thresh;
	thresh.Init(m_nWidth, m_nHeight, m_nDepth, m_ppVolData);
	thresh.Do(ppBone, (voltype)DEF_BONETHRESH_H, (voltype)DEF_BONETHRESH_H, CVpThreshold::GREATERTHAN);
	thresh.Init(m_nWidth, m_nHeight, m_nDepth, m_ppVolData);	
	thresh.Do(ppBone, (voltype)bone_threshold_, (voltype)bone_threshold_, CVpThreshold::GREATERTHAN);

	// 3. slice range - 상악과 하악을 나눌 기준 슬라이스를 찾기 위한 슬라이스 범위 설정
	// 우선 초기값으로 startz는 볼륨의 끝슬라이스, endz는 볼륨의 중간 슬라이스로 설정하여 startz부터 endz까지 체크함
	int startz = m_nDepth - 1;
	int endz = m_nDepth >> 1;

	// 4. 턱 끝 및 코 끝이 있는 슬라이스 찾기
	unsigned char check1 = false;

	// 4.1 턱 끝 슬라이스 찾기
	// 볼륨의 끝 슬라이스에서 위로 올라가며 턱 끝 검출 - 밝기 값을 체크하여 턱 끝 검출
	//#pragma omp parallel for
	for (int z = m_nDepth - 1; z >= 0; z--)
	{
		for (int y = 0; y < m_nHeight; y++)
		{
			int yidx = y*m_nWidth;
			for (int x = 0; x < m_nWidth; x++)
			{
				int xy = x + yidx;
				if ((!check1) && (ppBone[z][xy] != 0) && (y < (m_nHeight >> 1)))
				{
					startz = z;
					check1 = true;
					break;
				}
			}
			if (check1) break;
		}
		if (check1) break;
	}

	int nosez = endz;
	int nosey = INT_MAX;
	// threshold를 이용하여 사람 대략적인 피부영역 검출 - 코 끝을 검출하기 위함
	unsigned char** ppSkin = NULL;
	SafeNew2D(ppSkin, m_nWH, m_nDepth);
	thresh.Do(ppSkin, (voltype)tissue_threshold_, (voltype)tissue_threshold_, CVpThreshold::GREATERTHAN);

	// 4.2 코 끝 슬라이스 검출
	// 피부영역으로 부터 코끝이 있는 슬라이스를 검출하기 위해 피부영역의 각 슬라이스 별로 y의 최소값 체크
	// 코끝의 y값은 최소값이기 때문에 최소 y값을 가지고 있는 슬라이스를 찾으면 그 슬라이스가 코 끝이 존재하는 슬라이스
	//#pragma omp parallel for
	for (int z = startz; z >= 0; z--)
	{
		for (int y = 0; y < m_nHeight; y++)
		{
			int yidx = y*m_nWidth;
			for (int x = 0; x < m_nWidth; x++)
			{
				int xy = x + yidx;
				if (ppSkin[z][xy] != 0)
				{
					if (nosey > y)
					{
						nosey = y;
						nosez = z;
					}
				}
			}
		}
	}
	SAFE_DELETE_VOLUME(ppSkin, m_nDepth);

	int chinendz = startz;
	int noseendz = nosez;

	// 5. sagittal projection mask 영상을 이용하여 턱 끝과 코 끝 사이에 있는 인중 위치 찾기
	CImage3D<voltype> ax_volume;
	CImage3D< unsigned char> ax_mask, sag_mask;
	ax_volume.Set(m_nWidth, m_nHeight, m_nDepth, m_ppVolData);
	unsigned char** ppBoneMask4Proj = ax_volume.ThresholdingEx(DEF_BONETHRESH_MH);
	ax_mask.Set(m_nWidth, m_nHeight, m_nDepth, ppBoneMask4Proj);

	int sagw, sagh, sagd;
	// 뼈영역이 저장되어 있는 마스크로 부터 sagittal 영상 계산
	unsigned char** ppSagMask = ax_mask.CreateSagittal(&sagw, &sagh, &sagd);
	sag_mask.Set(sagw, sagh, sagd, ppSagMask);
	SAFE_DELETE_VOLUME(ppBoneMask4Proj, m_nDepth);

	// projection image of sagittal mask
	// 뼈 영역의 sagittal 볼륨 마스크를 projection하여 코와 턱 사이에 있는 인중 위치 검출
	unsigned char* pPrjSag = sag_mask.CreateProjImageFromVolume();

	// 5.1 projection mask에서 x위치가 가장 큰 위치인 치아 detection - 치아(앞니)가 위치하는 슬라이스 찾기위함
	int maxx = 0;
	int maxy = chinendz;
	for (int y = noseendz; y < chinendz; y++)
	{
		int yidx = y*m_nWidth;
		for (int x = m_nWidth - 1; x >= (m_nWidth >> 1); x--)
		{
			if (pPrjSag[x + yidx])
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
	int teethz = maxy;

	// 5.2 projection mask에서 x위치가 가장 작은 위치 (코 밑 인중) detection
	int minx = m_nWidth - 1;
	int miny = noseendz;
	for (int y = noseendz; y < teethz; y++)
	{
		int yidx = y*m_nWidth;
		for (int x = m_nWidth - 1; x >= (m_nWidth >> 1); x--)
		{
			if (pPrjSag[x + yidx])
			{
				if (minx > x)
				{
					minx = x;
					miny = y;
				}
				break;
			}
		}
	}
	SAFE_DELETE_ARRAY(pPrjSag);

	// 분할 슬라이스 저장 - 위에서 검출한 인중과 치아의 중간지점을 분할 슬라이스로 설정
	int splitz = ((teethz + miny) >> 1);

	// 6. 분할 슬라이스에서 tmj 시작영역 검출 및 그 외 영역 모두 제거하는 작업 수행
	// connected component labeling을 2d 슬라이스 상에서 수행하고 각 컴포넌트 중심점의 x좌표를 오름차순하여 정리
	CVpConnComponent conncomp2d;
	conncomp2d.Create(m_nWidth, m_nHeight);
	conncomp2d.Do(ppBone[splitz], m_nWidth);
	conncomp2d.Sort(CVpConnComponent::XCOORD);

	// 분할 슬라이스에서 tmj영역은 양 끝에 있으므로 컴포넌트 중심의 x좌표가 가장 큰 컴포넌트와 가장 작은 컴포넌트가 양 tmj의
	// 시작 위치가 됨
	int leavedid[2];
	leavedid[0] = 0;
	leavedid[1] = conncomp2d.GetComponentCnt() - 1;
	conncomp2d.LeaveOnly(leavedid, 2);

	// tmj 시작 영역만 export
	unsigned char* pSplitZMask = conncomp2d.Export2D();
	conncomp2d.Release();

	// tmj 시작 영역만 남기고 모두 지움
	// 그러면 분할 슬라이스에서 tmj영역만 남고 나머지 영역은 모두 제거하여 상악과 하악이 분리된다.
	memcpy(ppBone[splitz], pSplitZMask, sizeof(unsigned char)*m_nWH);
	SAFE_DELETE_ARRAY(pSplitZMask);

	// 7. 3차원 connected component labeling 적용 및 하악 검출
	// 상, 하악이 분리되어 있는 상태에서 3d ccl을 실행하면
	// 가장 큰 컴포넌트 두개가 위 아래로 분포하게 되고 아래에 있는 컴포넌트가 하악이 된다.
	GxConnectedComponentLabeling gxccl;
	gxccl.Create(m_nWidth, m_nHeight, m_nDepth, ppBone);
	gxccl.Get3DConnectedComponent();
	gxccl.SortVolumeSize();
	gxccl.GetDnVolData(ppBone);
	gxccl.Release();

	// 픽셀 사이즈가 작은 경우, 즉 볼륨의 크기가 큰 경우 morphology의 dilation을 이용하여 하악의 영역을 넓혀줌
	if (m_fPixelSize < DEF_THRESH_PIXELSIZE_H)
	{
		printf("Do Dilation3D - pixel size : %f\r\n", m_fPixelSize);

		CVpMorph morph;
		morph.Init3D(m_nWidth, m_nHeight, m_nDepth, ppBone);
		morph.Dilation3D(3);
	}

	// 8. set value in the result mask - 하악 부분은 255로 나머지는 0으로 설정. (255로 설정된 부분만 vr을 적용하면 됨)
	for (int z = 0; z < m_nOrigDepth; z++)
	{
		for (int y = 0; y < m_nOrigHeight; y++)
		{
			int yidx = y*m_nOrigWidth;
			for (int x = 0; x < m_nOrigWidth; x++)
			{
				int scalex = Min((x / m_nScale), m_nWidth - 1);
				int scaley = Min((y / m_nScale), m_nHeight - 1);
				int scalez = Min((z / m_nScale), m_nDepth - 1);
				int scaleidxxy = scalex + scaley*m_nWidth;

				int xy = x + yidx;
				ppOutMask[z][xy] = ppBone[scalez][scaleidxxy];
			}
		}
	}

	// release
	SAFE_DELETE_VOLUME(ppBone, m_nDepth);
}
