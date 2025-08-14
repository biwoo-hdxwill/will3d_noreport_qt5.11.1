#include "collide.h"

#include <time.h>
#include <iostream>

#include <QDebug>

#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../UIModule/UIGLObjects/gl_object.h"


///////////////////////////////////////////////////////////////////////////////////////////////
// public functions
///////////////////////////////////////////////////////////////////////////////////////////////

//#define COUNT_FACE_FINAL 14000
#define LEVEL 3 // gl object를 쪼갠 횟수

using std::exception;
using std::cout;
using std::endl;

Collide::Collide() {
	rot_axis_[0] = glm::rotate(glm::radians(-90.0f), vec3(0.0, 0.0, 1.0)); //X축
	rot_axis_[1] = glm::rotate(glm::radians(180.0f), vec3(1.0, 0.0, 0.0)); //Y축
	rot_axis_[2] = glm::rotate(glm::radians(90.0f), vec3(1.0, 0.0, 0.0)); //Z축

	prog_shader_ = 0;
	proj_view_mat_ = mat4(1.0);

	cur_axis_ = X;
	sub_iter_end_ = 0;
	sub_iter_ = 0;
	is_out_coli_triangle_ = true;
}

void Collide::run(GLuint prog_shader, const glm::mat4& mat_proj_view,
					 const std::vector<GLObject*>& objs, const std::vector<glm::mat4>& obj_model_mats,
					 std::vector<int>& out_colli_obj_ids, int target_obj_idx) {
	is_out_coli_triangle_ = false;
	std::vector<int> target_objs;
	if (target_obj_idx >= 0)
		target_objs.push_back(target_obj_idx);

	run(prog_shader, mat_proj_view, objs, obj_model_mats, out_colli_obj_ids, std::vector<std::vector<SubTriangles>>(), target_objs);
}

void Collide::run(GLuint prog_shader, const glm::mat4 & mat_proj_view,
				  const std::vector<GLObject*>& objs, const std::vector<glm::mat4>& obj_model_mats,
				  std::vector<int>& out_colli_obj_ids, std::vector<int> target_obj_idx) {
	is_out_coli_triangle_ = false;
	run(prog_shader, mat_proj_view, objs, obj_model_mats, out_colli_obj_ids, std::vector<std::vector<SubTriangles>>(), target_obj_idx);
}
 
void Collide::run(GLuint prog_shader, const glm::mat4& mat_proj_view,
					 const std::vector<GLObject*>& objs, const std::vector<glm::mat4>& obj_model_mats,
					 std::vector<int>& out_colli_obj_ids,
					 std::vector<std::vector<SubTriangles>>& out_colli_sub, std::vector<int> target_obj_idx) {
	//clock_t t_start, t_end;
	//t_start = clock();
	
	prog_shader_ = prog_shader;
	proj_view_mat_ = mat_proj_view;
	objs_ = objs;
	obj_model_mats_ = obj_model_mats;

	glEnable(GL_DEPTH_TEST);
	glColorMask(false, false, false, false);

	glUseProgram(prog_shader);

	std::vector<int> PCS; //Potential Collision Set

	if (target_obj_idx.size() == 0) {
		for (int j = 0; j < objs_.size(); j++) //두개씩 묶어서 object level의 collide를 한다.
		{
			for (int i = 0; i < objs_.size(); i++) {
				if (i == j)
					continue;

				std::vector<int> objIndices;
				objIndices.push_back(j);
				objIndices.push_back(i);
				SetupPCS_ObjLevel(objIndices);
				cur_axis_ = X;
				if (objIndices.size() == 2) {
					if (std::find(PCS.begin(), PCS.end(), i) == PCS.end())
						PCS.push_back(i);
					if (std::find(PCS.begin(), PCS.end(), j) == PCS.end())
						PCS.push_back(j);
				}
			}
		}
	} else {
		for (int j = 0; j < target_obj_idx.size(); j++) {
			for (int i = 0; i < objs_.size(); i++) {
				int index = target_obj_idx[j];
				if (i == index)
					continue;

				std::vector<int> objIndices;
				objIndices.push_back(index);
				objIndices.push_back(i);
				SetupPCS_ObjLevel(objIndices);
				cur_axis_ = X;
				if (objIndices.size() == 2) {
					if (std::find(PCS.begin(), PCS.end(), i) == PCS.end())
						PCS.push_back(i);
					if (std::find(PCS.begin(), PCS.end(), index) == PCS.end())
						PCS.push_back(index);
				}
			}
		}
	}

	if (PCS.size()) {
		std::vector<int> lookup;
		for (const auto& elem : PCS) {
			sub_objs_.push_back(objs_[elem]);
			sub_obj_model_mats_.push_back(obj_model_mats[elem]);

			if (objs_[elem]->draw_mode() == GL_QUADS)
				sub_obj_divided_n_.push_back(4);
			else if (objs_[elem]->draw_mode() == GL_TRIANGLES)
				sub_obj_divided_n_.push_back(3);
			else
				sub_obj_divided_n_.push_back(1);

			lookup.push_back(elem);
		}

		std::vector<std::vector<SubTriangles>> objTs;
		objTs.resize(sub_objs_.size());

		for (int i = 0; i < sub_objs_.size(); i++) {
			SubTriangles T;
			T.index = 0;
			T.count = sub_objs_[i]->cnt_indices();

			objTs[i].push_back(T);
		}

		sub_iter_end_ = LEVEL;
		sub_iter_ = 1;

		PCS.clear();

		SetupPCS_SubLevel(objTs);

		for (int i = 0; i < objTs.size(); i++) {
			if (objTs[i].size()) {
				PCS.push_back(i);
			}
		}

		if (PCS.size() == 1)
			PCS.clear();

		if (is_out_coli_triangle_) {
			out_colli_sub.resize(objs_.size());
			if (PCS.size()) {
				for (const auto& elem : PCS) {
					out_colli_obj_ids.push_back(lookup[elem]);
					out_colli_sub[lookup[elem]] = objTs[elem];
				}
			}
		} else {
			if (PCS.size()) {
				for (const auto& elem : PCS)
					out_colli_obj_ids.push_back(lookup[elem]);
			}
		}
	}

	glColorMask(true, true, true, true);

	//t_end = clock();
	//float elapsedTime = static_cast<float>(t_end - t_start) / CLOCKS_PER_SEC;
	//cout << "collide FPS = " << 1.0f / elapsedTime << endl;

}

