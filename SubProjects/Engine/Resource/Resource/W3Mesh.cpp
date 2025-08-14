#include "W3Mesh.h"
#include "../../Resource/W3Mask.h"
#include "../../Common/Common/W3Memory.h"
#include <tuple>
#include <cassert>

using namespace Mesh;

CW3Mesh::CW3Mesh(void) :
	CW3Resource(ERESOURCE_TYPE::MESH)
{
	
}


CW3Mesh::~CW3Mesh(void)
{
	for( auto &x : m_mapVertex )	SAFE_DELETE_OBJECT(x.second);
	for( auto &x : m_listFace )		SAFE_DELETE_OBJECT(x);
	for( auto &x : m_mapEdges )		SAFE_DELETE_OBJECT(x.second);

	m_mapVertex.clear();
	m_listFace.clear();
	m_mapEdges.clear();
}

void CW3Mesh::addTriangle(Triangle& triangle)
{
	VertexElem v1 = std::make_pair(
		std::get<0>(triangle[0]), 
		new Vertex(std::get<1>(triangle[0]), std::get<2>(triangle[0]), std::get<3>(triangle[0]))
		);
	VertexElem v2 = std::make_pair(
		std::get<0>(triangle[1]), 
		new Vertex(std::get<1>(triangle[1]), std::get<2>(triangle[1]), std::get<3>(triangle[1]))
		);
	VertexElem v3 = std::make_pair(
		std::get<0>(triangle[2]), 
		new Vertex(std::get<1>(triangle[2]), std::get<2>(triangle[2]), std::get<3>(triangle[2]))
		);

	HalfEdgeElem E1(Edge(v1.first, v2.first), 
		new HalfEdge(v2.second, nullptr, nullptr, nullptr, nullptr));
	HalfEdgeElem E2(Edge(v2.first, v3.first), 
		new HalfEdge(v3.second, nullptr, nullptr, nullptr, nullptr));
	HalfEdgeElem E3(Edge(v3.first, v1.first), 
		new HalfEdge(v1.second, nullptr, nullptr, nullptr, nullptr));

	v1.second->_pEdge = E1.second;
	v2.second->_pEdge = E2.second;
	v3.second->_pEdge = E3.second;

	QVector3D vec1;
	vec1.setX(v2.second->_x - v1.second->_x);
	vec1.setY(v2.second->_y - v1.second->_y);
	vec1.setZ(v2.second->_z - v1.second->_z);
	QVector3D vec2;
	vec2.setX(v3.second->_x - v2.second->_x);
	vec2.setY(v3.second->_y - v2.second->_y);
	vec2.setZ(v3.second->_z - v2.second->_z);

	Face *F = new Face(E1.second, E2.second, E3.second, QVector3D::crossProduct(vec2, vec1));

	E1.second->_pFace = F;
	E2.second->_pFace = F;
	E3.second->_pFace = F;

	E1.second->_pNext = E2.second;	E2.second->_pNext = E3.second;	E3.second->_pNext = E1.second;
	E1.second->_pPrev = E3.second;	E2.second->_pPrev = E1.second;	E3.second->_pPrev = E2.second;

	if( m_mapEdges.find(Edge(v2.first, v1.first)) != m_mapEdges.end() ){
		E1.second->_pInv = m_mapEdges.find(Edge(v2.first, v1.first))->second;
		m_mapEdges.find(Edge(v2.first, v1.first))->second->_pInv = E1.second;
	}
	if( m_mapEdges.find(Edge(v3.first, v2.first)) != m_mapEdges.end() ){
		E2.second->_pInv = m_mapEdges.find(Edge(v3.first, v2.first))->second;
		m_mapEdges.find(Edge(v3.first, v2.first))->second->_pInv = E2.second;
	}
	if( m_mapEdges.find(Edge(v1.first, v3.first)) != m_mapEdges.end() ){
		E3.second->_pInv = m_mapEdges.find(Edge(v1.first, v3.first))->second;
		m_mapEdges.find(Edge(v1.first, v3.first))->second->_pInv = E3.second;
	}

	m_mapEdges.insert(E1);
	m_mapEdges.insert(E2);
	m_mapEdges.insert(E3);
	//m_listHalfEdge.push_back(E1.second);
	//m_listHalfEdge.push_back(E2.second);
	//m_listHalfEdge.push_back(E3.second);
	m_listFace.push_back(F);

	m_mapVertex.insert(v1);
	m_mapVertex.insert(v2);
	m_mapVertex.insert(v3);
}

