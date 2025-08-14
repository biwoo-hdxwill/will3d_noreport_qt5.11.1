#pragma once
/*=========================================================================

File:			class CW3Collider
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun, Hong Jung
First date:		2016-03-30
Last modify:	2016-05-23

=========================================================================*/
#include "../Resource/W3OBBTree.h"
#include "rescontainer_global.h"

class CW3OBBTree;
///////////////////////////////////////////////////////////////////////////
//
//	* CW3Collider
//	OBB 의 충돌을 검사하는 class
//
///////////////////////////////////////////////////////////////////////////

class RESCONTAINER_EXPORT CW3Collider {
public:
	CW3Collider();
	~CW3Collider() {}

	//void initCollision(CW3OBBTree *obbTree);
	bool collisionDetection(CW3OBBTree* obbTreeA, const mat4& mvpA,
							CW3OBBTree *obbTreeB, const mat4& mvpB);

private:
	bool collision(CW3OBBTree *obbTreeA, const mat4& mvpA, CW3OBBTree *obbTreeB, const mat4& mvpB);
};

