#pragma once
/*=========================================================================

File:		class CW3Camera
Language:	C++11
Library:	Qt 5.2.1, Standard C++ Library

=========================================================================*/
#include "resource_global.h"
#include "../../Common/Common/W3Point3D.h"
#include "../../Common/Common/W3Vector3D.h"
#include "../../Common/Common/W3Vector4D.h"
#include "../../Common/Common/W3Matrix4x4.h"

/*
	Camera class for Volume Rendering.
	Defines three vectors (up, right, back) and position of a camera.
	Supports translating & rotating operation.
*/
class RESOURCE_EXPORT CW3Camera
{
public:
	CW3Camera(void);
	// copy assignment.
	CW3Camera& operator=(CW3Camera& camera);
	~CW3Camera(void);

public:
	// public functions.
	inline void setCamera(CW3Point3D pos, CW3Vector3D up, CW3Vector3D back){
		m_position = pos;
		m_up = up;
		m_up.normalize();
		m_back = back;
		m_back.normalize();
		// set right vector.
		m_right = CW3Vector3D::crossProduct(m_up, m_back);
		m_right.normalize();}
	inline CW3Point3D	getPosition(void)		{ return m_position; }
	inline CW3Vector3D	getViewDirection(void)	{ return m_back.inverseVector(); }
	inline CW3Vector3D	getUpVec(void)		{ return m_up; }
	inline CW3Vector3D	getRightVec(void)	{ return m_right; }
	inline CW3Vector3D	getBackVec(void)	{ return m_back; }

	inline void	translate(CW3Vector3D vector){
		m_position.setX(m_position.x()+vector.x());
		m_position.setY(m_position.y()+vector.y());
		m_position.setZ(m_position.z()+vector.z());}
	void rotate(CW3Matrix4x4 rotMat);

private:
	// private fields.
	CW3Point3D	m_position;
	CW3Vector3D	m_up;
	CW3Vector3D	m_right;
	CW3Vector3D	m_back;
};
