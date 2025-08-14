#pragma once
/*=========================================================================

File:			class CW3ResourceContainer
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2016-04-15
Last modify:	2016-04-15

=========================================================================*/
#include <qobject.h>
#include "rescontainer_global.h"

class CW3TRDsurface;

class RESCONTAINER_EXPORT CW3ResourceContainer : public QObject {
	Q_OBJECT
public:
	CW3ResourceContainer();
	~CW3ResourceContainer();

	inline void setFacePhoto3D(CW3TRDsurface *photo) { m_pgFacePhoto3D = photo; }
	inline void setFaceMC3D(CW3TRDsurface *photo) { m_pgFaceMC3D = photo; }

	inline CW3TRDsurface* getFacePhoto3D() { return m_pgFacePhoto3D; }
	inline CW3TRDsurface* getFaceMC3D() { return m_pgFaceMC3D; }

	inline bool* getCollisionContainer() { return m_isCollided; }
	inline bool* getImplantThereContainer() { return m_isImplantThere; }

	void reset();

signals:
	void sigInitFacePhoto3D();

private:
	CW3TRDsurface	*m_pgFacePhoto3D = nullptr;
	CW3TRDsurface	*m_pgFaceMC3D = nullptr;

	bool m_isCollided[28];
	bool m_isImplantThere[28];
};
