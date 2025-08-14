#include "active_block_resource.h"
/*=========================================================================

File:			class ActiveBlockResource
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-12-17
Last modify:	2015-12-17

=========================================================================*/
#if defined(__APPLE__)
#include </usr/local/Cellar/llvm/5.0.0/lib/clang/5.0.0/include/omp.h>
#else
#include <omp.h>
#endif
#include <ctime>

#include <QDebug>
#include <QElapsedTimer>

#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Resource/Resource/W3Image3D.h"

using common::Logger;
using common::LogType;

ActiveBlockResource::ActiveBlockResource(const CW3Image3D& vol, const int& block_size)
{
	nx_vol_ = vol.width();
	ny_vol_ = vol.height();
	nz_vol_ = vol.depth();
	m_blockSize = block_size;

	int nx = nx_vol_ / block_size;
	int ny = ny_vol_ / block_size;
	int nz = nz_vol_ / block_size;

	int width2 = nx * block_size;
	int height2 = ny * block_size;
	int depth2 = nz * block_size;

	int nxTotal, nyTotal, nzTotal;

	float stepCUBE_idx = float(block_size)*2.0f / (nx_vol_);
	float stepCUBE_idy = float(block_size)*2.0f / (ny_vol_);
	float stepCUBE_idz = float(block_size)*2.0f / (nz_vol_);

	float stepTEX_idx = float(block_size) / (nx_vol_);
	float stepTEX_idy = float(block_size) / (ny_vol_);
	float stepTEX_idz = float(block_size) / (nz_vol_);

	// Texture range: 0.5 ~ m_width-1+0.5
	float offsetTEX_idx = 0.0f;//0.5f / m_width;
	float offsetTEX_idy = 0.0f;// 0.5f / m_height;
	float offsetTEX_idz = 0.0f;// 0.5f / m_depth;

	bool isResX = false;
	bool isResY = false;
	bool isResZ = false;

	if (nx_vol_ > width2)
	{
		nxTotal = nx + 1;
		isResX = true;
	}
	else
	{
		nxTotal = nx;
		isResX = false;
	}

	if (ny_vol_ > height2)
	{
		nyTotal = ny + 1;
		isResY = true;
	}
	else
	{
		nyTotal = ny;
		isResY = false;
	}

	if (nz_vol_ > depth2)
	{
		nzTotal = nz + 1;
		isResZ = true;
	}
	else
	{
		nzTotal = nz;
		isResZ = false;
	}

	// This is for starting point
	nxTotal += 1;
	nyTotal += 1;
	nzTotal += 1;
	float* idxTEX = nullptr;
	float* idyTEX = nullptr;
	float* idzTEX = nullptr;

	m_width = nxTotal;
	m_height = nyTotal;
	m_depth = nzTotal;

	m_Nvertices = m_width * m_height*m_depth;

	float* idx = nullptr;
	float* idy = nullptr;
	float* idz = nullptr;
	W3::p_allocate_1D(&idx, m_width);
	W3::p_allocate_1D(&idy, m_height);
	W3::p_allocate_1D(&idz, m_depth);

	W3::p_allocate_1D(&idxTEX, m_width);
	W3::p_allocate_1D(&idyTEX, m_height);
	W3::p_allocate_1D(&idzTEX, m_depth);

	idx[0] = -1.0f;
	idy[0] = -1.0f;
	idz[0] = -1.0f;

	int num_thread = omp_get_max_threads();
	omp_set_num_threads(num_thread);

#pragma omp parallel for
	for (int i = 1; i < m_width - 1; i++)
		idx[i] = -1.0f + stepCUBE_idx * i;

	idx[m_width - 1] = 1.0f;

#pragma omp parallel for
	for (int i = 1; i < m_height - 1; i++)
		idy[i] = -1.0f + stepCUBE_idy * i;

	idy[m_height - 1] = 1.0f;

#pragma omp parallel for
	for (int i = 1; i < m_depth - 1; i++)
		idz[i] = -1.0f + stepCUBE_idz * i;

	idz[m_depth - 1] = 1.0f;

	// x 축만 뒤집힘
	idxTEX[0] = 1.0f - offsetTEX_idx;
	idyTEX[0] = offsetTEX_idy;
	idzTEX[0] = offsetTEX_idz;

#pragma omp parallel for
	for (int i = 1; i < m_width - 1; i++)
		idxTEX[i] = 1.0f - (offsetTEX_idx + stepTEX_idx * i);
	idxTEX[m_width - 1] = 1.0f - (1.0f - offsetTEX_idx);

#pragma omp parallel for
	for (int i = 1; i < m_height - 1; i++)
		idyTEX[i] = offsetTEX_idy + stepTEX_idy * i;
	idyTEX[m_height - 1] = 1.0f - offsetTEX_idy;

#pragma omp parallel for
	for (int i = 1; i < m_depth - 1; i++)
		idzTEX[i] = offsetTEX_idz + stepTEX_idz * i;
	idzTEX[m_depth - 1] = 1.0f - offsetTEX_idz;

	W3::p_allocate_1D(&m_verticesCoord, m_Nvertices * 3);
	W3::p_allocate_1D(&m_texCoord, m_Nvertices * 3);

#pragma omp parallel for
	for (int i = 0; i < m_depth; i++)
	{
		for (int j = 0; j < m_height; j++)
		{
			for (int k = 0; k < m_width; k++)
			{
				int id = (i*(m_width*m_height) + j * (m_width)+k) * 3;
				m_verticesCoord[id] = idx[k];
				m_verticesCoord[id + 1] = idy[j];
				m_verticesCoord[id + 2] = idz[i];

				m_texCoord[id] = idxTEX[k];
				m_texCoord[id + 1] = idyTEX[j];
				m_texCoord[id + 2] = idzTEX[i];
			}
		}
	}

	SAFE_DELETE_ARRAY(idx);
	SAFE_DELETE_ARRAY(idy);
	SAFE_DELETE_ARRAY(idz);

	SAFE_DELETE_ARRAY(idxTEX);
	SAFE_DELETE_ARRAY(idyTEX);
	SAFE_DELETE_ARRAY(idzTEX);

	unsigned int nBlock = (m_width - 1)*(m_height - 1)*(m_depth - 1);
	W3::p_allocate_1D(&m_minmax, nBlock);
	W3::p_allocate_1D(&m_ActiveIndex, nBlock * 6 * 6);

	InitMinMax(vol);
}

