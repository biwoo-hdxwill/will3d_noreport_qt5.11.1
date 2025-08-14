#pragma once

/**=================================================================================================

Project:		Renderer
File:			base_offscreen_renderer.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-03-22
Last modify: 	2018-03-22

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include "renderer_global.h"

#include <memory>
#include <QPixmap>

#include "../../Resource/Resource/image_2d.h"

class QOpenGLContext;
class QOffscreenSurface;
class RENDERER_EXPORT BaseOffscreenRenderer {

public:
	BaseOffscreenRenderer();
	virtual ~BaseOffscreenRenderer();

	BaseOffscreenRenderer(const BaseOffscreenRenderer&) = delete;
	BaseOffscreenRenderer& operator=(const BaseOffscreenRenderer&) = delete;

public:
	bool MakeCurrent();
	bool DoneCurrent();
	bool IsValid() const;


	void EnableGlobalShareContext();

	QPixmap GrabFrameBufferQPixmap(int width, int height);

protected:
	struct BufferBasic {
		int width = -1;
		int height = -1;
		uint texture = 0;
		uint depth = 0 ;
		bool initialized = false;
	};

	void BindFrameBufferObject() const;
	void ReleaseFrameBufferObject() const;

	void AttarchBasicBuffers(int width, int height);
	void ClearBasicBuffers();

	const BufferBasic& buf_basic() const { return buf_basic_; }

protected:
	std::unique_ptr<QOpenGLContext> context_;
	std::unique_ptr<QOffscreenSurface> surface_;

private:
	void InitializeInternal();
	bool InitGLcontext();
	void InitFBO();

	uint fbo_ = 0;

	BufferBasic buf_basic_;

	bool initialized_ = false;
};
