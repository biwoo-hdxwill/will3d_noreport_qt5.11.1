#include "W3OrientedBoundingBox.h"
/*=========================================================================

File:			class CW3OrientedBoundingBox
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun
First date:		2016-03-30
Last modify:	2016-03-30

=========================================================================*/
// 각 뭉치에 대한 bounding box 를 만듬
// Triangle 은 -1~1 사이의 volume cube 에서의 좌표
// Triangle 각 꼭지점은 index 순서로 중복되게 할당되어 있음

#if defined(__APPLE__)
#include <glm/gtx/norm.hpp>
#else
#include <gl/glm/gtx/norm.hpp>
#endif

#include "../../Common/GLfunctions/W3GLTypes.h"

CW3OrientedBoundingBox::CW3OrientedBoundingBox(QList<Triangle *> *triangles, bool isAxisAligned)
	: m_pgvTriangles(triangles) {

	m_mu = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	for (int i = 0; i < 3; i++)
		m_axis[i] = vec4(0.0f);
	m_halfDim = vec3(0.0f);

	m_AxisOrder[0] = 0;
	m_AxisOrder[1] = 1;
	m_AxisOrder[2] = 2;

	if (isAxisAligned) {
		createABB();
	} else {
		createOBB();
	}
}

CW3OrientedBoundingBox::~CW3OrientedBoundingBox() {
}