///////////////////////////////////////////////////////////////////////////////////////////////
// private functions
///////////////////////////////////////////////////////////////////////////////////////////////

void Collide::SetupPCS_ObjLevel(std::vector<int>& PCS)
{
	int cntObj = objs_.size();
	if (cntObj == 0 || PCS.size() == 0)
		return;

	glColorMask(false, false, false, false);

	GLuint queryID;
	glGenQueries(1, &queryID);

	auto objVisibleQuery = [&](int i)->bool
	{
		int queryResult;

		glDepthMask(false);
		glDepthFunc(GL_GEQUAL);
		glBeginQuery(GL_ANY_SAMPLES_PASSED, queryID);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		objs_[i]->Render();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		objs_[i]->Render();
		glEndQuery(GL_ANY_SAMPLES_PASSED);

		glGetQueryObjectiv(queryID, GL_QUERY_RESULT, &queryResult);

		glDepthFunc(GL_LEQUAL);
		glDepthMask(true);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		objs_[i]->Render();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		objs_[i]->Render();

		return (queryResult) ? false : true;
	};

	////1pass
	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);
	std::vector<bool> fullyVisible1;
	fullyVisible1.resize(cntObj);

	glDepthFunc(GL_LEQUAL);
	glDepthMask(true);

	glm::mat4 mvp;
	for (int i = 0; i < PCS.size(); i++)
	{
		mvp = proj_view_mat_ * rot_axis_[cur_axis_] * obj_model_mats_[PCS[i]];

		WGLSLprogram::setUniform(prog_shader_, "MVP", mvp);

		fullyVisible1[PCS[i]] = objVisibleQuery(PCS[i]);
	}
	//1pass end

	//2pass
	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);
	std::vector<bool> fullyVisible2;
	fullyVisible2.resize(cntObj);

	int eIdx = PCS.size() - 1;

	glDepthFunc(GL_LEQUAL);
	glDepthMask(true);

	for (int i = eIdx; i >= 0; i--)
	{
		mvp = proj_view_mat_ * rot_axis_[cur_axis_] * obj_model_mats_[PCS[i]];

		WGLSLprogram::setUniform(prog_shader_, "MVP", mvp);
		fullyVisible2[PCS[i]] = objVisibleQuery(PCS[i]);
	}
	//2pass end

	glDeleteQueries(1, &queryID);

	for (int i = 0; i < cntObj; i++)
	{
		if (fullyVisible1[i] && fullyVisible2[i])
		{
			auto iter = std::find(PCS.begin(), PCS.end(), i);
			if (iter != PCS.end())
				PCS.erase(iter);
		}
	}

	if (PCS.size() > 1)
	{
		if (cur_axis_ != Z)
		{
			cur_axis_ = static_cast<AlignAxis>(((int)cur_axis_ + 1) % 3);
			SetupPCS_ObjLevel(PCS);
			return;
		}
		else
		{
			cur_axis_ = X;
			return;
		}
	}
	else
	{
		cur_axis_ = X;
		return;
	}
}

