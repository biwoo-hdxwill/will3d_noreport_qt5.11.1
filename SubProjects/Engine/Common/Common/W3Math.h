#pragma once
/*=========================================================================

File:		Collection of basic mathmatic funcions
Language:	C++11
Library:	Standard C++ Library

=========================================================================*/
#include <vector>

#if defined(__APPLE__)
#include </usr/local/Cellar/llvm/5.0.0/lib/clang/5.0.0/include/omp.h>
#include <glm/glm.hpp>
#else
#include <omp.h>
#include <gl/glm/glm.hpp>
#endif
#include <xmmintrin.h>
#include <mmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

#include <qpoint.h>
#include <qmath.h>

#include "W3Memory.h"

namespace
{
	enum VolPlaneType { X_MAX, Y_MAX, Z_MAX, X_MIN, Y_MIN, Z_MIN, VOL_PLANE_END };
	const glm::vec3 kAxis[] = {
		glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f) };
	const float kEpsEqual = 0.000001f;
}

namespace W3
{
	const double kEpsPointComp = 0.000000001;

	inline int ramp(const int& y0, const int& y1, const float& x0, const float& x1, const float& x);
	inline float ramp(const float& y0, const float& y1, const float& x0, const float& x1, const float& x);

	template <typename TYPE>
	inline const float lerp(const TYPE& a, const TYPE& b, const float& t) { return a + t * (b - a); }
	template <typename TYPE>
	inline const TYPE trilerp(const TYPE& a000, const TYPE& a001, const TYPE& a010, const TYPE& a011,
		const TYPE& a100, const TYPE& a101, const TYPE& a110, const TYPE& a111,
		const float& dx, const float& dy, const float& dz);

	inline int trilerpSIMD(__m128i &x0, __m128i &x1, __m128 &d);
	//inline int trilerpSIMD(int a000, int a001, int a010, int a011, int a100, int a101, int a110, int a111, float dx, float dy, float dz);
	//inline float trilerpSIMD(const __m128 *x0, const __m128 *x1, const __m128 *d);
	inline void inverse4by4(float *mat, float *dst);
	template <typename T> inline const bool isInRange(const T& a, const T& min, const T& max) { return (a >= min && a <= max) ? true : false; }
	template <typename T> inline const bool isWithinRange(const T& a, const T& min, const T& max) { return (a > min && a < max) ? true : false; }
	template <typename T> inline const T& getMin(const T& a, const T& b) { return (a < b) ? a : b; }
	template <typename T> inline const T& getMax(const T& a, const T& b) { return (a > b) ? a : b; }
	template <typename T> inline const T& MAX3(const T& a, const T& b, const T& c) { return getMax(getMax(a, b), c); }
	template <typename T> inline const T& setRange(const T& a, const T& min, const T& max) { return getMax(min, getMin(a, max)); }
	inline void volumeDown(unsigned short*** volout, unsigned short** volin, unsigned int downFactor, int &w, int &h, int &d);
	inline void volumeDown(unsigned short** volout, unsigned short** volin, unsigned int downFactor, int &w, int &h, int &d);

	inline glm::vec3 NormailzeCoordWithZspacing(const glm::vec3& coordinate, const float& z_spacing) {
		if (z_spacing >= 1.0f)
			return glm::vec3(coordinate.x, coordinate.y, coordinate.z / z_spacing);
		else
			return glm::vec3(coordinate.x * z_spacing, coordinate.y * z_spacing, coordinate.z);
	}

	inline float getLength(const QPointF& st, const QPointF& et, float fPitch) {
		QPointF fDist(st - et);
		fDist.setX(fabs(fDist.rx() * fPitch));
		fDist.setY(fabs(fDist.ry() * fPitch));
		return sqrt((float)(fDist.rx() * fDist.rx() + fDist.ry() * fDist.ry()));
	}

