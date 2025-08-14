#include "W3Collider.h"
/*=========================================================================

File:			class CW3Collider
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun, Hong Jung
First date:		2016-03-30
Last modify:	2016-05-23

=========================================================================*/
#include <GL\glm\mat4x4.hpp>

CW3Collider::CW3Collider() {}

///////////////////////////////////////////////////////////////////////////
//
//	* initCollision(CW3OBBTree *obbTree)
//	OBB Tree 내 모든 노드의 충돌 상태를 초기화
//
///////////////////////////////////////////////////////////////////////////
//void CW3Collider::initCollision(CW3OBBTree *obbTree)
//{
//	count = 0;
//	obbTree->setCollision(false);
//
//	if (!obbTree->isLeaf())
//	{
//		initCollision(obbTree->getLeft());
//		initCollision(obbTree->getRight());
//	}
//}

///////////////////////////////////////////////////////////////////////////
//
//	* collisionDetection(CW3OBBTree *obbTreeA, mat4 mvpA, CW3OBBTree *obbTreeB, mat4 mvpB)
//	OBB Tree 두 개 사이의 재귀적 충돌 검사
//
///////////////////////////////////////////////////////////////////////////
bool CW3Collider::collisionDetection(CW3OBBTree *obbTreeA, const mat4& mvpA,
									 CW3OBBTree *obbTreeB, const mat4& mvpB) {
	if (!obbTreeA || !obbTreeB)
		return false;

	// 충돌체크
	bool isCollide = collision(obbTreeA, mvpA, obbTreeB, mvpB);
	if (!isCollide)
		return false;	// 충돌 false면 자식노드 체크할 필요 없음, 상위 노드로 이동

	if (obbTreeA->isLeaf() && obbTreeB->isLeaf())	// 두 OBB 모두 리프노드일 때 충돌 true면 충돌처리
	{
		//obbTreeA->setCollision(true);
		//obbTreeB->setCollision(true);
		return true;
	}

	if (!obbTreeA->isLeaf()) {
		CW3OBBTree *leftA = obbTreeA->getLeft();
		isCollide = collisionDetection(leftA, mvpA, obbTreeB, mvpB);	// 충돌 false일 때 다음 진행

		if (!isCollide) {
			CW3OBBTree *rightA = obbTreeA->getRight();
			isCollide = collisionDetection(rightA, mvpA, obbTreeB, mvpB);
		}
	}
	// obbA의 left가 충돌 true이고 리프일 때 right는 탐색할 필요 없음

	if (!isCollide)
		return false;

	// A Tree 의 leaf 노드가 아닌 경우 B Tree 의 자식 노드들과 충돌 검사를 할 필요가 없음
	if (!obbTreeA->isLeaf())
		return isCollide;

	if (!obbTreeB->isLeaf()) {
		CW3OBBTree *leftB = obbTreeB->getLeft();
		isCollide = collisionDetection(obbTreeA, mvpA, leftB, mvpB);

		if (!isCollide) {
			CW3OBBTree *rightB = obbTreeB->getRight();
			isCollide = collisionDetection(obbTreeA, mvpA, rightB, mvpB);
		}
	}
	// obbB의 left가 충돌 true이고 리프일 때 right는 탐색할 필요 없음

	if (!isCollide)
		return false;

	return true;
}