ActiveBlockResource::~ActiveBlockResource(void)
{
	SAFE_DELETE_ARRAY(m_verticesCoord);
	SAFE_DELETE_ARRAY(m_texCoord);
	SAFE_DELETE_ARRAY(m_minmax);
	SAFE_DELETE_ARRAY(m_ActiveIndex);
}

void ActiveBlockResource::InitMinMax(const CW3Image3D& vol)
{
	if (!(&vol))
		return;

	common::Logger::instance()->Print(common::LogType::INF, "start InitMinMax");

	QElapsedTimer timer;
	timer.start();

	int bsx, bsy, bsz;
	int bstart_x, bstart_y, bstart_z;

	const int wm1 = m_width - 1;
	const int wm1_hm1 = (m_height - 1) * wm1;
	const int w_h = m_height * m_width;

#if 1
	int num_thread = omp_get_max_threads();
	omp_set_num_threads(num_thread);
#endif

#if 0
#pragma omp parallel for
	for (int i = 0; i < (m_width - 1)*(m_height - 1)*(m_depth - 1); ++i)
	{
		m_minmax[i].max = 0;
		m_minmax[i].min = 65535;
	}

	int block_z_count = nz_vol_ / m_blockSize;
	int block_y_count = ny_vol_ / m_blockSize;
	int block_x_count = nx_vol_ / m_blockSize;

#pragma omp parallel for
	for (int z = 0; z < nz_vol_; ++z)
	{
		int block_z_index = z / m_blockSize;
		block_z_index = block_z_index * block_y_count * block_x_count;
		for (int y = 0; y < ny_vol_; ++y)
		{
			int block_y_index = y / m_blockSize;
			block_y_index = block_y_index * block_x_count;
			for (int x = 0; x < nx_vol_; ++x)
			{
				int block_x_index = x / m_blockSize;
				int block_index = block_z_index + block_y_index + block_x_index;

				int val = 0;
				//int xy_index = (y * nx_vol_) + x;
				int xy_index = (y * nx_vol_) + (nx_vol_ - x - 1);

				val = vol.getData()[z][xy_index];

				if (val > m_minmax[block_index].max)
				{
					m_minmax[block_index].max = val;
				}
				if (val < m_minmax[block_index].min)
				{
					m_minmax[block_index].min = val;
				}
			}
		}
	}

	qDebug() << "InitMinMax 1 :" << timer.elapsed();
#endif
#if 1
	timer.restart();

#pragma omp parallel for
	for (int i = 0; i < m_depth - 1; ++i)
	{
		const int i_wm1_hm1 = i * wm1_hm1;

		for (int j = 0; j < m_height - 1; ++j)
		{
			const int j_wm1 = j * wm1;
			const int i_wm1_hm1_p_j_wm1 = i_wm1_hm1 + j_wm1;

			for (int k = 0; k < m_width - 1; ++k)
			{
				const int id = i_wm1_hm1_p_j_wm1 + k;

				int val = 0;
				int init_max = 0;
				int init_min = 65535;

				for (int ii = 0; ii < m_blockSize; ++ii)
				{
					int z_index = i * m_blockSize + ii;
					if (z_index >= nz_vol_)
					{
						break;
					}
					for (int jj = 0; jj < m_blockSize; ++jj)
					{
						int y_index = j * m_blockSize + jj;
						if (y_index >= ny_vol_)
						{
							break;
						}
						for (int kk = 0; kk < m_blockSize; ++kk)
						{
							int x_index = k * m_blockSize + kk;
							if (x_index >= nx_vol_)
							{
								break;
							}

							int xy_index = (y_index * nx_vol_) + (nx_vol_ - (x_index) - 1);

							val = vol.getData()[z_index][xy_index];

							if (val > init_max)
							{
								init_max = val;
							}
							if (val < init_min)
							{
								init_min = val;
							}
						}
					}
				}

				m_minmax[id].min = init_min;
				m_minmax[id].max = init_max;
			}
		}
	}

	//qDebug() << "InitMinMax 2 :" << timer.elapsed();
#endif
#if 0
	timer.restart();
#if 0
	int bsx, bsy, bsz;
	int bstart_x, bstart_y, bstart_z;

	const int wm1 = m_width - 1;
	const int wm1_hm1 = (m_height - 1) * wm1;
	const int w_h = m_height * m_width;
#endif
	for (int i = 0; i < m_depth - 1; ++i)
	{
		const int i_wm1_hm1 = i * wm1_hm1;

		if (i == m_depth - 2)
		{
			bsz = nz_vol_ - m_blockSize * i;
		}
		else
		{
			bsz = m_blockSize + 1;
		}

		bstart_z = (i == 0) ? 0 : -1;

		for (int j = 0; j < m_height - 1; ++j)
		{
			const int j_wm1 = j * wm1;
			const int i_wm1_hm1_p_j_wm1 = i_wm1_hm1 + j_wm1;

			if (j == m_height - 2)
			{
				bsy = ny_vol_ - m_blockSize * j;
			}
			else
			{
				bsy = m_blockSize + 1;
			}

			bstart_y = (j == 0) ? 0 : -1;

			for (int k = 0; k < m_width - 1; ++k)
			{
				const int id = i_wm1_hm1_p_j_wm1 + k;

				if (k == m_width - 2)
				{
					bsx = nx_vol_ - m_blockSize * k;
				}
				else
				{
					bsx = m_blockSize + 1;
				}

				bstart_x = (k == 0) ? 0 : -1;

				int val = 0;
				int init_max = 0;
				int init_min = 65535;

				for (int ii = bstart_z; ii < bsz; ++ii)
				{
					int z_index = i * m_blockSize + ii;
					if (z_index >= nz_vol_)
					{
						break;
					}
					for (int jj = bstart_y; jj < bsy; ++jj)
					{
						int y_index = j * m_blockSize + jj;
						if (y_index >= ny_vol_)
						{
							break;
						}
						for (int kk = bstart_x; kk < bsx; ++kk)
						{
							int x_index = k * m_blockSize + kk;
							if (x_index >= nx_vol_)
							{
								break;
							}

							int xy_index = (y_index * nx_vol_) + (nx_vol_ - (x_index)-1);

							val = vol.getData()[z_index][xy_index];

							if (val > init_max)
								init_max = val;
							if (val < init_min)
								init_min = val;
						}
					}
				}

				m_minmax[id].min = init_min;
				m_minmax[id].max = init_max;
			}
		}
	}
	qDebug() << "InitMinMax 3 :" << timer.elapsed();
#endif
	common::Logger::instance()->Print(common::LogType::INF, "end InitMinMax : " + QString::number(timer.elapsed()).toStdString() + " ms");
}