	inline float PixelDist(const QPointF& st, const QPointF& et) {
		QPointF fDist(st - et);
		fDist.setX(fabs(fDist.rx()));
		fDist.setY(fabs(fDist.ry()));
		return sqrt((float)(fDist.rx() * fDist.rx() + fDist.ry() * fDist.ry()));
	}
	inline float getAngle(const QPointF& st, const QPointF& mt, const QPointF& et) {
		QPointF ao(st - mt);
		QPointF bo(et - mt);
		QPointF ab(et - st);

		float aoLen = sqrt(static_cast<float>((ao.rx() * ao.rx() + ao.ry() * ao.ry())));
		float boLen = sqrt(static_cast<float>((bo.rx() * bo.rx() + bo.ry() * bo.ry())));
		float abLen = sqrt(static_cast<float>((ab.rx() * ab.rx() + ab.ry() * ab.ry())));
		return 180.0f / M_PI * acos((aoLen*aoLen + boLen * boLen - abLen * abLen) / (2 * aoLen * boLen));
	}

	inline float GetAngle(const glm::vec3& src, const glm::vec3& dst)
	{
		return acos(glm::clamp(glm::dot(src, dst), -1.f, 1.f)) * 180.0f / M_PI;
	}

	inline void GenerateSequencialPlanePointsInLine(const QPointF& start_pt, const QPointF& end_pt,
		std::vector<QPointF>& plane_points) {
		QPointF size(end_pt - start_pt);
		const int data_length = (int)sqrt((int)(size.rx() * size.rx() + size.ry() * size.ry()));
		plane_points.resize(data_length);
		const QPointF step = size / data_length;

		QPointF pt_curr = start_pt;
		for (auto& pt : plane_points) {
			pt = pt_curr;
			pt_curr += step;
		}
	}

	inline void GenerateSequencialPlanePointsInRect(const QPointF& start_pt, const QPointF& end_pt,
		std::vector<QPointF>& plane_points) {
		const QPointF kStepX(1.0, 0.0);
		const QPointF kStepY(0.0, 1.0);
		QPointF size(end_pt - start_pt);
		plane_points.reserve(size.rx()*size.ry());
		QPointF start_pos = start_pt;
		for (int y = 0; y < size.y(); ++y) {
			QPointF pt_curr = start_pos;
			for (int x = 0; x < size.x(); ++x) {
				plane_points.push_back(pt_curr);
				pt_curr += kStepX;
			}
			start_pos += kStepY;
		}
	}

	inline bool IsEqualPoint(const QPointF& start, const QPointF& end) {
		if (std::abs(start.x() - end.y()) < kEpsPointComp &&
			std::abs(start.x() - end.y()) < kEpsPointComp)
			return false;
		return true;
	}

	inline glm::vec3 FitVolRange(const glm::vec3& vol_range, const glm::vec3& cross_point, const glm::vec3& destination)
	{
		if (isInRange(destination.x, 0.f, vol_range.x) &&
			isInRange(destination.y, 0.f, vol_range.y) &&
			isInRange(destination.z, 0.f, vol_range.z))
		{
			return destination;
		}

		glm::vec3 direction = destination - cross_point;
#if 0
		glm::vec3 plane_point;
		for (int vol_id = 0; vol_id < VolPlaneType::VOL_PLANE_END; ++vol_id)
		{
			switch (vol_id)
			{
			case VolPlaneType::X_MAX:
			case VolPlaneType::Y_MAX:
			case VolPlaneType::Z_MAX:
				plane_point = vol_range;
				break;
			case VolPlaneType::X_MIN:
			case VolPlaneType::Y_MIN:
			case VolPlaneType::Z_MIN:
				plane_point = glm::vec3(0.0f);
				break;
			}
			float dot_up_direction = glm::dot(kAxis[vol_id], direction);
			if (std::fabsf(dot_up_direction) <= kEpsEqual)
			{
				continue;
			}

			glm::vec3 inv_w = plane_point - cross_point;
			float meet_position = glm::dot(kAxis[vol_id], inv_w) / dot_up_direction;
			if (isInRange(meet_position, 0.0f, 1.0f))
			{
				glm::vec3 ret_vec = cross_point + meet_position * direction;
				if (isInRange(ret_vec.x, 0.0f, vol_range.x) &&
					isInRange(ret_vec.y, 0.0f, vol_range.y) &&
					isInRange(ret_vec.z, 0.0f, vol_range.z))
				{
					return ret_vec;
				}
			}
		}
#else
		glm::vec3 end_point;
		for (int i = 0; i < VolPlaneType::VOL_PLANE_END; ++i)
		{
			switch (i)
			{
			case VolPlaneType::X_MAX:
			case VolPlaneType::Y_MAX:
			case VolPlaneType::Z_MAX:
				end_point = vol_range;
				break;
			case VolPlaneType::X_MIN:
			case VolPlaneType::Y_MIN:
			case VolPlaneType::Z_MIN:
				end_point = glm::vec3(0.f);
				break;
			}

			float dot_direction = glm::dot(kAxis[i], direction);
			if (std::fabsf(dot_direction) <= kEpsEqual || dot_direction < 0.f)
			{
				continue;
			}

			glm::vec3 end_dir = end_point - cross_point;
			float dot_end = glm::dot(kAxis[i], end_dir);
			if (std::fabsf(dot_end) <= kEpsEqual || dot_end < 0.f)
			{
				continue;
			}

			float meet_position = dot_end / dot_direction;
			if (isInRange(meet_position, 0.f, 1.f))
			{
				glm::vec3 ret_vec = cross_point + meet_position * direction;
				if (isInRange(ret_vec.x, 0.f, vol_range.x) &&
					isInRange(ret_vec.y, 0.f, vol_range.y) &&
					isInRange(ret_vec.z, 0.f, vol_range.z))
				{
					return ret_vec;
				}
			}
		}
#endif
		return cross_point;
	}
} // end of namespace W3

