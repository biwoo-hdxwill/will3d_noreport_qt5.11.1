#include "W3Collide.h"

#include <time.h>
#include <iostream>

#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../UIModule/UIGLObjects/W3GLObject.h"

///////////////////////////////////////////////////////////////////////////////////////////////
// public functions
///////////////////////////////////////////////////////////////////////////////////////////////

//#define COUNT_FACE_FINAL 14000
#define LEVEL 3 // gl object를 쪼갠 횟수
#define OUT_STR_LEVEL 0

using std::exception;
using std::cout;
using std::endl;

CW3Collide::CW3Collide() {
	m_rotAxis[0] = glm::rotate(glm::radians(-90.0f), vec3(0.0, 0.0, 1.0)); //X축
	m_rotAxis[1] = glm::rotate(glm::radians(180.0f), vec3(1.0, 0.0, 0.0)); //Y축
	m_rotAxis[2] = glm::rotate(glm::radians(90.0f), vec3(1.0, 0.0, 0.0)); //Z축

	m_progShader = 0;
	m_matProjView = mat4(1.0);

	m_curAxis = X;
	m_subIterEnd = 0;
	m_subIter = 0;
	m_isOutColliTriangle = true;
}

void CW3Collide::run(GLuint progShader, const glm::mat4& matProjView,
					 const std::vector<CW3GLObject*>& objs_, const std::vector<glm::mat4>& objModelMats,
					 std::vector<int>& outColliObjIDs) {
	m_isOutColliTriangle = false;
	run(progShader, matProjView, objs_, objModelMats, outColliObjIDs, std::vector<std::vector<SubTriangles>>());
}