void Collide::SetupPCS_SubLevel(std::vector<std::vector<SubTriangles>>& objTs, bool isDivide) {
	if (isDivide)
		DivideSubTriangle(objTs);

	glm::mat4 mvp;
	std::vector<std::vector<SubTriangles>> subPCS;
	subPCS.resize(objTs.size());

	auto subVisibleQuery = [&](int i) {
		int cntTs = objTs[i].size();

		if (cntTs > 0) {
			bool isCollide = false;

			glDepthMask(false);
			glDepthFunc(GL_GEQUAL);
			std::vector<GLuint> queryID;
			queryID.resize(cntTs);
			glGenQueries(cntTs, &queryID[0]);

			for (int k = 0; k < cntTs; k++) {
				auto& T = objTs[i].at(k);
				glBeginQuery(GL_ANY_SAMPLES_PASSED, queryID[k]);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				sub_objs_[i]->Render(T.index, T.count);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				sub_objs_[i]->Render(T.index, T.count);
				glEndQuery(GL_ANY_SAMPLES_PASSED);
			}

			glDepthFunc(GL_LEQUAL);
			glDepthMask(true);

			for (int k = 0; k < cntTs; k++) {
				auto& T = objTs[i].at(k);

				int queryResult;
				glGetQueryObjectiv(queryID[k], GL_QUERY_RESULT, &queryResult);

				if (queryResult
					&&
					std::find(subPCS[i].begin(), subPCS[i].end(), T) == subPCS[i].end())
				{
					subPCS[i].push_back(T);
				}
			}

			glDeleteQueries(cntTs, &queryID[0]);

			for (const auto& T : objTs[i]) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				sub_objs_[i]->Render(T.index, T.count);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				sub_objs_[i]->Render(T.index, T.count);
			}
		}
	};

	//1pass
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(true);
	glClear(GL_DEPTH_BUFFER_BIT);

	mvp = proj_view_mat_ * rot_axis_[cur_axis_] * sub_obj_model_mats_[0];
	WGLSLprogram::setUniform(prog_shader_, "MVP", mvp);

	for (const auto& T : objTs[0]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		sub_objs_[0]->Render(T.index, T.count);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		sub_objs_[0]->Render(T.index, T.count);
	}

	for (int i = 1; i < objTs.size(); i++) {
		mvp = proj_view_mat_ * rot_axis_[cur_axis_] * sub_obj_model_mats_[i];
		WGLSLprogram::setUniform(prog_shader_, "MVP", mvp);

		subVisibleQuery(i);
	}
	//1pass end

	//2pass
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(true);
	glClear(GL_DEPTH_BUFFER_BIT);

	int eIdx = sub_objs_.size() - 1;
	mvp = proj_view_mat_ * rot_axis_[cur_axis_] * sub_obj_model_mats_[eIdx];
	WGLSLprogram::setUniform(prog_shader_, "MVP", mvp);

	for (const auto& T : objTs[eIdx]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		sub_objs_[eIdx]->Render(T.index, T.count);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		sub_objs_[eIdx]->Render(T.index, T.count);
	}

	for (int i = eIdx - 1; i >= 0; i--) {
		mvp = proj_view_mat_ * rot_axis_[cur_axis_] * sub_obj_model_mats_[i];
		WGLSLprogram::setUniform(prog_shader_, "MVP", mvp);

		subVisibleQuery(i);
	}
	//2pass end
	objTs.assign(subPCS.begin(), subPCS.end());


	int checkObjTs = 0;
	for (const auto& elem : objTs) {
		if (elem.size()) {
			++checkObjTs;

			if (checkObjTs > 1)
				break;
		}
	}

	if (checkObjTs < 2) {
		cout << "PCS is set one object." << endl;
		return;
	}

	if (sub_iter_ < sub_iter_end_) {
		++sub_iter_;
		cur_axis_ = static_cast<AlignAxis>(((int)cur_axis_ + 1) % 3);
		return SetupPCS_SubLevel(objTs);
	} else if (cur_axis_ != Z) {
		cur_axis_ = static_cast<AlignAxis>(((int)cur_axis_ + 1) % 3);
		return SetupPCS_SubLevel(objTs, false);
	} else
		return;
	}

void Collide::DivideSubTriangle(std::vector<std::vector<SubTriangles>>& objTs, int samples) {
	if (sub_iter_end_ < sub_iter_)
		return;

	for (int i = 0; i < objTs.size(); i++) {
		//if (sub_obj_max_level_[i] < sub_iter_)
		//	continue;

		std::vector<SubTriangles> Ts = objTs[i];

		std::vector<SubTriangles> dividedTs;
		for (auto& T : Ts) {
			if (T.count == sub_obj_divided_n_[i]) {
				dividedTs.push_back(T);
				continue;
			}

			int dividedcount = (int)((T.count / sub_obj_divided_n_[i]) / samples) * sub_obj_divided_n_[i];
			//dividedcount = ((int)(dividedcount / 3)) * 3;

			int endFor = T.index + T.count;

			int k;
			for (k = T.index; k + dividedcount <= endFor; k += dividedcount) {
				SubTriangles dividedT;
				dividedT.index = k;
				dividedT.count = dividedcount;
				dividedTs.push_back(dividedT);
			}

			if (endFor - k > 2) {
				SubTriangles remainT;
				remainT.index = k;
				remainT.count = endFor - k;
				dividedTs.push_back(remainT);
			}
		}

		objTs[i] = dividedTs;
	}
}