int W3::ramp(const int& y0, const int& y1, const float& x0, const float& x1, const float& x) {
	if (x0 == x1)
		return x;
	else
		return static_cast<int>(((x <= x0) ? y0 : ((x < x1) ? (y0 + (y1 - y0)*(x - x0) / (x1 - x0)) : y1) + 0.5f));
}

float W3::ramp(const float& y0, const float& y1, const float& x0, const float& x1, const float& x) {
	if (x0 == x1)
		return y0;
	else
		return (x <= x0) ? y0 : ((x < x1) ? (y0 + (y1 - y0)*(x - x0) / (x1 - x0)) : y1);
}

template <typename TYPE>
const TYPE W3::trilerp(const TYPE& a000, const TYPE& a001, const TYPE& a010, const TYPE& a011,
	const TYPE& a100, const TYPE& a101, const TYPE& a110, const TYPE& a111,
	const float& dx, const float& dy, const float& dz) {
	/*
	const float a00 = lerp(a000, a001, dx);
	const float a01 = lerp(a010, a011, dx);
	const float a10 = lerp(a100, a101, dx);
	const float a11 = lerp(a110, a111, dx);

	const float a0 = lerp(a00, a01, dy);
	const float a1 = lerp(a10, a11, dy);

	return static_cast<TYPE>(lerp(a0, a1, dz));
	*/
	return static_cast<TYPE>(lerp(
		lerp(lerp(a000, a001, dx), lerp(a010, a011, dx), dy),
		lerp(lerp(a100, a101, dx), lerp(a110, a111, dx), dy), dz));
}

int W3::trilerpSIMD(__m128i &mmx0i, __m128i &mmx1i, __m128 &mmd) {
	//__m128i mmx0i = _mm_loadu_si128((__m128i*)x0);
	//__m128i mmx1i = _mm_loadu_si128((__m128i*)x1);
	//__m128 mmd = _mm_loadu_ps(d);

	//__m128i mmx0i = _mm_set_epi32(a110, a100, a010, a000);
	//__m128i mmx1i = _mm_set_epi32(a111, a101, a011, a001);
	//__m128 mmd = _mm_set_ps(0.0f, dz, dy, dx);

	__m128 mmx0 = _mm_cvtepi32_ps(mmx0i);
	__m128 mmx1 = _mm_cvtepi32_ps(mmx1i);

	const __m128 mm1md = _mm_sub_ps(_mm_set1_ps(1.0f), mmd);

	const __m128 z1100 = _mm_shuffle_ps(mm1md, mmd, _MM_SHUFFLE(2, 2, 2, 2));
	//__m128 z0101 = _mm_shuffle_ps(ztmp, ztmp, _MM_SHUFFLE(0, 2, 0, 2));
	const __m128 ytmp = _mm_shuffle_ps(mm1md, mmd, _MM_SHUFFLE(1, 1, 1, 1));
	const __m128 y1010 = _mm_shuffle_ps(ytmp, ytmp, _MM_SHUFFLE(2, 0, 2, 0));

	const __m128 x0000 = _mm_shuffle_ps(mm1md, mm1md, _MM_SHUFFLE(0, 0, 0, 0));
	const __m128 x1111 = _mm_shuffle_ps(mmd, mmd, _MM_SHUFFLE(0, 0, 0, 0));
	//__m128 sw_yz = _mm_mul_ps(z0101, y0011);
	const __m128 sw_yz = _mm_mul_ps(z1100, y1010);

	const __m128i mmResult = _mm_cvtps_epi32(_mm_dp_ps(sw_yz, _mm_add_ps(_mm_mul_ps(x0000, mmx0), _mm_mul_ps(x1111, mmx1)), 0xff));

	return *((int*)(&mmResult));
}

