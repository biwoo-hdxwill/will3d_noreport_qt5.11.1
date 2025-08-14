#include "W3Image3D.h"

#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3Memory.h"

#include "W3ImageHeader.h"

CW3Image3D::CW3Image3D(unsigned int width, unsigned int height, unsigned int depth) :
	m_nWidth(width), m_nHeight(height), m_nDepth(depth) {
	if (width < 0 || height < 0 || depth < 1) {
		common::Logger::instance()->Print(common::LogType::ERR,
										  "CW3Image3D::CW3Image3D : Invalid Size of Volume Creation.");
	}

	resize(width, height, depth);
}

CW3Image3D::~CW3Image3D(void) {
	this->cleanUp();
}

void CW3Image3D::resize(const int nWidth, const int nHeight, const int nDepth) {
	if (m_ppusData) {
		for (int i = 0; i < m_nDepth; i++)
			SAFE_DELETE_ARRAY(m_ppusData[i]);
		SAFE_DELETE_ARRAY(m_ppusData);
	}

	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_nDepth = nDepth;

	const int sliceSize = m_nWidth * m_nHeight;
	m_ppusData = SAFE_ALLOC_VOLUME(unsigned short, m_nDepth, sliceSize);

	//for (int i = 0; i < m_nDepth; i++)
	//	std::memset(m_ppusData[i], 0, sizeof(unsigned short) * sliceSize);
}

void CW3Image3D::cleanUp(void) {
	if (m_ppusData) {
		for (int i = 0; i < m_nDepth; i++)
			SAFE_DELETE_ARRAY(m_ppusData[i]);
		SAFE_DELETE_ARRAY(m_ppusData);
	}

	SAFE_DELETE_ARRAY(m_iHistogram);
}

void CW3Image3D::setHistogram(int* histogram, const int size) {
	SAFE_DELETE_ARRAY(m_iHistogram);

	m_iHistogram = SAFE_ALLOC_1D(int, size);
	std::memcpy(m_iHistogram, histogram, sizeof(int)*size);
	m_iHistoSize = size;
}

unsigned short** CW3Image3D::exportScaleP(int scale, int* outWidth, int* outHeight, int* outDepth) const {
	if (m_ppusData == NULL) return nullptr;

	int finalw = (int)(((float)m_nWidth / scale) + .5f);
	int finalh = (int)(((float)m_nHeight / scale) + .5f);
	int finald = (int)(((float)m_nDepth / scale) + .5f);

	unsigned short** data = SAFE_ALLOC_VOLUME(unsigned short, finald, finalw*finalh);

	finald = std::max(1, finald);

	for (int i = 0; i < finald; i++)
		memset(data[i], 0, sizeof(unsigned short)*finalw*finalh);

	for (int z = 0; z < finald; z++) {
		for (int y = 0; y < finalh; y++) {
			int iy = y * finalw;
			for (int x = 0; x < finalw; x++) {
				data[z][iy + x] = m_ppusData[z*scale][(x + y * m_nWidth)*scale];
			}
		}
	}

	*outWidth = finalw;
	*outHeight = finalh;
	*outDepth = finald;

	return data;
}

glm::vec4 CW3Image3D::GetVolumeInfo(const glm::vec3& volume_coord) const {
	int idx_x = static_cast<int>(roundf(volume_coord.x * 10) / 10);
	int idx_y = static_cast<int>(roundf(volume_coord.y * 10) / 10);
	int idx_z = static_cast<int>(roundf(volume_coord.z * 10) / 10);

	if ((idx_x >= 0 && idx_x < m_nWidth) &&
		(idx_y >= 0 && idx_y < m_nHeight) &&
		(idx_z >= 0 && idx_z < m_nDepth)) {
		auto hu = m_ppusData[idx_z][idx_y * m_nWidth + idx_x];
		return glm::vec4(idx_x + 1, idx_y + 1, idx_z + 1, hu + m_fIntercept);
	}

	if (idx_x < 0)
		idx_x = 0;
	else if (idx_x > m_nWidth)
		idx_x = m_nWidth;

	if (idx_y < 0)
		idx_y = 0;
	else if (idx_y > m_nHeight)
		idx_y = m_nHeight;

	if (idx_z < 0)
		idx_z = 0;
	else if (idx_z > m_nDepth)
		idx_z = m_nDepth;

	return glm::vec4(idx_x + 1, idx_y + 1, idx_z + 1, common::dicom::kInvalidHU);
}
void CW3Image3D::GetVolumeHU(const std::vector<glm::vec3>& volume_coords,
							 std::vector<short>& data) const {
	data.resize(volume_coords.size());
	for (int index = 0; index < volume_coords.size(); ++index) {
		data[index] = GetVolumeInfo(volume_coords[index]).w;
	}
}
float CW3Image3D::GetBasePixelSize() const {
	if (m_fSliceSpacing / m_fPixelSpacing >= 1.0f)
		return m_fPixelSpacing;
	else
		return m_fSliceSpacing;
}
/*

void CW3Image3D::getHistogram(int* histogram)
{
	if (histogram)
		delete[] histogram;

	histogram = new int[m_iHistoSize];
	memcpy(histogram, m_iHistogram, sizeof(int)*m_iHistoSize);
}*/
