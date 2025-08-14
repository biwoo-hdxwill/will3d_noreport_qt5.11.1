#pragma once
/*=========================================================================

File:		Mesh primitive definitions. (vertex, edge, face)
Language:	C++11
Library:	Qt 5.2.1, Standard C++ Library

=========================================================================*/
#include <qvector3d.h>

namespace Mesh 
{
	// Table Entry Type. (index)
	typedef unsigned int ID_VERTEX;
	typedef unsigned int ID_HALFEDGE;
	typedef unsigned int ID_FACE;

	typedef std::pair<ID_VERTEX, ID_VERTEX>				Edge;
	typedef std::tuple<ID_VERTEX, float, float, float>	VertexInfo;
	typedef	VertexInfo	Triangle[3];

	// forward declaration.
	struct _Vertex;
	struct _HalfEdge;
	struct _Face;

	/* 
		Vertex structure definition.
	*/
	typedef struct _Vertex {
		float		_x;
		float		_y;
		float		_z;
		_HalfEdge*	_pEdge;		// outgoing edge pointer.
		QVector3D	_normal;	// vertex normal.

		_Vertex() {
			_x = _y = _z = 0.0f;
			_pEdge = nullptr;
		}
		_Vertex(float x, float y, float z, _HalfEdge *pE = nullptr) {
			_x = x;
			_y = y;
			_z = z;
			_pEdge = pE;
		}
	}Vertex;
	/*
		Face structure definition. (restriction : triangular faces)
	*/
	typedef struct _Face {
		_HalfEdge*	_pEdges[3];
		QVector3D	_normal;

		_Face() {
			_pEdges[0] = _pEdges[1] = _pEdges[2] = nullptr;
		}
		_Face(_HalfEdge* e1, _HalfEdge* e2, _HalfEdge* e3) {
			_pEdges[0] = e1;
			_pEdges[1] = e2;
			_pEdges[2] = e3;
		}
		_Face(_HalfEdge* e1, _HalfEdge* e2, _HalfEdge* e3, QVector3D& normal) {
			_pEdges[0] = e1;
			_pEdges[1] = e2;
			_pEdges[2] = e3;
			_normal = normal;
		}
	}Face;

	/*
		Half-Edge structure definition. (half-edge structure)
	*/
	typedef struct _HalfEdge {
		_Vertex*		_pTarget;	// target vertex pointer.
		_Face*			_pFace;		// incident face pointer. (belongs to)
		_HalfEdge*		_pPrev;		// previous edge pointer.
		_HalfEdge*		_pNext;		// next edge pointer.
		_HalfEdge*		_pInv;		// opposite edge pointer.

		_HalfEdge() :	_pTarget(nullptr), _pFace(nullptr), _pPrev(nullptr), _pNext(nullptr), _pInv(nullptr) {}
		_HalfEdge(Vertex* pt, Face* pf, _HalfEdge* pp, _HalfEdge* pn, _HalfEdge* pi) {
			_pTarget = pt;
			_pFace = pf;
			_pPrev = pp;
			_pNext = pn;
			_pInv = pi;
		}
	}HalfEdge;

}
