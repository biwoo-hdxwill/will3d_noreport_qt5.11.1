#pragma once
/*=========================================================================

File:			class MarchingCube
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2016-01-13
Last date:		2016-01-13

=========================================================================*/
#include "../../Common/GLfunctions/WGLHeaders.h"
#include "../../Common/GLfunctions/W3GLTypes.h"

#include "MCTable.h"
#include "surfacing_global.h"

struct vertex_cube
{
	glm::vec3 vPos;
	float val = 0.0f;
};	// airway

class SURFACING_EXPORT MarchingCube
{
public: 
	MarchingCube();	// airway

	typedef float Ein; // internal data format

	// airway
	bool initializeAirway(glm::vec3 &vStart, glm::vec3 &vSize, float fIsoVal);
	void setCubeVerticesPos(const int& x, const int& y, const int& z);
	inline bool isSurfaceCube(void) const { return (m_nEdge != 0); }
	int calculateIndex(void);
	void polygonize(std::vector<tri_STL>& vMesh, unsigned int areaVal);

	inline glm::vec3 getVertexPos(const float& x, const float& y, const float& z) {
		glm::vec3 vPos = m_vStart + glm::vec3(m_vSize.x*x, m_vSize.y*y, m_vSize.z*z);
		return vPos;
	}

	inline glm::vec3 getCenterPos(const int& x, const int& y, const int& z) {
		return getVertexPos(x + 0.5f, y + 0.5f, z + 0.5f);
	}

	inline void	setVertexVal(const int& iVert, const float& fVal) { m_vertex[iVert].val = fVal; }
	inline float getVertexVal(const int& iVert) { return m_vertex[iVert].val; }

protected:
	struct FOV {
		int m_ngx = 0; // # of grid in x axis
		int m_ngy = 0; // # of grid in y axis
		int m_ngz = 0; // # of grid in z axis
		Ein m_x0 = 0.0f; // start coord of x axis
		Ein m_x1 = 0.0f; // end coord of x axis
		Ein m_y0 = 0.0f; // start coord of y axis
		Ein m_y1 = 0.0f; // end coord of y axis
		Ein m_z0 = 0.0f; // start coord of z axis
		Ein m_z1 = 0.0f; // end coord of z axis
		FOV();
		FOV(int ngx, int ngy, int ngz, Ein x0, Ein x1, Ein y0, Ein y1, Ein z0, Ein z1);

		inline Ein xip(int i) { return (m_x1 - m_x0) * i / m_ngx + m_x0; }
		inline Ein yip(int i) { return (m_y1 - m_y0) * i / m_ngy + m_y0; }
		inline Ein zip(int i) { return (m_z1 - m_z0) * i / m_ngz + m_z0; }
		inline int xpi(Ein x) { return (x - m_x0) / (m_x1 - m_x0) * m_ngx; }
		inline int ypi(Ein y) { return (y - m_y0) / (m_y1 - m_y0) * m_ngy; }
		inline int zpi(Ein z) { return (z - m_z0) / (m_z1 - m_z0) * m_ngz; }
	};

	class DownIdx {
	public:
		int m_nx;
		int m_ny;
		int m_nz;
		float m_down;
		DownIdx(int nx, int ny, int nz, float down);

		int operator()(int iz)
		{
			return int(iz * m_down);
		}

		int operator()(int ix, int iy)
		{
			return m_nx * int(iy * m_down) + int(ix * m_down);
		}

		int operator()(int ix, int iy, int iz)
		{
			return m_nx * m_ny * int(iz * m_down) + m_nx * int(iy * m_down) + int(ix * m_down);
		}
	};

	class Idx12 {
	public:
		int m_npx;
		int m_npy;
		int m_npz;
		float m_down;
		Idx12(int npx, int npy, int npz, float down);

