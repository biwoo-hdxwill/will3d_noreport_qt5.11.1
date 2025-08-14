#pragma once
/*=========================================================================

File:			class CW3GLFunctions
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2016-04-15
Last date:		2016-04-15

=========================================================================*/
#include <string>
#include <vector>
#include "WGLHeaders.h"
#include "glfunctions_global.h"

class GLFUNCTIONS_EXPORT CW3GLFunctions {
 public:
  static void clearView(bool isDepthTest);
  static void clearView(bool isDepthTest, GLenum cullFace);
  static void clearView(bool isDepthTest, float r, float g, float b, float a);
  static void clearView(bool isDepthTest, GLenum cullFace, float r, float g,
                        float b, float a);
  static void clearDepth(GLenum depthFunc);
  static void drawView(unsigned int vaoHandler, int N);
  static void drawView(unsigned int vaoHandler, int N, GLenum cullFace);
  static void drawView(unsigned int vao, unsigned int vbo_indices, int n, GLenum cull_face);
  static void drawViewTriangles(unsigned int vaoHandler, int N,
                                GLenum cullFace);
  static void drawPoints(unsigned int vaoHandler, int N);
  static void drawLines(unsigned int vaoHandler, int N);
  static void drawWire(unsigned int vaoHandler, int N);
  static void init2DTex(unsigned int &texHandler, unsigned int width,
                        unsigned int height, unsigned short *data);
  static void init2DTexSR(unsigned int &texHandler, unsigned int width,
                          unsigned int height, unsigned char *data);
  static void update2DTex(unsigned int &texHandler, unsigned int width,
                          unsigned int height, float *data,
                          bool isDifferentSize);
  static void update2DTex(unsigned int &texHandler, unsigned int width,
                          unsigned int height, unsigned short *data,
                          bool isDifferentSize);
  static void update2DTex(unsigned int &texHandler, unsigned int width,
                          unsigned int height, unsigned char *data,
                          bool isDifferentSize);
  static void update2DTex(unsigned int &texHandler, unsigned int width,
                          unsigned int height, const unsigned char *data,
                          bool isDifferentSize);
  static void update3DTex(unsigned int &texHandler, unsigned int width,
                          unsigned int height, unsigned int depth,
                          unsigned short *data, bool isDifferentSize);
  static void update3DTex2Dpointer(unsigned int &texHandler, unsigned int width,
                                   unsigned int height, unsigned int depth,
                                   unsigned short **data, bool isDifferentSize);
  static void initTF2DTex(unsigned int &texHandler, unsigned int width,
                          unsigned int height, float *data);
  static void initTF2DTex(unsigned int &texHandler, unsigned int width, unsigned int height);
  static void initVol3DTex2Dpointer(unsigned int &texHandler,
                                    unsigned int width, unsigned int height,
                                    unsigned int depth, unsigned short **data);
  static void initVol3DTex2Dpointer(unsigned int &texHandler,
                                    unsigned int width, unsigned int height,
                                    unsigned int depth, unsigned short *data);
  static void InitVRCutVol3DTex2Dpointer(unsigned int &texHandler,
                                         unsigned int width,
                                         unsigned int height,
                                         unsigned int depth,
                                         unsigned short **data);
  static void initVolMinMax3DTex(unsigned int &texHandler, unsigned int width,
                                 unsigned int height, unsigned int depth,
                                 unsigned short *data);
  static void initVolStep3DTex(unsigned int &texHandler, unsigned int width,
                               unsigned int height, unsigned int depth,
                               unsigned char *data);
  static void initVBO(unsigned int *vbo, float *vertCoord, float *texCoord,
                      int Ndata, unsigned int *vertIndex, int Nindex);
  static void initVBO(unsigned int *vbo, float *vertCoord, float *texCoord,
                      float *normals, int Ndata, unsigned int *vertIndex,
                      int Nindex);
  static void initVBO(unsigned int *vbo, float *vertCoord, float *texCoord,
                      int Ndata);
  static void initVBOPR(unsigned int *vbo, float *vertCoord, int Ndata);
  static void initVBOpointOnly(unsigned int *vbo, float *vertCoord, int Ndata,
                               unsigned int *vertIndex, int Nindex);
  static void InitVAOvn(unsigned int* vao, unsigned int* vbo);
  static void initVAO(unsigned int *vao, unsigned int *vbo);
  static void initVAOImplant(unsigned int *vao, unsigned int *vbo);
  static void initVAOSR(unsigned int *vao, unsigned int *vbo);
  static void initVAOpointOnly(unsigned int *vao, unsigned int *vbo);
  static void initVAOPR(unsigned int *vao, unsigned int *vbo);
  static void initFrameBuffer(unsigned int &FBhandler,
                              unsigned int &DepthHandler,
                              unsigned int &texHandler, unsigned int nx,
                              unsigned int ny);
  static void initFrameBufferID(unsigned int &FBhandler,
                                unsigned int &DepthHandler,
                                unsigned int &texHandler, unsigned int nx,
                                unsigned int ny);

  static void initFrameBufferWOdepth(unsigned int &FBhandler,
                                     unsigned int &texHandler, unsigned int nx,
                                     unsigned int ny);
  static void readTexture2D(unsigned int Texhandler, float *data);
  static void readTextureR2D(unsigned int Texhandler, float *data);
  static void readTextureRG2D(unsigned int Texhandler, float *data);
  static void readTextureRGB2D(unsigned int Texhandler, float *data);
  static void printError(int a_lineNum, const char *msg);
  static void checkFramebufferStatus();

