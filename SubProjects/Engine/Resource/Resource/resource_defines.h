#pragma once
/**********************************************************************************************
 Project: 			Resource
 File:				resouce_defines.h
 Language:			C++11
 Library:			Qt 5.8.0
 author:			Seo Seok Man
 First date:		2017-08-31
 Last modify:		2017-08-31
 **********************************************************************************************/

namespace image {
/****************************************************************************************
* @enum	ImageFormat
*
* @brief	RGBA32 : sequence of unsigned char R, G, B, A
* 			GRAY8 : unsigned char
* 			GRAY16UI : unsigned short
* 			GRAY16 : short
****************************************************************************************/
enum class ImageFormat {
	RGBA32,
	GRAY8,
	GRAY16UI,
	GRAY16,
	UNKNOWN
};

typedef struct IFARGB32 {
	unsigned char r = 0u;
	unsigned char g = 0u;
	unsigned char b = 0u;
	unsigned char a = 0u;
} RGBA32;

typedef unsigned char	GRAY8;
typedef unsigned short	GRAY16UI;
typedef short			GRAY16;

enum class ImageCategory {		
	CT,
	CT_PROJ,
	CT_CAPTURE,
	PANO,
	CEPH,	
	INTRAORAL_SENSOR,
	CAMERA,
	TMJ,	
	ALL,
	UNKNOWN
};

enum class ImageSubCategory {
	CEPH_LATERAL,
	CEPH_FRONTAL,
	CEPH_SMV,
	CEPH_CARPUS,
	CT_VOLUME,
	CT_PROJECT,
	CT_CAPTURE,
	IO_01,
	IO_02,
	IO_03,
	IO_04,
	IO_05,
	IO_06,
	IO_07,
	IO_08,
	IO_09,
	IO_10,
	IO_11,
	IO_12,
	IO_13,
	IO_14,
	CC_LEFT,
	CC_CENTER,
	CC_RIGHT,
	NONE
};
} // end of namespace image