void W3::inverse4by4(float *mat, float *dst) {
	// by Cramer's rule
	float tmp[12]; /* temp array for pairs */
	float src[16]; /* array of transpose source matrix */
	float det; /* determinant */
	/* transpose matrix */
	for (int i = 0; i < 4; i++) {
		src[i] = mat[i * 4];
		src[i + 4] = mat[i * 4 + 1];
		src[i + 8] = mat[i * 4 + 2];
		src[i + 12] = mat[i * 4 + 3];
	}
	/* calculate pairs for first 8 elements (cofactors) */
	tmp[0] = src[10] * src[15];
	tmp[1] = src[11] * src[14];
	tmp[2] = src[9] * src[15];
	tmp[3] = src[11] * src[13];
	tmp[4] = src[9] * src[14];
	tmp[5] = src[10] * src[13];
	tmp[6] = src[8] * src[15];
	tmp[7] = src[11] * src[12];
	tmp[8] = src[8] * src[14];
	tmp[9] = src[10] * src[12];
	tmp[10] = src[8] * src[13];
	tmp[11] = src[9] * src[12];
	/* calculate first 8 elements (cofactors) */
	dst[0] = tmp[0] * src[5] + tmp[3] * src[6] + tmp[4] * src[7];
	dst[0] -= tmp[1] * src[5] + tmp[2] * src[6] + tmp[5] * src[7];
	dst[1] = tmp[1] * src[4] + tmp[6] * src[6] + tmp[9] * src[7];
	dst[1] -= tmp[0] * src[4] + tmp[7] * src[6] + tmp[8] * src[7];
	dst[2] = tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7];
	dst[2] -= tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7];
	dst[3] = tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6];
	dst[3] -= tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6];
	dst[4] = tmp[1] * src[1] + tmp[2] * src[2] + tmp[5] * src[3];
	dst[4] -= tmp[0] * src[1] + tmp[3] * src[2] + tmp[4] * src[3];
	dst[5] = tmp[0] * src[0] + tmp[7] * src[2] + tmp[8] * src[3];
	dst[5] -= tmp[1] * src[0] + tmp[6] * src[2] + tmp[9] * src[3];
	dst[6] = tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3];
	dst[6] -= tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3];
	dst[7] = tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2];
	dst[7] -= tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2];
	/* calculate pairs for second 8 elements (cofactors) */
	tmp[0] = src[2] * src[7];
	tmp[1] = src[3] * src[6];
	tmp[2] = src[1] * src[7];
	tmp[3] = src[3] * src[5];
	tmp[4] = src[1] * src[6];
	tmp[5] = src[2] * src[5];
	tmp[6] = src[0] * src[7];
	tmp[7] = src[3] * src[4];
	tmp[8] = src[0] * src[6];
	tmp[9] = src[2] * src[4];
	tmp[10] = src[0] * src[5];
	tmp[11] = src[1] * src[4];
	/* calculate second 8 elements (cofactors) */
	dst[8] = tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15];
	dst[8] -= tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15];
	dst[9] = tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15];
	dst[9] -= tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15];
	dst[10] = tmp[2] * src[12] + tmp[7] * src[13] + tmp[10] * src[15];
	dst[10] -= tmp[3] * src[12] + tmp[6] * src[13] + tmp[11] * src[15];
	dst[11] = tmp[5] * src[12] + tmp[8] * src[13] + tmp[11] * src[14];
	dst[11] -= tmp[4] * src[12] + tmp[9] * src[13] + tmp[10] * src[14];
	dst[12] = tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9];
	dst[12] -= tmp[4] * src[11] + tmp[0] * src[9] + tmp[3] * src[10];
	dst[13] = tmp[8] * src[11] + tmp[0] * src[8] + tmp[7] * src[10];
	dst[13] -= tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8];
	dst[14] = tmp[6] * src[9] + tmp[11] * src[11] + tmp[3] * src[8];
	dst[14] -= tmp[10] * src[11] + tmp[2] * src[8] + tmp[7] * src[9];
	dst[15] = tmp[10] * src[10] + tmp[4] * src[8] + tmp[9] * src[9];
	dst[15] -= tmp[8] * src[9] + tmp[11] * src[10] + tmp[5] * src[8];
	/* calculate determinant */
	det = src[0] * dst[0] + src[1] * dst[1] + src[2] * dst[2] + src[3] * dst[3];
	/* calculate matrix inverse */
	det = 1 / det;
	for (int j = 0; j < 16; j++)
		dst[j] *= det;
}

