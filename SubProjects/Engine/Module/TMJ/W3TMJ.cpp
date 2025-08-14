#include "W3TMJ.h"

#include "hdxdentallib.h"

CW3TMJ::CW3TMJ()
{
}

CW3TMJ::~CW3TMJ()
{
}
// TMJ 가시화를 위한 상악부분 검출 함수 - 상악부분이 255, 나머지는 0으로 ppOutRemoveMask에 저장됨
// 즉, volume rendering할때 255로 셋팅되어 있는 부분을 제외하고 volume rendering을 수행해야 함
void CW3TMJ::ActionforTMJ(int w, int h, int d, voltype** ppVolume, unsigned char** ppOutRemoveMask, float pixelsize)
{
	CHDXDentalLib::ActionforTMJ(w, h, d, ppVolume, ppOutRemoveMask, pixelsize);
}

// TMJ 가시화를 위한 상악부분 검출 함수, 보완된 버전 - 상악부분이 255, 나머지는 0으로 ppOutRemoveMask에 저장됨
// 즉, volume rendering할때 255로 셋팅되어 있는 부분을 제외하고 volume rendering을 수행해야 함
void CW3TMJ::ActionforTMJEx(int w, int h, int d, voltype** ppVolume, unsigned char** ppOutRemoveMask, float pixelsize)
{
	CHDXDentalLib::ActionforTMJEx(w, h, d, ppVolume, ppOutRemoveMask, pixelsize);
}

// TMJ 가시화를 위한 하악부분 검출 함수 - 하악부분이 255, 나머지는 0으로 ppOutRemoveMask에 저장됨
// 즉, volume rendering할때 255로 셋팅되어 있는 부분만 volume rendering을 수행해야 함
void CW3TMJ::ActionforTMJEx2(int w, int h, int d, voltype** ppVolume, unsigned char** ppOutMask, float pixelsize)
{
	CHDXDentalLib::ActionforTMJEx2(w, h, d, ppVolume, ppOutMask, pixelsize);
}

// TMJ 가시화를 위한 하악부분 검출 함수 - 하악부분이 255, 나머지는 0으로 ppOutRemoveMask에 저장됨
// 즉, volume rendering할때 255로 셋팅되어 있는 부분만 volume rendering을 수행해야 함
// 입력볼륨 : unsigned short ( + intercept = signed short)
void CW3TMJ::ActionforTMJEx2(int w, int h, int d, usvoltype** ppUSVolume, unsigned char** ppOutMask, float pixelsize, voltype intercept)
{
	CHDXDentalLib::ActionforTMJEx2(w, h, d, ppUSVolume, ppOutMask, pixelsize, intercept);
}

void CW3TMJ::ActionforTMJEx2(int w, int h, int d, usvoltype** ppUSVolume, unsigned char** ppOutMask, float pixelsize, voltype intercept, voltype boneThreshold, voltype tissueThreshold)
{
	CHDXDentalLib::ActionforTMJEx2(w, h, d, ppUSVolume, ppOutMask, pixelsize, intercept, boneThreshold, tissueThreshold);
}
