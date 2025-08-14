#pragma once
/*=========================================================================

File:		class CW3Image3D
Language:	C++11
Library:	Qt 5.4.0, Standard C++ Library

=========================================================================*/
#include <memory>

#if defined(__APPLE__)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtc/type_ptr.hpp>
#else
#include <GL/glm/glm.hpp>
#include <GL/glm/gtc/matrix_transform.hpp>
#include <GL/glm/gtx/transform2.hpp>
#include <GL/glm/gtc/type_ptr.hpp>
#endif

#include "../../Common/Common/W3Types.h"

#include "W3Resource.h"
#include "resource_global.h"

class CW3ImageHeader;

/*
	* Volume Data Structure.
	* Descriptions.
		- super class : CW3Resource
*/
class RESOURCE_EXPORT CW3Image3D : public CW3Resource {
public:
	explicit CW3Image3D(unsigned int width, unsigned int height, unsigned int depth = 1);
	virtual ~CW3Image3D(void) override;

	CW3Image3D(const CW3Image3D&) = delete;
	CW3Image3D& operator=(const CW3Image3D&) = delete;

public:
	// public functions.
	inline unsigned short**	getData(void) const { return m_ppusData; }
	inline CW3ImageHeader* getHeader(void) const { return m_pHeader.get(); }
	inline void	setHeader(std::shared_ptr<CW3ImageHeader> pHeader) { m_pHeader = pHeader; }
	inline const unsigned short getMin(void) const noexcept { return m_usMin; }
	inline const unsigned short getMax(void) const noexcept { return m_usMax; }
	inline void setMinMax(const unsigned short sMin, const unsigned short sMax) noexcept { m_usMin = sMin; m_usMax = sMax; }
	inline int* getHistogram() const { return m_iHistogram; }
	inline const int getHistoSize() const noexcept { return m_iHistoSize; }

	/* de-allocate volume memory. */
	void resize(const int nWidth, const int nHeight, const int nDepth);
	void cleanUp(void);

	inline const unsigned int sizeSlice(void) const noexcept { return m_nWidth * m_nHeight; }
	inline const unsigned int sizeVol(void) const noexcept { return m_nWidth * m_nHeight*m_nDepth; }
	inline const unsigned int width(void) const	noexcept { return m_nWidth; }
	inline const unsigned int height(void) const noexcept { return m_nHeight; }
	inline const unsigned int depth(void) const	noexcept { return m_nDepth; }
	inline const float pixelSpacing(void) const	noexcept { return m_fPixelSpacing; }
	inline const float sliceSpacing(void) const noexcept { return m_fSliceSpacing; }
	inline const int windowCenter(void) const noexcept { return m_nWindowCenter; }
	inline const int windowWidth(void) const noexcept { return m_nWindowWidth; }
	inline const float slope(void) const noexcept { return m_fSlope; }
	inline const float intercept(void) const noexcept { return m_fIntercept; }
	inline void setWindowing(const int center, const int width) noexcept { m_nWindowCenter = center; m_nWindowWidth = width; }
	inline void	setPixelSpacing(const float pSpacing) noexcept { m_fPixelSpacing = pSpacing; }
	inline void	setSliceSpacing(const float sSpacing) noexcept { m_fSliceSpacing = sSpacing; }
	inline void	setSlope(const float slope) noexcept { m_fSlope = slope; }
	inline void	setIntercept(const float intercept) noexcept { m_fIntercept = intercept; }

	inline void set_bits_stored(const int bits_stored) { bits_stored_ = bits_stored; }
	inline const int bits_stored() const noexcept { return bits_stored_; }
	inline void set_pixel_representation(const int pixel_representation) { pixel_representation_ = pixel_representation; }
	inline const int pixel_representation() const noexcept { return pixel_representation_; }

	inline void set_pixel_representation_offset(const int pixel_representation_offset) { pixel_representation_offset_ = pixel_representation_offset; }
	inline const int pixel_representation_offset() const noexcept { return pixel_representation_offset_; }

	///////////////////////////////////////////////
	// v1.0.2
	inline const int start_image_x() const noexcept { return start_image_x_; }
	inline const int start_image_y() const noexcept { return start_image_y_; }
	inline const int start_image_num() const noexcept { return start_image_num_; }
	inline void	set_start_image_x(const int& x) noexcept { start_image_x_ = x; }
	inline void	set_start_image_y(const int& y) noexcept { start_image_y_ = y; }
	inline void	set_start_image_num(const int& num) noexcept { start_image_num_ = num; }
	///////////////////////////////////////////////

	void setHistogram(int* histogram, const int size);

	inline void	setThreshold(int airThreshold, int tissueThreshold, int boneThreshold) noexcept {
		m_nAirThreshold = airThreshold;
		m_nTissueThreshold = tissueThreshold;
		m_nBoneThreshold = boneThreshold;
	}
	inline void	setSliceLoc(const SliceLoc& sliceLoc) { m_sliceLoc = sliceLoc; }
	inline const int getAirTissueThreshold()  const noexcept { return m_nAirThreshold; }
	inline const int getTissueBoneThreshold() const noexcept { return m_nTissueThreshold; }
	inline const int getBoneTeethThreshold()  const noexcept { return m_nBoneThreshold; }
	inline const SliceLoc& getSliceLoc() const noexcept { return m_sliceLoc; }

	inline void setSecondToFirst(glm::mat4 &mat) { m_SecondToFirst = mat; }
	inline const glm::mat4& getSecondToFirst() const noexcept { return m_SecondToFirst; }

	unsigned short** exportScaleP(int scale, int* outWidth, int* outHeight, int* outDepth) const;

	glm::vec4 GetVolumeInfo(const glm::vec3& volume_coord) const;
	void GetVolumeHU(const std::vector<glm::vec3>& volume_coords, std::vector<short>& data) const;
	float GetBasePixelSize() const;

private:
	// private member fields.
	std::shared_ptr<CW3ImageHeader>	m_pHeader = nullptr;

	///////////////////////////////////////////////
	// v1.0.2
	int			start_image_x_ = 0;
	int			start_image_y_ = 0;
	int			start_image_num_ = 0;
	///////////////////////////////////////////////

	unsigned int m_nWidth;
	unsigned int m_nHeight;
	unsigned int m_nDepth;

	unsigned short m_usMin = 0; // minimum intensity
	unsigned short m_usMax = 0;	// maximum intensity

	unsigned short** m_ppusData = nullptr; // volume data
	float m_fPixelSpacing = 1.0f;
	float m_fSliceSpacing = 1.0f;
	int	m_nWindowCenter = 0; // windowing.
	int	m_nWindowWidth = 0;
	float m_fSlope;
	float m_fIntercept;
	int* m_iHistogram = nullptr;
	int m_iHistoSize = 0;
	int bits_stored_ = 16;
	int pixel_representation_ = 0;
	int pixel_representation_offset_ = 0;

	glm::mat4 m_SecondToFirst;

	int m_nAirThreshold = 0;
	int m_nTissueThreshold = 0;
	int m_nBoneThreshold = 0;

	SliceLoc m_sliceLoc;
};
