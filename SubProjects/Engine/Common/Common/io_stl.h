#pragma once

/*=========================================================================

File:			class IO_STL
Language:		C++11
Library:              Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-01-29
Last modify:	2016-01-29

=========================================================================*/
#include "common_global.h"

#include "../../Common/Common/W3Types.h"

namespace stl {
struct Triangle {
	TPoint3D<float> normal;
	TPoint3D<float> v1;
	TPoint3D<float> v2;
	TPoint3D<float> v3;
	Triangle(TPoint3D<float> normalp, TPoint3D<float> v1p,
			 TPoint3D<float> v2p, TPoint3D<float> v3p) :
		normal(normalp), v1(v1p), v2(v2p), v3(v3p) {}
};

std::ostream& operator<<(std::ostream& out, const Triangle& t);

struct STLdata {
	std::string name;
	std::vector<Triangle> Triangles;

	STLdata(std::string namep) : name(namep) {}
	STLdata() {}
};
}

class COMMON_EXPORT IO_STL {
public:
	static void readBinarySTLFile(QString filename, S3DMeshData &sMeshData, bool bComputeOSCoord);
	static void readAsciiSTLFile(const char* filename, S3DMeshData &sMeshData);

	static void ReadBinarySTLfile(const QString& stl_path, stl::STLdata* dst_STLdata);
private:
};
