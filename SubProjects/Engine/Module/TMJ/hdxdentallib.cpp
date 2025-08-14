#include "hdxdentallib.h"

#include "../../Common/Common/W3Memory.h"
#include "../../Core/ImageProcessing/Inc/VpSegCanal.h"
#include "../../Core/ImageProcessing/Inc/VpSegTMJ.h"


CHDXDentalLib::CHDXDentalLib()
{
}

CHDXDentalLib::~CHDXDentalLib()
{
}

// 3차원 시드점을 입력했을때 canal을 찾는 함수
// w, h, d, ppVolume : 입력볼륨
// x1, y1, z1, x2, y2, z2 : 각 시드1, 2 (3차원)
// ppOutCanal : canal 영역 저장 마스크 (0, 255)
// pixel size : pixel size
void CHDXDentalLib::Action(int w, int h, int d, usvoltype** ppVolume, 
	int x1, int y1, int z1, int x2, int y2, int z2, unsigned char** ppOutCanal, float pixelsize)
{
	// canal을 찾는 클래스 CVpSegCanal형 변수 선언
	CVpSegCanal segcanal;
	segcanal.Init(w, h, d, ppVolume, pixelsize);
	// 시드설정
	segcanal.SetSeeds3D(x1, y1, z1, x2, y2, z2);
	// canal detection
	segcanal.Do(ppOutCanal);
	// release
	segcanal.Release();
}

void CHDXDentalLib::Action(int w, int h, int d, unsigned short** ppVolume,
	int x1, int y1, int z1, int x2, int y2, int z2, std::vector<glm::vec3>* pvOutCanal, float pixelsize)
{
	// canal을 찾는 클래스 CVpSegCanal형 변수 선언
	CVpSegCanal segcanal;
	segcanal.Init(w, h, d, ppVolume, pixelsize);

	printf("-- Init finished\n");

	// 시드설정
	segcanal.SetSeeds3D(x1, y1, z1, x2, y2, z2);

	printf("-- SetSeeds3D finished\n");

	// canal detection
	segcanal.Do(pvOutCanal);

	printf("-- Do finished\n");

	// release
	segcanal.Release();

	printf("-- Release finished\n");
}

// 2차원 시드점을 입력했을때 canal을 찾는 함수
// w, h, d, ppVolume : 입력볼륨
// x1, y1, x2, y2 : 파노라마로 부터 얻은 각 시드1, 2 (2차원)
// ppOutCanal : canal 영역 저장 마스크 (0, 255)
// pixel size : pixel size
void CHDXDentalLib::ActionFrom2D(int w, int h, int d, usvoltype** ppVolume, 
	int x1, int y1, int x2, int y2, unsigned char** ppOutCanal, float pixelsize)
{
	// canal을 찾는 클래스 CVpSegCanal형 변수 선언
	CVpSegCanal segcanal;
	segcanal.Init(w, h, d, ppVolume, pixelsize);
	// 시드설정
	segcanal.SetSeeds2D(x1, y1, x2, y2);
	// canal detection from panorama
	segcanal.DoFromPano(ppOutCanal);
	// release
	segcanal.Release();
}

// 2차원 시드점을 입력했을때 canal을 찾는 함수인데 2차원 (x,y) 시드 좌표에서 3차원 (x,y,z)를 찾고 각 시드의 z좌표를 반환해줌
void CHDXDentalLib::ActionFrom2D(int w, int h, int d, usvoltype** ppVolume, 
	int x1, int y1, int x2, int y2, unsigned char** ppOutCanal, float pixelsize, int* startslice, int* endslice)
{
	// canal을 찾는 클래스 CVpSegCanal형 변수 선언
	CVpSegCanal segcanal;
	segcanal.Init(w, h, d, ppVolume, pixelsize);
	// 시드설정
	segcanal.SetSeeds2D(x1, y1, x2, y2);
	// canal detection from panorama
	segcanal.DoFromPano(ppOutCanal);
	// 두 시드의 3차원 z좌표 반환
	segcanal.GetSeedSlice(startslice, endslice);
	// release
	segcanal.Release();
}