/*
	* Voxelization.
		- for all faces, (surface voxelization)
		- scan line - for 3D.
*/
void CW3Mesh::voxelize(CW3Mask& mask)
{
	mask.resetBuffer();

	// For all faces, perform voxelization of surface.
	for( const auto& face : m_listFace )
		voxelize_triangle(face, mask);

	// grow a little. (connectivity issues)


	// Scan-lines. (3D)
	for( W3INT i=0; i<mask.depth(); i++ ) {
		for( W3INT j=0; j<mask.height(); j++ ) {
			W3INT min = 0, max = -1;
			for( W3INT k=0; k<mask.width(); k++ ) {
				if( mask.data()[i][j*mask.width() + k] == 1 ){
					min = k;
					break;
				}
			}
			for( W3INT k=mask.width()-1; k>=0; k-- ) {
				if( mask.data()[i][j*mask.width() + k] == 1 ){
					max = k;
					break;
				}
			}
			for( W3INT k=min; k<=max; k++ )
				mask.data()[i][j*mask.width() + k] = 1;
		}
	}
}
void CW3Mesh::voxelize_triangle(Face *face, CW3Mask& mask)
{
	// project 3D-triangle to the 2D-plane.
	// line algorithm.
	// scan line (for 2D)
	// back-projection points to 3D-triangle.

	// 2D-plane projected points.
	QPointF pt1;
	QPointF pt2;
	QPointF pt3;

	std::vector<QPoint> list2D;

	switch ( Mesh::projPlane(face->_normal) )
	{
	case Mesh::PLANE::XY:
		pt1.setX(face->_pEdges[0]->_pTarget->_x);
		pt1.setY(face->_pEdges[0]->_pTarget->_y);
		pt2.setX(face->_pEdges[1]->_pTarget->_x);
		pt2.setY(face->_pEdges[1]->_pTarget->_y);
		pt3.setX(face->_pEdges[2]->_pTarget->_x);
		pt3.setY(face->_pEdges[2]->_pTarget->_y);
		getInner2DPoints(pt1, pt2, pt3, list2D);
		backProjectionFrmXY(list2D, face, mask);
		break;
	case Mesh::PLANE::YZ:
		pt1.setX(face->_pEdges[0]->_pTarget->_y);
		pt1.setY(face->_pEdges[0]->_pTarget->_z);
		pt2.setX(face->_pEdges[1]->_pTarget->_y);
		pt2.setY(face->_pEdges[1]->_pTarget->_z);
		pt3.setX(face->_pEdges[2]->_pTarget->_y);
		pt3.setY(face->_pEdges[2]->_pTarget->_z);
		getInner2DPoints(pt1, pt2, pt3, list2D);
		backProjectionFrmYZ(list2D, face, mask);
		break;
	case Mesh::PLANE::ZX:
		pt1.setY(face->_pEdges[0]->_pTarget->_x);
		pt1.setX(face->_pEdges[0]->_pTarget->_z);
		pt2.setY(face->_pEdges[1]->_pTarget->_x);
		pt2.setX(face->_pEdges[1]->_pTarget->_z);
		pt3.setY(face->_pEdges[2]->_pTarget->_x);
		pt3.setX(face->_pEdges[2]->_pTarget->_z);
		getInner2DPoints(pt1, pt2, pt3, list2D);
		backProjectionFrmZX(list2D, face, mask);
		break;
	}
}



void CW3Mesh::getInner2DPoints(QPointF& pt1, QPointF& pt2, QPointF& pt3, std::vector<QPoint>& list)
{
	const W3INT maxX = static_cast<int>(std::ceilf(std::max(pt1.x(), std::max(pt2.x(), pt3.x()))));
	const W3INT maxY = static_cast<int>(std::ceilf(std::max(pt1.y(), std::max(pt2.y(), pt3.y()))));
	const W3INT minX = static_cast<int>(std::floorf(std::min(pt1.x(), std::min(pt2.x(), pt3.x()))));
	const W3INT minY = static_cast<int>(std::floorf(std::min(pt1.y(), std::min(pt2.y(), pt3.y()))));
	const int width = maxX - minX + 1;
	const int height = maxY - minY + 1;
	char **img = new W3CHAR*[height];
	for( W3INT i=0; i<height; i++ )
		img[i] = new W3CHAR[width];
	for( W3INT i=0; i<height; i++ )
	for( W3INT j=0; j<width; j++ )
		img[i][j] = 0;

	// line voxelization.
	// for three line segments,
	W3FLOAT deltaX = 0.0f;
	W3FLOAT deltaY = 0.0f;
	W3FLOAT error = 0.0f;
	W3FLOAT deltaErr = 0.0f;
	W3INT y = 0;
	
	// pt1 -- pt2
	lineAlgorithm(pt1, pt2, img, width, height, minX, minY);
	lineAlgorithm(pt2, pt3, img, width, height, minX, minY);
	lineAlgorithm(pt3, pt1, img, width, height, minX, minY);

	// scan-line for 2D.
	for( W3INT i=0; i<height; i++ ) {
		W3INT min=0, max=-1;
		for( W3INT j=0; j<width; j++ ){
			if( img[i][j] == 1 ){
				min = j;
				break;
			}
		}
		for( W3INT j=width-1; j>=0; j-- ){
			if( img[i][j] == 1 ){
				max = j;
				break;
			}
		}
		for( W3INT j=min; j<=max; j++ )
			img[i][j] = 1;
	}

	// 2D-Pointerization.
	for( W3INT i=0; i<height; i++ )
	for( W3INT j=0; j<width; j++ )
	{
		if( img[i][j] == 1 )
			list.push_back(QPoint(j+minX, i+minY));
	}
	for( W3INT i=0; i<height; i++ )
		SAFE_DELETE_ARRAY(img[i]);
	SAFE_DELETE_ARRAY(img);
}