void CW3Collide::run(GLuint progShader, const glm::mat4& matProjView,
					 const std::vector<CW3GLObject*>& objs_, const std::vector<glm::mat4>& objModelMats,
					 std::vector<int>& outColliObjIDs,
					 std::vector<std::vector<SubTriangles>>& outColliSub) {
	//clock_t t_start, t_end;
	//t_start = clock();
	eDebug.resize(7, 0.0f);
	cntQuery = 0;

	m_progShader = progShader;
	m_matProjView = matProjView;
	m_objs = objs_;
	m_objModelMats = objModelMats;

	glEnable(GL_DEPTH_TEST);
	glColorMask(false, false, false, false);

	glUseProgram(progShader);

	std::vector<int> PCS; //Potential Collision Set

	for (int j = 0; j < objs_.size(); j++) //두개씩 묶어서 object level의 collide를 한다.
	{
		for (int i = 0; i < objs_.size(); i++) {
			if (i == j)
				continue;

			std::vector<int> objIndices;
			objIndices.push_back(j);
			objIndices.push_back(i);
			setupPCS_ObjLevel(objIndices);
			m_curAxis = X;
			if (objIndices.size() == 2) {
				if (std::find(PCS.begin(), PCS.end(), i) == PCS.end())
					PCS.push_back(i);
				if (std::find(PCS.begin(), PCS.end(), j) == PCS.end())
					PCS.push_back(j);
			}
		}
	}

	//for (int i = 0; i < objs.size(); i++)
	//	PCS.push_back(i);
	//
	//m_curAxis = X;
	//setupPCS_ObjLevel(PCS);
	//
	//if (PCS.size() == 1)
	//	PCS.clear();

	if (PCS.size()) {
		std::vector<int> lookup;
		for (const auto& elem : PCS) {
			m_subObjs.push_back(objs_[elem]);
			m_subObjModelMats.push_back(objModelMats[elem]);

			if (objs_[elem]->getDrawMode() == GL_QUADS)
				m_subObjDividedN.push_back(4);
			else if (objs_[elem]->getDrawMode() == GL_TRIANGLES)
				m_subObjDividedN.push_back(3);
			else
				m_subObjDividedN.push_back(1);

			lookup.push_back(elem);
		}

		std::vector<std::vector<SubTriangles>> objTs;
		objTs.resize(m_subObjs.size());

		for (int i = 0; i < m_subObjs.size(); i++) {
			SubTriangles T;
			T.index = 0;
			T.count = m_subObjs[i]->getNindices();

			//int w = T.count;
			//int dividedCount = 0;
			//while (w > COUNT_FACE_FINAL*m_subObjDividedN[i])
			//{
			//	w = (int)((w / m_subObjDividedN[i] / 2) * m_subObjDividedN[i]);
			//	dividedCount++;
			//}
			//m_subObjMaxLevel.push_back(dividedCount);

			objTs[i].push_back(T);
		}

		//int maxLevel = 0;
		//for (const auto& elem : m_subObjMaxLevel)
		//{
		//#if OUT_STR_LEVEL
		//	cout << "obj maximum level = " << elem + 1 << endl;
		//#endif
		//	cout << "obj maximum level = " << elem + 1 << endl;
		//
		//	maxLevel = (elem > maxLevel) ? elem : maxLevel;
		//}

		m_subIterEnd = LEVEL;
		m_subIter = 1;

		PCS.clear();

		//clock_t cc;
		//cc = clock();

		setupPCS_SubLevel(objTs);

		//eDebug[0] += ((float)(clock() - cc)) / CLOCKS_PER_SEC;

		for (int i = 0; i < objTs.size(); i++) {
			if (objTs[i].size()) {
				PCS.push_back(i);
			}
		}

		if (PCS.size() == 1)
			PCS.clear();

		if (m_isOutColliTriangle) {
			outColliSub.resize(objs_.size());
			if (PCS.size()) {
				for (const auto& elem : PCS) {
					outColliObjIDs.push_back(lookup[elem]);
					outColliSub[lookup[elem]] = objTs[elem];
				}
			}
		} else {
			if (PCS.size()) {
				for (const auto& elem : PCS)
					outColliObjIDs.push_back(lookup[elem]);
			}
		}
	}

	glColorMask(true, true, true, true);

	//t_end = clock();
	//float elapsedTime = static_cast<float>(t_end - t_start) / CLOCKS_PER_SEC;
	//cout << "collide FPS = " << 1.0f / elapsedTime << endl;

#if OUT_STR_LEVEL
	for (int i = 0; i < eDebug.size(); i++) {
		cout << QString("eDebugTime[%1] = ").arg(i).toStdString().c_str() << eDebug[i] << endl;
	}

	cout << "== == == == == == == == == == == == == == == == == == == == == == == == == == ==" << endl;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////
// private functions
///////////////////////////////////////////////////////////////////////////////////////////////

void CW3Collide::setupPCS_ObjLevel(std::vector<int>& PCS) {
	int cntObj = m_objs.size();
	if (cntObj == 0 || PCS.size() == 0)
		return;

	glColorMask(false, false, false, false);

	GLuint queryID;
	glGenQueries(1, &queryID);

	auto objVisibleQuery = [&](int i)->bool {
		int queryResult;

		glDepthMask(false);
		glDepthFunc(GL_GEQUAL);
		glBeginQuery(GL_ANY_SAMPLES_PASSED, queryID);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		m_objs[i]->render();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		m_objs[i]->render();
		glEndQuery(GL_ANY_SAMPLES_PASSED);

		glGetQueryObjectiv(queryID, GL_QUERY_RESULT, &queryResult);
#if OUT_STR_LEVEL
		++cntQuery;
#endif
		glDepthFunc(GL_LEQUAL);
		glDepthMask(true);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		m_objs[i]->render();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		m_objs[i]->render();

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
	for (int i = 0; i < PCS.size(); i++) {
		mvp = m_matProjView * m_rotAxis[m_curAxis] * m_objModelMats[PCS[i]];

		WGLSLprogram::setUniform(m_progShader, "MVP", mvp);

		fullyVisible1[PCS[i]] = objVisibleQuery(PCS[i]);
	}
	//1pass end

	//2pass
	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);
	std::vector<bool> fullyVisible2;
	fullyVisible2.resize(cntObj);

	int eIdx = (int)PCS.size() - 1;

	glDepthFunc(GL_LEQUAL);
	glDepthMask(true);

	for (int i = eIdx; i >= 0; i--) {
		mvp = m_matProjView * m_rotAxis[m_curAxis] * m_objModelMats[PCS[i]];

		WGLSLprogram::setUniform(m_progShader, "MVP", mvp);
		fullyVisible2[PCS[i]] = objVisibleQuery(PCS[i]);
	}
	//2pass end

	glDeleteQueries(1, &queryID);

	for (int i = 0; i < cntObj; i++) {
		if (fullyVisible1[i] && fullyVisible2[i]) {
			auto iter = std::find(PCS.begin(), PCS.end(), i);
			if (iter != PCS.end())
				PCS.erase(iter);
		}
	}

#if OUT_STR_LEVEL
	cout << "level(0), ";
	cout << "iteration " << m_curAxis << endl;
	cout << "PCS(" << PCS.size() << ")";

	cout << ", query count(" << cntQuery << ")" << endl;
#endif

	if (PCS.size() > 1) {
		if (m_curAxis != Z) {
			m_curAxis = static_cast<AlignAxis>(((int)m_curAxis + 1) % 3);
			setupPCS_ObjLevel(PCS);
			return;
		} else {
			m_curAxis = X;
			return;
		}
} else {
		m_curAxis = X;
		return;
	}
}

void CW3Collide::setupPCS_SubLevel(std::vector<std::vector<SubTriangles>>& objTs, bool isDivide) {
	if (isDivide)
		divideSubTriangle(objTs);

	glm::mat4 mvp;
	std::vector<std::vector<SubTriangles>> subPCS;
	subPCS.resize(objTs.size());

	auto subVisibleQuery = [&](int i) {
#if OUT_STR_LEVEL
		clock_t cc;
		cc = clock();
#endif
		int cntTs = objTs[i].size();

		if (cntTs > 0) {
			bool isCollide = false;

#if OUT_STR_LEVEL
			clock_t cc_ = clock();
			clock_t cc__ = clock();
#endif
			glDepthMask(false);
			glDepthFunc(GL_GEQUAL);
			std::vector<GLuint> queryID;
			queryID.resize(cntTs);
			glGenQueries(cntTs, &queryID[0]);

			for (int k = 0; k < cntTs; k++) {
				auto& T = objTs[i].at(k);
				glBeginQuery(GL_ANY_SAMPLES_PASSED, queryID[k]);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				m_subObjs[i]->render(T.index, T.count);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				m_subObjs[i]->render(T.index, T.count);
				glEndQuery(GL_ANY_SAMPLES_PASSED);
			}

			glDepthFunc(GL_LEQUAL);
			glDepthMask(true);

#if OUT_STR_LEVEL
			eDebug[5] += ((float)(clock() - cc__)) / CLOCKS_PER_SEC;
			cc__ = clock();
#endif

			for (int k = 0; k < cntTs; k++) {
				auto& T = objTs[i].at(k);

				int queryResult;
				glGetQueryObjectiv(queryID[k], GL_QUERY_RESULT, &queryResult);

#if OUT_STR_LEVEL
				++cntQuery;
#endif
				if (queryResult
					&&
					std::find(subPCS[i].begin(), subPCS[i].end(), T) == subPCS[i].end())
				{
					subPCS[i].push_back(T);
				}
			}

			glDeleteQueries(cntTs, &queryID[0]);
#if OUT_STR_LEVEL
			eDebug[6] += ((float)(clock() - cc__)) / CLOCKS_PER_SEC;
			eDebug[4] += ((float)(clock() - cc_)) / CLOCKS_PER_SEC;
#endif

			for (const auto& T : objTs[i]) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				m_subObjs[i]->render(T.index, T.count);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				m_subObjs[i]->render(T.index, T.count);
			}
		}
#if OUT_STR_LEVEL
		eDebug[2] += ((float)(clock() - cc)) / CLOCKS_PER_SEC;
#endif
	};

	//1pass
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(true);
	glClear(GL_DEPTH_BUFFER_BIT);

	mvp = m_matProjView * m_rotAxis[m_curAxis] * m_subObjModelMats[0];
	WGLSLprogram::setUniform(m_progShader, "MVP", mvp);

	for (const auto& T : objTs[0]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		m_subObjs[0]->render(T.index, T.count);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		m_subObjs[0]->render(T.index, T.count);
	}

	for (int i = 1; i < objTs.size(); i++) {
		mvp = m_matProjView * m_rotAxis[m_curAxis] * m_subObjModelMats[i];
		WGLSLprogram::setUniform(m_progShader, "MVP", mvp);

#if OUT_STR_LEVEL
		clock_t cc;
		cc = clock();
#endif
		subVisibleQuery(i);
#if OUT_STR_LEVEL
		eDebug[1] += ((float)(clock() - cc)) / CLOCKS_PER_SEC;
#endif
	}
	//1pass end

	//2pass
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(true);
	glClear(GL_DEPTH_BUFFER_BIT);

	int eIdx = (int)m_subObjs.size() - 1;
	mvp = m_matProjView * m_rotAxis[m_curAxis] * m_subObjModelMats[eIdx];
	WGLSLprogram::setUniform(m_progShader, "MVP", mvp);

	for (const auto& T : objTs[eIdx]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		m_subObjs[eIdx]->render(T.index, T.count);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		m_subObjs[eIdx]->render(T.index, T.count);
	}

	for (int i = eIdx - 1; i >= 0; i--) {
		mvp = m_matProjView * m_rotAxis[m_curAxis] * m_subObjModelMats[i];
		WGLSLprogram::setUniform(m_progShader, "MVP", mvp);

#if OUT_STR_LEVEL
		clock_t cc;
		cc = clock();
#endif
		subVisibleQuery(i);
#if OUT_STR_LEVEL
		eDebug[1] += ((float)(clock() - cc)) / CLOCKS_PER_SEC;
#endif
	}
	//2pass end

	objTs.assign(subPCS.begin(), subPCS.end());

#if OUT_STR_LEVEL

	int count = 0;
	for (int i = 0; i < objTs.size(); i++) {
		cout << "PCS[" << i << "]" << " = " << objTs[i].size() << endl;
	}

	cout << "level(" << (m_subIter) << "), ";
	cout << "iteration " << (m_subIter) << endl;

	cout << ", query count(" << cntQuery << ")" << endl;
#endif

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

	if (m_subIter < m_subIterEnd) {
		++m_subIter;
		m_curAxis = static_cast<AlignAxis>(((int)m_curAxis + 1) % 3);
		return setupPCS_SubLevel(objTs);
	} else if (m_curAxis != Z) {
		m_curAxis = static_cast<AlignAxis>(((int)m_curAxis + 1) % 3);
		return setupPCS_SubLevel(objTs, false);
	} else
		return;
	}

void CW3Collide::divideSubTriangle(std::vector<std::vector<SubTriangles>>& objTs, int samples) {
	if (m_subIterEnd < m_subIter)
		return;

	for (int i = 0; i < objTs.size(); i++) {
		//if (m_subObjMaxLevel[i] < m_subIter)
		//	continue;

		std::vector<SubTriangles> Ts = objTs[i];

		std::vector<SubTriangles> dividedTs;
		for (auto& T : Ts) {
			if (T.count == m_subObjDividedN[i]) {
				dividedTs.push_back(T);
				continue;
			}

			int dividedcount = (int)((T.count / m_subObjDividedN[i]) / samples) * m_subObjDividedN[i];
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