  static void initVBO(unsigned int *vbo, float *vertCoord, float *texCoord,
                      int Nvert, int Ntex);
  static void initVBO(unsigned int *vbo,
                      const std::vector<glm::vec3> &vertCoord,
                      const std::vector<glm::vec3> &texCoord);
  static void initVBO(unsigned int *vbo,
                      const std::vector<glm::vec3> &vertCoord,
                      const std::vector<glm::vec3> &normalCoord,
                      const std::vector<unsigned int> indices);
  static void initVBO(unsigned int *vbo,
                      const std::vector<glm::vec3> &vertCoord,
                      const std::vector<glm::vec2> &texCoord,
                      const std::vector<unsigned int> indices);

  static void initVAOVBO(unsigned int *vao, unsigned int *vbo,
                         const std::vector<glm::vec3> &vertices,
                         const std::vector<glm::vec3> &normals,
                         const std::vector<unsigned int> &indices);
  static void initVAOVBO(unsigned int *vao, unsigned int *vbo,
                         const std::vector<glm::vec3> &vertices,
                         const std::vector<glm::vec3> &normals);
  static void InitVAOVBO(unsigned int* vao, unsigned int* vbo, const std::vector<glm::vec3>& vertices);
  static void initVAOVBO(unsigned int *vao, unsigned int *vbo,
                         const std::vector<glm::vec3> &vertices,
                         const std::vector<unsigned int> &indices);
  static void initVAOVBO(unsigned int *vao, unsigned int *vbo,
                         const std::vector<glm::vec3> &vertices,
                         const std::vector<glm::vec2> &texCoords,
                         const std::vector<unsigned int> &indices);

  static void initFrameBufferMultiTexture(unsigned int &FBhandler,
                                          unsigned int &DepthHandler,
                                          unsigned int *texHandler,
                                          unsigned int nx, unsigned int ny,
                                          GLenum *texNum, int texHandlerCount);
  static void initFrameBufferMultiTexture(unsigned int &FBhandler,
                                          unsigned int &DepthHandler,
                                          std::vector<unsigned int> &texHandler,
                                          unsigned int nx, unsigned int ny,
                                          std::vector<GLenum> &texNum);
  static void initFrameBufferMultiTexture(unsigned int &FBhandler,
										  std::vector<unsigned int> &DepthHandler,
										  std::vector<unsigned int> &texHandler,
										  unsigned int nx, unsigned int ny,
										  std::vector<GLenum> &texNum);
  static void writeFileDepthmap(const unsigned int width,
                                const unsigned int height,
                                const QString &filePath);
  static void writeFileJPGdepthmap(const unsigned int width,
                                   const unsigned int height,
                                   const QString &filePath);
  static void writeFileFrambuffer(unsigned int width, unsigned int height,
                                  const QString &filePath);
  static void writeFileJPGFrambuffer(unsigned int width, unsigned int height,
                                     const QString &filePath);
  static void writeFileTexture2D(const GLenum texHandler, unsigned int width,
                                 unsigned int height, const QString &filePath);
  static void writeFileRawTexture2D(const GLenum texHandler, unsigned int width,
                                    unsigned int height,
                                    const QString &filePath);
  static void writeFileJPGTexture2D(const GLenum texHandler, unsigned int width,
                                    unsigned int height,
                                    const QString &filePath);
  static void writeFileBMPTexture2D(const GLenum texHandler, unsigned int width,
                                    unsigned int height,
                                    const QString &filePath);
  

  static bool SaveTexture2D(const QString& path, const GLuint handler, const GLenum format, const GLenum type);
  static void GetTexture(const GLuint handler, const GLenum target, const GLenum format, const GLenum type, void* pixels);

  static void PrintCurrentGPUMemoryKB(const char* str = "GPU");
  /**********************************************************************************************
   * @param	position	view coordinate
   * @param	format  	The format of the pixel data.
   * 						ex) GL_RED, GL_RGBA
   * @param	type		The data type of the pixel dataThe type.
   * 						ex) GL_UNSIGNED_BYTE, GL_FLOAT
   *
   * @return	The pick color.
   **************************************************************************************************/
  static glm::vec4 readPickColor(const glm::vec2 &position, GLenum format,
                                 GLenum type);

  static void setDepthStencilAttarch(uint handler);
  static void setBlend(bool isEnable);

 public:
  static void Update2DTexRGBA32F(uint &handle, uint width, uint height,
                                 float *data, bool size_different);
  static void Update2DTexRGBA32UI(uint &handle, uint width, uint height,
                                  uchar *data, bool size_different);
  static void Update2DTexRGB32UI(uint &handle, uint width, uint height,
                                 uchar *data, bool size_different);
  static void Update2DTex16UI(uint &handle, uint width, uint height,
                              ushort *data, bool size_different);
  static void Update2DTex16(uint &handle, uint width, uint height, short *data,
                            bool size_different);
  static void Update2DTex8(uint &handle, uint width, uint height, uchar *data,
                           bool size_different);
  static void Update3DTex(uint &handle, uint width, uint height, uint depth,
                          ushort *data, bool size_different);
  static void Update3DTex2Dpointer(uint &handle, uint width, uint height,
                                   uint depth, ushort **data,
                                   bool size_different);

 private:
  static void GenTexture2D(uint &handle);
  static void GenTexture3D(uint &handle, int option);


  template <typename T>
  static void SetTexture2DData(T *data, int width, int height, int format,
                               int internal_format, int type);
  template <typename T>
  static void SetTexture3DData(T *data, int width, int height, int depth,
                               int format, int internal_format, int type);

  template <typename T>
  static void SetTexture3DDataArray(T **data, int width, int height, int depth,
                                    int format, int internal_format, int type);

  template <typename T>
  static void UpdateTexture2D(uint &handle, T *data, int width, int height,
                              int format, int internal_format, int type,
                              bool size_different);
};
