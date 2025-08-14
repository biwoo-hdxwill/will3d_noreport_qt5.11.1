#include "W3GLFunctions.h"
/*=========================================================================

File:			class CW3GLFunctions
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2016-04-15
Last date:		2016-04-15

=========================================================================*/
#include <QImage>
#include <QImageWriter>
#include <QDebug>

#include "../Common/W3Logger.h"
#include "../Common/W3Memory.h"
using common::Logger;
using std::cerr;

void CW3GLFunctions::clearView(bool isDepthTest) {
  clearView(isDepthTest, 0.0f, 0.0f, 0.0f, 0.0f);
}

void CW3GLFunctions::clearView(bool isDepthTest, GLenum cullFace) {
  clearView(isDepthTest, cullFace, 0.0f, 0.0f, 0.0f, 0.0f);
}

void CW3GLFunctions::clearView(bool isDepthTest, float r, float g, float b,
							   float a) {
  glClearColor(r, g, b, a);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if (isDepthTest) {
	glEnable(GL_DEPTH_TEST);
  } else {
	glDisable(GL_DEPTH_TEST);
  }
}

void CW3GLFunctions::clearView(bool isDepthTest, GLenum cullFace, float r,
							   float g, float b, float a) {
  glClearColor(r, g, b, a);

  if (isDepthTest) {
	glEnable(GL_DEPTH_TEST);

	if (cullFace == GL_BACK) {
	  glClearDepth(1.0f);
	  glDepthFunc(GL_LESS);
	} else {
	  glClearDepth(0.0f);
	  glDepthFunc(GL_GREATER);
	}
  } else {
	glDisable(GL_DEPTH_TEST);
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void CW3GLFunctions::clearDepth(GLenum depthFunc) {
  if (depthFunc == GL_LESS)
	glClearDepth(1.0f);
  else
	glClearDepth(0.0f);
  glClear(GL_DEPTH_BUFFER_BIT);
}

void CW3GLFunctions::drawWire(unsigned int vaoHandler, int N) {
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  glBindVertexArray(vaoHandler);

  glDrawElements(GL_TRIANGLES, N, GL_UNSIGNED_INT, NULL);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  printError(__LINE__, "CW3GLFunctions::drawWire");
}

void CW3GLFunctions::drawView(unsigned int vaoHandler, int N)
{
	glBindVertexArray(vaoHandler);

	glDrawElements(GL_TRIANGLES, N, GL_UNSIGNED_INT, NULL);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CW3GLFunctions::drawView(unsigned int vaoHandler, int N, GLenum cullFace) {
	if (!vaoHandler)
	{
		return;
	}

  glEnable(GL_CULL_FACE);
  
  //CW3GLFunctions::printError(__LINE__, "1 CW3GLFunctions::drawView");
  //qDebug() << "1 CW3GLFunctions::drawView";

  glCullFace(cullFace);

  //CW3GLFunctions::printError(__LINE__, "2 CW3GLFunctions::drawView");
  //qDebug() << "2 CW3GLFunctions::drawView";

  glBindVertexArray(vaoHandler);

  //CW3GLFunctions::printError(__LINE__, "3 CW3GLFunctions::drawView");
  //qDebug() << "3 CW3GLFunctions::drawView";

  glDrawElements(GL_TRIANGLES, N, GL_UNSIGNED_INT, NULL);
  //glDrawArrays(GL_TRIANGLES, 0, N);

  //CW3GLFunctions::printError(__LINE__, "4 CW3GLFunctions::drawView");
  //qDebug() << "4 CW3GLFunctions::drawView";

  glBindVertexArray(0);

  //CW3GLFunctions::printError(__LINE__, "5 CW3GLFunctions::drawView");
  //qDebug() << "5 CW3GLFunctions::drawView";

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  //CW3GLFunctions::printError(__LINE__, "6 CW3GLFunctions::drawView");
  //qDebug() << "6 CW3GLFunctions::drawView";

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  //CW3GLFunctions::printError(__LINE__, "7 CW3GLFunctions::drawView");
  //qDebug() << "7 CW3GLFunctions::drawView";

  glDisable(GL_CULL_FACE);
}

void CW3GLFunctions::drawView(unsigned int vao, unsigned int vbo_indices, int n, GLenum cull_face)
{
	glEnable(GL_CULL_FACE);
	glCullFace(cull_face);

	glBindVertexArray(vao);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_indices);

	glDrawElements(GL_TRIANGLES, n, GL_UNSIGNED_INT, NULL);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisable(GL_CULL_FACE);
}

void CW3GLFunctions::drawViewTriangles(unsigned int vaoHandler, int N,
									   GLenum cullFace) {
  glEnable(GL_CULL_FACE);
  glCullFace(cullFace);
  glBindVertexArray(vaoHandler);
  glDrawElements(GL_TRIANGLES, N, GL_UNSIGNED_INT, NULL);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDisable(GL_CULL_FACE);
}

void CW3GLFunctions::drawPoints(unsigned int vaoHandler, int N) {
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
  glBindVertexArray(vaoHandler);
  glDrawArrays(GL_POINTS, 0, N);
  glBindVertexArray(0);
}

void CW3GLFunctions::drawLines(unsigned int vaoHandler, int N) {
  glEnable(GL_LINE_SMOOTH);
  glBindVertexArray(vaoHandler);
  glLineWidth(1.0f);
  glDrawArrays(GL_LINES, 0, N);
  glBindVertexArray(0);
  glDisable(GL_LINE_SMOOTH);
}

void CW3GLFunctions::init2DTex(unsigned int &texHandler, unsigned int width,
							   unsigned int height, unsigned short *data) {
	if (texHandler)
	{
		glDeleteTextures(1, &texHandler);
		texHandler = 0;
	}

	glGenTextures(1, &texHandler);
	glBindTexture(GL_TEXTURE_2D, texHandler);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R16, width, height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED,
					GL_UNSIGNED_SHORT, data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

void CW3GLFunctions::init2DTexSR(unsigned int &texHandler, unsigned int width,
								 unsigned int height, unsigned char *data) {
  if (texHandler) {
	glDeleteTextures(1, &texHandler);
	texHandler = 0;
  }

  glGenTextures(1, &texHandler);
  glBindTexture(GL_TEXTURE_2D, texHandler);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, width, height);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB,
				  GL_UNSIGNED_BYTE, data);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

void CW3GLFunctions::update2DTex(unsigned int &texHandler, unsigned int width,
								 unsigned int height, float *data,
								 bool isDifferentSize) {
  if (texHandler && isDifferentSize) {
	glDeleteTextures(1, &texHandler);
	texHandler = 0;

	glGenTextures(1, &texHandler);

	glBindTexture(GL_TEXTURE_2D, texHandler);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT,
					data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  } else if (!texHandler) {
	glGenTextures(1, &texHandler);

	glBindTexture(GL_TEXTURE_2D, texHandler);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT,
					data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  } else {
	glBindTexture(GL_TEXTURE_2D, texHandler);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT,
					data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  }
}

void CW3GLFunctions::update2DTex(unsigned int &texHandler, unsigned int width,
								 unsigned int height, unsigned short *data,
								 bool isDifferentSize) {
  if (texHandler && isDifferentSize) {
	glDeleteTextures(1, &texHandler);
	texHandler = 0;

	glGenTextures(1, &texHandler);

	glBindTexture(GL_TEXTURE_2D, texHandler);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // packing of pixel data, 1 byte
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R16, width, height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED,
					GL_UNSIGNED_SHORT, data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);  // return to defalut 4 byte
  } else if (!texHandler) {
	glGenTextures(1, &texHandler);

	glBindTexture(GL_TEXTURE_2D, texHandler);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R16, width, height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED,
					GL_UNSIGNED_SHORT, data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  } else {
	glBindTexture(GL_TEXTURE_2D, texHandler);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED,
					GL_UNSIGNED_SHORT, data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  }
}

void CW3GLFunctions::update2DTex(unsigned int &texHandler, unsigned int width,
								 unsigned int height, unsigned char *data,
								 bool isDifferentSize) {
  if (texHandler && isDifferentSize) {
	glDeleteTextures(1, &texHandler);
	texHandler = 0;

	glGenTextures(1, &texHandler);

	glBindTexture(GL_TEXTURE_2D, texHandler);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, width, height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED,
					GL_UNSIGNED_BYTE, data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  } else if (!texHandler) {
	glGenTextures(1, &texHandler);

	glBindTexture(GL_TEXTURE_2D, texHandler);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, width, height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED,
					GL_UNSIGNED_BYTE, data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  } else {
	glBindTexture(GL_TEXTURE_2D, texHandler);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED,
					GL_UNSIGNED_BYTE, data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  }
}

void CW3GLFunctions::update2DTex(unsigned int &texHandler, unsigned int width,
								 unsigned int height, const unsigned char *data,
								 bool isDifferentSize) {
  if (texHandler && isDifferentSize) {
	glDeleteTextures(1, &texHandler);
	texHandler = 0;

	glGenTextures(1, &texHandler);

	glBindTexture(GL_TEXTURE_2D, texHandler);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, width, height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED,
					GL_UNSIGNED_BYTE, data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  } else if (!texHandler) {
	glGenTextures(1, &texHandler);

	glBindTexture(GL_TEXTURE_2D, texHandler);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, width, height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED,
					GL_UNSIGNED_BYTE, data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  } else {
	glBindTexture(GL_TEXTURE_2D, texHandler);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED,
					GL_UNSIGNED_BYTE, data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  }
}

void CW3GLFunctions::update3DTex(unsigned int &texHandler, unsigned int width,
								 unsigned int height, unsigned int depth,
								 unsigned short *data, bool isDifferentSize) {
  if (texHandler && isDifferentSize) {
	glDeleteTextures(1, &texHandler);
	texHandler = 0;

	glGenTextures(1, &texHandler);

	glBindTexture(GL_TEXTURE_3D, texHandler);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// pixel transfer happens here from client to OpenGL server
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexStorage3D(GL_TEXTURE_3D, 1, GL_R16, width, height, depth);
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, GL_RED,
					GL_UNSIGNED_SHORT, data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  } else if (!texHandler) {
	glGenTextures(1, &texHandler);
	glBindTexture(GL_TEXTURE_3D, texHandler);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// pixel transfer happens here from client to OpenGL server
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexStorage3D(GL_TEXTURE_3D, 1, GL_R16, width, height, depth);
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, GL_RED,
					GL_UNSIGNED_SHORT, data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  } else {
	glBindTexture(GL_TEXTURE_3D, texHandler);
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, GL_RED,
					GL_UNSIGNED_SHORT, data);
  }
}

