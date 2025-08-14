#include "base_offscreen_renderer.h"

#include "../../Common/GLfunctions/WGLHeaders.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/GLfunctions/WGLHeaders.h"

#include <assert.h>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QImage>
#include <QImageWriter>
#include <QFile>

#include "../../Common/Common/W3Logger.h"

using std::ifstream;
using std::string;

BaseOffscreenRenderer::BaseOffscreenRenderer() {
	InitializeInternal();
}

BaseOffscreenRenderer::~BaseOffscreenRenderer() {
	this->MakeCurrent();

	if (fbo_) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &fbo_);
		fbo_ = 0;
	}

	this->DoneCurrent();
}

bool BaseOffscreenRenderer::MakeCurrent() {

	if (context_->isValid()) { 
		context_->makeCurrent(surface_.get());
		return true;
	}
	else {
		common::Logger::instance()->Print(common::LogType::ERR, "The opengl context is invalid.");
		return false;
	}
}

bool BaseOffscreenRenderer::DoneCurrent() {

	if (context_->isValid()) {
		context_->doneCurrent();
		return true;
	}
	else {
		common::Logger::instance()->Print(common::LogType::ERR, "The opengl context is invalid.");
		return false;
	}
}

bool BaseOffscreenRenderer::IsValid() const {
	return (initialized_ && context_ && fbo_);
}
QPixmap BaseOffscreenRenderer::GrabFrameBufferQPixmap(int width, int height) {
	if (MakeCurrent()) {

		BindFrameBufferObject();

		int size = width*height;

		float* buffer = new float[size * 4];
		memset(buffer, 0, sizeof(float)*size * 4);

		glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, buffer);

		uchar* img_buffer = new uchar[size * 4];
		for (int j = 0; j < height; j++) {
			for (int i = 0; i < width * 4; i++) {
				int imgIdx = ((j * width * 4) + i);
				int bufIdx = (((height - 1) - j)*width * 4) + i;

				uchar val;
				val = (uchar)(buffer[bufIdx] * 255.0f);

				img_buffer[imgIdx] = val;
			}
		}

		QImage image(img_buffer, width, height, QImage::Format_RGBA8888);

		QPixmap pixmap = QPixmap::fromImage(image);

		delete[] buffer;
		delete[] img_buffer;
		ReleaseFrameBufferObject();

		DoneCurrent();

		return pixmap;
	}

	return QPixmap();

}

void BaseOffscreenRenderer::EnableGlobalShareContext() {
	context_->setShareContext(QOpenGLContext::globalShareContext());
}

void BaseOffscreenRenderer::BindFrameBufferObject() const {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
}

void BaseOffscreenRenderer::ReleaseFrameBufferObject() const {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**=================================================================================================
Attarch basic buffers.

FBO가 Bind되어 있어야 하고 MakeCurrent된 상태이어야 함. ColorTexture 한개와 DepthBuffer 한개를 생성한다.

Parameters:
width -    	buffer width.
height -   	buffer height.
 *===============================================================================================**/

void BaseOffscreenRenderer::AttarchBasicBuffers(int width, int height) {
	if (fbo_ == 0) {
		common::Logger::instance()->Print(common::LogType::ERR, "BaseOffscreenRenderer::AttarchBasicBuffer: invalid fbo.");
		assert(false);
		return;
	}

	if (buf_basic_.texture) {
		glDeleteTextures(1, &(buf_basic_.texture));
		buf_basic_.texture = 0;
	}

	glGenTextures(1, &buf_basic_.texture);
	glBindTexture(GL_TEXTURE_2D, buf_basic_.texture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (buf_basic_.depth) {
		glDeleteRenderbuffers(1, &buf_basic_.depth);
		buf_basic_.depth = 0;
	}

	glGenRenderbuffers(1, &buf_basic_.depth);
	glBindRenderbuffer(GL_RENDERBUFFER, buf_basic_.depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buf_basic_.texture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buf_basic_.depth);	

	buf_basic_.width = width;
	buf_basic_.height = height;
	buf_basic_.initialized = true;
}

void BaseOffscreenRenderer::ClearBasicBuffers() {

	if (buf_basic_.texture) {
		glDeleteTextures(1, &(buf_basic_.texture));
		buf_basic_.texture = 0;
	}

	if (buf_basic_.depth) {
		glDeleteRenderbuffers(1, &buf_basic_.depth);
		buf_basic_.depth = 0;
	}

	buf_basic_.initialized = false;
}

void BaseOffscreenRenderer::InitializeInternal() {
	bool res = InitGLcontext();

	initialized_ = false;

	if (!res)
		return;

	if (this->MakeCurrent()) {

		GLenum glewErr = glewInit();
		if (GLEW_OK != glewErr) {
			common::Logger::instance()->Print(common::LogType::ERR, "glew initialization failed.");
			this->DoneCurrent();
			return;
		}

		InitFBO();
		this->DoneCurrent();

		initialized_ = true; 
	}
}

bool BaseOffscreenRenderer::InitGLcontext() {

	context_.reset(new QOpenGLContext());
	surface_.reset(new QOffscreenSurface());
	//context_->setShareContext(QOpenGLContext::globalShareContext());

	if (!context_->create()) {
		common::Logger::instance()->Print(common::LogType::ERR, "The opengl context is invalid.");		
		return false;
	}

	QSurfaceFormat format;
	format.setSamples(4);
	context_->setFormat(format);
	surface_->setFormat(context_->format()); //sampling 추가.
	surface_->create();
	return true;
}

void BaseOffscreenRenderer::InitFBO() {
	if (fbo_) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &fbo_);
		fbo_ = 0;
	}

	glGenFramebuffers(1, &fbo_);
}