///////////////////////////////////////////////////////////////////////////
//
//	* createOBB()
//	OBB 생성 함수
//
///////////////////////////////////////////////////////////////////////////
void CW3OrientedBoundingBox::createOBB() {
	int numVertics = m_pgvTriangles->size() * 3;	// vertex 의 갯수
	int numTriangles = m_pgvTriangles->size();	// triangle mesh 의 갯수

	for (int i = 0; i < numTriangles; i++) {
		m_vVert.push_back(m_pgvTriangles->at(i)->v1.x);
		m_vVert.push_back(m_pgvTriangles->at(i)->v1.y);
		m_vVert.push_back(m_pgvTriangles->at(i)->v1.z);

		m_vVert.push_back(m_pgvTriangles->at(i)->v0.x);
		m_vVert.push_back(m_pgvTriangles->at(i)->v0.y);
		m_vVert.push_back(m_pgvTriangles->at(i)->v0.z);

		m_vVert.push_back(m_pgvTriangles->at(i)->v2.x);
		m_vVert.push_back(m_pgvTriangles->at(i)->v2.y);
		m_vVert.push_back(m_pgvTriangles->at(i)->v2.z);
	}

	// compute mean value
	Vector3f sum(0.0f, 0.0f, 0.0f);
	Vector3f mu(0.0f, 0.0f, 0.0f);	// mean value
	float sumM = 0.0f;

	for (int i = 0; i < numTriangles; i++) {
		Vector3f p(m_vVert.at(i * 9), m_vVert.at(i * 9 + 1), m_vVert.at(i * 9 + 2));
		Vector3f q(m_vVert.at(i * 9 + 3), m_vVert.at(i * 9 + 4), m_vVert.at(i * 9 + 5));
		Vector3f r(m_vVert.at(i * 9 + 6), m_vVert.at(i * 9 + 7), m_vVert.at(i * 9 + 8));

		float m = ((q - p).cross((r - p))).norm() * 0.5f;	// 삼격형 넓이

		sumM += m;

		sum += (m * ((p + q + r) / 3.0f));	// 삼각형 무게중심
		// m 을 inverse 해서 곱하고 나중에 sumM 을 나누는 것이 아니고 삼각형 갯수로 나눠야 하지 않나?
	}
	mu = (sum / sumM);

	// covariance matrix
	Matrix3f C = Matrix3f::Zero();
	for (int i = 0; i < numTriangles; i++) {
		Vector3f p(m_vVert.at(i * 9), m_vVert.at(i * 9 + 1), m_vVert.at(i * 9 + 2));
		Vector3f q(m_vVert.at(i * 9 + 3), m_vVert.at(i * 9 + 4), m_vVert.at(i * 9 + 5));
		Vector3f r(m_vVert.at(i * 9 + 6), m_vVert.at(i * 9 + 7), m_vVert.at(i * 9 + 8));

		Vector3f pbar = p - mu;
		Vector3f qbar = q - mu;
		Vector3f rbar = r - mu;

		float m = ((q - p).cross((r - p))).norm() * 0.5f;

		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				C(j, k) +=
					m
					* ((pbar(j) + qbar(j) + rbar(j))
					   * (pbar(k) + qbar(k) + rbar(k))
					   + (pbar(j) * pbar(k))
					   + (qbar(j) * qbar(k))
					   + (rbar(j) * rbar(k)));
			}
		}
	}

	Eigen::SelfAdjointEigenSolver<Matrix3f> es;
	es.compute(C);

	Vector3f eigenValues = es.eigenvalues();
	Matrix3f eigenVectors = es.eigenvectors();
	Vector3f eigenVectorAc = es.eigenvectors().col(0);
	Vector3f eigenVectorBc = es.eigenvectors().col(1);
	Vector3f eigenVectorCc = es.eigenvectors().col(2);
	vec3 eigenVectorA(eigenVectorAc.x(), eigenVectorAc.y(), eigenVectorAc.z());
	vec3 eigenVectorB(eigenVectorBc.x(), eigenVectorBc.y(), eigenVectorBc.z());
	vec3 eigenVectorC(eigenVectorCc.x(), eigenVectorCc.y(), eigenVectorCc.z());

	vec3 r = eigenVectorA;
	vec3 u = eigenVectorB;
	vec3 f = eigenVectorC;

	//float test = glm::dot(u, f);

	// vertex 들이 모두 거의 한 평면 위에 있을 경우
	// 하나의 eigen vector 가 정상적이지 않을 수 있기 때문에
	// 나머지 두 eigen vector 를 이용해서 구함
	//float len = 0.0f;
	//float e = 0.001;

	//len = eigenValues[0];
	//if (len <= e)
	//	r = glm::cross(u, f);

	//len = eigenValues[1];
	//if (len <= e)
	//	u = glm::cross(f, r);

	//len = eigenValues[2];
	//if (len <= e)
	//	f = glm::cross(r, u);

	r /= glm::length(r);
	u /= glm::length(u);
	f /= glm::length(f);

	//m_axis[0] = glm::vec4(r, 0.0f);
	//m_axis[1] = glm::vec4(u, 0.0f);
	//m_axis[2] = glm::vec4(f, 0.0f);

	mat3 eigenTransform;

	eigenTransform[0][0] = r.x;
	eigenTransform[0][1] = r.y;
	eigenTransform[0][2] = r.z;

	eigenTransform[1][0] = u.x;
	eigenTransform[1][1] = u.y;
	eigenTransform[1][2] = u.z;

	eigenTransform[2][0] = f.x;
	eigenTransform[2][1] = f.y;
	eigenTransform[2][2] = f.z;

	// p: eigenvector 축에 대한 좌표
	vec3 p = glm::inverse(eigenTransform) * vec3(m_vVert.at(0) - mu(0), m_vVert.at(1) - mu(1), m_vVert.at(2) - mu(2));

	vec3 minP = p;
	vec3 maxP = p;

	int selectedId = 0;
	float minx = 100000;
	for (int i = 1; i < numVertics; i++) {
		if (minx > m_vVert.at(i * 3)) {
			selectedId = i;
			minx = m_vVert.at(i * 3);
		}

		p = glm::inverse(eigenTransform) * vec3(m_vVert.at(i * 3) - mu(0), m_vVert.at(i * 3 + 1) - mu(1), m_vVert.at(i * 3 + 2) - mu(2));

		minP.x = std::min(minP.x, p.x);
		minP.y = std::min(minP.y, p.y);
		minP.z = std::min(minP.z, p.z);

		maxP.x = std::max(maxP.x, p.x);
		maxP.y = std::max(maxP.y, p.y);
		maxP.z = std::max(maxP.z, p.z);
	}

	// 육면체 한 축의 길이가 너무 작을 경우 최소값 지정
	vec3 dist = maxP - minP;

	//if (dist.x < e)
	//	dist.x = e * 2.0f;
	//if (dist.y < e)
	//	dist.y = e * 2.0f;
	//if (dist.z < e)
	//	dist.z = e * 2.0f;

	//m_halfDim = vec3(fabs(dist.x * 0.5f), fabs(dist.y * 0.5f), fabs(dist.z * 0.5f));

	// cube 의 8개 꼭지점
	vec3 p1 = vec3(mu(0), mu(1), mu(2)) + (r * minP.x) + (u * minP.y) + (f * minP.z);
	vec3 p2 = vec3(mu(0), mu(1), mu(2)) + (r * minP.x) + (u * minP.y) + (f * maxP.z);
	vec3 p3 = vec3(mu(0), mu(1), mu(2)) + (r * minP.x) + (u * maxP.y) + (f * maxP.z);
	vec3 p4 = vec3(mu(0), mu(1), mu(2)) + (r * minP.x) + (u * maxP.y) + (f * minP.z);

	vec3 p5 = vec3(mu(0), mu(1), mu(2)) + (r * maxP.x) + (u * minP.y) + (f * minP.z);
	vec3 p6 = vec3(mu(0), mu(1), mu(2)) + (r * maxP.x) + (u * minP.y) + (f * maxP.z);
	vec3 p7 = vec3(mu(0), mu(1), mu(2)) + (r * maxP.x) + (u * maxP.y) + (f * maxP.z);
	vec3 p8 = vec3(mu(0), mu(1), mu(2)) + (r * maxP.x) + (u * maxP.y) + (f * minP.z);

	glm::vec3 mutmp = (p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8) / 8.0f;
	m_mu2 = m_mu = glm::vec4(mutmp, 1.0f);

	vec3 axisAr = (p1 + p2 + p3 + p4) / 4.0f;
	m_axis[0] = glm::vec4(axisAr, 1.0f) - m_mu;
	m_axis2[0] = m_axis[0];

	vec3 axisAu = (p1 + p2 + p5 + p6) / 4.0f;
	m_axis[1] = glm::vec4(axisAu, 1.0f) - m_mu;
	m_axis2[1] = m_axis[1];

	vec3 axisAf = (p1 + p4 + p5 + p8) / 4.0f;
	m_axis[2] = glm::vec4(axisAf, 1.0f) - m_mu;
	m_axis2[2] = m_axis[2];

	m_halfDim = vec3(glm::length(m_axis2[0]), glm::length(m_axis2[1]), glm::length(m_axis2[2]));

	// vertex 분할을 위한 기준 평면 설정
	// 기준 평면은 최장축에 수직하게 반으로 나누는 평면
	float longest = std::max(std::max(dist.x, dist.y), dist.z);
	if (longest == dist.x) {
		m_vHalfPlaneLongedtAxis = getPlaneEquation((p5 + p1) * 0.5f, (p6 + p2) * 0.5f, (p7 + p3) * 0.5f);
		//cout << "logest axos : x" << endl;
	} else if (longest == dist.y) {
		m_vHalfPlaneLongedtAxis = getPlaneEquation((p4 + p1) * 0.5f, (p3 + p2) * 0.5f, (p8 + p5) * 0.5f);
		//cout << "logest axos : y" << endl;
	} else {
		m_vHalfPlaneLongedtAxis = getPlaneEquation((p2 + p1) * 0.5f, (p4 + p3) * 0.5f, (p6 + p5) * 0.5f);
		//cout << "logest axos : z" << endl;
	}

	m_OBBPoint[0] = glm::vec4(p1, 1.0f);
	m_OBBPoint[1] = glm::vec4(p2, 1.0f);
	m_OBBPoint[2] = glm::vec4(p3, 1.0f);
	m_OBBPoint[3] = glm::vec4(p4, 1.0f);
	m_OBBPoint[4] = glm::vec4(p5, 1.0f);
	m_OBBPoint[5] = glm::vec4(p6, 1.0f);
	m_OBBPoint[6] = glm::vec4(p7, 1.0f);
	m_OBBPoint[7] = glm::vec4(p8, 1.0f);
}

