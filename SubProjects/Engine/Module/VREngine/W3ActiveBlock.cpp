#include "W3ActiveBlock.h"
/*=========================================================================

File:			class CW3ActiveBlock
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

//#include <QDebug>

#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Resource/Resource/W3Image3D.h"

using common::Logger;
using common::LogType;

CW3ActiveBlock::CW3ActiveBlock(
	int nx, int ny, int nz,
	int blockSize, float stepX, float stepY, float stepZ,
	float stepXtex, float stepYtex, float stepZtex,
	float offsetXtex, float offsetYtex, float offsetZtex,
	int nxVol, int nyVol, int nzVol, CW3Image3D *vol)
	: m_width(nx), m_height(ny), m_depth(nz), m_blockSize(blockSize), volume_(vol) {
	float* idxTEX = nullptr;
	float* idyTEX = nullptr;
	float* idzTEX = nullptr;

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

	for (int i = 1; i < nx - 1; i++)
		idx[i] = -1.0f + stepX * i;

	idx[nx - 1] = 1.0f;

	for (int i = 1; i < ny - 1; i++)
		idy[i] = -1.0f + stepY * i;

	idy[ny - 1] = 1.0f;

	for (int i = 1; i < nz - 1; i++)
		idz[i] = -1.0f + stepZ * i;

	idz[nz - 1] = 1.0f;

	// x 축만 뒤집힘
	idxTEX[0] = 1.0f - offsetXtex;
	idyTEX[0] = offsetYtex;
	idzTEX[0] = offsetZtex;

	for (int i = 1; i < nx - 1; i++)
		idxTEX[i] = 1.0f - (offsetXtex + stepXtex * i);
	idxTEX[nx - 1] = 1.0f - (1.0f - offsetXtex);

	for (int i = 1; i < ny - 1; i++)
		idyTEX[i] = offsetYtex + stepYtex * i;
	idyTEX[ny - 1] = 1.0f - offsetYtex;

	for (int i = 1; i < nz - 1; i++)
		idzTEX[i] = offsetZtex + stepZtex * i;
	idzTEX[nz - 1] = 1.0f - offsetZtex;

	W3::p_allocate_1D(&m_verticesCoord, m_Nvertices * 3);
	W3::p_allocate_1D(&m_texCoord, m_Nvertices * 3);

	for (int i = 0; i < nz; i++) {
		for (int j = 0; j < ny; j++) {
			for (int k = 0; k < nx; k++) {
				int id = (i*(nx*ny) + j * (nx)+k) * 3;
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

	//float *BlockGrad = nullptr;
	//W3::p_allocate_1D(&BlockGrad, nBlock);

#if 1
	InitMinMax();
#else
	int bsx, bsy, bsz;
	int bstart_x, bstart_y, bstart_z;
	int tmpMax, tmpMin, val;

	for (int i = 0; i < m_depth - 1; i++) {
		if (i == m_depth - 2) {
			bsz = nzVol - m_blockSize * i;
		} else {
			bsz = m_blockSize + 1;
		}

		bstart_z = (i == 0) ? 0 : -1;

		for (int j = 0; j < m_height - 1; j++) {
			if (j == m_height - 2) {
				bsy = nyVol - m_blockSize * j;
			} else {
				bsy = m_blockSize + 1;
			}

			bstart_y = (j == 0) ? 0 : -1;

			for (int k = 0; k < m_width - 1; k++) {
				if (k == m_width - 2) {
					bsx = nxVol - m_blockSize * k;
				} else {
					bsx = m_blockSize + 1;
				}

				bstart_x = (k == 0) ? 0 : -1;

				tmpMax = -65535;
				tmpMin = 65535;

				for (int ii = bstart_z; ii < bsz; ii++) {
					for (int jj = bstart_y; jj < bsy; jj++) {
						for (int kk = bstart_x; kk < bsx; kk++) {
							val = vol->getData()[(i*m_blockSize + ii)][(j*m_blockSize + jj)*nxVol + (nxVol - (k*m_blockSize + kk) - 1)];
							if (val > tmpMax) {
								tmpMax = val;
							}
							if (val < tmpMin) {
								tmpMin = val;
							}
						}
					}
				}

				//m_minmax[i*(m_width-1)*(m_height-1) + j*(m_width-1) + m_width - 2 - k].min = tmpMin;
				//m_minmax[i*(m_width-1)*(m_height-1) + j*(m_width-1) + m_width - 2 - k].max = tmpMax;

				m_minmax[i*(m_width - 1)*(m_height - 1) + j * (m_width - 1) + k].min = tmpMin;
				m_minmax[i*(m_width - 1)*(m_height - 1) + j * (m_width - 1) + k].max = tmpMax;
			}
		}
	}
#endif
}

CW3ActiveBlock::~CW3ActiveBlock(void) {
	SAFE_DELETE_ARRAY(m_verticesCoord);
	SAFE_DELETE_ARRAY(m_texCoord);
	SAFE_DELETE_ARRAY(m_minmax);
	SAFE_DELETE_ARRAY(m_ActiveIndex);
}

void CW3ActiveBlock::InitMinMax() {
	if (!volume_)
		return;

	clock_t start = clock();

	int bsx, bsy, bsz;
	int bstart_x, bstart_y, bstart_z;
	int init_max, init_min, val;
	int nz_vol = volume_->depth();
	int ny_vol = volume_->height();
	int nx_vol = volume_->width();

	const int wm1 = m_width - 1;
	const int wm1_hm1 = (m_height - 1) * wm1;
	const int w_h = m_height * m_width;
	for (int i = 0; i < m_depth - 1; i++) {
		const int i_wm1_hm1 = i * wm1_hm1;

		if (i == m_depth - 2) {
			bsz = nz_vol - m_blockSize * i;
		} else {
			bsz = m_blockSize + 1;
		}

		bstart_z = (i == 0) ? 0 : -1;

		for (int j = 0; j < m_height - 1; j++) {
			const int j_wm1 = j * wm1;
			const int i_wm1_hm1_p_j_wm1 = i_wm1_hm1 + j_wm1;

			if (j == m_height - 2) {
				bsy = ny_vol - m_blockSize * j;
			} else {
				bsy = m_blockSize + 1;
			}

			bstart_y = (j == 0) ? 0 : -1;

			for (int k = 0; k < m_width - 1; k++) {
				const int id = i_wm1_hm1_p_j_wm1 + k;

				if (k == m_width - 2) {
					bsx = nx_vol - m_blockSize * k;
				} else {
					bsx = m_blockSize + 1;
				}

				bstart_x = (k == 0) ? 0 : -1;

				init_max = -65535;
				init_min = 65535;

				for (int ii = bstart_z; ii < bsz; ii++) {
					int z_index = i * m_blockSize + ii;
					if (z_index >= nz_vol)
						break;
					for (int jj = bstart_y; jj < bsy; jj++) {
						int y_index = j * m_blockSize + jj;
						if (y_index >= ny_vol)
							break;
						for (int kk = bstart_x; kk < bsx; kk++) {
							int x_index = k * m_blockSize + kk;
							if (x_index >= nx_vol)
								break;
							int xy_index = (j * m_blockSize + jj) * nx_vol + (nx_vol - (k * m_blockSize + kk) - 1);

							val = volume_->getData()[z_index][xy_index];

#if 0
							if (vr_cut_mask_) {
								unsigned short mask_val = vr_cut_mask_[z_index][xy_index];
								if (mask_val & 0x0001 << cur_vr_cut_history_step_)
									continue;
							}
#endif

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
	
	clock_t end = clock();
	float elapsed_time = static_cast<float>(end - start) / CLOCKS_PER_SEC;
	auto logger = Logger::instance();
	logger->Print(LogType::DBG, "InitMinMax : " + std::to_string(elapsed_time));

	//qDebug() << "InitMinMax :" << elapsed_time;
}

void CW3ActiveBlock::setActiveBlock(int minValue, int maxValue) {
	// front:	1 5 7 3
	// back	:	0 2 6 4
	// left	:	0 1 3 2
	// right:	7 5 4 6
	// up	:	2 3 7 6
	// down	:	1 0 4 5

	clock_t start = clock();

	if (minValue >= maxValue) {
		printf("ERROR: TF min max is not correct!\n");
	}
	m_nActiveBlock = 0;

	int bsx, bsy, bsz;
	int bstart_x, bstart_y, bstart_z;
	int nz_vol = volume_->depth();
	int ny_vol = volume_->height();
	int nx_vol = volume_->width();

	const int wm1 = m_width - 1;
	const int wm1_hm1 = (m_height - 1) * wm1;
	const int w_h = m_height * m_width;
	const int numCubeFace = 36;
	for (int i = 0; i < m_depth - 1; i++) {
		const int i_wm1_hm1 = i * wm1_hm1;
		const int i_w_h = i * w_h;
		const int ip1_w_h = (i + 1)*w_h;

		if (i == m_depth - 2) {
			bsz = nz_vol - m_blockSize * i;
		} else {
			bsz = m_blockSize + 1;
		}

		bstart_z = (i == 0) ? 0 : -1;

		for (int j = 0; j < m_height - 1; j++) {
			const int j_wm1 = j * wm1;
			const int i_wm1_hm1_p_j_wm1 = i_wm1_hm1 + j_wm1;
			const int jp1_w = (j + 1)*m_width;
			const int j_w = j * m_width;

			if (j == m_height - 2) {
				bsy = ny_vol - m_blockSize * j;
			} else {
				bsy = m_blockSize + 1;
			}

			bstart_y = (j == 0) ? 0 : -1;

			for (int k = 0; k < m_width - 1; k++) {
				const int id = i_wm1_hm1_p_j_wm1 + k;

				if (k == m_width - 2) {
					bsx = nx_vol - m_blockSize * k;
				} else {
					bsx = m_blockSize + 1;
				}

				bstart_x = (k == 0) ? 0 : -1;

				bool pass_block = true;
				if (vr_cut_mask_) {
#if 0
					pass_block = false;
#else
					for (int ii = bstart_z; ii < bsz; ii++) {
						int z_index = i * m_blockSize + ii;
						if (z_index >= nz_vol)
							break;
						for (int jj = bstart_y; jj < bsy; jj++) {
							int y_index = j * m_blockSize + jj;
							if (y_index >= ny_vol)
								break;
							for (int kk = bstart_x; kk < bsx; kk++) {
								int x_index = k * m_blockSize + kk;
								if (x_index >= nx_vol)
									break;
								int xy_index = (j * m_blockSize + jj) * nx_vol + (nx_vol - (k * m_blockSize + kk) - 1);

								unsigned short mask_val = vr_cut_mask_[z_index][xy_index];
								if (mask_val & 0x0001 << cur_vr_cut_history_step_)
									pass_block = true;
								else
									pass_block = false;

								if (!pass_block)
									break;
							}

							if (!pass_block)
								break;
						}

						if (!pass_block)
							break;
					}
#endif
				} else {
					pass_block = false;
				}

				if (m_minmax[id].min <= maxValue && m_minmax[id].max >= minValue && !pass_block) {
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
	//logger->Print(LogType::DBG, "setActiveBlock : " + std::to_string(elapsed_time));

	//qDebug() << "setActiveBlock :" << elapsed_time;
}

void CW3ActiveBlock::SetVRCutActiveBlock(unsigned short** vr_cut_mask, int cur_vr_cut_history_step, int min_value, int max_value) {
	vr_cut_mask_ = vr_cut_mask;
	cur_vr_cut_history_step_ = cur_vr_cut_history_step;

#if 0
	InitMinMax();
#endif
	setActiveBlock(min_value, max_value);
}

void CW3ActiveBlock::SetVRCutActiveBlockHistoryStep(int cur_vr_cut_history_step, int min_value, int max_value) {
	cur_vr_cut_history_step_ = cur_vr_cut_history_step;

#if 0
	InitMinMax();
#endif
	setActiveBlock(min_value, max_value);
}

//void CW3ActiveBlock::createOctreeLv5(void)
//{
//	/*****************************
//		* creating octree for ESL
//		* max-level : 5
//		**************************/
//	float leafVsizeX = volume_->width()/32.0f;
//	float leafVsizeY = volume_->height()/32.0f;
//	float leafVsizeZ = volume_->depth()/32.0f;
//
//	omp_set_num_threads(4);
//#pragma omp parallel for
//	// create Lv.5 (Leaf-nodes)
//	for( int i=0; i<32; i++ )
//	for( int j=0; j<32; j++ )
//	for( int k=0; k<32; k++ )
//	{
//		short min = std::numeric_limits<short>::max();
//		short max = std::numeric_limits<short>::min();
//		for( int m=i*leafVsizeZ; m<i*leafVsizeZ+leafVsizeZ; m++ )
//		for( int n=j*leafVsizeY; n<j*leafVsizeY+leafVsizeY; n++ )
//		for( int o=k*leafVsizeX; o<k*leafVsizeX+leafVsizeX; o++ )
//		{
//			short val = volume_->getData()[m][n*volume_->width()+o];
//			if( val > max )	max = val;
//			if( val < min ) min = val;
//		}
//		int idx = 1 + 8 + 8*8 + 8*8*8 + 8*8*8*8; // start index of Lv.3
//		idx += (k/16*16*16*16 + j/16*2*16*16*16 + i/16*4*16*16*16); // start index of current Node (Lv.1) -> make 16x16x16 node.
//		idx += (k%16/8*8*8*8 + j%16/8*2*8*8*8 + i%16/8*4*8*8*8);
//		idx += (k%8/4*4*4*4 + j%8/4*2*4*4*4 + i%8/4*4*4*4*4);
//		idx += (k%4/2*2*2*2 + j%4/2*2*2*2*2 + i%4/2*4*2*2*2);
//		idx += (k%2 + j%2*2 + i%2*2*2);
//
//		m_ocTreeLevel[idx] = 5;
//		m_ocTreeMaxMin[idx].x = max/32768.0f;
//		m_ocTreeMaxMin[idx].y = min/32768.0f;
//	}
//
//	// create Lv.4
//	omp_set_num_threads(4);
//#pragma omp parallel for
//	for( int idx = 1+8+8*8+8*8*8; idx<1+8+8*8+8*8*8+8*8*8*8; idx++ )
//	{
//		float min = std::numeric_limits<float>::max();
//		float max = std::numeric_limits<float>::min();
//		for( int i=idx*8+1; i<idx*8+9; i++ ){
//			if( m_ocTreeMaxMin[i].x > max )	max = m_ocTreeMaxMin[i].x;
//			if( m_ocTreeMaxMin[i].y < min )	min = m_ocTreeMaxMin[i].y;
//		}
//		m_ocTreeLevel[idx] = 4;
//		m_ocTreeMaxMin[idx].x = max;
//		m_ocTreeMaxMin[idx].y = min;
//	}
//
//	// create Lv.3
//	omp_set_num_threads(4);
//#pragma omp parallel for
//	for( int idx = 1+8+8*8; idx<1+8+8*8+8*8*8; idx++ ) // for all Lv.3 nodes.
//	{
//		float min = std::numeric_limits<float>::max();
//		float max = std::numeric_limits<float>::min();
//		for( int i=idx*8+1; i<idx*8+9; i++ ){
//			if( m_ocTreeMaxMin[i].x > max )	max = m_ocTreeMaxMin[i].x;
//			if( m_ocTreeMaxMin[i].y < min )	min = m_ocTreeMaxMin[i].y;
//		}
//		m_ocTreeLevel[idx] = 3;
//		m_ocTreeMaxMin[idx].x = max;
//		m_ocTreeMaxMin[idx].y = min;
//	}
//
//	// create Lv.2
//	omp_set_num_threads(4);
//#pragma omp parallel for
//	for( int idx = 1+8; idx < 1+8+8*8; idx++ ) // for all Lv.2 nodes.
//	{
//		float min = std::numeric_limits<float>::max();
//		float max = std::numeric_limits<float>::min();
//		for( int i=idx*8+1; i<idx*8+9; i++ ) {
//			if( m_ocTreeMaxMin[i].x > max )	max = m_ocTreeMaxMin[i].x;
//			if( m_ocTreeMaxMin[i].y < min )	min = m_ocTreeMaxMin[i].y;
//		}
//		m_ocTreeLevel[idx] = 2;
//		m_ocTreeMaxMin[idx].x = max;
//		m_ocTreeMaxMin[idx].y = min;
//	}
//
//	// create Lv.1
//	for( int idx = 1; idx < 1+8; idx++ ) // for all Lv.1 nodes.
//	{
//		float min = std::numeric_limits<float>::max();
//		float max = std::numeric_limits<float>::min();
//		for( int i=idx*8+1; i<idx*8+9; i++ ) {
//			if( m_ocTreeMaxMin[i].x > max )	max = m_ocTreeMaxMin[i].x;
//			if( m_ocTreeMaxMin[i].y < min )	min = m_ocTreeMaxMin[i].y;
//		}
//		m_ocTreeLevel[idx] = 1;
//		m_ocTreeMaxMin[idx].x = max;
//		m_ocTreeMaxMin[idx].y = min;
//	}
//
//	// create root node. (Lv.0)
//	float min = std::numeric_limits<float>::max();
//	float max = std::numeric_limits<float>::min();
//	for( int i = 1; i < 1+8; i++ )
//	{
//		if( m_ocTreeMaxMin[i].x > max )	max = m_ocTreeMaxMin[i].x;
//		if( m_ocTreeMaxMin[i].y < min )	min = m_ocTreeMaxMin[i].y;
//	}
//	m_ocTreeLevel[0] = 0;
//	m_ocTreeMaxMin[0].x = max;
//	m_ocTreeMaxMin[0].y = min;
//}
//