void CHDXDentalLib::ActionFrom2D(int w, int h, int d, unsigned short** ppVolume,
	int x1, int y1, int x2, int y2, std::vector<glm::vec3>* pvOutCanal,
	float pixelsize, int* startslice, int* endslice)
{
	// canal을 찾는 클래스 CVpSegCanal형 변수 선언
	CVpSegCanal segcanal;
	segcanal.Init(w, h, d, ppVolume, pixelsize);
	// 시드설정
	segcanal.SetSeeds2D(x1, y1, x2, y2);
	// canal detection from panorama
	segcanal.DoFromPano(pvOutCanal);
	// 두 시드의 3차원 z좌표 반환
	segcanal.GetSeedSlice(startslice, endslice);
	// release
	segcanal.Release();
}

// TMJ 가시화를 위한 상악부분 검출 함수 - 상악부분이 255, 나머지는 0으로 ppOutRemoveMask에 저장됨
// 즉, volume rendering할때 255로 셋팅되어 있는 부분을 제외하고 volume rendering을 수행해야 함
void CHDXDentalLib::ActionforTMJ(int w, int h, int d, voltype** ppVolume, 
	unsigned char** ppOutRemoveMask, float pixelsize)
{
	CVpSegTMJ segtmj;
	segtmj.Init(w, h, d, ppVolume, pixelsize);
	segtmj.Do(ppOutRemoveMask, true);
	segtmj.Release();
}

// TMJ 가시화를 위한 상악부분 검출 함수, 보완된 버전 - 상악부분이 255, 나머지는 0으로 ppOutRemoveMask에 저장됨
// 즉, volume rendering할때 255로 셋팅되어 있는 부분을 제외하고 volume rendering을 수행해야 함
void CHDXDentalLib::ActionforTMJEx(int w, int h, int d, voltype** ppVolume,
	unsigned char** ppOutRemoveMask, float pixelsize)
{
	CVpSegTMJ segtmj;
	segtmj.Init(w, h, d, ppVolume, pixelsize);
	segtmj.DoEx(ppOutRemoveMask);
	segtmj.Release();
}

// TMJ 가시화를 위한 하악부분 검출 함수 - 하악부분이 255, 나머지는 0으로 ppOutRemoveMask에 저장됨
// 즉, volume rendering할때 255로 셋팅되어 있는 부분만 volume rendering을 수행해야 함
void CHDXDentalLib::ActionforTMJEx2(int w, int h, int d, voltype** ppVolume,
	unsigned char** ppOutMask, float pixelsize)
{
	CVpSegTMJ segtmj;
	segtmj.Init(w, h, d, ppVolume, pixelsize);
	segtmj.DoEx2(ppOutMask);
	segtmj.Release();
}

// TMJ 가시화를 위한 하악부분 검출 함수 - 하악부분이 255, 나머지는 0으로 ppOutRemoveMask에 저장됨
// 즉, volume rendering할때 255로 셋팅되어 있는 부분만 volume rendering을 수행해야 함
// 입력볼륨 : unsigned short ( + intercept = signed short)
void CHDXDentalLib::ActionforTMJEx2(int w, int h, int d, usvoltype** ppUSVolume,
	unsigned char** ppOutMask, float pixelsize, voltype intercept)
{
	CVpSegTMJ segtmj;
	voltype** ppVolume = NULL;
	int wh = w*h;
	SafeNew2D(ppVolume, wh, d);
	for (int z = 0; z < d; z++)
	{
		for (int xy = 0; xy < wh; xy++)
		{
			ppVolume[z][xy] = (((voltype)ppUSVolume[z][xy]) + intercept);
		}
	}

	segtmj.Init(w, h, d, ppVolume, pixelsize);
	segtmj.DoEx2(ppOutMask);
	segtmj.Release();

	SAFE_DELETE_VOLUME(ppVolume, d);
}

void CHDXDentalLib::ActionforTMJEx2(int w, int h, int d, usvoltype** ppUSVolume, unsigned char** ppOutMask, float pixelsize, voltype intercept, voltype boneThreshold, voltype tissueThreshold)
{
	CVpSegTMJ segtmj;
	voltype** ppVolume = NULL;
	int wh = w*h;
	SafeNew2D(ppVolume, wh, d);
	for (int z = 0; z < d; z++)
	{
		for (int xy = 0; xy < wh; xy++)
		{
			ppVolume[z][xy] = (((voltype)ppUSVolume[z][xy]) + intercept);
		}
	}

	//
	segtmj.Init(w, h, d, ppVolume, pixelsize, boneThreshold + intercept, tissueThreshold + intercept);
	segtmj.DoEx2(ppOutMask);
	segtmj.Release();
	//

	SAFE_DELETE_VOLUME(ppVolume, d);
}
