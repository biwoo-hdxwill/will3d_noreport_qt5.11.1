#pragma once
/*=========================================================================

File:			class CW3OBBTree
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun, Hong Jung
First date:		2016-03-30
Last date:		2016-05-20

=========================================================================*/
#include <qstring.h>
#include <qlist.h>

#include "W3OrientedBoundingBox.h"
#include "../../Common/GLfunctions/W3GLTypes.h"

#include "resource_global.h"
////////////////////////////////////////////////////////////////////////
//
//	* CW3OBBTree
//	OBB Tree
//	vertex 를 받아 OBB Tree를 구성
//	recursive 하게 Left, Right 노드를 생성함
//	
///////////////////////////////////////////////////////////////////////////

class RESOURCE_EXPORT CW3OBBTree
{
public:
	//CW3OBBTree(std::vector<Triangle *> *triangles, int minTriangle, QString level = "0");
	CW3OBBTree(QList<Triangle *> *triangles, int minTriangle, bool isAxisAligned = false, QString level = "0");
	~CW3OBBTree();

	inline bool isLeaf() { return m_bIsLeaf; }
	inline CW3OBBTree *getLeft() { return m_pLeft; }
	inline CW3OBBTree *getRight() { return m_pRight; }

	//inline void setCollision(bool isCollide)
	//{
	//	m_pOBB->setCollision(isCollide);
	//}

	//inline bool isCollide()
	//{
	//	return m_pOBB->isCollide();
	//}

	inline CW3OrientedBoundingBox *getOBB() { return m_pOBB; }

private:
	void createTree(QList<Triangle *> *triangles);

private:
	QList<Triangle *> *m_pTriangles;

	int m_nSizeTriangles;	// 현재 triangle mesh 갯수

	CW3OBBTree *m_pLeft = nullptr;	// left 노드
	CW3OBBTree *m_pRight = nullptr;	// right 노드

	QList<Triangle *> *m_plvertLeft = nullptr;
	QList<Triangle *> *m_plvertRight = nullptr;

	CW3OrientedBoundingBox *m_pOBB = nullptr;	// 실제 OBB 객체

	bool m_bIsLeaf = false;	// 현재 노드가 leaf 인지 여부

	QString m_qsLevel;

	int m_nMinTriangle;	// 분할을 위한 triangle mesh 의 최소 갯수

	bool m_isAxisAligned;
};

