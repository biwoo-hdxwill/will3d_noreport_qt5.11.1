#pragma once

/*=========================================================================

File:		class WGLTypes
			OpenGL type 들을 모아놓은 파일
Language:	C++11
Library:	OpenGL 4.0 , GLM
Author:			Seo Seok Man
First date:		2017-06-22
Last modify:	2017-06-22

=========================================================================*/
#if defined(__APPLE__)
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#else
#define GLM_SWIZZLE

//#define GL_TEXTURE0 0x84C0
typedef unsigned int GLenum;

#include <GL/glm/glm.hpp>
#endif

#include <vector>

typedef struct _Triangle {
	_Triangle(const glm::vec3& arg1, const glm::vec3& arg2, const glm::vec3& arg3) {
		v0 = arg1;
		v1 = arg2;
		v2 = arg3;
	}
	_Triangle() { }

	glm::vec3 v0 = glm::vec3(0.0f);
	glm::vec3 v1 = glm::vec3(0.0f);
	glm::vec3 v2 = glm::vec3(0.0f);
} Triangle;

typedef struct _STL_TRI_SE {
	glm::vec3		normal, v1, v2, v3;
	unsigned short	cntAttributes = 0;
	unsigned int	nColorVal = 0;
	glm::vec3		fColor;
} tri_STL;	// airway

typedef struct PACK_TEXTURE {
	PACK_TEXTURE(unsigned int aTexBuffer,
				 unsigned int aTexNum,
				 unsigned int aHandler,
				 int aTexNum_)
		: tex_buffer(aTexBuffer),
		tex_num(aTexNum),
		handler(aHandler),
		_tex_num(aTexNum_) {
	}

	PACK_TEXTURE() {}

	unsigned int tex_buffer = 0;
	unsigned int tex_num = 0;
	unsigned int handler = 0;
	int _tex_num = 0;
}PackTexture;

typedef struct PACK_CLIPPING {
	bool is_clipping = false;
	std::vector<glm::vec4> planes;
}PackClipping;

enum SharpenLevel {
	SHARPEN_OFF = 0,
	SHARPEN_LEVEL_1,
	SHARPEN_LEVEL_2,
	SHARPEN_LEVEL_3,
};