///////////////////////////////////////////////////////////////////////////
//
//	* collision(CW3OrientedBoundingBox *obbA, mat4 mvpA, CW3OrientedBoundingBox *obbB, mat4 mvpB)
//	separating axis theory 를 사용하여 OBB A 와 B 의 충돌여부를 검사
//
///////////////////////////////////////////////////////////////////////////
bool CW3Collider::collision(CW3OBBTree *obbTreeA, const mat4& mvpA, CW3OBBTree *obbTreeB, const mat4& mvpB) {
	CW3OrientedBoundingBox *obbA = obbTreeA->getOBB();
	CW3OrientedBoundingBox *obbB = obbTreeB->getOBB();
	obbA->setMVP(mvpA);
	obbB->setMVP(mvpB);
	//bool bEnd = obbA->setModelScaled(mvpA);
	//bEnd |= obbB->setModelScaled(mvpB);

	//if (bEnd)
	//	return true;

	vec3 muA = obbA->getMean();
	vec3 axisAx = obbA->getAxis(0);
	vec3 axisAy = obbA->getAxis(1);
	vec3 axisAz = obbA->getAxis(2);
	vec3 halfDimA = obbA->getHalfDim();

	vec3 muB = obbB->getMean();
	vec3 axisBx = obbB->getAxis(0);
	vec3 axisBy = obbB->getAxis(1);
	vec3 axisBz = obbB->getAxis(2);
	vec3 halfDimB = obbB->getHalfDim();

	// separating axis theory
	vec3 T = muB - muA;

	vec3 L(0.0f);
	float D = 0.0f;
	float rA = 0.0f;
	float rB = 0.0f;

	float e = 0.0f;

	// 15개의 분리축 후보를 검사하여 분리축이 발견되면 충돌하지 않은 것으로 판정

	// case 1 : L = Ax
	D = 0.0f;	rA = 0.0f;	rB = 0.0f;
	L = axisAx.xyz;
	L = L / glm::length(L);
	D = fabs(glm::dot(T, L));
	rA = halfDimA.x;
	rB = fabs(glm::dot(axisBx, L))
		+ fabs(glm::dot(axisBy, L))
		+ fabs(glm::dot(axisBz, L));

	if (D + e > rA + rB) {
		return false;
	}

	// case 2 : L = Ay
	D = 0.0f;	rA = 0.0f;	rB = 0.0f;
	L = axisAy;
	L = L / glm::length(L);
	D = fabs((glm::dot(T, L)));
	rA = halfDimA.y;
	rB = fabs(glm::dot(axisBx, L))
		+ fabs(glm::dot(axisBy, L))
		+ fabs(glm::dot(axisBz, L));

	if (D + e > rA + rB) {
		return false;
	}

	// case 3 : L = Az
	D = 0.0f;	rA = 0.0f;	rB = 0.0f;
	L = axisAz;
	L = L / glm::length(L);
	D = fabs(glm::dot(T, L));
	rA = halfDimA.z;
	rB = fabs(glm::dot(axisBx, L))
		+ fabs(glm::dot(axisBy, L))
		+ fabs(glm::dot(axisBz, L));

	if (D + e > rA + rB) {
		return false;
	}

	// case 4 : L = Bx
	D = 0.0f;	rA = 0.0f;	rB = 0.0f;
	L = axisBx;
	L = L / glm::length(L);
	D = fabs(glm::dot(T, L));
	rA = fabs(glm::dot(axisAx, L))
		+ fabs(glm::dot(axisAy, L))
		+ fabs(glm::dot(axisAz, L));
	rB = halfDimB.x;

	if (D + e > rA + rB) {
		return false;
	}

	// case 5 : L = By
	D = 0.0f;	rA = 0.0f;	rB = 0.0f;
	L = axisBy;
	L = L / glm::length(L);
	D = fabs(glm::dot(T, L));
	rA = fabs(glm::dot(axisAx, L))
		+ fabs(glm::dot(axisAy, L))
		+ fabs(glm::dot(axisAz, L));
	rB = halfDimB.y;

	if (D + e > rA + rB) {
		return false;
	}

	// case 6 : L = Bz
	D = 0.0f;	rA = 0.0f;	rB = 0.0f;
	L = axisBz;
	L = L / glm::length(L);
	D = fabs(glm::dot(T, L));
	rA = fabs(glm::dot(axisAx, L))
		+ fabs(glm::dot(axisAy, L))
		+ fabs(glm::dot(axisAz, L));
	rB = halfDimB.z;

	if (D + e > rA + rB) {
		return false;
	}

	// case 7 : L = Ax X Bx
	D = 0.0f;	rA = 0.0f;	rB = 0.0f;
	L = glm::cross(axisAx, axisBx);
	L = L / glm::length(L);
	D = fabs(glm::dot(T, L));
	rA = fabs(glm::dot(axisAx, L))
		+ fabs(glm::dot(axisAy, L))
		+ fabs(glm::dot(axisAz, L));
	rB = fabs(glm::dot(axisBx, L))
		+ fabs(glm::dot(axisBy, L))
		+ fabs(glm::dot(axisBz, L));

	if (D + e > rA + rB) {
		return false;
	}

	// case 8 : L = Ax X By
	D = 0.0f;	rA = 0.0f;	rB = 0.0f;
	L = glm::cross(axisAx, axisBy);
	L = L / glm::length(L);
	D = fabs(glm::dot(T, L));
	rA = fabs(glm::dot(axisAx, L))
		+ fabs(glm::dot(axisAy, L))
		+ fabs(glm::dot(axisAz, L));
	rB = fabs(glm::dot(axisBx, L))
		+ fabs(glm::dot(axisBy, L))
		+ fabs(glm::dot(axisBz, L));

	if (D + e > rA + rB) {
		return false;
	}

	// case 9 : L = Ax X Bz
	D = 0.0f;	rA = 0.0f;	rB = 0.0f;
	L = glm::cross(axisAx, axisBz);
	L = L / glm::length(L);
	D = fabs(glm::dot(T, L));
	rA = fabs(glm::dot(axisAx, L))
		+ fabs(glm::dot(axisAy, L))
		+ fabs(glm::dot(axisAz, L));
	rB = fabs(glm::dot(axisBx, L))
		+ fabs(glm::dot(axisBy, L))
		+ fabs(glm::dot(axisBz, L));

	if (D + e > rA + rB) {
		return false;
	}

	// case 10 : L = Ay X Bx
	D = 0.0f;	rA = 0.0f;	rB = 0.0f;
	L = glm::cross(axisAy, axisBx);
	L = L / glm::length(L);
	D = fabs(glm::dot(T, L));
	rA = fabs(glm::dot(axisAx, L))
		+ fabs(glm::dot(axisAy, L))
		+ fabs(glm::dot(axisAz, L));
	rB = fabs(glm::dot(axisBx, L))
		+ fabs(glm::dot(axisBy, L))
		+ fabs(glm::dot(axisBz, L));

	if (D + e > rA + rB) {
		return false;
	}

	// case 11 : L = Ay X By
	D = 0.0f;	rA = 0.0f;	rB = 0.0f;
	L = glm::cross(axisAy, axisBy);
	L = L / glm::length(L);
	D = fabs(glm::dot(T, L));
	rA = fabs(glm::dot(axisAx, L))
		+ fabs(glm::dot(axisAy, L))
		+ fabs(glm::dot(axisAz, L));
	rB = fabs(glm::dot(axisBx, L))
		+ fabs(glm::dot(axisBy, L))
		+ fabs(glm::dot(axisBz, L));

	if (D + e > rA + rB) {
		return false;
	}

	// case 12 : L = Ay X Bz
	D = 0.0f;	rA = 0.0f;	rB = 0.0f;
	L = glm::cross(axisAy, axisBz);
	L = L / glm::length(L);
	D = fabs(glm::dot(T, L));
	rA = fabs(glm::dot(axisAx, L))
		+ fabs(glm::dot(axisAy, L))
		+ fabs(glm::dot(axisAz, L));
	rB = fabs(glm::dot(axisBx, L))
		+ fabs(glm::dot(axisBy, L))
		+ fabs(glm::dot(axisBz, L));

	if (D + e > rA + rB) {
		return false;
	}

	// case 13 : L = Az X Bx
	D = 0.0f;	rA = 0.0f;	rB = 0.0f;
	L = glm::cross(axisAz, axisBx);
	L = L / glm::length(L);
	D = fabs(glm::dot(T, L));
	rA = fabs(glm::dot(axisAx, L))
		+ fabs(glm::dot(axisAy, L))
		+ fabs(glm::dot(axisAz, L));
	rB = fabs(glm::dot(axisBx, L))
		+ fabs(glm::dot(axisBy, L))
		+ fabs(glm::dot(axisBz, L));

	if (D + e > rA + rB) {
		return false;
	}

	// case 14 : L = Az X By
	D = 0.0f;	rA = 0.0f;	rB = 0.0f;
	L = glm::cross(axisAz, axisBy);
	L = L / glm::length(L);
	D = fabs(glm::dot(T, L));
	rA = fabs(glm::dot(axisAx, L))
		+ fabs(glm::dot(axisAy, L))
		+ fabs(glm::dot(axisAz, L));
	rB = fabs(glm::dot(axisBx, L))
		+ fabs(glm::dot(axisBy, L))
		+ fabs(glm::dot(axisBz, L));

	if (D + e > rA + rB) {
		return false;
	}

	// case 15 : L = Az X Bz
	D = 0.0f;	rA = 0.0f;	rB = 0.0f;
	L = glm::cross(axisAz, axisBz);
	L = L / glm::length(L);
	D = fabs(glm::dot(T, L));
	rA = fabs(glm::dot(axisAx, L))
		+ fabs(glm::dot(axisAy, L))
		+ fabs(glm::dot(axisAz, L));
	rB = fabs(glm::dot(axisBx, L))
		+ fabs(glm::dot(axisBy, L))
		+ fabs(glm::dot(axisBz, L));

	if (D + e > rA + rB) {
		return false;
	}

	return true;
}
