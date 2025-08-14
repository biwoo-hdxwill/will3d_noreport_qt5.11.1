#include "io_stl.h"

#include <exception>
#include <iostream>
#include <fstream>
#include <QFile>
#include <qdatastream.h>

using std::exception;
using std::cout;
using std::endl;
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <streambuf>

namespace stl {
std::ostream& operator<<(std::ostream& out, const TPoint3D<float>& p) {
	out << "(" << p.x << ", " << p.y << ", " << p.z << ")" << std::endl;
	return out;
}

std::ostream& operator<<(std::ostream& out, const Triangle& t) {
	out << "---- TRIANGLE ----" << std::endl;
	out << t.normal << std::endl;
	out << t.v1 << std::endl;
	out << t.v2 << std::endl;
	out << t.v3 << std::endl;
	return out;
}

float parse_float(std::ifstream& s) {
	char f_buf[sizeof(float)];
	s.read(f_buf, 4);
	float* fptr = (float*)f_buf;
	return *fptr;
}

const TPoint3D<float> parse_point(std::ifstream& s) {
	float x = parse_float(s);
	float y = parse_float(s);
	float z = parse_float(s);
	return TPoint3D<float>(x, y, z);
}
}

using namespace stl;

void IO_STL::readBinarySTLFile(QString filename, S3DMeshData &sMeshData, bool bComputeOSCoord) {
	try {
		sMeshData.listNormals.clear();
		sMeshData.listVertices.clear();

		QFile fileSTL(filename);

		if (!fileSTL.open(QIODevice::ReadOnly))
			throw std::runtime_error("reading from STL file has failed.");

		fileSTL.seek(0);

		QDataStream datastream(&fileSTL);

		datastream.setByteOrder(QDataStream::LittleEndian);
		datastream.setFloatingPointPrecision(QDataStream::SinglePrecision);

		quint32 nTriangles;
		quint16 nControlBytes;

		fileSTL.seek(80);
		datastream >> nTriangles;

		//QByteArray barray = fileSTL.read(4);
		//memcpy(&nTriangles, barray.constData(), 4);

		quint32 nCountTriangles = 0;

		//double dLimitOut = 2000;
		sMeshData.vecBoundMin = QVector3D(10000, 10000, 10000);
		sMeshData.vecBoundMax = QVector3D(-10000, -10000, -10000);

		while (nCountTriangles < nTriangles) {
			float nx, ny, nz, x[3], y[3], z[3];
			fileSTL.seek(84 + nCountTriangles * 50 + 0 + 0);
			datastream >> nx;
			fileSTL.seek(84 + nCountTriangles * 50 + 0 + 4);
			datastream >> ny;
			fileSTL.seek(84 + nCountTriangles * 50 + 0 + 8);
			datastream >> nz;
			fileSTL.seek(84 + nCountTriangles * 50 + 12 + 0);
			datastream >> x[0];
			fileSTL.seek(84 + nCountTriangles * 50 + 12 + 4);
			datastream >> y[0];
			fileSTL.seek(84 + nCountTriangles * 50 + 12 + 8);
			datastream >> z[0];
			fileSTL.seek(84 + nCountTriangles * 50 + 24 + 0);
			datastream >> x[1];
			fileSTL.seek(84 + nCountTriangles * 50 + 24 + 4);
			datastream >> y[1];
			fileSTL.seek(84 + nCountTriangles * 50 + 24 + 8);
			datastream >> z[1];
			fileSTL.seek(84 + nCountTriangles * 50 + 36 + 0);
			datastream >> x[2];
			fileSTL.seek(84 + nCountTriangles * 50 + 36 + 4);
			datastream >> y[2];
			fileSTL.seek(84 + nCountTriangles * 50 + 36 + 8);
			datastream >> z[2];
			fileSTL.seek(84 + nCountTriangles * 50 + 48);
			datastream >> nControlBytes;
			//if (datastream.status() != QDataStream::Ok) {}
			// update bounding info.
			for (int i = 0; i < 3; i++) {
				if (x[i] < sMeshData.vecBoundMin.x())
					sMeshData.vecBoundMin.setX(x[i]);
				if (y[i] < sMeshData.vecBoundMin.y())
					sMeshData.vecBoundMin.setY(y[i]);
				if (z[i] < sMeshData.vecBoundMin.z())
					sMeshData.vecBoundMin.setZ(z[i]);

				if (x[i] > sMeshData.vecBoundMax.x())
					sMeshData.vecBoundMax.setX(x[i]);
				if (y[i] > sMeshData.vecBoundMax.y())
					sMeshData.vecBoundMax.setY(y[i]);
				if (z[i] > sMeshData.vecBoundMax.z())
					sMeshData.vecBoundMax.setZ(z[i]);
			}
			// add new data
			sMeshData.listNormals.append(QVector3D(nx, ny, nz));
			sMeshData.listVertices.append(QVector3D(x[0], y[0], z[0]));
			sMeshData.listVertices.append(QVector3D(x[1], y[1], z[1]));
			sMeshData.listVertices.append(QVector3D(x[2], y[2], z[2]));

			++nCountTriangles;
		}

		//if (bComputeOSCoord)
			//UpdateImplantMeshOSCoord(sMeshData); //우선 주석처리함 . 추후에 다시 확인해야함.

		fileSTL.close();
	} catch (std::runtime_error& e) {
		cout << "IO_STL: " << e.what() << endl;
	}
}