void CW3OrientedBoundingBox::createABB() {
	int numVertics = m_pgvTriangles->size() * 3;	// vertex 의 갯수
	int numTriangles = m_pgvTriangles->size();	// triangle mesh 의 갯수

	//glm::vec3 mu(0.0f);

	//for (int i = 0; i < numTriangles; i++)
	//{
	//	mu += m_pgvTriangles->at(i)->p;
	//	mu += m_pgvTriangles->at(i)->q;
	//	mu += m_pgvTriangles->at(i)->r;
	//}

	//mu = mu / float(numVertics);

	vec3 r = glm::vec3(1.0f, 0.0f, 0.0f);
	vec3 u = glm::vec3(0.0f, 1.0f, 0.0f);
	vec3 f = glm::vec3(0.0f, 0.0f, 1.0f);

	vec3 minP = glm::vec3(std::numeric_limits<float>::max());
	vec3 maxP = -minP;

	for (int i = 0; i < numTriangles; i++) {
		glm::vec3 pt = m_pgvTriangles->at(i)->v1;// -mu;

		minP.x = std::min(minP.x, pt.x);
		minP.y = std::min(minP.y, pt.y);
		minP.z = std::min(minP.z, pt.z);

		maxP.x = std::max(maxP.x, pt.x);
		maxP.y = std::max(maxP.y, pt.y);
		maxP.z = std::max(maxP.z, pt.z);

		pt = m_pgvTriangles->at(i)->v0;// -mu;

		minP.x = std::min(minP.x, pt.x);
		minP.y = std::min(minP.y, pt.y);
		minP.z = std::min(minP.z, pt.z);

		maxP.x = std::max(maxP.x, pt.x);
		maxP.y = std::max(maxP.y, pt.y);
		maxP.z = std::max(maxP.z, pt.z);

		pt = m_pgvTriangles->at(i)->v2;// -mu;

		minP.x = std::min(minP.x, pt.x);
		minP.y = std::min(minP.y, pt.y);
		minP.z = std::min(minP.z, pt.z);

		maxP.x = std::max(maxP.x, pt.x);
		maxP.y = std::max(maxP.y, pt.y);
		maxP.z = std::max(maxP.z, pt.z);
	}

	// 육면체 한 축의 길이가 너무 작을 경우 최소값 지정
	vec3 dist = maxP - minP;

	//if (dist.x < e)
	//	dist.x = e * 2.0f;
	//if (dist.y < e)
	//	dist.y = e * 2.0f;
	//if (dist.z < e)
	//	dist.z = e * 2.0f;

	//m_halfDim = vec3(fabs(dist.x * 0.5f), fabs(dist.y * 0.5f), fabs(dist.z * 0.5f));

	// cube 의 8개 꼭지점
	//vec3 p1 = mu + (r * minP.x) + (u * minP.y) + (f * minP.z);
	//vec3 p2 = mu + (r * minP.x) + (u * minP.y) + (f * maxP.z);
	//vec3 p3 = mu + (r * minP.x) + (u * maxP.y) + (f * maxP.z);
	//vec3 p4 = mu + (r * minP.x) + (u * maxP.y) + (f * minP.z);

	//vec3 p5 = mu + (r * maxP.x) + (u * minP.y) + (f * minP.z);
	//vec3 p6 = mu + (r * maxP.x) + (u * minP.y) + (f * maxP.z);
	//vec3 p7 = mu + (r * maxP.x) + (u * maxP.y) + (f * maxP.z);
	//vec3 p8 = mu + (r * maxP.x) + (u * maxP.y) + (f * minP.z);

	vec3 p1 = (r * minP.x) + (u * minP.y) + (f * minP.z);
	vec3 p2 = (r * minP.x) + (u * minP.y) + (f * maxP.z);
	vec3 p3 = (r * minP.x) + (u * maxP.y) + (f * maxP.z);
	vec3 p4 = (r * minP.x) + (u * maxP.y) + (f * minP.z);

	vec3 p5 = (r * maxP.x) + (u * minP.y) + (f * minP.z);
	vec3 p6 = (r * maxP.x) + (u * minP.y) + (f * maxP.z);
	vec3 p7 = (r * maxP.x) + (u * maxP.y) + (f * maxP.z);
	vec3 p8 = (r * maxP.x) + (u * maxP.y) + (f * minP.z);

	glm::vec3 mutmp = (p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8) / 8.0f;
	m_mu2 = m_mu = glm::vec4(mutmp, 1.0f);

	vec3 axisAr = (p1 + p2 + p3 + p4) / 4.0f;
	m_axis[0] = glm::vec4(axisAr, 1.0f) - m_mu;
	m_axis2[0] = m_axis[0];

	vec3 axisAu = (p1 + p2 + p5 + p6) / 4.0f;
	m_axis[1] = glm::vec4(axisAu, 1.0f) - m_mu;
	m_axis2[1] = m_axis[1];

	vec3 axisAf = (p1 + p4 + p5 + p8) / 4.0f;
	m_axis[2] = glm::vec4(axisAf, 1.0f) - m_mu;
	m_axis2[2] = m_axis[2];

	m_halfDim = vec3(glm::length(m_axis2[0]), glm::length(m_axis2[1]), glm::length(m_axis2[2]));

	// vertex 분할을 위한 기준 평면 설정
	// 기준 평면은 최장축에 수직하게 반으로 나누는 평면
	float longest = std::max(std::max(dist.x, dist.y), dist.z);
	if (longest == dist.x) {
		m_vHalfPlaneLongedtAxis = getPlaneEquation((p5 + p1) * 0.5f, (p6 + p2) * 0.5f, (p7 + p3) * 0.5f);
		//cout << "logest axos : x" << endl;
	} else if (longest == dist.y) {
		m_vHalfPlaneLongedtAxis = getPlaneEquation((p4 + p1) * 0.5f, (p3 + p2) * 0.5f, (p8 + p5) * 0.5f);
		//cout << "logest axos : y" << endl;
	} else {
		m_vHalfPlaneLongedtAxis = getPlaneEquation((p2 + p1) * 0.5f, (p4 + p3) * 0.5f, (p6 + p5) * 0.5f);
		//cout << "logest axos : z" << endl;
	}

	m_OBBPoint[0] = glm::vec4(p1, 1.0f);
	m_OBBPoint[1] = glm::vec4(p2, 1.0f);
	m_OBBPoint[2] = glm::vec4(p3, 1.0f);
	m_OBBPoint[3] = glm::vec4(p4, 1.0f);
	m_OBBPoint[4] = glm::vec4(p5, 1.0f);
	m_OBBPoint[5] = glm::vec4(p6, 1.0f);
	m_OBBPoint[6] = glm::vec4(p7, 1.0f);
	m_OBBPoint[7] = glm::vec4(p8, 1.0f);
}

