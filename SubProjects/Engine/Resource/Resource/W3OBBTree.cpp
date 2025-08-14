#include "W3OBBTree.h"
/*=========================================================================

File:			class CW3OBBTree
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun
First date:		2016-03-30
Last modify:	2016-03-30

=========================================================================*/
// 2진으로 쪼개는 역할
// Triangle 은 -1~1 사이의 volume cube 에서의 좌표
// Triangle 각 꼭지점은 index 순서로 중복되게 할당되어 있음
// 각 3D object 마다 하나의 CW3OBBTree root node 를 갖음
CW3OBBTree::CW3OBBTree(QList<Triangle *> *triangles, int minTriangle, bool isAxisAligned, QString level)
	: m_nMinTriangle(minTriangle), m_qsLevel(level), m_pTriangles(triangles), m_isAxisAligned(isAxisAligned) {
	m_nSizeTriangles = triangles->size();
	createTree(triangles);
}

CW3OBBTree::~CW3OBBTree() {
	if (m_bIsLeaf) {
		while (m_pTriangles->size()) {
			auto iter = m_pTriangles->begin();
			SAFE_DELETE_OBJECT(*iter);
			m_pTriangles->erase(iter);
		}
	}

	SAFE_DELETE_OBJECT(m_pTriangles);

	SAFE_DELETE_OBJECT(m_pLeft);
	SAFE_DELETE_OBJECT(m_pRight);
	SAFE_DELETE_OBJECT(m_pOBB);
}

///////////////////////////////////////////////////////////////////////////
//
//	* createTree(std::vector<Triangle *> *triangles)
//	vertex 를 두 그룹으로 나눠서 모든 leaf 노드가 최소 삼각형 갯수를 가질 때까지
//	이진트리를 구성
//
///////////////////////////////////////////////////////////////////////////
void CW3OBBTree::createTree(QList<Triangle *> *triangles) {
	m_pOBB = new CW3OrientedBoundingBox(triangles, m_isAxisAligned);
	// bounding box (m_vOBBPoint) 만들고 separting plane (m_vHalfPlaneLongedtAxis) 까지 만듬

	QByteArray level = m_qsLevel.toLatin1();

	if (m_nSizeTriangles <= m_nMinTriangle) {
		m_bIsLeaf = true;
		return;
	}

	m_plvertLeft = new QList<Triangle*>;
	m_plvertRight = new QList<Triangle*>;

	vec4 separatePlane = m_pOBB->getTreeSeparatePlane();

	// 삼격형이 분할평면에 걸쳐져 있어도 삼각형을 구성하는 세 점을 모두 한쪽 노드로 분류
	m_plvertLeft->clear();
	m_plvertRight->clear();
	for (int i = 0; i < m_nSizeTriangles; i++) {
		bool left[3] = { false, false, false };

		for (int j = 0; j < 3; j++) {
			vec3 vertex(0.0f);
			switch (j) {
			case 0:
				vertex = triangles->at(i)->v1;
				break;
			case 1:
				vertex = triangles->at(i)->v0;
				break;
			case 2:
				vertex = triangles->at(i)->v2;
				break;
			}

			if ((vertex.x * separatePlane.x) + (vertex.y * separatePlane.y)
				+ (vertex.z * separatePlane.z) + separatePlane.w > 0.0f) {
				left[j] = true;
			}
		}

		if (left[0] || left[1] || left[2]) {
			m_plvertLeft->push_back(triangles->at(i));
		} else {
			m_plvertRight->push_back(triangles->at(i));
		}
	}

	int sizeVertLeft = m_plvertLeft->size();
	int sizeVertRight = m_plvertRight->size();

	//if (sizeVertLeft == 0 || sizeVertRight == 0)
	//	continue;
	//else
	//	break;

// 장축을 기준으로 vertex 를 분리했을 때, left 나 right 중 한 쪽의 triangle mesh 의 갯수가 0일 경우
// index 순서대로 triangle mesh 의 갯수를 반으로 나눔
	if (sizeVertLeft == 0 || sizeVertRight == 0) {
		m_plvertLeft->clear();
		m_plvertRight->clear();

		if (m_nSizeTriangles > m_nMinTriangle) {
			sizeVertLeft = m_nSizeTriangles / 2;
			sizeVertRight = m_nSizeTriangles - sizeVertLeft;

			if (sizeVertLeft == 0 || sizeVertRight == 0) {
				m_bIsLeaf = true;

				SAFE_DELETE_OBJECT(m_plvertLeft);
				SAFE_DELETE_OBJECT(m_plvertRight);
				return;
			}
			for (int i = 0; i < sizeVertLeft; i++) {
				m_plvertLeft->push_back(triangles->at(i));
			}

			for (int i = 0; i < sizeVertRight; i++) {
				m_plvertRight->push_back(triangles->at(sizeVertLeft + i));
			}
		} else {
			m_bIsLeaf = true;

			SAFE_DELETE_OBJECT(m_plvertLeft);
			SAFE_DELETE_OBJECT(m_plvertRight);
			return;
		}
	}

	// 트리 구성을 위한 재귀적 객체 생성
	m_pLeft = new CW3OBBTree(m_plvertLeft, m_nMinTriangle, m_isAxisAligned, m_qsLevel + QString(" L"));
	m_pRight = new CW3OBBTree(m_plvertRight, m_nMinTriangle, m_isAxisAligned, m_qsLevel + QString(" R"));
}
