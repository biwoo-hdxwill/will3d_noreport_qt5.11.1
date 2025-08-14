#include "texture_pack.h"

#include <QDebug>

#include "../../Common/Common/W3Logger.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLHeaders.h"
TexturePack::TexturePack() {
  for (int i = 0; i < Will3DEngine::VOL_TYPE_END; i++) {
	tex_vol_handler_[i] = 0;
  }
}

TexturePack::~TexturePack() {
  auto print_logger = []() {
	common::Logger* logger = common::Logger::instance();
	logger->Print(common::LogType::ERR,
				  "TexturePack::~TexturePack: Please clearGL to destroy this.");
  };

  for (int i = 0; i < Will3DEngine::VOL_TYPE_END; i++) {
	if (tex_vol_handler_[i]) {
	  print_logger();
	  assert(false);
	}
  }
  if (tex_tf_handler_) {
	print_logger();
	assert(false);
  }
}

void TexturePack::ClearGL() {
  for (int i = 0; i < Will3DEngine::VOL_TYPE_END; i++) {
	ClearVolumeTexture((Will3DEngine::VolType)i);
  }

  ClearTf();
}
void TexturePack::ClearVolumeTexture(const Will3DEngine::VolType& vol_type) {

  if (tex_vol_handler_[vol_type]) {
	glDeleteTextures(1, &tex_vol_handler_[vol_type]);
	tex_vol_handler_[vol_type] = 0;
  }
}

void TexturePack::ClearTf() {
  if (tex_tf_handler_) {
	glDeleteTextures(1, &tex_tf_handler_);
	tex_tf_handler_ = 0;
  }
  if (pbo_)
  {
	  glDeleteBuffers(1, &pbo_);
	  pbo_ = 0;
  }
}

void TexturePack::SetVolumeTexture(const Will3DEngine::VolType& vol_type,
								   unsigned short** data, int width, int height,
								   int depth, unsigned int* out_handler) {
  ClearVolumeTexture(vol_type);

  unsigned int hd = 0;

  if (width * height * depth != 0)
	CW3GLFunctions::initVol3DTex2Dpointer(hd, width, height, depth, data);

  tex_vol_handler_[vol_type] = hd;
  *out_handler = hd;
}
void TexturePack::SetVolumeTexture(const Will3DEngine::VolType& vol_type,
								   unsigned short* data, int width, int height,
								   int depth, unsigned int* out_handler) {
  ClearVolumeTexture(vol_type);

  unsigned int hd = 0;

  if (width * height * depth != 0)
	CW3GLFunctions::initVol3DTex2Dpointer(hd, width, height, depth, data);

  tex_vol_handler_[vol_type] = hd;
  *out_handler = hd;
}

void TexturePack::SetTfTexture(float* data, int width, int height,
							   unsigned int* out_handler) {
  glActiveTexture((GLenum)GL_TEXTURE_ID::GL_TEXTURE_TF);

  if (tex_tf_handler_ != 0 && (tf_width_ != width || tf_height_ != height))
  {
	  glBindTexture(GL_TEXTURE_2D, tex_tf_handler_);
	  glDeleteTextures(1, &tex_tf_handler_);
	  tex_tf_handler_ = 0;
  }

  if (pbo_ && (tf_width_ != width || tf_height_ != height))
  {
	  glDeleteTextures(1, &pbo_);
	  pbo_ = 0;
  }

  if (pbo_ == 0)
  {
	  glGenBuffers(1, &pbo_);
	  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_);
	  glBufferData(GL_PIXEL_UNPACK_BUFFER, sizeof(float) * 4 * width * height, nullptr, GL_STREAM_DRAW);
	  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  }

  if (tex_tf_handler_ == 0)
  {
	  CW3GLFunctions::initTF2DTex(tex_tf_handler_, width, height);
	  tf_width_ = width;
	  tf_height_ = height;	  
  }  

  glBindTexture(GL_TEXTURE_2D, tex_tf_handler_);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_);

  void* mapped_buffer = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
  if (mapped_buffer)
  {
	  memcpy(mapped_buffer, data, sizeof(float) * 4 * width * height);
	  glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

	  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA, GL_FLOAT, nullptr);
	  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  }
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  *out_handler = tex_tf_handler_;
}