void CW3GLFunctions::update3DTex2Dpointer(
	unsigned int &texHandler, unsigned int width, unsigned int height,
	unsigned int depth, unsigned short **data, bool isDifferentSize) {
  if (texHandler && isDifferentSize) {
	glDeleteTextures(1, &texHandler);
	texHandler = 0;

	glGenTextures(1, &texHandler);

	glBindTexture(GL_TEXTURE_3D, texHandler);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// pixel transfer happens here from client to OpenGL server
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexStorage3D(GL_TEXTURE_3D, 1, GL_R16, width, height, depth);
	for (int i = 0; i < depth; i++) {
	  glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, i, width, height, 1, GL_RED,
					  GL_UNSIGNED_SHORT, data[i]);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  } else if (!texHandler) {
	glGenTextures(1, &texHandler);
	glBindTexture(GL_TEXTURE_3D, texHandler);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// pixel transfer happens here from client to OpenGL server
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexStorage3D(GL_TEXTURE_3D, 1, GL_R16, width, height, depth);
	for (int i = 0; i < depth; i++) {
	  glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, i, width, height, 1, GL_RED,
					  GL_UNSIGNED_SHORT, data[i]);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  } else {
	glBindTexture(GL_TEXTURE_3D, texHandler);
	for (int i = 0; i < depth; i++) {
	  glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, i, width, height, 1, GL_RED,
					  GL_UNSIGNED_SHORT, data[i]);
	}
  }
}

void CW3GLFunctions::initTF2DTex(unsigned int &texHandler, unsigned int width,
	unsigned int height, float *data) {
	if (texHandler)
	{
		glDeleteTextures(1, &texHandler);
		texHandler = 0;
	}

	glGenTextures(1, &texHandler);
	glBindTexture(GL_TEXTURE_2D, texHandler);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA, GL_FLOAT, data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

void CW3GLFunctions::initTF2DTex(unsigned int &texHandler, unsigned int width, unsigned int height) 
{
	if (texHandler)
	{
		glDeleteTextures(1, &texHandler);
		texHandler = 0;
	}

	glGenTextures(1, &texHandler);
	glBindTexture(GL_TEXTURE_2D, texHandler);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
}

// Jung : clamp_to_border 로 왜 했었지?
void CW3GLFunctions::initVol3DTex2Dpointer(unsigned int &texHandler,
										   unsigned int width,
										   unsigned int height,
										   unsigned int depth,
										   unsigned short **data) {
#if 0
  int nCurGPUMemKb = 0;
  glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &nCurGPUMemKb);
  std::string strLogMsg = QString("Before 3D Texture set, current gpu mem : %1")
							  .arg(nCurGPUMemKb)
							  .toStdString();
  common::Logger::instance()->Print(common::LogType::INF, strLogMsg);
#endif

  if (texHandler)
  {
	  glDeleteTextures(1, &texHandler);
	  texHandler = 0;
  }

	glGenTextures(1, &texHandler);
	glBindTexture(GL_TEXTURE_3D, texHandler);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	// pixel transfer happens here from client to OpenGL server
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexStorage3D(GL_TEXTURE_3D, 1, GL_R16, width, height, depth);
	for (int i = 0; i < depth; i++) {
	  glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, i, width, height, 1, GL_RED,
					  GL_UNSIGNED_SHORT, data[i]);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

#if 0
  glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &nCurGPUMemKb);
  strLogMsg = QString("After 3D Texture set, current gpu mem : %1")
				  .arg(nCurGPUMemKb)
				  .toStdString();
  common::Logger::instance()->Print(common::LogType::INF, strLogMsg);