///////////////////////////////////////////////////////////////////////////
//
//	* getPlaneEquation(glm::vec3 P, glm::vec3 Q, glm::vec3 R)
//	세 점으로 평면의 방적식을 구함
//
///////////////////////////////////////////////////////////////////////////
glm::vec4 CW3OrientedBoundingBox::getPlaneEquation(glm::vec3 P, glm::vec3 Q, glm::vec3 R) {
	glm::vec3 PQ = Q - P;
	glm::vec3 PR = R - P;
	glm::vec3 normal = glm::cross(PQ, PR);
	float len = glm::length(normal);
	normal.x /= len;
	normal.y /= len;
	normal.z /= len;
	float d = -glm::dot(normal, P);

	return glm::vec4(normal.x, normal.y, normal.z, d);
}

///////////////////////////////////////////////////////////////////////////
//
//	* setMVP(mat4 mvp)
//	object 의 mvp 를 OBB 에 적용
//
///////////////////////////////////////////////////////////////////////////
void CW3OrientedBoundingBox::setMVP(const mat4&  mvp) {
	m_mu2 = mvp * m_mu;

	//printf("dot01: %f\n", glm::dot(glm::vec3(m_axis[0]), glm::vec3(m_axis[1])));
	//printf("dot02: %f\n", glm::dot(glm::vec3(m_axis[0]), glm::vec3(m_axis[2])));
	//printf("dot03: %f\n", glm::dot(glm::vec3(m_axis[2]), glm::vec3(m_axis[1])));

	m_axis2[0] = mvp * m_axis[0];
	m_axis2[1] = mvp * m_axis[1];
	m_axis2[2] = mvp * m_axis[2];

	//printf("dotmvp1: %f\n", glm::dot(glm::vec3(mvp[0]), glm::vec3(mvp[1])));
	//printf("dotmvp2: %f\n", glm::dot(glm::vec3(mvp[0]), glm::vec3(mvp[2])));
	//printf("dotmvp3: %f\n", glm::dot(glm::vec3(mvp[2]), glm::vec3(mvp[1])));
	//
	//float test = glm::dot(glm::vec3(glm::normalize(m_axis2[0])), glm::vec3(glm::normalize(m_axis2[1])));

	//printf("dot1: %f\n", test);
	//if (abs(test) > 1.0f)
	//{
	//	printf("x axis: %f, %f, %f\n", m_axis2[0].x, m_axis2[0].y, m_axis2[0].z);
	//	printf("y axis: %f, %f, %f\n", m_axis2[1].x, m_axis2[1].y, m_axis2[1].z);
	//}

	//printf("dot2: %f\n", glm::dot(glm::normalize(glm::vec3(m_axis2[0])), glm::normalize(glm::vec3(m_axis2[2]))));
	//printf("dot3: %f\n", glm::dot(glm::normalize(glm::vec3(m_axis2[2])), glm::normalize(glm::vec3(m_axis2[1]))));

	m_halfDim = vec3(glm::length(m_axis2[0]), glm::length(m_axis2[1]), glm::length(m_axis2[2]));
}