void CW3Mesh::lineAlgorithm(QPointF& pt0, QPointF& pt1, W3CHAR** img, const W3INT width, const W3INT height, const W3INT minX, const W3INT minY)
{
	// pt0 -- pt1
	QPointF P0, P1;
	if( pt0.x() < pt1.x() ){ P0 = pt0; P1 = pt1; }
	else { P0 = pt1; P1 = pt0; }
	W3FLOAT deltaX = P1.x() - P0.x();
	W3FLOAT deltaY = P1.y() - P0.y();
	W3FLOAT error = 0.0f;
	W3FLOAT deltaErr = (deltaY / deltaX);
	W3INT y = static_cast<W3INT>(P0.y());
	if( deltaX < 0.000001f )
	{
		W3INT x = static_cast<W3INT>(pt0.x());
		W3INT y0 = std::min(P0.y(), P1.y());
		W3INT y1 = std::max(P0.y(), P1.y());
		for( W3INT dy = y0; dy<=y1; dy++ )
			img[dy-minY][x-minX] = 1;
		return;
	}
	for( W3INT x = static_cast<W3INT>(P0.x()); x < static_cast<W3INT>(std::ceilf(P1.x())); x++ )
	{
		img[y-minY][x-minX] = 1;
		error += std::abs(deltaErr);
		while( error >= 0.5f ) {
			img[y-minY][x-minX] = 1;
			P1.y() - P0.y() > 0 ? y++ : y--;
			error -= 1.0f;
		}
	}
}

void CW3Mesh::backProjectionFrmXY(std::vector<QPoint>& list, Face* face, CW3Mask& mask)
{
	QVector3D v(0.0f, 0.0f, 1.0f);
	QVector3D n = face->_normal;
	QVector3D P0(face->_pEdges[0]->_pTarget->_x, face->_pEdges[0]->_pTarget->_y, face->_pEdges[0]->_pTarget->_z);
	for( auto& pt : list )
	{
		QVector3D P(pt.x(), pt.y(), 0.0f);
		W3FLOAT d = QVector3D::dotProduct(P0-P, n) / QVector3D::dotProduct(v, n);
		QVector3D target = P + v*d;
		// set 1 for mask position.
		W3INT x = static_cast<W3INT>(target.x());
		W3INT y = static_cast<W3INT>(target.y());
		W3INT z = static_cast<W3INT>(target.z());
		if( x>=0 && x<mask.width() && y>=0 && y<mask.height() && z>=0 && z<mask.depth() )
			mask.data()[z][y * mask.width() + x] = 1;
	}
}
void CW3Mesh::backProjectionFrmYZ(std::vector<QPoint>& list, Face* face, CW3Mask& mask)
{
	QVector3D v(1.0f, 0.0f, 0.0f);
	QVector3D n = face->_normal;
	QVector3D P0(face->_pEdges[0]->_pTarget->_x, face->_pEdges[0]->_pTarget->_y, face->_pEdges[0]->_pTarget->_z);
	for( auto& pt : list )
	{
		QVector3D P(0.0f, pt.x(), pt.y());
		W3FLOAT d = QVector3D::dotProduct(P0-P, n) / QVector3D::dotProduct(v, n);
		QVector3D target = P + v*d;
		// set 1 for surrounded mask position.
		W3INT x = static_cast<W3INT>(target.x());
		W3INT y = static_cast<W3INT>(target.y());
		W3INT z = static_cast<W3INT>(target.z());
		if( x>=0 && x<mask.width() && y>=0 && y<mask.height() && z>=0 && z<mask.depth() )
			mask.data()[z][y * mask.width() + x] = 1;
	}
}
void CW3Mesh::backProjectionFrmZX(std::vector<QPoint>& list, Face* face, CW3Mask& mask)
{
	QVector3D v(0.0f, 1.0f, 0.0f);
	QVector3D n = face->_normal;
	QVector3D P0(face->_pEdges[0]->_pTarget->_x, face->_pEdges[0]->_pTarget->_y, face->_pEdges[0]->_pTarget->_z);
	for( auto& pt : list )
	{
		QVector3D P(pt.y(), 0.0f, pt.x());
		W3FLOAT d = QVector3D::dotProduct(P0-P, n) / QVector3D::dotProduct(v, n);
		QVector3D target = P + v*d;
		// set 1 for surrounded mask position.
		W3INT x = static_cast<W3INT>(target.x());
		W3INT y = static_cast<W3INT>(target.y());
		W3INT z = static_cast<W3INT>(target.z());
		if( x>=0 && x<mask.width() && y>=0 && y<mask.height() && z>=0 && z<mask.depth() )
			mask.data()[z][y * mask.width() + x] = 1;
	}
}

void CW3Mesh::subdivision(void)
{
	
}