#endif
}
void CW3GLFunctions::initVol3DTex2Dpointer(unsigned int &texHandler,
										   unsigned int width,
										   unsigned int height,
										   unsigned int depth,
										   unsigned short *data) {
	if (texHandler)
	{
		glDeleteTextures(1, &texHandler);
		texHandler = 0;
	}

	glGenTextures(1, &texHandler);
	glBindTexture(GL_TEXTURE_3D, texHandler);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	// pixel transfer happens here from client to OpenGL server
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexStorage3D(GL_TEXTURE_3D, 1, GL_R16, width, height, depth);
	for (int i = 0; i < depth; i++) {
	  glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, i, width, height, 1, GL_RED,
					  GL_UNSIGNED_SHORT, &data[i * width * height]);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

void CW3GLFunctions::InitVRCutVol3DTex2Dpointer(unsigned int &texHandler,
												unsigned int width,
												unsigned int height,
												unsigned int depth,
												unsigned short **data) {
#if 0
  int nCurGPUMemKb = 0;
  glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &nCurGPUMemKb);
  std::string strLogMsg =
	  QString("Before VR Cut 3D Texture set, current gpu mem : %1")
		  .arg(nCurGPUMemKb)
		  .toStdString();
  common::Logger::instance()->Print(common::LogType::INF, strLogMsg);
#endif

  if (texHandler)
  {
	  glDeleteTextures(1, &texHandler);
	  texHandler = 0;
  }

	glGenTextures(1, &texHandler);
	glBindTexture(GL_TEXTURE_3D, texHandler);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// pixel transfer happens here from client to OpenGL server
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexStorage3D(GL_TEXTURE_3D, 1, GL_R16UI, width, height, depth);
	for (int i = 0; i < depth; i++) {
	  glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, i, width, height, 1,
					  GL_RED_INTEGER, GL_UNSIGNED_SHORT, data[i]);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

#if 0
  glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &nCurGPUMemKb);
  strLogMsg = QString("After VR Cut 3D Texture set, current gpu mem : %1")
				  .arg(nCurGPUMemKb)
				  .toStdString();
  common::Logger::instance()->Print(common::LogType::INF, strLogMsg);
#endif
}

void CW3GLFunctions::initVolMinMax3DTex(unsigned int &texHandler,
										unsigned int width, unsigned int height,
										unsigned int depth,
										unsigned short *data) {
	if (texHandler)
	{
		glDeleteTextures(1, &texHandler);
		texHandler = 0;
	}

	glGenTextures(1, &texHandler);
	glBindTexture(GL_TEXTURE_3D, texHandler);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// pixel transfer happens here from client to OpenGL server
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexStorage3D(GL_TEXTURE_3D, 1, GL_RG16, width, height, depth);
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, GL_RG,
					GL_UNSIGNED_SHORT, data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

void CW3GLFunctions::initVolStep3DTex(unsigned int &texHandler,
									  unsigned int width, unsigned int height,
									  unsigned int depth, unsigned char *data) {
	if (texHandler)
	{
		glDeleteTextures(1, &texHandler);
		texHandler = 0;
	}

	glGenTextures(1, &texHandler);
	glBindTexture(GL_TEXTURE_3D, texHandler);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// pixel transfer happens here from client to OpenGL server
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexStorage3D(GL_TEXTURE_3D, 1, GL_R8, width, height, depth);
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, GL_RED,
					GL_UNSIGNED_BYTE, data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

// Jung : VBO 개수 일관성있게 바꾸기
void CW3GLFunctions::initVBO(unsigned int *vbo, float *vertCoord,
							 float *texCoord, int Ndata,
							 unsigned int *vertIndex, int Nindex) {
  if (vbo[0]) {
	glDeleteBuffers(3, vbo);
  }

  glGenBuffers(3, vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, Ndata * sizeof(float), vertCoord,
			   GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, Ndata * sizeof(float), texCoord,
			   GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, Nindex * sizeof(unsigned int),
			   vertIndex, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CW3GLFunctions::initVBO(unsigned int *vbo, float *vertCoord,
							 float *texCoord, float *normals, int Ndata,
							 unsigned int *vertIndex, int Nindex) {
  if (vbo[0]) {
	glDeleteBuffers(4, vbo);
  }

  glGenBuffers(4, vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, Ndata * 3 * sizeof(float), vertCoord,
			   GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, Ndata * 3 * sizeof(float), normals,
			   GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
  glBufferData(GL_ARRAY_BUFFER, Ndata * 2 * sizeof(float), texCoord,
			   GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, Nindex * sizeof(unsigned int),
			   vertIndex, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CW3GLFunctions::initVBO(unsigned int *vbo, float *vertCoord,
							 float *texCoord, int Ndata) {
  if (vbo[0]) {
	glDeleteBuffers(3, vbo);
  }

  glGenBuffers(3, vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, Ndata * sizeof(float), vertCoord,
			   GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, Ndata * sizeof(float), texCoord,
			   GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CW3GLFunctions::initVBOPR(unsigned int *vbo, float *vertCoord, int Ndata) {
  if (vbo[0]) {
	glDeleteBuffers(1, vbo);
  }

  glGenBuffers(1, vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, Ndata * sizeof(float), vertCoord,
			   GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CW3GLFunctions::initVBOpointOnly(unsigned int *vbo, float *vertCoord,
									  int Ndata, unsigned int *vertIndex,
									  int Nindex) {
  if (vbo[0]) {
	glDeleteBuffers(2, vbo);
  }

  glGenBuffers(2, vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, Ndata * sizeof(float), vertCoord,
			   GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, Nindex * sizeof(unsigned int),
			   vertIndex, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CW3GLFunctions::InitVAOvn(unsigned int* vao, unsigned int* vbo)
{
	if (*vao)
	{
		glDeleteVertexArrays(1, vao);
		*vao = 0;
	}

	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, static_cast<float*>(NULL));
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, static_cast<float*>(NULL));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CW3GLFunctions::initVAO(unsigned int *vao, unsigned int *vbo) {
  if (*vao) {
	glDeleteVertexArrays(1, vao);
  }

  glGenVertexArrays(1, vao);
  printError(__LINE__, "glGenVertexArrays failed.");

  glBindVertexArray(*vao);

  glEnableVertexAttribArray(0);  // for vertexloc
  glEnableVertexAttribArray(1);  // for vertexcol

  // the vertex location is the same as the vertex color
  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CW3GLFunctions::initVAOImplant(unsigned int *vao, unsigned int *vbo) {
  if (*vao) {
	glDeleteVertexArrays(1, vao);
  }

  glGenVertexArrays(1, vao);
  glBindVertexArray(*vao);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  // glEnableVertexAttribArray(2);

  // the vertex location is the same as the vertex color
  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);  // coordinate
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);  // normal
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);

  glBindVertexArray(0); 
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CW3GLFunctions::initVAOSR(unsigned int *vao, unsigned int *vbo) {
  if (*vao) {
	glDeleteVertexArrays(1, vao);
  }

  glGenVertexArrays(1, vao);
  glBindVertexArray(*vao);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  // the vertex location is the same as the vertex color
  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CW3GLFunctions::initVAOpointOnly(unsigned int *vao, unsigned int *vbo) {
  if (*vao) {
	glDeleteVertexArrays(1, vao);
  }

  glGenVertexArrays(1, vao);
  glBindVertexArray(*vao);

  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CW3GLFunctions::initVAOPR(unsigned int *vao, unsigned int *vbo) {
  if (*vao) glDeleteVertexArrays(1, vao);

  glGenVertexArrays(1, vao);
  glBindVertexArray(*vao);

  glEnableVertexAttribArray(0);

  // the vertex location is the same as the vertex color
  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CW3GLFunctions::initFrameBuffer(unsigned int &FBhandler,
									 unsigned int &DepthHandler,
									 unsigned int &texHandler, unsigned int nx,
									 unsigned int ny) {
  if (FBhandler) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &FBhandler);
	FBhandler = 0;
  }

  printError(__LINE__, "initFrameBuffer failed.");

  if (texHandler) {
	glDeleteTextures(1, &texHandler);
	texHandler = 0;
  }

  printError(__LINE__, "initFrameBuffer failed.");

  if (DepthHandler) {
	glDeleteRenderbuffers(1, &DepthHandler);
	DepthHandler = 0;
  }

  printError(__LINE__, "initFrameBuffer failed.");

  glGenTextures(1, &texHandler);

  printError(__LINE__, "initFrameBuffer failed.");

  glBindTexture(GL_TEXTURE_2D, texHandler);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, nx, ny);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  printError(__LINE__, "initFrameBuffer failed.");

  glGenRenderbuffers(1, &DepthHandler);
  glBindRenderbuffer(GL_RENDERBUFFER, DepthHandler);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, nx, ny);

  printError(__LINE__, "initFrameBuffer failed.");

  glGenFramebuffers(1, &FBhandler);
  glBindFramebuffer(GL_FRAMEBUFFER, FBhandler);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
						 texHandler, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
							GL_RENDERBUFFER, DepthHandler);

  printError(__LINE__, "initFrameBuffer failed.");

  checkFramebufferStatus();

  printError(__LINE__, "initFrameBuffer failed.");

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  printError(__LINE__, "initFrameBuffer failed.");
}

void CW3GLFunctions::initFrameBufferID(unsigned int &FBhandler,
									   unsigned int &DepthHandler,
									   unsigned int &texHandler,
									   unsigned int nx, unsigned int ny) {
  if (FBhandler) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &FBhandler);
	FBhandler = 0;
  }

  if (texHandler) {
	glDeleteTextures(1, &texHandler);
	texHandler = 0;
  }

  if (DepthHandler) {
	glDeleteRenderbuffers(1, &DepthHandler);
	DepthHandler = 0;
  }

  glGenTextures(1, &texHandler);

  glBindTexture(GL_TEXTURE_2D, texHandler);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_RED, nx, ny);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glGenRenderbuffers(1, &DepthHandler);
  glBindRenderbuffer(GL_RENDERBUFFER, DepthHandler);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, nx, ny);

  glGenFramebuffers(1, &FBhandler);
  glBindFramebuffer(GL_FRAMEBUFFER, FBhandler);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
						 texHandler, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
							GL_RENDERBUFFER, DepthHandler);

  checkFramebufferStatus();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  printError(__LINE__, "initFrameBufferID failed.");
}

void CW3GLFunctions::initFrameBufferWOdepth(unsigned int &FBhandler,
											unsigned int &texHandler,
											unsigned int nx, unsigned int ny) {
  if (FBhandler) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &FBhandler);
	FBhandler = 0;
  }

  if (texHandler) {
	glDeleteTextures(1, &texHandler);
	texHandler = 0;
  }

  glGenTextures(1, &texHandler);

  glBindTexture(GL_TEXTURE_2D, texHandler);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, nx, ny);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glGenFramebuffers(1, &FBhandler);
  glBindFramebuffer(GL_FRAMEBUFFER, FBhandler);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
						 texHandler, 0);

  checkFramebufferStatus();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  printError(__LINE__, "initFrameBufferWOdepth failed.");
}

