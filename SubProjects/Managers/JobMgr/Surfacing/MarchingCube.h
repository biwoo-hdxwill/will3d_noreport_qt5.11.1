#pragma once
/*=========================================================================

File:			class MarchingCube
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2016-01-13
Last date:		2016-01-13

=========================================================================*/

#include "../jobmgr_global.h"

#include "MCTable.h"
#include "../../../Engine/Common/GLfunctions/WGLHeaders.h"
#include "../../../Engine/Common/Common/W3Mesh.h"

class CW3Image3D;

struct vertex_cube
{
	glm::vec3 vPos;
	W3FLOAT val;
};	// airway

class JOBMGR_EXPORT MarchingCube
{
public: 
	MarchingCube();	// airway

	typedef float Ein; // internal data format

	// airway
	W3BOOL initializeAirway(glm::vec3 &vStart, glm::vec3 &vSize, W3FLOAT fIsoVal);
	void setCubeVerticesPos(const W3INT& x, const W3INT& y, const W3INT& z);
	inline W3BOOL isSurfaceCube(void) const { return (m_nEdge != 0); }
	int calculateIndex(void);
	void polygonize(std::vector<tri_STL>& vMesh, W3UINT areaVal);

	inline glm::vec3 getVertexPos(const W3FLOAT& x, const W3FLOAT& y, const W3FLOAT& z) {
		glm::vec3 vPos = m_vStart + glm::vec3(m_vSize.x*x, m_vSize.y*y, m_vSize.z*z);
		return vPos;
	}

	inline glm::vec3 getCenterPos(const W3INT& x, const W3INT& y, const W3INT& z) {
		return getVertexPos(x + 0.5f, y + 0.5f, z + 0.5f);
	}

	inline void	setVertexVal(const W3INT& iVert, const W3FLOAT& fVal) { m_vertex[iVert].val = fVal; }
	inline W3FLOAT getVertexVal(const W3INT& iVert) { return m_vertex[iVert].val; }
	//

protected:
	struct FOV {
		W3INT m_ngx; // # of grid in x axis
		W3INT m_ngy; // # of grid in y axis
		W3INT m_ngz; // # of grid in z axis
		Ein m_x0; // start coord of x axis
		Ein m_x1; // end coord of x axis
		Ein m_y0; // start coord of y axis
		Ein m_y1; // end coord of y axis
		Ein m_z0; // start coord of z axis
		Ein m_z1; // end coord of z axis
		FOV();
		FOV(W3INT ngx, W3INT ngy, W3INT ngz, Ein x0, Ein x1, Ein y0, Ein y1, Ein z0, Ein z1);

		inline Ein xip(W3INT i)
		{
			return (m_x1 - m_x0) * i / m_ngx + m_x0;
		}
		inline Ein yip(W3INT i)
		{
			return (m_y1 - m_y0) * i / m_ngy + m_y0;
		}
		inline Ein zip(W3INT i)
		{
			return (m_z1 - m_z0) * i / m_ngz + m_z0;
		}
		inline W3INT xpi(Ein x)
		{
			return (x - m_x0) / (m_x1 - m_x0) * m_ngx;
		}
		inline W3INT ypi(Ein y)
		{
			return (y - m_y0) / (m_y1 - m_y0) * m_ngy;
		}
		inline W3INT zpi(Ein z)
		{
			return (z - m_z0) / (m_z1 - m_z0) * m_ngz;
		}
	};

	class DownIdx {
	public:
		W3INT m_nx;
		W3INT m_ny;
		W3INT m_nz;
		W3FLOAT m_down;
		DownIdx(W3INT nx, W3INT ny, W3INT nz, W3FLOAT down);

		W3INT operator()(W3INT iz)
		{
			return W3INT(iz * m_down);
		}

		W3INT operator()(W3INT ix, W3INT iy)
		{
			return m_nx * W3INT(iy * m_down) + W3INT(ix * m_down);
		}

		W3INT operator()(W3INT ix, W3INT iy, W3INT iz)
		{
			return m_nx * m_ny * W3INT(iz * m_down) + m_nx * W3INT(iy * m_down) + W3INT(ix * m_down);
		}

	};

	class Idx12 {
	public:
		W3INT m_npx;
		W3INT m_npy;
		W3INT m_npz;
		W3FLOAT m_down;
		Idx12(W3INT npx, W3INT npy, W3INT npz, W3FLOAT down);

		W3INT operator()(W3INT ix, W3INT iy, W3INT iz, W3INT e12)
		{
			W3INT ix2 = ix;
			W3INT iy2 = iy;
			W3INT iz2 = iz;
			W3INT e3 = 0; // axis x: 0, y: 1, z: 2
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
			return W3ULONGLONG(3) * W3ULONGLONG(ix2 + m_npx * iy2 + m_npx * m_npy * iz2) + W3ULONGLONG(e3);
		}
	};

	class Idx3 {
	public:
		W3INT m_npx;
		W3INT m_npy;
		W3INT m_npz;
		Idx3(W3INT npx, W3INT npy, W3INT npz);

		void operator()(W3INT& ix, W3INT& iy, W3INT& iz, W3INT& e3, W3ULONGLONG a)
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
	static glm::vec3 linearInterp(const glm::vec4& p1, const glm::vec4& p2, W3FLOAT val);

	static void execute(
		std::vector<glm::vec3>& pointsOut,			// output points
		std::vector<glm::u32vec3>& trianglesOut,	// output triangles
		W3FLOAT threshold,						// input marching cube min value
		W3USHORT** data,							// uint16 3d volume data
		W3INT nx, W3INT ny, W3INT nz,					// dimmension of 3d volume data
		W3INT down,
		std::vector<glm::vec3>* normalsOut
	);

	static void execute(
		std::vector<glm::vec3>& pointsOut,
		std::vector<std::vector<W3INT>>& trianglesOut,
		W3FLOAT threshold,
		W3USHORT** data,
		W3INT nx, W3INT ny, W3INT nz,
		W3FLOAT down,
		std::vector<glm::vec3>* normals
	);

	static void executeForAirway(
		std::vector<tri_STL> &meshSTL,
		W3FLOAT threshold,
		W3FLOAT** data,
		W3INT nx, W3INT ny, W3INT nz,
		W3INT down
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
	W3INT				m_nIndex;
	W3INT				m_nEdge;
	W3FLOAT			m_fIsoVal;
	glm::vec3		m_vStart;
	glm::vec3		m_vSize;
	vertex_cube		m_vertex[8];
	//
};
