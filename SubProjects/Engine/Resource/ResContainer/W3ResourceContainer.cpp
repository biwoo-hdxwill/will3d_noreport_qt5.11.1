#include "W3ResourceContainer.h"
/*=========================================================================

File:			class CW3ResourceContainer
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2016-04-15
Last modify:	2016-04-15

=========================================================================*/
CW3ResourceContainer::CW3ResourceContainer() {
	for (int i = 0; i < 28; i++)
		m_isCollided[i] = false;

	for (int i = 0; i < 28; i++)
		m_isImplantThere[i] = false;
}

CW3ResourceContainer::~CW3ResourceContainer() {}

void CW3ResourceContainer::reset() {
	m_pgFacePhoto3D = nullptr;

	for (int i = 0; i < 28; i++)
		m_isCollided[i] = false;

	for (int i = 0; i < 28; i++)
		m_isImplantThere[i] = false;
}
