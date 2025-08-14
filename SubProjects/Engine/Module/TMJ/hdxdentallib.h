#pragma once

#include <vector>

#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif

#include "../../Core/ImageProcessing/Inc/VpGlobalDefine.h"

class CHDXDentalLib
{
public:
	CHDXDentalLib();
	~CHDXDentalLib();

public:
	// 3차원 시드점을 입력했을때 canal을 찾는 함수
	static void Action(int w, int h, int d, usvoltype** ppVolume, int x1, int y1, int z1, int x2, int y2, int z2, unsigned char** ppOutCanal, float pixelsize);
	static void Action(int w, int h, int d, usvoltype** ppVolume, int x1, int y1, int z1, int x2, int y2, int z2, std::vector<glm::vec3>* pvOutCanal, float pixelsize);

	// 2차원 시드점을 입력했을때 canal을 찾는 함수
	static void ActionFrom2D(int w, int h, int d, usvoltype** ppVolume, int x1, int y1, int x2, int y2, unsigned char** ppOutCanal, float pixelsize);

	// 2차원 시드점을 입력했을때 canal을 찾는 함수인데 2차원 (x,y) 시드 좌표에서 3차원 (x,y,z)를 찾고 각 시드의 z좌표를 반환해줌
	static void ActionFrom2D(int w, int h, int d, usvoltype** ppVolume, int x1, int y1, int x2, int y2, unsigned char** ppOutCanal, float pixelsize, int* startslice, int* endslice);
	static void ActionFrom2D(int w, int h, int d, usvoltype** ppVolume, int x1, int y1, int x2, int y2, std::vector<glm::vec3>* pvOutCanal, float pixelsize, int *startslice, int *endslice);

	// TMJ 가시화를 위한 상악부분 검출 함수
	static void ActionforTMJ(int w, int h, int d, voltype** ppVolume, unsigned char** ppOutRemoveMask, float pixelsize);

	// TMJ 가시화를 위한 상악부분 검출 함수 - 보완된 버전
	static void ActionforTMJEx(int w, int h, int d, voltype** ppVolume, unsigned char** ppOutRemoveMask, float pixelsize);

	// TMJ 가시화를 위한 하악부분 검출 함수
	static void ActionforTMJEx2(int w, int h, int d, voltype** ppVolume, unsigned char** ppOutMask, float pixelsize);
	static void ActionforTMJEx2(int w, int h, int d, usvoltype** ppUSVolume, unsigned char** ppOutMask, float pixelsize, voltype intercept);
	static void ActionforTMJEx2(int w, int h, int d, usvoltype** ppUSVolume, unsigned char** ppOutMask, float pixelsize, voltype intercept, voltype boneThreshold, voltype tissueThreshold);
};