void CW3GLFunctions::readTexture2D(unsigned int Texhandler, float *data) {
  glBindTexture(GL_TEXTURE_2D, Texhandler);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data);
}

void CW3GLFunctions::readTextureR2D(unsigned int Texhandler, float *data) {
  glBindTexture(GL_TEXTURE_2D, Texhandler);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, data);
}

void CW3GLFunctions::readTextureRG2D(unsigned int Texhandler, float *data) {
  glBindTexture(GL_TEXTURE_2D, Texhandler);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, data);
}

void CW3GLFunctions::readTextureRGB2D(unsigned int Texhandler, float *data) {
  glBindTexture(GL_TEXTURE_2D, Texhandler);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, data);
}

void CW3GLFunctions::printError(int a_lineNum, const char *msg) {
  common::Logger *logger = common::Logger::instance();
  if (!logger->IsDebugMode()) return;

  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
	std::string errMsg = std::string("[line: ") + std::to_string(a_lineNum) +
						 std::string(" ] ") + msg;

	logger->Print(common::LogType::ERR, errMsg);
	switch (err) {
	  case GL_INVALID_ENUM:
		logger->Print(
			common::LogType::ERR,
			"An unacceptable value is specified for an enumerated argument.");
		assert(false);
		break;
	  case GL_INVALID_VALUE:
		logger->Print(common::LogType::ERR,
					  "A numeric argument is out of range.");
		assert(false);
		break;
	  case GL_INVALID_OPERATION:
		logger->Print(
			common::LogType::ERR,
			"The specified operation is not allowed in the current state.");
		assert(false);
		break;
	  case GL_INVALID_FRAMEBUFFER_OPERATION:
		logger->Print(common::LogType::ERR,
					  "The framebuffer object is not complete.");
		assert(false);
		break;
	  case GL_OUT_OF_MEMORY:
		logger->Print(
			common::LogType::ERR,
			"There is not enough memory left to execute the command.");
		assert(false);
		break;
	  case GL_STACK_UNDERFLOW:
		logger->Print(common::LogType::ERR,
					  "An attempt has been made to perform an operation that "
					  "would cause an internal stack to underflow.");
		assert(false);
		break;
	  case GL_STACK_OVERFLOW:
		logger->Print(common::LogType::ERR,
					  "An attempt has been made to perform an operation that "
					  "would cause an internal stack to overflow.");
		assert(false);
		break;
	  default:
		logger->Print(common::LogType::ERR, "UNKNOWN ERROR CODE.");
		assert(false);
		break;
	}
  }
}

void CW3GLFunctions::checkFramebufferStatus() {
  GLenum complete = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (complete != GL_FRAMEBUFFER_COMPLETE) {
	  common::Logger::instance()->Print(
		  common::LogType::ERR, 
		  std::string("ERROR: framebuffer is not complete! / ") + QString::number(complete).toStdString()
	  );
  }
}

