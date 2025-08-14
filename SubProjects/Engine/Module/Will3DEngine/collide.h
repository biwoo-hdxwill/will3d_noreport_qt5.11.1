#pragma once

/**=================================================================================================

Project:		Renderer
File:			collide.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-09-18
Last modify: 	2018-09-18

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include <vector>

#include "../../Common/GLfunctions/WGLHeaders.h"
#include "will3dengine_global.h"

class GLObject;

/// <summary>
/// 3D Surface간의 충돌 검사 클래스.
/// </summary>

class WILL3DENGINE_EXPORT Collide
{
private:
	typedef struct _SUBTRIANGLE {
		int count = 0;
		int index = 0;

		bool operator ==(const _SUBTRIANGLE& arg) {
			return (this->index == arg.index);
		}
	} SubTriangles;

public:
	Collide();
	~Collide() {};

	/// <summary>
	/// Run collision detection.
	/// this function must be called after makecurrent.
	/// </summary>
	/// <param name="prog_shader">The program shader.</param>
	/// <param name="mat_proj_view">The matrix. projtion and view.</param>
	/// <param name="objs">The objects.</param>
	/// <param name="obj_model_mats">The object model matrix.</param>
	/// <param name="out_colli_obj_ids">output. collision object IDs</param>
	void run(GLuint prog_shader, const glm::mat4& mat_proj_view,
		const std::vector<GLObject*>& objs, const std::vector<glm::mat4>& obj_model_mats,
		std::vector<int>& out_colli_obj_ids, int target_obj_idx = -1);

	/// <summary>
	/// Run collision detection.
	/// this function must be called after makecurrent.
	/// </summary>
	/// <param name="prog_shader">The program shader.</param>
	/// <param name="mat_proj_view">The matrix. projtion and view.</param>
	/// <param name="objs">The objects.</param>
	/// <param name="obj_model_mats">The object model matrix.</param>
	/// <param name="out_colli_obj_ids">output. collision object IDs</param>
	void run(GLuint prog_shader, const glm::mat4& mat_proj_view,
			 const std::vector<GLObject*>& objs, const std::vector<glm::mat4>& obj_model_mats,
			 std::vector<int>& out_colli_obj_ids, std::vector<int> target_obj_idx);

	/// <summary>
	/// Run collision detection.
	/// this function must be called after makecurrent.
	/// </summary>
	/// <param name="prog_shader">The program shader.</param>
	/// <param name="mat_proj_view">The matrix. projtion and view.</param>
	/// <param name="objs">The objects.</param>
	/// <param name="obj_model_mats">The object model matrix.</param>
	/// <param name="out_colli_obj_ids">output. collision object IDs</param>
	/// <param name="out_colli_sub">output. collision triangle in object</param>
	void run(GLuint prog_shader, const glm::mat4& mat_proj_view,
		const std::vector<GLObject*>& objs, const std::vector<glm::mat4>& obj_model_mats,
		std::vector<int>& out_colli_obj_ids,
		std::vector<std::vector<SubTriangles>>& out_colli_sub, std::vector<int> target_obj_idx);

private:
	enum AlignAxis { X, Y, Z };

	void SetupPCS_ObjLevel(std::vector<int>& PCS);
	void SetupPCS_SubLevel(std::vector<std::vector<SubTriangles>>& objTs, bool isDivide = true);

	void DivideSubTriangle(std::vector<std::vector<SubTriangles>>& objTs, int samples = 2);

private:

	glm::mat4 rot_axis_[3];

	GLuint prog_shader_;
	glm::mat4 proj_view_mat_;

	std::vector<GLObject*> objs_;
	std::vector<glm::mat4> obj_model_mats_;

	std::vector<GLObject*> sub_objs_;
	std::vector<glm::mat4> sub_obj_model_mats_;
	std::vector<int> sub_obj_divided_n_;

	std::vector<int> sub_obj_max_level_;

	AlignAxis cur_axis_;

	int sub_iter_end_;
	int sub_iter_;

	bool is_out_coli_triangle_;

};