void ActiveBlockResource::setActiveBlock(int minValue, int maxValue)
{
	// front:	1 5 7 3
	// back	:	0 2 6 4
	// left	:	0 1 3 2
	// right:	7 5 4 6
	// up	:	2 3 7 6
	// down	:	1 0 4 5

	clock_t start = clock();

	if (minValue >= maxValue)
	{
		printf("ERROR: TF min max is not correct!\n");
	}
	m_nActiveBlock = 0;

	int bsx, bsy, bsz;
	int bstart_x, bstart_y, bstart_z;

	const int wm1 = m_width - 1;
	const int wm1_hm1 = (m_height - 1) * wm1;
	const int w_h = m_height * m_width;
	const int numCubeFace = 36;
	for (int i = 0; i < m_depth - 1; i++)
	{
		const int i_wm1_hm1 = i * wm1_hm1;
		const int i_w_h = i * w_h;
		const int ip1_w_h = (i + 1)*w_h;

		for (int j = 0; j < m_height - 1; j++)
		{
			const int j_wm1 = j * wm1;
			const int i_wm1_hm1_p_j_wm1 = i_wm1_hm1 + j_wm1;
			const int jp1_w = (j + 1)*m_width;
			const int j_w = j * m_width;

			for (int k = 0; k < m_width - 1; k++)
			{
				const int id = i_wm1_hm1_p_j_wm1 + k;

				if (m_minmax[id].min <= maxValue && m_minmax[id].max >= minValue)
				{
					const int _0 = i_w_h + jp1_w + k + 1;
					const int _1 = ip1_w_h + jp1_w + k + 1;
					const int _2 = i_w_h + j_w + k + 1;
					const int _3 = ip1_w_h + j_w + k + 1;
					const int _4 = i_w_h + jp1_w + k;
					const int _5 = ip1_w_h + jp1_w + k;
					const int _6 = i_w_h + j_w + k;
					const int _7 = ip1_w_h + j_w + k;

					unsigned int* p_active_index = &m_ActiveIndex[numCubeFace * m_nActiveBlock];
					// front
					*(p_active_index++) = _1;
					*(p_active_index++) = _5;
					*(p_active_index++) = _3;
					*(p_active_index++) = _3;
					*(p_active_index++) = _5;
					*(p_active_index++) = _7;

					// back
					*(p_active_index++) = _0;
					*(p_active_index++) = _2;
					*(p_active_index++) = _4;
					*(p_active_index++) = _4;
					*(p_active_index++) = _2;
					*(p_active_index++) = _6;

					// left
					*(p_active_index++) = _0;
					*(p_active_index++) = _1;
					*(p_active_index++) = _2;
					*(p_active_index++) = _2;
					*(p_active_index++) = _1;
					*(p_active_index++) = _3;

					// right
					*(p_active_index++) = _7;
					*(p_active_index++) = _5;
					*(p_active_index++) = _6;
					*(p_active_index++) = _6;
					*(p_active_index++) = _5;
					*(p_active_index++) = _4;

					// up
					*(p_active_index++) = _2;
					*(p_active_index++) = _3;
					*(p_active_index++) = _6;
					*(p_active_index++) = _6;
					*(p_active_index++) = _3;
					*(p_active_index++) = _7;

					// down
					*(p_active_index++) = _1;
					*(p_active_index++) = _0;
					*(p_active_index++) = _5;
					*(p_active_index++) = _5;
					*(p_active_index++) = _0;
					*(p_active_index++) = _4;

					++m_nActiveBlock;
				}
			}
		}
	}

	clock_t end = clock();
	float elapsed_time = static_cast<float>(end - start) / CLOCKS_PER_SEC;
	auto logger = Logger::instance();
	logger->Print(LogType::DBG, "setActiveBlock : " + std::to_string(elapsed_time));
}
