#pragma once
/*=========================================================================

File:		class CW3Mesh
Language:	C++11
Library:	Qt 5.2.1, Standard C++ Library

=========================================================================*/
#include "resource_global.h"
#include "W3MeshPrimitive.h"
#include "W3Resource.h"
#include "../../Common/Common/W3Types.h"
#include <set>
#include <map>
#include <vector>
#include <qvector3d.h>

class CW3Mask;

using namespace Mesh;
namespace Mesh{

	typedef	std::pair<Edge, HalfEdge*>			HalfEdgeElem;
	typedef std::map<Edge, HalfEdge*>			HalfEdgeMap;
	typedef std::pair<ID_VERTEX, Vertex*>		VertexElem;
	typedef	std::map<ID_VERTEX, Vertex*>		VertexMap;
	typedef	std::vector<Face*>					FaceList;


	// Plane ID.
	enum PLANE {
		XY = 0,
		YZ,
		ZX
	};

	// get projection plane ID.
	// direction of projection is max(normal vector components).
	extern "C++"
		__forceinline PLANE projPlane(QVector3D& vec) {
			if( std::abs(vec.x()) > std::abs(vec.y()) )
				return (std::abs(vec.x()) > std::abs(vec.z()) ? YZ : XY);
			else
				return (std::abs(vec.y()) > std::abs(vec.z()) ? ZX : XY);
	}
}

/*
	* Mesh Representation class.
		- Implemented by Half-edge structure.
		- Functions Supported:
			subdivision.
			voxelization.
*/
class RESOURCE_EXPORT CW3Mesh : public CW3Resource
{
public:
	CW3Mesh(void);
	~CW3Mesh(void);

public:
	//////////////////////////////
	///// Public Interfaces. /////
	//////////////////////////////
	// getter functions.
	VertexMap		listVertex(void) { return m_mapVertex; }
	FaceList		listFace(void) { return m_listFace; }

	// add triangle to mesh.
	void addTriangle(Triangle& triangle);
	// voxelization.
	void voxelize(CW3Mask& mask);
	// subdivision.
	void subdivision(void);

private:
	// project 3D-triangle to the 2D-plane.
	// getInner2DPoints()
	// backProjection()
	void voxelize_triangle(Face* face, CW3Mask& mask);
	// line algorithm (2D with given point; vertices.)
	// scan line for 2D triangle.
	// return plane's points.
	void getInner2DPoints(QPointF& pt1, QPointF& pt2, QPointF& pt3, std::vector<QPoint>& list);
	void lineAlgorithm(QPointF& pt0, QPointF& pt1, W3CHAR** img, const W3INT width, const W3INT height, const W3INT minX, const W3INT minY);
	// back-project 2D-points to 3D triangle.
	void backProjectionFrmXY(std::vector<QPoint>& list, Face* face, CW3Mask& mask);
	void backProjectionFrmYZ(std::vector<QPoint>& list, Face* face, CW3Mask& mask);
	void backProjectionFrmZX(std::vector<QPoint>& list, Face* face, CW3Mask& mask);
	
	
private:
	// private member fields.
	VertexMap		m_mapVertex;
	FaceList		m_listFace;
	//HalfEdgeList	m_listHalfEdge;
	HalfEdgeMap		m_mapEdges;
};

