#include "AutoCanal.h"

#include "../../../Engine/Core/ImageProcessing/Inc/VpGlobalDefine.h"
#include "../../../Engine/Core/ImageProcessing/Inc/VpSegCanal.h"
#include "Inc/VpSegCanalEx.h"

CW3AutoCanal::CW3AutoCanal()
{
}

CW3AutoCanal::~CW3AutoCanal()
{
}

void CW3AutoCanal::ActionEx(
	int w, int h, int d, unsigned short** ppVolume, 
	int x1, int y1, int z1, int x2, int y2, int z2,
	unsigned char** ppOutCanal, float pixelsize, unsigned short thdBone)
{
	CVpSegCanalEx segcanal;
	segcanal.Init(w, h, d, ppVolume, pixelsize, thdBone);
	segcanal.SetSeeds3D(x1, y1, z1, x2, y2, z2);
	segcanal.Do(ppOutCanal);
	segcanal.Release();
}

//void CW3AutoCanal::ActionEx(
//	int w, int h, int d, unsigned short** ppVolume, 
//	int x1, int y1, int z1, int x2, int y2, int z2, 
//	std::vector<glm::vec3>* pvOutCanal, float pixelsize, unsigned short thdBone)
//{
//	CVpSegCanalEx segcanal;
//	segcanal.Init(w, h, d, ppVolume, pixelsize, thdBone);
//	segcanal.SetSeeds3D(x1, y1, z1, x2, y2, z2);
//	segcanal.Do(pvOutCanal);
//	segcanal.Release();
//}

void CW3AutoCanal::ActionFrom2DEx(
	int w, int h, int d, unsigned short** ppVolume, 
	int x1, int y1, int x2, int y2, 
	unsigned char** ppOutCanal, float pixelsize, int * startslice, int * endslice, unsigned short thdBone)
{
	CVpSegCanalEx segcanal;
	segcanal.Init(w, h, d, ppVolume, pixelsize, thdBone);
	segcanal.SetSeeds2D(x1, y1, x2, y2);
	segcanal.Do(ppOutCanal);
	segcanal.GetSeedSlice(startslice, endslice);
	segcanal.Release();
}

//void CW3AutoCanal::ActionFrom2DEx(
//	int w, int h, int d, unsigned short** ppVolume,
//	int x1, int y1, int x2, int y2, std::vector<glm::vec3>* pvOutCanal, 
//	float pixelsize, int *startslice, int *endslice, unsigned short thdBone)
//{
//	CVpSegCanalEx segcanal;
//	segcanal.Init(w, h, d, ppVolume, pixelsize, thdBone);
//	segcanal.SetSeeds2D(x1, y1, x2, y2);
//	segcanal.Do(pvOutCanal);
//	segcanal.GetSeedSlice(startslice, endslice);
//	segcanal.Release();
//}