void CW3GLFunctions::initVBO(unsigned int *vbo, float *vertCoord,
							 float *texCoord, int Nvert, int Ntex) {
  glGenBuffers(2, vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, Nvert * sizeof(float), vertCoord,
			   GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, Ntex * sizeof(float), texCoord, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void CW3GLFunctions::initVBO(unsigned int *vbo,
							 const std::vector<glm::vec3> &vertCoord,
							 const std::vector<glm::vec3> &texCoord) {
  glGenBuffers(2, vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, vertCoord.size() * 3 * sizeof(float),
			   &vertCoord[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, texCoord.size() * 3 * sizeof(float),
			   &texCoord[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void CW3GLFunctions::initVBO(unsigned int *vbo,
							 const std::vector<glm::vec3> &vertCoord,
							 const std::vector<glm::vec3> &normalCoord,
							 const std::vector<unsigned int> indices) {
  glGenBuffers(3, vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, (vertCoord.size()) * sizeof(float) * 3,
			   &vertCoord[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, (normalCoord.size()) * sizeof(float) * 3,
			   &normalCoord[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, (indices.size()) * sizeof(unsigned int),
			   &indices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void CW3GLFunctions::initVBO(unsigned int *vbo,
							 const std::vector<glm::vec3> &vertCoord,
							 const std::vector<glm::vec2> &texCoord,
							 const std::vector<unsigned int> indices) {
  glGenBuffers(3, vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, (vertCoord.size()) * sizeof(float) * 3,
			   &vertCoord[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, (texCoord.size()) * sizeof(float) * 2,
			   &texCoord[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, (indices.size()) * sizeof(unsigned int),
			   &indices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CW3GLFunctions::initVAOVBO(unsigned int *vao, unsigned int *vbo,
								const std::vector<glm::vec3> &vertices,
								const std::vector<glm::vec3> &normals,
								const std::vector<unsigned int> &indices) {
  if (vertices.size() == 0) {
	common::Logger::instance()->Print(
		common::LogType::ERR, "CW3GLFunctions::initVAOVBO: vert empty.");
	assert(false);
  }

  // create vbo
  glGenBuffers(3, vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, (vertices.size()) * sizeof(float) * 3,
			   &vertices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, (normals.size()) * sizeof(float) * 3,
			   &normals[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, (indices.size()) * sizeof(unsigned int),
			   &indices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // create vao
  glGenVertexArrays(1, vao);
  glBindVertexArray(*vao);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void CW3GLFunctions::initVAOVBO(unsigned int *vao, unsigned int *vbo,
								const std::vector<glm::vec3> &vertices,
								const std::vector<glm::vec3> &normals) {
  if (vertices.size() == 0) {
	common::Logger::instance()->Print(
		common::LogType::ERR, "CW3GLFunctions::initVAOVBO: vert empty.");
	assert(false);
  }
  // create vbo
  glGenBuffers(3, vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, (vertices.size()) * sizeof(float) * 3,
			   &vertices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, (normals.size()) * sizeof(float) * 3,
			   &normals[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // create vao
  glGenVertexArrays(1, vao);
  glBindVertexArray(*vao);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CW3GLFunctions::InitVAOVBO(unsigned int* vao, unsigned int* vbo, const std::vector<glm::vec3>& vertices)
{
	if (vertices.size() == 0)
	{
		common::Logger::instance()->Print(common::LogType::ERR, "CW3GLFunctions::initVAOVBO: vert empty.");
		assert(false);
	}
	glGenBuffers(1, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 3, &vertices[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, vao);

	glBindVertexArray(*vao);
	{
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);
	}
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CW3GLFunctions::initVAOVBO(unsigned int *vao, unsigned int *vbo,
								const std::vector<glm::vec3> &vertices,
								const std::vector<unsigned int> &indices) {
  if (vertices.size() == 0) {
	common::Logger::instance()->Print(
		common::LogType::ERR, "CW3GLFunctions::initVAOVBO: vert empty.");
	assert(false);
  }
  glGenBuffers(2, vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, (vertices.size()) * sizeof(float) * 3,
			   &vertices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, (indices.size()) * sizeof(unsigned int),
			   &indices[0], GL_STATIC_DRAW);

  glGenVertexArrays(1, vao);

  glBindVertexArray(*vao);
  {
	glEnableVertexAttribArray(0);  // Vertex position
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0,
						  ((GLubyte *)NULL + (0)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
  }
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void CW3GLFunctions::initVAOVBO(unsigned int *vao, unsigned int *vbo,
								const std::vector<glm::vec3> &vertices,
								const std::vector<glm::vec2> &texCoords,
								const std::vector<unsigned int> &indices) {
  if (vertices.size() == 0) {
	common::Logger::instance()->Print(
		common::LogType::ERR, "CW3GLFunctions::initVAOVBO: vert empty.");
	assert(false);
  }
  // create vbo
  glGenBuffers(3, vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, (vertices.size()) * sizeof(float) * 3,
			   &vertices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, (texCoords.size()) * sizeof(float) * 2,
			   &texCoords[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, (indices.size()) * sizeof(unsigned int),
			   &indices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // create vao
  glGenVertexArrays(1, vao);
  glBindVertexArray(*vao);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CW3GLFunctions::initFrameBufferMultiTexture(
	unsigned int &FBhandler, unsigned int &DepthHandler,
	unsigned int *texHandler, unsigned int nx, unsigned int ny, GLenum *texNum,
	int texHandlerCount) {
  for (int i = 0; i < texHandlerCount; i++) {
	glActiveTexture(texNum[i]);

	if (texHandler[i]) {
	  glDeleteTextures(1, (texHandler + i));
	  *(texHandler + i) = 0;
	}

	glGenTextures(1, texHandler + i);

	glBindTexture(GL_TEXTURE_2D, *(texHandler + i));
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, nx, ny);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  if (DepthHandler) {
	glDeleteRenderbuffers(1, &DepthHandler);
	DepthHandler = 0;
  }

  glGenRenderbuffers(1, &DepthHandler);
  glBindRenderbuffer(GL_RENDERBUFFER, DepthHandler);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, nx, ny);

  if (FBhandler) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &FBhandler);
	FBhandler = 0;
  }

  glGenFramebuffers(1, &FBhandler);
  glBindFramebuffer(GL_FRAMEBUFFER, FBhandler);

  GLenum colorAttachIdx = GL_COLOR_ATTACHMENT0;
  for (int i = 0; i < texHandlerCount; i++) {
	glFramebufferTexture2D(GL_FRAMEBUFFER, colorAttachIdx++, GL_TEXTURE_2D,
						   *(texHandler + i), 0);
  }

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
							GL_RENDERBUFFER, DepthHandler);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  CW3GLFunctions::checkFramebufferStatus();
}

void CW3GLFunctions::initFrameBufferMultiTexture(
	unsigned int &FBhandler, unsigned int &DepthHandler,
	std::vector<unsigned int> &texHandler, unsigned int nx, unsigned int ny,
	std::vector<GLenum> &texNum) {
  for (int i = 0; i < texHandler.size(); i++) {
	  if (texHandler[i])
	  {
		  glDeleteTextures(1, &(texHandler[i]));
		  (texHandler[i]) = 0;
	  }

	glGenTextures(1, &texHandler[i]);

	glActiveTexture(texNum[i]);
	glBindTexture(GL_TEXTURE_2D, (texHandler[i]));
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, nx, ny);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  if (DepthHandler) {
	glDeleteRenderbuffers(1, &DepthHandler);
	DepthHandler = 0;
  }

  glGenRenderbuffers(1, &DepthHandler);
  glBindRenderbuffer(GL_RENDERBUFFER, DepthHandler);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, nx, ny);

  if (FBhandler) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &FBhandler);
	FBhandler = 0;
  }

  glGenFramebuffers(1, &FBhandler);
  glBindFramebuffer(GL_FRAMEBUFFER, FBhandler);

  GLenum colorAttachIdx = GL_COLOR_ATTACHMENT0;
  for (int i = 0; i < texHandler.size(); i++) {
	glFramebufferTexture2D(GL_FRAMEBUFFER, colorAttachIdx++, GL_TEXTURE_2D,
						   texHandler[i], 0);
  }

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
							GL_RENDERBUFFER, DepthHandler);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  CW3GLFunctions::checkFramebufferStatus();
}

void CW3GLFunctions::initFrameBufferMultiTexture(
	unsigned int &FBhandler, std::vector<unsigned int> &DepthHandler,
	std::vector<unsigned int> &texHandler, unsigned int nx, unsigned int ny,
	std::vector<GLenum> &texNum) {
  for (int i = 0; i < texHandler.size(); i++) {
	if (texHandler[i]) {
	  glDeleteTextures(1, &(texHandler[i]));
	  (texHandler[i]) = 0;
	}

	glGenTextures(1, &texHandler[i]);

	glActiveTexture(texNum[i]);
	glBindTexture(GL_TEXTURE_2D, (texHandler[i]));
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, nx, ny);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  for (int i = 0; i < DepthHandler.size(); i++) {
	if (DepthHandler[i]) {
	  glDeleteRenderbuffers(1, &DepthHandler[i]);
	  DepthHandler[i] = 0;
	}

	glGenRenderbuffers(1, &DepthHandler[i]);
	glBindRenderbuffer(GL_RENDERBUFFER, DepthHandler[i]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, nx, ny);
  }

  if (FBhandler) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &FBhandler);
	FBhandler = 0;
  }

  glGenFramebuffers(1, &FBhandler);
  glBindFramebuffer(GL_FRAMEBUFFER, FBhandler);

  GLenum colorAttachIdx = GL_COLOR_ATTACHMENT0;
  for (int i = 0; i < texHandler.size(); i++) {
	glFramebufferTexture2D(GL_FRAMEBUFFER, colorAttachIdx++, GL_TEXTURE_2D,
						   texHandler[i], 0);
  }

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
							GL_RENDERBUFFER, DepthHandler[0]);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  CW3GLFunctions::checkFramebufferStatus();
}
void CW3GLFunctions::writeFileDepthmap(const unsigned int width,
									   const unsigned int height,
									   const QString &filePath) {
  printf("W, H: %d, %d\n", width, height);
  float *depthMap = nullptr;
  W3::p_allocate_1D(&depthMap, width * height);

  glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depthMap);

  FILE *FWRITE = nullptr;

#if defined(__APPLE__)
  FWRITE = fopen(filePath.toStdString().c_str(), "wb");
#else
  fopen_s(&FWRITE, filePath.toStdString().c_str(), "wb");
#endif

  if (FWRITE == nullptr) return;

  fwrite(depthMap, sizeof(float), width * height, FWRITE);
  fclose(FWRITE);
  SAFE_DELETE_ARRAY(depthMap);
}

void CW3GLFunctions::writeFileJPGdepthmap(const unsigned int width,
										  const unsigned int height,
										  const QString &filePath) {
  printf("W, H: %d, %d\n", width, height);
  int size = width * height;

  float *buffer = new float[size];
  memset(buffer, 0, sizeof(float) * size);

  uchar *imgBuffer = new uchar[size * 4];
  memset(imgBuffer, 0, sizeof(uchar) * size * 4);

  glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, buffer);

  float max = std::numeric_limits<float>::min();
  float min = std::numeric_limits<float>::max();

  for (int i = 0; i < size; i++) {
	max = (max > buffer[i]) ? max : buffer[i];
	min = (min < buffer[i]) ? min : buffer[i];
  }

  auto normUCHAR = [](float max, float min, float data) -> uchar {
	return static_cast<uchar>(((data - min) / (max - min)) * 255.0);
  };

  for (int i = 0; i < height; i++) {
	for (int j = 0; j < width; j++) {
	  int imgIdx = ((i * width) * 4 + j * 4);
	  int bufIdx = ((height - i - 1) * width) + j;

	  uchar gray = normUCHAR(max, min, buffer[bufIdx]);

	  imgBuffer[imgIdx] = gray;
	  imgBuffer[imgIdx + 1] = gray;
	  imgBuffer[imgIdx + 2] = gray;
	  imgBuffer[imgIdx + 3] = 1.0f;
	}
  }

  QImage image(imgBuffer, width, height, QImage::Format_RGBA8888);

  QImageWriter imgWriter(filePath, "jpg");
  imgWriter.write(image);

  delete[] buffer;
  delete[] imgBuffer;
}

void CW3GLFunctions::writeFileFrambuffer(unsigned int width,
										 unsigned int height,
										 const QString &filePath) {
  printf("W, H: %d, %d\n", width, height);
  int size = width * height;

  float *buffer = new float[size * 4];
  memset(buffer, 0, sizeof(float) * size * 4);
  glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, buffer);

  float *imgBuffer = new float[size];
  for (int i = 0; i < height; i++) {
	for (int j = 0; j < width; j++) {
	  int imgIdx = ((i * width) + j);
	  int bufIdx = ((i * width * 4) + j * 4);

	  float r = buffer[bufIdx];
	  float g = buffer[bufIdx + 1];
	  float b = buffer[bufIdx + 2];

	  float gray = ((r + g + b) / 3.0f);

	  imgBuffer[imgIdx] = (gray > 1.0f) ? 1.0f : gray;
	}
  }

  FILE *FWRITE = nullptr;

#if defined(__APPLE__)
  FWRITE = fopen(filePath.toStdString().c_str(), "wb");
#else
  fopen_s(&FWRITE, filePath.toStdString().c_str(), "wb");
#endif

  if (FWRITE == nullptr) return;

  fwrite(imgBuffer, sizeof(float), width * height, FWRITE);
  fclose(FWRITE);

  delete[] imgBuffer;
  delete[] buffer;
}

void CW3GLFunctions::writeFileJPGFrambuffer(unsigned int width,
											unsigned int height,
											const QString &filePath) {
  printf("W, H: %d, %d\n", width, height);
  int size = width * height;

  float *buffer = new float[size * 4];
  memset(buffer, 0, sizeof(float) * size * 4);
  glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, buffer);

  uchar *imgBuffer = new uchar[size * 4];
  for (int j = 0; j < height; j++) {
	for (int i = 0; i < width * 4; i++) {
	  int imgIdx = ((j * width * 4) + i);
	  int bufIdx = (((height - 1) - j) * width * 4) + i;

	  // buffer값이 0.0f ~ 1.0f라 가정하고 255을 곱함.
	  uchar val = (uchar)(buffer[bufIdx] * 255.0f);
	  imgBuffer[imgIdx] = (val) > 255 ? 255 : (val < 0) ? 0 : val;
	}
  }

  QImage image(imgBuffer, width, height, QImage::Format_RGBA8888);

  QImageWriter imgWriter(filePath, "jpg");
  imgWriter.write(image);

  delete[] buffer;
  delete[] imgBuffer;
}
void CW3GLFunctions::writeFileTexture2D(const GLenum texHandler,
										unsigned int width, unsigned int height,
										const QString &filePath) {
  printf("W, H: %d, %d\n", width, height);
  int size = width * height;

  float *buffer = new float[size * 4];
  memset(buffer, 0, sizeof(float) * size * 4);

  float *imgBuffer = new float[size];
  glBindTexture(GL_TEXTURE_2D, texHandler);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, buffer);

  for (int i = 0; i < height; i++) {
	for (int j = 0; j < width; j++) {
	  int imgIdx = ((i * width) + j);
	  int bufIdx = ((height - i - 1) * width * 4) + j * 4;

	  float r = buffer[bufIdx];
	  float g = buffer[bufIdx + 1];
	  float b = buffer[bufIdx + 2];

	  float gray = ((r + g + b) / 3.0f);

	  imgBuffer[imgIdx] = (gray > 1.0f) ? 1.0f : gray;
	}
  }

  FILE *FWRITE = nullptr;

#if defined(__APPLE__)
  FWRITE = fopen(filePath.toStdString().c_str(), "wb");
#else
  fopen_s(&FWRITE, filePath.toStdString().c_str(), "wb");
#endif

  if (FWRITE == nullptr) return;

  fwrite(imgBuffer, sizeof(float), width * height, FWRITE);
  fclose(FWRITE);

  delete[] imgBuffer;
  delete[] buffer;
}
void CW3GLFunctions::writeFileJPGTexture2D(const GLenum texHandler,
										   unsigned int width,
										   unsigned int height,
										   const QString &filePath) {
  printf("W, H: %d, %d\n", width, height);
  int size = width * height;

  float *buffer = new float[size * 4];
  memset(buffer, 0, sizeof(float) * size * 4);

  uchar *imgBuffer = new uchar[size * 4];
  memset(imgBuffer, 0, sizeof(uchar) * size * 4);

  glBindTexture(GL_TEXTURE_2D, texHandler);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, buffer);

  float max = std::numeric_limits<float>::min();
  float min = std::numeric_limits<float>::max();

  for (int i = 0; i < size * 4; i++) {
	max = (max > buffer[i]) ? max : buffer[i];
	min = (min < buffer[i]) ? min : buffer[i];
  }

  auto normUCHAR = [](float max, float min, float data) -> uchar {
	return static_cast<uchar>(((data - min) / (max - min)) * 255.0);
  };
  for (int i = 0; i < height; i++) {
	for (int j = 0; j < width; j++) {
	  int imgIdx = ((i * width) * 4 + j * 4);
	  int bufIdx = ((height - i - 1) * width * 4) + j * 4;

	  imgBuffer[imgIdx] = normUCHAR(max, min, buffer[bufIdx]);
	  imgBuffer[imgIdx + 1] = normUCHAR(max, min, buffer[bufIdx + 1]);
	  imgBuffer[imgIdx + 2] = normUCHAR(max, min, buffer[bufIdx + 2]);
	  imgBuffer[imgIdx + 3] = normUCHAR(max, min, buffer[bufIdx + 3]);
	}
  }

  QImage image(imgBuffer, width, height, QImage::Format_RGBA8888);

  QImageWriter imgWriter(filePath, "jpg");
  imgWriter.write(image);

  delete[] buffer;
  delete[] imgBuffer;
}
void CW3GLFunctions::writeFileBMPTexture2D(const GLenum texHandler,
										   unsigned int width,
										   unsigned int height,
										   const QString &filePath) {
  printf("W, H: %d, %d\n", width, height);
  int size = width * height;

  float *buffer = new float[size * 4];
  memset(buffer, 0, sizeof(float) * size * 4);

  uchar *imgBuffer = new uchar[size * 4];
  memset(imgBuffer, 0, sizeof(uchar) * size * 4);

  glBindTexture(GL_TEXTURE_2D, texHandler);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, buffer);

  float max = std::numeric_limits<float>::min();
  float min = std::numeric_limits<float>::max();

  for (int i = 0; i < size * 4; i++) {
	max = (max > buffer[i]) ? max : buffer[i];
	min = (min < buffer[i]) ? min : buffer[i];
  }

  auto normUCHAR = [](float max, float min, float data) -> uchar {
	return static_cast<uchar>(((data - min) / (max - min)) * 255.0);
  };
  for (int i = 0; i < height; i++) {
	for (int j = 0; j < width; j++) {
	  int imgIdx = ((i * width) * 4 + j * 4);
	  int bufIdx = ((height - i - 1) * width * 4) + j * 4;

	  imgBuffer[imgIdx] = normUCHAR(max, min, buffer[bufIdx]);
	  imgBuffer[imgIdx + 1] = normUCHAR(max, min, buffer[bufIdx + 1]);
	  imgBuffer[imgIdx + 2] = normUCHAR(max, min, buffer[bufIdx + 2]);
	  imgBuffer[imgIdx + 3] = normUCHAR(max, min, buffer[bufIdx + 3]);
	}
  }

  QImage image(imgBuffer, width, height, QImage::Format_RGBA8888);

  QImageWriter imgWriter(filePath, "bmp");
  imgWriter.write(image);

  delete[] buffer;
  delete[] imgBuffer;
}
void CW3GLFunctions::writeFileRawTexture2D(const GLenum texHandler,
										   unsigned int width,
										   unsigned int height,
										   const QString &filePath) {
  printf("W, H: %d, %d\n", width, height);
  int size = width * height;

  float *buffer = new float[size * 4];
  memset(buffer, 0, sizeof(float) * size * 4);

  float *imgBuffer = new float[size];
  glBindTexture(GL_TEXTURE_2D, texHandler);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, buffer);

  for (int i = 0; i < height; i++) {
	for (int j = 0; j < width; j++) {
	  int imgIdx = ((i * width) + j);
	  int bufIdx = ((height - i - 1) * width * 4) + j * 4;

	  float r = buffer[bufIdx];
	  float g = buffer[bufIdx + 1];
	  float b = buffer[bufIdx + 2];

	  float gray = ((r + g + b) / 3.0f);

	  imgBuffer[imgIdx] = (gray > 1.0f) ? 1.0f : gray;
	}
  }

  FILE *FWRITE = nullptr;

#if defined(__APPLE__)
  FWRITE = fopen(filePath.toStdString().c_str(), "wb");
#else
  fopen_s(&FWRITE, filePath.toStdString().c_str(), "wb");
#endif

  if (FWRITE == nullptr) return;

  fwrite(imgBuffer, sizeof(float), width * height, FWRITE);
  fclose(FWRITE);

  delete[] imgBuffer;
  delete[] buffer;
}

glm::vec4 CW3GLFunctions::readPickColor(const glm::vec2 &position,
										GLenum format, GLenum type) {
  float x = position.x;
  float y = position.y;

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  if (x < viewport[0] || y < viewport[1] || x > viewport[2] ||
	  y > viewport[3]) {
	return vec4();
  }

  if (type == GL_FLOAT) {
	float res[4];
	glReadPixels(x, viewport[3] - y, 1, 1, format, type, &res);
	return vec4(res[0], res[1], res[2], res[3]);
  } else {
	unsigned char res[4];
	glReadPixels(x, viewport[3] - y, 1, 1, format, type, &res);
	return vec4(res[0], res[1], res[2], res[3]);
  }
}

void CW3GLFunctions::setDepthStencilAttarch(uint handler) {
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
							GL_RENDERBUFFER, handler);
}

void CW3GLFunctions::setBlend(bool isEnable) {
  if (isEnable) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  } else {
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
  }
}

void CW3GLFunctions::Update2DTexRGBA32F(uint &handle, uint width, uint height,
										float *data, bool size_different) {
  UpdateTexture2D(handle, data, width, height, GL_RGBA, GL_RGBA32F, GL_FLOAT,
				  size_different);
}

void CW3GLFunctions::Update2DTexRGBA32UI(uint &handle, uint width, uint height,
										 uchar *data, bool size_different) {
  UpdateTexture2D(handle, data, width, height, GL_RGBA, GL_RGBA8,
				  GL_UNSIGNED_BYTE, size_different);
}

void CW3GLFunctions::Update2DTexRGB32UI(uint &handle, uint width, uint height,
										uchar *data, bool size_different) {
  UpdateTexture2D(handle, data, width, height, GL_RGB, GL_RGB8,
				  GL_UNSIGNED_BYTE, size_different);
}

void CW3GLFunctions::Update2DTex16UI(uint &handle, uint width, uint height,
									 ushort *data, bool size_different) {
  UpdateTexture2D(handle, data, width, height, GL_RED, GL_R16,
				  GL_UNSIGNED_SHORT, size_different);
}

void CW3GLFunctions::Update2DTex16(uint &handle, uint width, uint height,
								   short *data, bool size_different) {
  UpdateTexture2D(handle, data, width, height, GL_RED, GL_R16, GL_SHORT,
				  size_different);
}

void CW3GLFunctions::Update2DTex8(uint &handle, uint width, uint height,
								  uchar *data, bool size_different) {
  UpdateTexture2D(handle, data, width, height, GL_RED, GL_R8, GL_UNSIGNED_BYTE,
				  size_different);
}
void CW3GLFunctions::Update3DTex(uint &handle, uint width, uint height,
								 uint depth, ushort *data,
								 bool size_different) {
  if (handle && size_different) {
	glDeleteTextures(1, &handle);
	handle = 0;

	GenTexture3D(handle, GL_CLAMP_TO_EDGE);
	SetTexture3DData(data, width, height, depth, GL_RED, GL_R16,
					 GL_UNSIGNED_SHORT);
  } else if (!handle) {
	GenTexture3D(handle, GL_CLAMP_TO_EDGE);
	SetTexture3DData(data, width, height, depth, GL_RED, GL_R16,
					 GL_UNSIGNED_SHORT);
  } else {
	glBindTexture(GL_TEXTURE_3D, handle);
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, GL_RED,
					GL_UNSIGNED_SHORT, data);
  }
}

void CW3GLFunctions::Update3DTex2Dpointer(uint &handle, uint width, uint height,
										  uint depth, ushort **data,
										  bool size_different) {
  if (handle && size_different) {
	glDeleteTextures(1, &handle);
	handle = 0;

	GenTexture3D(handle, GL_CLAMP_TO_EDGE);
	SetTexture3DDataArray(data, width, height, depth, GL_RED, GL_R16,
						  GL_UNSIGNED_SHORT);
  } else if (!handle) {
	GenTexture3D(handle, GL_CLAMP_TO_EDGE);
	SetTexture3DDataArray(data, width, height, depth, GL_RED, GL_R16,
						  GL_UNSIGNED_SHORT);
  } else {
	glBindTexture(GL_TEXTURE_3D, handle);
	for (int i = 0; i < depth; i++) {
	  glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, i, width, height, 1, GL_RED,
					  GL_UNSIGNED_SHORT, data[i]);
	}
  }
}
void CW3GLFunctions::GenTexture2D(uint &handle) {
  glGenTextures(1, &handle);
  glBindTexture(GL_TEXTURE_2D, handle);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void CW3GLFunctions::GenTexture3D(uint &handle, int option) {
  glGenTextures(1, &handle);
  glBindTexture(GL_TEXTURE_3D, handle);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, option);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, option);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, option);
}

template <typename T>
void CW3GLFunctions::SetTexture2DData(T *data, int width, int height,
									  int format, int internal_format,
									  int type) {
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexStorage2D(GL_TEXTURE_2D, 1, internal_format, width, height);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, type, data);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

template <typename T>
void CW3GLFunctions::SetTexture3DData(T *data, int width, int height, int depth,
									  int format, int internal_format,
									  int type) {
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexStorage3D(GL_TEXTURE_3D, 1, internal_format, width, height, depth);
  glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, format, type,
				  data);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

template <typename T>
void CW3GLFunctions::SetTexture3DDataArray(T **data, int width, int height,
										   int depth, int format,
										   int internal_format, int type) {
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexStorage3D(GL_TEXTURE_3D, 1, internal_format, width, height, depth);
  for (int i = 0; i < depth; i++)
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, i, width, height, 1, format, type,
					data[i]);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

template <typename T>
void CW3GLFunctions::UpdateTexture2D(uint &handle, T *data, int width,
									 int height, int format,
									 int internal_format, int type,
									 bool size_different) {
  if (handle && size_different) {
	glDeleteTextures(1, &handle);
	handle = 0;
	GenTexture2D(handle);
	SetTexture2DData(data, width, height, format, internal_format, type);
  } else if (!handle) {
	GenTexture2D(handle);
	SetTexture2DData(data, width, height, format, internal_format, type);
  } else {
	glBindTexture(GL_TEXTURE_2D, handle);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, type, data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  }
}

void CW3GLFunctions::PrintCurrentGPUMemoryKB(const char* str) {
  int curr_memory;
  glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &curr_memory);
  std::cout << str << "_" << "curr_memory = " << curr_memory << "(KB)" << std::endl;
}

bool CW3GLFunctions::SaveTexture2D(const QString& path, const GLuint handler, const GLenum format, const GLenum type)
{
	glBindTexture(GL_TEXTURE_2D, handler);

	GLint width = 0, height = 0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	glBindTexture(GL_TEXTURE_2D, 0);

	int texture_size = width * height;

	uchar* texture = new uchar[texture_size * 4];
	memset(texture, 0, texture_size * 4 * sizeof(uchar));
	GetTexture(handler, GL_TEXTURE_2D, format, type, texture);

	QImage image(texture, width, height, QImage::Format_RGBA8888);
	bool saved = image.save(path, "PNG");

	SAFE_DELETE_ARRAY(texture);

	return saved;
}

void CW3GLFunctions::GetTexture(const GLuint handler, const GLenum target, const GLenum format, const GLenum type, void* pixels)
{
	glBindTexture(target, handler);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glGetTexImage(target, 0, format, type, pixels);

	glPixelStorei(GL_PACK_ALIGNMENT, 4);

	glBindTexture(target, 0);
}
