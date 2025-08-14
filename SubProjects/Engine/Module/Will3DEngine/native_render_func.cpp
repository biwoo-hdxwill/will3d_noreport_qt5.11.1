#include "native_render_func.h"

#include "../../Common/GLfunctions/WGLHeaders.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"

#include "../Renderer/volume_renderer.h"
#include "../../UIModule/UIGLObjects/view_plane_obj_gl.h"

#include "texture_pack.h"

void NativeRenderFunc::SavePresetVolumeImage(VolumeRenderer & renderer, const QString & file_path) {

	uint fbo;
	glGenFramebuffers(1, &fbo);
	enum TexID{
		TEXID_FRONT = 0,
		TEXID_BACK,
		TEXID_RAYCASTING,
		TEXID_SCREEN,
		TEXID_END
	};
	std::vector<uint> tex_handler, tex_buffer, tex_num;
	std::vector<int> _tex_num;
	tex_handler.resize(TEXID_END);

	uint depth_handler;

	for (int i = 0; i < TEXID_END; i++) {
		tex_buffer.push_back(GL_COLOR_ATTACHMENT0 + i);
		tex_num.push_back((unsigned int)TexturePack::GL_TEXTURE_ID::GL_TEXTURE_END + i);
		_tex_num.push_back((unsigned int)TexturePack::GL_TEXTURE_ID::GL_TEXTURE_END_ + i);
	}

	const int kViewportWidth = 300;
	const int kViewportHeight = 300;
	CW3GLFunctions::initFrameBufferMultiTexture(fbo, depth_handler,
												tex_handler, kViewportWidth,
												kViewportHeight, tex_num);
	std::vector<PackTexture> pack_texture;

	for (int i = 0; i < TEXID_END; i++) {
		PackTexture pack;
		pack.handler = tex_handler[i];
		pack.tex_buffer = tex_buffer[i];
		pack.tex_num = tex_num[i];
		pack._tex_num = _tex_num[i];
		pack_texture.push_back(pack);
	}

	uint vao_cube = 0;
	int indices_vol = renderer.GetActiveIndices();
	renderer.UpdateActiveBlockVAO(&vao_cube);

	std::unique_ptr<ViewPlaneObjGL> view_plane;
	view_plane.reset(new ViewPlaneObjGL);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, kViewportWidth, kViewportHeight);

	glm::vec3 vol_scale = renderer.GetModelScale();
#if 0
	float cam_fov = glm::length(vol_scale) * sqrt(2.0f);
#else
	float cam_fov = std::max(vol_scale.x, vol_scale.y);
#endif

	float left = -cam_fov;
	float right = cam_fov;
	float bottom = -cam_fov;
	float top = cam_fov;

	glm::mat4 projection = glm::ortho(left, right, bottom, top, 0.0f, cam_fov * 2.0f);
	glm::mat4 mvp = 
		projection *
		glm::lookAt(glm::vec3(0.0f, -cam_fov, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)) *
		glm::scale(vol_scale);

	renderer.SetStepSize();
	renderer.SetVolumeTexture();
	renderer.SetTFtexture();
	renderer.DrawFrontFace(
		vao_cube,
		indices_vol,
		tex_buffer[TEXID_FRONT],
		mvp);

	glActiveTexture(tex_num[TEXID_FRONT]);
	CW3GLFunctions::writeFileBMPTexture2D(tex_handler[TEXID_FRONT], kViewportWidth, kViewportHeight, file_path);
	renderer.DrawBackFace(
		vao_cube,
		indices_vol,
		tex_buffer[TEXID_BACK],
		mvp);
	glDepthFunc(GL_LESS);

	PackClipping pack_clipping;
	renderer.DrawFinalFrontFace(
		view_plane.get(),
		pack_texture[TEXID_FRONT],
		pack_texture[TEXID_BACK],
		pack_clipping);


	glDrawBuffer(tex_buffer[TEXID_RAYCASTING]); {
		CW3GLFunctions::clearView(true, GL_BACK);

		renderer.DrawRaycasting(view_plane.get(), tex_buffer[TEXID_RAYCASTING],
								  mvp, pack_texture[TEXID_FRONT], pack_texture[TEXID_BACK]);
	}


	renderer.DrawTextureToTexture(view_plane.get(), tex_buffer[TEXID_SCREEN], pack_texture[TEXID_RAYCASTING]);

	glActiveTexture(tex_num[TEXID_SCREEN]);
	CW3GLFunctions::writeFileBMPTexture2D(tex_handler[TEXID_SCREEN], kViewportWidth, kViewportHeight, file_path);
	if (fbo) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &fbo);
	}
	if (depth_handler) {
		glDeleteRenderbuffers(1, &depth_handler);
	}
	if (vao_cube) {
		glDeleteVertexArrays(1, &vao_cube);
	}
	view_plane->ClearVAOVBO();
	if (tex_handler.size() > 0) {
		glDeleteTextures(tex_handler.size(), &tex_handler[0]);
	}

}