void W3::volumeDown(unsigned short*** volout, unsigned short** volin, unsigned int downFactor, int &w, int &h, int &d) {
	int nVolWidth = w;
	int nVolHeight = h;
	int nVolDepth = d;

	w = (float)nVolWidth / downFactor;
	h = (float)nVolHeight / downFactor;
	d = (float)nVolDepth / downFactor;

	size_t sResize = w * h*d;

	//SAFE_DELETE_ARRAY(*volout);

	//W3::p_allocate_1D(volout, sResize);
	W3::p_allocate_volume(volout, d, w*h);

	float norm = 1.0f / (downFactor*downFactor*downFactor);

#if defined(_OPENMP)
	int numThread = omp_get_max_threads();
#else
	int numThread = 1;
#endif
	omp_set_num_threads(numThread);

#pragma omp parallel for

	for (int i = 0; i < d; i++) {
		for (int j = 0; j < h; j++) {
			for (int k = 0; k < w; k++) {
				int tmp = 0;
				for (int ii = 0; ii < downFactor; ii++) {
					for (int jj = 0; jj < downFactor; jj++) {
						for (int kk = 0; kk < downFactor; kk++) {
							tmp += volin[i*downFactor + ii][(j*downFactor + jj)*nVolWidth + k * downFactor + kk];
						}
					}
				}
				//(*volout)[i*w*h + j*w + k] = tmp*norm;
				(*volout)[i][j*w + k] = tmp * norm;
			}
		}
	}
}

void W3::volumeDown(unsigned short** volout, unsigned short** volin, unsigned int downFactor, int &w, int &h, int &d) {
	int nVolWidth = w;
	int nVolHeight = h;
	int nVolDepth = d;

	w = (float)nVolWidth / downFactor;
	h = (float)nVolHeight / downFactor;
	d = (float)nVolDepth / downFactor;

	size_t sResize = w * h*d;

	SAFE_DELETE_ARRAY(*volout);

	W3::p_allocate_1D(volout, sResize);
	//W3::p_allocate_volume(volout, d, w*h);

	float norm = 1.0f / (downFactor*downFactor*downFactor);

#if defined(_OPENMP)
	int numThread = omp_get_max_threads();
#else
	int numThread = 1;
#endif
	omp_set_num_threads(numThread);

#pragma omp parallel for

	for (int i = 0; i < d; i++) {
		for (int j = 0; j < h; j++) {
			for (int k = 0; k < w; k++) {
				int tmp = 0;
				for (int ii = 0; ii < downFactor; ii++) {
					for (int jj = 0; jj < downFactor; jj++) {
						for (int kk = 0; kk < downFactor; kk++) {
							tmp += volin[i*downFactor + ii][(j*downFactor + jj)*nVolWidth + k * downFactor + kk];
						}
					}
				}
				(*volout)[i*w*h + j * w + k] = tmp * norm;
				//(*volout)[i][j*w + k] = tmp*norm;
			}
		}
	}
}