/* smseo : unused functions */
// 3차원 시드점을 입력했을때 canal을 찾는 함수
// w, h, d, ppVolume : 입력볼륨
// x1, y1, z1, x2, y2, z2 : 각 시드1, 2 (3차원)
// ppOutCanal : canal 영역 저장 마스크 (0, 255)
// pixel size : pixel size
//void CW3AutoCanal::Action(
//	int w, int h, int d, unsigned short** ppVolume, 
//	int x1, int y1, int z1, int x2, int y2, int z2, 
//	unsigned char** ppOutCanal, float pixelsize)
//{
//	// canal을 찾는 클래스 CVpSegCanal형 변수 선언
//	CVpSegCanal segcanal;
//	segcanal.Init(w, h, d, ppVolume, pixelsize);
//	// 시드설정
//	segcanal.SetSeeds3D(x1, y1, z1, x2, y2, z2);
//	// canal detection
//	segcanal.Do(ppOutCanal);
//	// release
//	segcanal.Release();
//}
//void CW3AutoCanal::Action(
//	int w, int h, int d, unsigned short** ppVolume, 
//	int x1, int y1, int z1, int x2, int y2, int z2,
//	std::vector<glm::vec3>* pvOutCanal, float pixelsize)
//{
//	// canal을 찾는 클래스 CVpSegCanal형 변수 선언
//	CVpSegCanal segcanal;
//	segcanal.Init(w, h, d, ppVolume, pixelsize);
//	printf("-- init finished\n");
//
//	// 시드설정
//	segcanal.SetSeeds3D(x1, y1, z1, x2, y2, z2);
//	printf("-- SetSeeds3D finished\n");
//
//	// canal detection
//	segcanal.Do(pvOutCanal);
//	printf("-- Do finished\n");
//
//	// release
//	segcanal.Release();
//	printf("-- Release finished\n");
//}
// 2차원 시드점을 입력했을때 canal을 찾는 함수
// w, h, d, ppVolume : 입력볼륨
// x1, y1, x2, y2 : 파노라마로 부터 얻은 각 시드1, 2 (2차원)
// ppOutCanal : canal 영역 저장 마스크 (0, 255)
// pixel size : pixel size
//void CW3AutoCanal::ActionFrom2D(
//	int w, int h, int d, unsigned short** ppVolume, 
//	int x1, int y1, int x2, int y2, 
//	unsigned char** ppOutCanal, float pixelsize)
//{
//	// canal을 찾는 클래스 CVpSegCanal형 변수 선언
//	CVpSegCanal segcanal;
//	segcanal.Init(w, h, d, ppVolume, pixelsize);
//	// 시드설정
//	segcanal.SetSeeds2D(x1, y1, x2, y2);
//	// canal detection from panorama
//	segcanal.DoFromPano(ppOutCanal);
//	// release
//	segcanal.Release();
//}
// 2차원 시드점을 입력했을때 canal을 찾는 함수인데 
// 2차원 (x,y) 시드 좌표에서 3차원 (x,y,z)를 찾고 각 시드의 z좌표를 반환해줌
//void CW3AutoCanal::ActionFrom2D(
//	int w, int h, int d, unsigned short** ppVolume, 
//	int x1, int y1, int x2, int y2, 
//	unsigned char** ppOutCanal, float pixelsize, int* startslice, int* endslice)
//{
//	// canal을 찾는 클래스 CVpSegCanal형 변수 선언
//	CVpSegCanal segcanal;
//	segcanal.Init(w, h, d, ppVolume, pixelsize);
//	// 시드설정
//	segcanal.SetSeeds2D(x1, y1, x2, y2);
//	// canal detection from panorama
//	segcanal.DoFromPano(ppOutCanal);
//	// 두 시드의 3차원 z좌표 반환
//	segcanal.GetSeedSlice(startslice, endslice);
//	// release
//	segcanal.Release();
//}
//
//void CW3AutoCanal::ActionFrom2D(
//	int w, int h, int d, unsigned short** ppVolume,
//	int x1, int y1, int x2, int y2, 
//	std::vector<glm::vec3>* pvOutCanal, float pixelsize, int* startslice, int* endslice)
//{
//	// canal을 찾는 클래스 CVpSegCanal형 변수 선언
//	CVpSegCanal segcanal;
//	segcanal.Init(w, h, d, ppVolume, pixelsize);
//	// 시드설정
//	segcanal.SetSeeds2D(x1, y1, x2, y2);
//	// canal detection from panorama
//	segcanal.DoFromPano(pvOutCanal);
//	// 두 시드의 3차원 z좌표 반환
//	segcanal.GetSeedSlice(startslice, endslice);
//	// release
//	segcanal.Release();
//}
// 
//void CW3AutoCanal::ActionFrom2DEx(
//	int w, int h, int d, unsigned short** ppVolume, 
//	int x1, int y1, int x2, int y2,
//	unsigned char** ppOutCanal, float pixelsize, unsigned short thdBone)
//{
//	CVpSegCanalEx segcanal;
//	segcanal.Init(w, h, d, ppVolume, pixelsize, thdBone);
//	segcanal.SetSeeds2D(x1, y1, x2, y2);
//	segcanal.Do(ppOutCanal);
//	segcanal.Release();
//}