void IO_STL::readAsciiSTLFile(const char* filename, S3DMeshData &sMeshData) {
	using namespace	std;
	try {
		ifstream fileSTL(filename);

		if (!fileSTL.is_open())
			throw std::runtime_error("reading from STL file has failed.");

		string buffer, comment;
		int nVertexCounter = 0;

		sMeshData.vecBoundMin = QVector3D(10000, 10000, 10000);
		sMeshData.vecBoundMax = QVector3D(-10000, -10000, -10000);
		while (fileSTL >> buffer) {
			if (buffer == "solid") {
				std::getline(fileSTL, comment, '\n');
			} else if (buffer == "facet") {
				fileSTL >> comment; // "normal"

				float fNormalX, fNormalY, fNormalZ;

				fileSTL >> fNormalX >> fNormalY >> fNormalZ;

				sMeshData.listNormals.append(QVector3D(fNormalX, fNormalY,
													   fNormalZ));
			} else if (buffer == "outer") {
				fileSTL >> comment; // "loop"
			} else if (buffer == "vertex") {
				float fX, fY, fZ;

				fileSTL >> fX >> fY >> fZ;

				sMeshData.listVertices.append(QVector3D(fX, fY, fZ));

				//std::cout << fX << ", " << fY << ", " << fZ << std::endl;
				if (fX < sMeshData.vecBoundMin.x())
					sMeshData.vecBoundMin.setX(fX);
				if (fY < sMeshData.vecBoundMin.y())
					sMeshData.vecBoundMin.setY(fY);
				if (fZ < sMeshData.vecBoundMin.z())
					sMeshData.vecBoundMin.setZ(fZ);

				if (fX > sMeshData.vecBoundMax.x())
					sMeshData.vecBoundMax.setX(fX);
				if (fY > sMeshData.vecBoundMax.y())
					sMeshData.vecBoundMax.setY(fY);
				if (fZ > sMeshData.vecBoundMax.z())
					sMeshData.vecBoundMax.setZ(fZ);

				++nVertexCounter;
			} else if (buffer == "endloop") {
				nVertexCounter = 0;
			} else if (buffer == "endfacet") {
			} else if (buffer == "endsolid") {
				std::getline(fileSTL, comment, '\n');
			}
		}

		fileSTL.close();
	} catch (std::runtime_error& e) {
		cout << "IO_STL: " << e.what() << endl;
	}
}

void IO_STL::ReadBinarySTLfile(const QString & stl_path, stl::STLdata* dst_stl_data) {
	std::ifstream stl_file(stl_path.toStdString().c_str(), std::ios::in | std::ios::binary);
	if (!stl_file) {
		std::cout << "ERROR: COULD NOT READ FILE" << std::endl;
		assert(false);
	}

	char header_info[80] = "";
	char n_triangles[4];
	stl_file.read(header_info, 80);
	stl_file.read(n_triangles, 4);
	std::string h(header_info);

	dst_stl_data = new STLdata(h);
	unsigned int* r = (unsigned int*)n_triangles;
	unsigned int num_triangles = *r;
	for (unsigned int i = 0; i < num_triangles; i++) {
		auto normal = parse_point(stl_file);
		auto v1 = parse_point(stl_file);
		auto v2 = parse_point(stl_file);
		auto v3 = parse_point(stl_file);
		dst_stl_data->Triangles.push_back(Triangle(normal, v1, v2, v3));
		char dummy[2];
		stl_file.read(dummy, 2);
	}
}
