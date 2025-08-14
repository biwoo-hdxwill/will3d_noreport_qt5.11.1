#pragma once

/*=========================================================================

File:		Type-name Redefinition.
Language:	C++11
Library:	Standard C++ Library
Author:		Hong Jung
First Date:	2015-12-02
Last Date:	2015-12-02

=========================================================================*/
#include <QList>
#include <QVector3D>

template<typename T>
class TPoint3D {
public:
	T x;
	T y;
	T z;

	TPoint3D() {}
	TPoint3D(T xp, T yp, T zp) : x(xp), y(yp), z(zp) {}
};

typedef struct T_S3DMeshData { //skpark 20151015 STL
	QList<QVector3D> listNormals;
	QList<QVector3D> listVertices;
	QVector3D vecBoundMin;
	QVector3D vecBoundMax;
} S3DMeshData;

//thyoo: range (0 ~ 100)
typedef struct _ADJUST_OTF {
	float bright = 0.0f;
	float opacity = 0.0f;
	float contrast = 0.0f;
} AdjustOTF;

typedef struct _SLICELOC {
	int maxilla;
	int teeth;
	int chin;
	int nose;
	bool segment_maxilla_mandible = false;

	_SLICELOC() {
		maxilla = teeth = chin = nose = 0;
	}
} SliceLoc;
