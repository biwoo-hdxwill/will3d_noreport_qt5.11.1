#pragma once

/**=================================================================================================

Project: 			Panorama
File:				pano_algorithm.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2016-08-23
Last modify:		2016-08-23

Copyright (c) 2017 All rights reserved by HDXWILL.

 *===============================================================================================**/
#include <vector>

#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif

#include "panorama_global.h"

class CW3Image3D;

class PANORAMA_EXPORT PanoAlgorithm
{
public:
	static void RunMandibleAutoArch(const CW3Image3D* vol, std::vector<glm::vec3>& points, const int target_slice_number = -1);
	static void RunMaxillaAutoArch(const CW3Image3D* vol, std::vector<glm::vec3>& points, const int target_slice_number = -1);
	static void runAutoCanal(unsigned char **out, glm::vec3 *coord, float pixelsize, CW3Image3D *pVolume);
	static void runAutoCanal(std::vector<glm::vec3> *out, glm::vec3 *coord, float pixelsize, CW3Image3D *pVolume);
	static void runAutoCanal(unsigned char **out, glm::vec3 *coord, float pixelsize, CW3Image3D* pVolume, int *startZ, int *endZ);
	static void runAutoCanal(std::vector<glm::vec3> *out, float pixelsize, CW3Image3D *pVolume);
};