		int operator()(int ix, int iy, int iz, int e12)
		{
			int ix2 = ix;
			int iy2 = iy;
			int iz2 = iz;
			int e3 = 0; // axis x: 0, y: 1, z: 2
			switch (e12) {
			case 0:
				ix2 += 0;		iy2 += 0;		iz2 += 0;		e3 = 0;
				break;
			case 1:
				ix2 += m_down;	iy2 += 0;		iz2 += 0;		e3 = 2;
				break;
			case 2:
				ix2 += 0;		iy2 += 0;		iz2 += m_down;	e3 = 0;
				break;
			case 3:
				ix2 += 0;		iy2 += 0;		iz2 += 0;		e3 = 2;
				break;
			case 4:
				ix2 += 0;		iy2 += m_down;	iz2 += 0;		e3 = 0;
				break;
			case 5:
				ix2 += m_down;	iy2 += m_down;	iz2 += 0;		e3 = 2;
				break;
			case 6:
				ix2 += 0;		iy2 += m_down;	iz2 += m_down;	e3 = 0;
				break;
			case 7:
				ix2 += 0;		iy2 += m_down;	iz2 += 0;		e3 = 2;
				break;
			case 8:
				ix2 += 0;		iy2 += 0;		iz2 += 0;		e3 = 1;
				break;
			case 9:
				ix2 += m_down;	iy2 += 0;		iz2 += 0;		e3 = 1;
				break;
			case 10:
				ix2 += m_down;	iy2 += 0;		iz2 += m_down;	e3 = 1;
				break;
			case 11:
				ix2 += 0;		iy2 += 0;		iz2 += m_down;	e3 = 1;
				break;
			}
			return unsigned long long(3) * unsigned long long(ix2 + m_npx * iy2 + m_npx * m_npy * iz2) + unsigned long long(e3);
		}
	};

	class Idx3 {
	public:
		int m_npx;
		int m_npy;
		int m_npz;
		Idx3(int npx, int npy, int npz);

		void operator()(int& ix, int& iy, int& iz, int& e3, unsigned long long a)
		{
			e3 = a % 3;
			a = a / 3;
			ix = a % m_npx;
			a = a / m_npx;
			iy = a % m_npy;
			a = a / m_npy;
			iz = a;
		}
	};

public:
	// final functions
	static glm::vec3 linearInterp(const glm::vec4& p1, const glm::vec4& p2, float val);

	static void execute(
		std::vector<glm::vec3>& pointsOut,			// output points
		std::vector<glm::u32vec3>& trianglesOut,	// output triangles
		float threshold,						// input marching cube min value
		unsigned short** data,							// uint16 3d volume data
		int nx, int ny, int nz,					// dimmension of 3d volume data
		int down,
		std::vector<glm::vec3>* normalsOut
	);

	static void execute(
		std::vector<glm::vec3>& pointsOut,
		std::vector<std::vector<int>>& trianglesOut,
		float threshold,
		unsigned short** data,
		int nx, int ny, int nz,
		float down,
		std::vector<glm::vec3>* normals
	);

	static void executeForAirway(
		std::vector<tri_STL> &meshSTL,
		float threshold,
		float** data,
		int nx, int ny, int nz,
		int down
	);
	//
	
	// marching cube
	//static void execute(
	//	std::vector<glm::vec3>& points,				// output points
	//	std::vector<std::vector<int>>& triangles,	// output triangles
	//	float minValue,								// input marching cube min value
	//	uint16** data,								// uint16 3d volume data
	//	int nx, int ny, int nz,						// dimmension of 3d volume data
	//	const std::function<glm::vec3(const glm::vec4&, const glm::vec4&, float)> intersection	// marching cube intersection function
	//	// use MarchingCube2::intersection
	//	);
	///*static */void execute(
	//	std::vector<glm::vec3>* points,				// output points
	//	std::vector<glm::u32vec3>* triangles,	// output triangles
	//	float threshold,								// input marching cube min value
	//	uint16** data,								// uint16 3d volume data
	//	int nx, int ny, int nz,						// dimmension of 3d volume data
	//	int down,
	//	std::vector<glm::vec3>* normals//,
	//	//const std::function<glm::vec3(const glm::vec4&, const glm::vec4&, float)> intersection	// marching cube intersection function
	//	//														= MarchingCube::linearInterp	// use MarchingCube2::intersection
	//	);

	/*inline void setVolume(CW3Image3D *vol)
	{
		m_pgVol = vol;
	}*/

	//void execute(
	//	std::vector<glm::vec3>* points,				// output points
	//	std::vector<glm::u32vec3>* triangles,	// output triangles
	//	float threshold,								// input marching cube min value
	//	int down,
	//	std::vector<glm::vec3>* normals
	//);

private:
	//CW3Image3D *m_pgVol;

	// airway
	int				m_nIndex;
	int				m_nEdge;
	float			m_fIsoVal;
	glm::vec3		m_vStart;
	glm::vec3		m_vSize;
	vertex_cube		m_vertex[8];
};
