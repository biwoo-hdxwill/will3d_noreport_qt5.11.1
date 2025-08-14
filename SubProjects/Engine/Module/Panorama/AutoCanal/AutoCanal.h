#pragma once

class  CW3AutoCanal
{
public:
	CW3AutoCanal();
	~CW3AutoCanal();

public:
	// 3차원 시드점을 입력했을때 canal을 찾는 함수(개선된 버전)
	static void ActionEx(
		int w, int h, int d, unsigned short** ppVolume, 
		int x1, int y1, int z1, int x2, int y2, int z2,
		unsigned char** ppOutCanal, float pixelsize, unsigned short thdBone);

	//static void ActionEx(
	//	int w, int h, int d, unsigned short** ppVolume, 
	//	int x1, int y1, int z1, int x2, int y2, int z2,
	//	std::vector<glm::vec3>* pvOutCanal, float pixelsize, unsigned short thdBone);

	// 2차원 시드점을 입력했을때 canal을 찾는 함수인데 
	// 2차원 (x,y) 시드 좌표에서 3차원 (x,y,z)를 찾고 각 시드의 z좌표를 반환해줌(개선된 버전)
	static void ActionFrom2DEx(
		int w, int h, int d, unsigned short** ppVolume,
		int x1, int y1, int x2, int y2, 
		unsigned char** ppOutCanal, float pixelsize, int* startslice, int* endslice, unsigned short thdBone);

	//static void ActionFrom2DEx(
	//	int w, int h, int d, unsigned short** ppVolume, 
	//	int x1, int y1, int x2, int y2,
	//	std::vector<glm::vec3>* pvOutCanal, float pixelsize, int *startslice, int *endslice, unsigned short thdBone);

	/* smseo : unused functions  */
	//  3차원 시드점을 입력했을때 canal을 찾는 함수
	// static void Action(
	//	int w, int h, int d, unsigned short** ppVolume,
	//	int x1, int y1, int z1, int x2, int y2, int z2,
	//	unsigned char** ppOutCanal, float pixelsize);
	//
	// static void Action(
	//	int w, int h, int d, unsigned short** ppVolume,
	//	int x1, int y1, int z1, int x2, int y2, int z2,
	//	std::vector<glm::vec3>* pvOutCanal, float pixelsize);
	//
	// 2차원 시드점을 입력했을때 canal을 찾는 함수
	// static void ActionFrom2D(
	//	int w, int h, int d, unsigned short** ppVolume,
	//	int x1, int y1, int x2, int y2,
	//	unsigned char** ppOutCanal, float pixelsize);
	//
	// 2차원 시드점을 입력했을때 canal을 찾는 함수인데
	// 2차원 (x,y) 시드 좌표에서 3차원 (x,y,z)를 찾고 각 시드의 z좌표를 반환해줌
	// static void ActionFrom2D(
	//	int w, int h, int d, unsigned short** ppVolume,
	//	int x1, int y1, int x2, int y2,
	//	unsigned char** ppOutCanal, float pixelsize, int* startslice, int* endslice);
	//
	// static void ActionFrom2D(
	//	int w, int h, int d, unsigned short** ppVolume,
	//	int x1, int y1, int x2, int y2,
	//	std::vector<glm::vec3>* pvOutCanal, float pixelsize, int *startslice, int *endslice);
	//
	// / 2차원 시드점을 입력했을때 canal을 찾는 함수(개선된 버전)
	// static void ActionFrom2DEx(
	//	int w, int h, int d, unsigned short** ppVolume,
	//	int x1, int y1, int x2, int y2,
	//	unsigned char** ppOutCanal, float pixelsize, unsigned short thdBone);
};
