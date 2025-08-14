#include "W3Camera.h"

CW3Camera::CW3Camera(void)
{
}

CW3Camera& CW3Camera::operator=(CW3Camera& camera)
{
	m_position = camera.getPosition();
	m_up = camera.getUpVec();
	m_right = camera.getRightVec();
	m_back = camera.getBackVec();

	return *this;
}

CW3Camera::~CW3Camera(void)
{
}

void CW3Camera::rotate(CW3Matrix4x4 rotMat)
{ 
	CW3Vector4D tmpVec;
	CW3Vector4D resVec;

	// rotate Camera position.
	tmpVec.setX(m_position.x());
	tmpVec.setY(m_position.y());
	tmpVec.setZ(m_position.z());
	resVec = rotMat * tmpVec;
	m_position.setX(resVec.x());
	m_position.setY(resVec.y());
	m_position.setZ(resVec.z());

	// rotate vectors.
	tmpVec.setX(m_up.x());
	tmpVec.setY(m_up.y());
	tmpVec.setZ(m_up.z());
	resVec = rotMat * tmpVec;
	m_up.setX(resVec.x());
	m_up.setY(resVec.y());
	m_up.setZ(resVec.z());

	tmpVec.setX(m_right.x());
	tmpVec.setY(m_right.y());
	tmpVec.setZ(m_right.z());
	resVec = rotMat * tmpVec;
	m_right.setX(resVec.x());
	m_right.setY(resVec.y());
	m_right.setZ(resVec.z());

	tmpVec.setX(m_back.x());
	tmpVec.setY(m_back.y());
	tmpVec.setZ(m_back.z());
	resVec = rotMat * tmpVec;
	m_back.setX(resVec.x());
	m_back.setY(resVec.y());
	m_back.setZ(resVec.z());
}