bool CW3OrientedBoundingBox::setModelScaled(const mat4&  model) {
	m_mu2 = model * m_mu;

	int maxVertexId = -1;
	float maxDist = 0.0f;
	glm::vec4 vertexDir;
	for (int i = 0; i < 4; i++) {
		glm::vec4 tmpDir = model * m_OBBPoint[i] - m_mu2;
		float tmp = glm::length(tmpDir);
		if (maxDist < tmp) {
			maxDist = tmp;
			maxVertexId = i;
			vertexDir = tmpDir;
		}
	}

	if (glm::length(vertexDir) < 1.0f) {
		return true;
	}

	glm::vec4 axis[3];
	axis[0] = model * m_axis[0];
	axis[1] = model * m_axis[1];
	axis[2] = model * m_axis[2];

	m_halfDim = vec3(glm::length(axis[0]), glm::length(axis[1]), glm::length(axis[2]));

	axisOrdering();

	glm::vec4 mainAxis = glm::normalize(axis[m_AxisOrder[0]]);
	mainAxis = glm::dot(mainAxis, vertexDir)*mainAxis;

	glm::vec4 thirdAxis = glm::vec4(glm::normalize(glm::cross(glm::vec3(mainAxis), glm::vec3(axis[m_AxisOrder[1]]))), 0.0f);
	thirdAxis = glm::dot(thirdAxis, vertexDir)*thirdAxis;

	glm::vec4 secondAxis = glm::vec4(glm::normalize(glm::cross(glm::vec3(mainAxis), glm::vec3(thirdAxis))), 0.0f);
	secondAxis = glm::dot(secondAxis, vertexDir)*secondAxis;

	m_axis2[0] = mainAxis;
	m_axis2[1] = secondAxis;
	m_axis2[2] = thirdAxis;

	//printf("dot1: %f\n", glm::dot(glm::normalize(glm::vec3(m_axis2[0])), glm::normalize(glm::vec3(m_axis2[1]))));
	//printf("dot2: %f\n", glm::dot(glm::normalize(glm::vec3(m_axis2[0])), glm::normalize(glm::vec3(m_axis2[2]))));
	//printf("dot3: %f\n", glm::dot(glm::normalize(glm::vec3(m_axis2[2])), glm::normalize(glm::vec3(m_axis2[1]))));

	m_halfDim = vec3(glm::length(m_axis2[0]), glm::length(m_axis2[1]), glm::length(m_axis2[2]));

	//printf("second dim: %f, %f, %f\n", m_halfDim.x, m_halfDim.y, m_halfDim.z);
	return false;
}

