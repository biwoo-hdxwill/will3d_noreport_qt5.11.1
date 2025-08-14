#pragma once
/*=========================================================================

File:			class CW3TMJJobMgr
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun
First date:		2017-03-09
Last date:		2017-03-09

=========================================================================*/
#include "tmj_global.h"
#include <QObject>
#include "../../Core/ImageProcessing/Inc/VpGlobalDefine.h"


class TMJ_EXPORT CW3TMJ : public QObject {
	Q_OBJECT

public:
	CW3TMJ();
	~CW3TMJ();

	// TMJ 가시화를 위한 상악부분 검출 함수
	void ActionforTMJ(int w, int h, int d, voltype** ppVolume, unsigned char** ppOutRemoveMask, float pixelsize);

	// TMJ 가시화를 위한 상악부분 검출 함수 - 보완된 버전
	void ActionforTMJEx(int w, int h, int d, voltype** ppVolume, unsigned char** ppOutRemoveMask, float pixelsize);

	// TMJ 가시화를 위한 하악부분 검출 함수
	void ActionforTMJEx2(int w, int h, int d, voltype** ppVolume, unsigned char** ppOutMask, float pixelsize);
	void ActionforTMJEx2(int w, int h, int d, usvoltype** ppUSVolume, unsigned char** ppOutMask, float pixelsize, voltype intercept);
	void ActionforTMJEx2(int w, int h, int d, usvoltype** ppUSVolume, unsigned char** ppOutMask, float pixelsize, voltype intercept, voltype boneThreshold, voltype tissueThreshold);

private:

};
