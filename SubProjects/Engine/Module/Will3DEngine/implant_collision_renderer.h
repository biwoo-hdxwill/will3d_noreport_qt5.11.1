 #pragma once
/**=================================================================================================

Project:		Renderer
File:			implant_collision_renderer.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-06-01
Last modify: 	2018-06-01

Copyright (c) 2018 HDXWILL. All rights reserved.

*===============================================================================================**/
#include <memory>
#include <QObject>

#if defined(__APPLE__)
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#else
#include <GL/glm/vec3.hpp>
#include <GL/glm/mat4x4.hpp>
#endif

#include "../Renderer/implant_slice_renderer.h"
#include "will3dengine_global.h"

class ImplantObjGL;
class GLObject;

class WILL3DENGINE_EXPORT ImplantCollisionRenderer : public QObject, public BaseOffscreenRenderer{
	Q_OBJECT

public:
	ImplantCollisionRenderer(QObject* parent = 0);
	~ImplantCollisionRenderer();

	ImplantCollisionRenderer(const ImplantCollisionRenderer&) = delete;
	ImplantCollisionRenderer& operator=(const ImplantCollisionRenderer&) = delete;

public:
	void CheckCollide(const glm::mat4& projection_view,
					  std::vector<int>* collided_ids,
					  bool is_move_implant);

private:
	void ConnectObjGL();
	void CreateImplantObj(int implant_id);
	void CreateNerveObj(int nerve_id);
	private slots:
	void slotUpdateImplant();
	void slotUpdateNerve();
	void slotNerveInitializeVAOVBO();
	void ClearNerveVAOVBO();
	void ClearImplantVAOVBO();
private:

	uint prog_render_ = 0;

	std::map<int, std::unique_ptr<ImplantObjGL>> implant_objs_;
	std::map<int, std::unique_ptr<GLObject>> nerve_objs_;
	unsigned int depth_buffer_ = 0;
	bool is_connted_obj_gl_ = false;
	bool is_update_nerve_ = false;
	bool is_update_implant_ = false;

};