void CW3OrientedBoundingBox::axisOrdering() {
	if (m_halfDim.x >= m_halfDim.y && m_halfDim.x >= m_halfDim.z) {
		m_AxisOrder[0] = 0;
		if (m_halfDim.y >= m_halfDim.z) {
			m_AxisOrder[1] = 1;
			m_AxisOrder[2] = 2;
		} else {
			m_AxisOrder[1] = 2;
			m_AxisOrder[2] = 1;
		}
	} else if (m_halfDim.y >= m_halfDim.x && m_halfDim.y >= m_halfDim.z) {
		m_AxisOrder[0] = 1;
		if (m_halfDim.x >= m_halfDim.z) {
			m_AxisOrder[1] = 0;
			m_AxisOrder[2] = 2;
		} else {
			m_AxisOrder[1] = 2;
			m_AxisOrder[2] = 0;
		}
	} else if (m_halfDim.z >= m_halfDim.x && m_halfDim.z >= m_halfDim.y) {
		m_AxisOrder[0] = 2;
		if (m_halfDim.x >= m_halfDim.y) {
			m_AxisOrder[1] = 0;
			m_AxisOrder[2] = 1;
		} else {
			m_AxisOrder[1] = 1;
			m_AxisOrder[2] = 0;
		}
	}
}
