#include "W3Classification.h"

#include <iostream>

#include <qmath.h>
#include <QDebug>
#include <QElapsedTimer>
#include <QString>

#if defined(__APPLE__)
#include </usr/local/Cellar/llvm/5.0.0/lib/clang/5.0.0/include/omp.h>
#else
#include <omp.h>
#endif

#include <opencv2/core.hpp>

#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/common.h"
#include "../../Resource/Resource/W3Image3D.h"

#define SMOOTHING_HISTOGRAM 0

namespace
{
	const int kSmoothHistogramBinFactor = 500;
	const int kSmoothHistoBin = 25;
	const int kSmoothHistoSegCnt = 15;
	const int kSerchValleyDistLimit = 1500;
	const int kSerchValleyClimbMax = 50;
	const int kClimbingThreshold = 40;

	bool IsFreeFOV(const CW3Image3D& vol)
	{
		int size_slice_mm = (int)((float)(vol.width() * vol.height()) *
			vol.pixelSpacing() * vol.pixelSpacing());
		return (size_slice_mm < common::dicom::kFreeFOVSliceMaximumSizeMM) ? true
			: false;
	}
}  // namespace
void CW3Classification::ThresholderVolume(const CW3Image3D& vol,
	int* thd_air_tissue,
	int* thd_tissue_bone,
	int* thd_bone_teeth)
{
	qDebug() << "start CW3Classification::ThresholderVolume";

	QElapsedTimer timer;
	timer.start();

	int histogram_size = vol.getHistoSize();
#if 1
	int* histogram = vol.getHistogram();

	int air_max = std::min(4000, histogram_size);
	int tissue_max = std::min(air_max + 2000, histogram_size);
#if 1
	std::vector<int> smooth_histogram;
	smooth_histogram.resize(histogram_size, 0);
	Common::SmoothingHisto(histogram, histogram_size, smooth_histogram, 150, 5, false);
	int max_index = 10;
	for (int i = 11; i < histogram_size; ++i)
	{
		if (smooth_histogram[i] > smooth_histogram[max_index])
		{
			max_index = i;
		}
	}
#if 0
	air_max = max_index;
	tissue_max = std::min(air_max + 2000, histogram_size);

	*thd_air_tissue = ValleyEmphasisThresholder(smooth_histogram.data(), 1, air_max);
	*thd_tissue_bone = ValleyEmphasisThresholder(smooth_histogram.data(), max_index, tissue_max);
	*thd_bone_teeth = ValleyEmphasisThresholder(smooth_histogram.data(), *thd_tissue_bone, histogram_size);
#else
	std::vector<int> peaks;
	std::vector<int> valleys;
	for (int i = 1; i < histogram_size - 1; ++i)
	{
		int left = smooth_histogram[i - 1];
		int center = smooth_histogram[i];
		int right = smooth_histogram[i + 1];

		if (left < center && right <= center &&	center > smooth_histogram[max_index] * 0.5f)
		{
			peaks.push_back(i);
		}
		/*else if (left > center && right >= center)
		{
			valleys.push_back(i);
		}*/
	}
	air_max = peaks.size() > 1 ? peaks.at(1) : max_index;
	*thd_air_tissue = ValleyEmphasisThresholder(smooth_histogram.data(), 1, air_max);

	int tissue_min = air_max;
	tissue_max = std::min(air_max + 2000, histogram_size);
	*thd_tissue_bone = ValleyEmphasisThresholder(smooth_histogram.data(), tissue_min, tissue_max);
	*thd_bone_teeth = ValleyEmphasisThresholder(smooth_histogram.data(), *thd_tissue_bone, histogram_size);
#endif
#else
	*thd_air_tissue = ValleyEmphasisThresholder(histogram, 1, air_max);
	*thd_tissue_bone = ValleyEmphasisThresholder(histogram, *thd_air_tissue, tissue_max);
	*thd_bone_teeth = ValleyEmphasisThresholder(histogram, *thd_tissue_bone, histogram_size - 1);
#endif

#else
	if (IsFreeFOV(vol))
	{
		*thd_air_tissue = vol.getMin() + 1;
	}
	else
	{
		std::vector<ushort> mip_data;
		std::vector<int> mip_histo;

		GenerateSagittalMIP(vol, mip_data, mip_histo);
		*thd_air_tissue = GetAirThreshold(vol, mip_data, mip_histo);
	}

#if 1
	int metal =
		vol.windowCenter() + ((float)vol.windowWidth())*0.001f - vol.intercept();
#endif

	*thd_bone_teeth = GetTeethThreshold(vol);
#if 0
	* thd_tissue_bone = GetTissueThreshold(vol, *thd_air_tissue, metal);
#else
	*thd_tissue_bone = GetTissueThreshold(vol, *thd_air_tissue, histogram_size);
#endif
#endif

	qDebug() << "end CW3Classification::ThresholderVolume :" << timer.elapsed() << "ms";
}

int CW3Classification::GetAirThreshold(const CW3Image3D& vol,
	const std::vector<ushort>& mip_data,
	const std::vector<int>& mip_histo)
{
#if 0
	qDebug() << "start CW3Classification::GetAirThreshold";

	QElapsedTimer timer;
	timer.start();

	int height = vol.height();
	int depth = vol.depth();
	unsigned short vol_min = (unsigned short)((float)vol.windowCenter() -
		(float)vol.windowWidth() * 0.5f);

	std::vector<uchar> air_mask;
	air_mask.resize(mip_data.size(), 0);

	SetAirMaskData(vol_min, mip_data, mip_histo, air_mask);

	int air = static_cast<int>(GetAirRegionAvgMIP(mip_data, air_mask));

	int* histo = vol.getHistogram();
	int histo_size = vol.getHistoSize();

	if (histo_size < 2) return 0.0f;

	std::vector<int> smooth_histo;
#if SMOOTHING_HISTOGRAM
	int histogram_bin = static_cast<int>(static_cast<float>(histo_size) / kSmoothHistogramBinFactor);
	histogram_bin = std::max(histogram_bin, 1);
	int histogram_segment_count = kSmoothHistoSegCnt;
	Common::SmoothingHisto(histo, histo_size, smooth_histo, histogram_bin, histogram_segment_count, false);
#else
	smooth_histo.insert(smooth_histo.begin(), &histo[0], &histo[histo_size - 1]);
#endif

	int left_valley = SerchHistoValleyOnTheLeft(air, smooth_histo);
	int right_valley = SerchHistoValleyOnTheRight(air, smooth_histo);

	qDebug() << "end CW3Classification::GetAirThreshold :" << timer.elapsed() << "ms";

	if (smooth_histo[left_valley] < smooth_histo[right_valley])
	{
		return left_valley;
	}
	else
		return right_valley;
#else
	int* histogram = vol.getHistogram();
	int histogram_size = vol.getHistoSize();
	return ValleyEmphasisThresholder(histogram, 0, histogram_size);
#endif
}
int CW3Classification::SerchHistoValleyOnTheLeft(int start_pos,
	std::vector<int>& histo)
{
	qDebug() << "start CW3Classification::SerchHistoValleyOnTheLeft";

	QElapsedTimer timer;
	timer.start();

	if (histo.size() <= start_pos)
	{
		return 0;
	}

	int histo_min = std::numeric_limits<int>::max();
	int climbing = 0;
	int left_valley = 0;

	for (int i = start_pos; i >= 0; i--)
	{
		int curr_histo = histo[i];
		if (histo_min > curr_histo)
		{
			left_valley = i;
			histo_min = curr_histo;
		}

		if (abs(histo_min - curr_histo) > kClimbingThreshold)
		{
			climbing++;
		}

		if (climbing > kSerchValleyClimbMax) break;

		if ((start_pos - i) > kSerchValleyDistLimit)
		{
			left_valley = start_pos;
			break;
		}
	}

	qDebug() << "end CW3Classification::SerchHistoValleyOnTheLeft :" << timer.elapsed() << "ms";

	return left_valley;
}

int CW3Classification::SerchHistoValleyOnTheRight(int start_pos,
	std::vector<int>& histo)
{
	qDebug() << "start CW3Classification::SerchHistoValleyOnTheRight";

	QElapsedTimer timer;
	timer.start();

	if (histo.size() <= start_pos)
	{
		return 0;
	}

	int histo_min = std::numeric_limits<int>::max();
	int climbing = 0;
	int right_valley = 0;

	for (int i = start_pos; i < histo.size(); i++)
	{
		int curr_histo = histo[i];
		if (histo_min > curr_histo)
		{
			right_valley = i;
			histo_min = curr_histo;
		}

		if (abs(histo_min - curr_histo) > kClimbingThreshold)
		{
			climbing++;
		}

		if (climbing > kSerchValleyClimbMax) break;

		if ((i - start_pos) > kSerchValleyDistLimit)
		{
			right_valley = start_pos;
			break;
		}
	}

	qDebug() << "end CW3Classification::SerchHistoValleyOnTheRight :" << timer.elapsed() << "ms";

	return right_valley;
}

int CW3Classification::GetTeethThreshold(const CW3Image3D& vol)
{
	qDebug() << "start CW3Classification::GetTeethThreshold";

	QElapsedTimer timer;
	timer.start();

	int metal =
		vol.windowCenter() + ((float)vol.windowWidth()) * 0.001f - vol.intercept();
	metal = std::min(metal, vol.getHistoSize() - 1);
	int* histo = vol.getHistogram();

	float N = (float)(vol.width() * vol.height() * vol.depth());
	int n = 0;
	for (int i = metal; i > 0; i--)
	{
		n += histo[i];
		if ((float)n / N > 0.0005f)
		{
			qDebug() << "end CW3Classification::GetTeethThreshold :" << timer.elapsed() << "ms";
			return i;
		}
	}

	qDebug() << "end CW3Classification::GetTeethThreshold :" << timer.elapsed() << "ms";

	return metal;
}

int CW3Classification::GetPeakCntInHistogram(const std::vector<int>& histo,
	int start, int end, int peak_bin)
{
	qDebug() << "start CW3Classification::GetPeakCntInHistogram";

	QElapsedTimer timer;
	timer.start();

	int peak_cnt = 0;

	if (start - peak_bin < 0 || end + peak_bin >= histo.size())
	{
		return peak_cnt;
	}

	for (int i = start; i < end; i++)
	{
		if (histo[i] > histo[i - peak_bin] && histo[i] > histo[i + peak_bin])
		{
			peak_cnt++;
			i += peak_bin;
		}
	}

	qDebug() << "end CW3Classification::GetPeakCntInHistogram :" << timer.elapsed() << "ms";

	return peak_cnt;
}
int CW3Classification::GetTissueThreshold(const CW3Image3D& vol,
	int thd_air_tissue,
	int thd_bone_teeth)
{
	qDebug() << "start CW3Classification::GetTissueThreshold";

	QElapsedTimer timer;
	timer.start();

	int histo_size = vol.getHistoSize();
	int* histo = vol.getHistogram();

	if (histo_size < 2) return 0.0f;

	std::vector<int> smooth_histo;
#if SMOOTHING_HISTOGRAM
	int histogram_bin = static_cast<int>(static_cast<float>(histo_size) / kSmoothHistogramBinFactor);
	histogram_bin = std::max(histogram_bin, 1);
	int histogram_segment_count = kSmoothHistoSegCnt;
	Common::SmoothingHisto(histo, histo_size, smooth_histo, histogram_bin, histogram_segment_count, true);
#else
	smooth_histo.resize(histo_size, 0);

	int num_thread = omp_get_max_threads();
	omp_set_num_threads(num_thread);

#pragma omp parallel for
	for (int i = 0; i < histo_size; ++i)
	{
		smooth_histo[i] = pow(histo[i], 2);
	}
#endif

	int threshold = ValleyEmphasisThresholder(histo, thd_air_tissue + 1, thd_bone_teeth);
	int old_threshold = threshold;
	qDebug() << "OtsuThresholder :" << threshold;

#if 0
	qDebug() << "ValleyEmphasisThresholder :" << ValleyEmphasisThresholder(&smooth_histo[0], thd_air_tissue, thd_bone_teeth);
	qDebug() << "EntropyOtsuThresholder :" << EntropyOtsuThresholder(&smooth_histo[0], thd_air_tissue, thd_bone_teeth);
#endif

	const int kPeakBin = 30;
	if (GetPeakCntInHistogram(smooth_histo, thd_air_tissue, threshold, kPeakBin) >= 2)
	{
		threshold = ValleyEmphasisThresholder(&smooth_histo[0], thd_air_tissue + 1, (threshold + thd_bone_teeth) / 2);
		qDebug() << "threshold :" << threshold;
	}
#if 1
	threshold = std::max(old_threshold, threshold);
#endif

	qDebug() << "end CW3Classification::GetTissueThreshold :" << timer.elapsed() << "ms";

	return threshold;
}
void CW3Classification::SetAirMaskData(const ushort& vol_min,
	const std::vector<ushort>& mip_data,
	const std::vector<int>& mip_histo,
	std::vector<uchar>& out_air_mask)
{
	qDebug() << "start CW3Classification::SetAirMaskData";

	QElapsedTimer timer;
	timer.start();

	int N = mip_data.size();
	int n = 0;

	int t0 = OtsuThresholder(&mip_histo[0], 0, mip_histo.size());

	for (int i = t0; i < mip_histo.size(); i++) n += mip_histo[i];

	if (((float)n / (float)N) < 0.15f) t0 = OtsuThresholder(&mip_histo[0], 0, t0);

	int t1 = OtsuThresholder(&mip_histo[0], 0, t0);

	for (int i = 0; i < mip_data.size(); i++)
	{
		if (mip_data[i] < t0)
		{
			if (mip_data[i] < t1 && mip_data[i] > vol_min)
			{
				out_air_mask[i] = 255;
			}
		}
	}

	qDebug() << "end CW3Classification::SetAirMaskData :" << timer.elapsed() << "ms";
}
float CW3Classification::GetAirRegionAvgMIP(const std::vector<ushort>& mip_data,
	const std::vector<uchar> air_mask)
{
	qDebug() << "start CW3Classification::GetAirRegionAvgMIP";

	QElapsedTimer timer;
	timer.start();

	float air_region_avg_mip = 0.0f;
	int air_region_cnt = 0;

	for (int i = 0; i < mip_data.size(); i++)
	{
		if (air_mask[i])
		{
			air_region_avg_mip += (float)mip_data[i];
			air_region_cnt++;
		}
	}

	if (air_region_cnt)
		air_region_avg_mip = air_region_avg_mip / (float)air_region_cnt;
	else
		air_region_avg_mip = 0.0f;

	qDebug() << "end CW3Classification::GetAirRegionAvgMIP :" << timer.elapsed() << "ms";

	return air_region_avg_mip;
}
void CW3Classification::GenerateSagittalMIP(const CW3Image3D& vol,
	std::vector<ushort>& out_mip_data,
	std::vector<int>& out_mip_histo)
{
	qDebug() << "start CW3Classification::GenerateSagittalMIP";

	QElapsedTimer timer;
	timer.start();

	ushort sag_mip_max;

	SagittalMIPData(vol, out_mip_data, sag_mip_max);

	out_mip_histo.resize(sag_mip_max + 1, 0);
	for (const auto& elem : out_mip_data)
	{
		out_mip_histo[elem]++;
	}

	qDebug() << "end CW3Classification::GenerateSagittalMIP :" << timer.elapsed() << "ms";
}

void CW3Classification::SagittalMIPData(const CW3Image3D& vol,
	std::vector<ushort>& out_mip_data,
	ushort& out_mip_max)
{
	qDebug() << "start CW3Classification::SagittalMIPData";

	QElapsedTimer timer;
	timer.start();

	const int width = vol.width();
	const int height = vol.height();
	const int depth = vol.depth();

	out_mip_data.resize(height * depth);  // sagittal data size.
	std::memset(&out_mip_data[0], 0, sizeof(ushort)*height * depth);

	out_mip_max = 0;

	ushort** vol_data = vol.getData();

	const ushort min = vol.getMin();

	int num_thread = omp_get_max_threads();
	omp_set_num_threads(num_thread);

#pragma omp parallel for
	for (int z = 0; z < depth - 1; z += 2)
	{
		for (int y = 0; y < height - 1; y += 2)
		{
			ushort val_max = min;
			ushort* slice_data = &vol_data[z][y*width];
			for (int x = 0; x < width - 1; x += 2)
			{
				ushort val = *slice_data;
				slice_data += 2;
				val_max = std::max(val_max, val);
			}

			out_mip_data[y + height * z] = (val_max);
			out_mip_data[y + 1 + height * z] = (val_max);
			out_mip_data[y + height * (z + 1)] = (val_max);
			out_mip_data[y + 1 + height * (z + 1)] = (val_max);
		}
	}

	for (int i = 0; i < height*depth; i++)
	{
		out_mip_max = std::max(out_mip_data[i], out_mip_max);
	}

	qDebug() << "end CW3Classification::SagittalMIPData :" << timer.elapsed() << "ms";
}

int CW3Classification::OtsuThresholder(const int* histogram, int min, int max)
{
	qDebug() << "start CW3Classification::OtsuThresholder";

	QElapsedTimer timer;
	timer.start();

	int threshold = 0;

	float sum = 0.0f;
	int total = 0.0;

	for (int i = min; i < max; i++)
	{
		sum += (float)(i * histogram[i]);
		total += histogram[i];
	}

	float sumB = 0.0f;
	int wB = 0;
	int wF = 0;

	float varMax = 0.0f;

	for (int i = min; i < max; i++)
	{
		wB += histogram[i];

		if (wB == 0) continue;

		wF = total - wB;

		if (wF == 0) break;

		sumB += (float)(histogram[i] * i);

		float mB = sumB / wB;
		float mF = (sum - sumB) / wF;

		float between = (float)wB * (float)wF * (mB - mF) * (mB - mF);

		if (between > varMax)
		{
			varMax = between;
			threshold = i;
		}
	}

	qDebug() << "end CW3Classification::OtsuThresholder :" << timer.elapsed() << "ms";

	return threshold;
}

int CW3Classification::EntropyOtsuThresholder(const int* histogram, int min, int max)
{
	qDebug() << "start CW3Classification::EntropyOtsuThresholder";

	QElapsedTimer timer;
	timer.start();

	float hn = 0.0f;
	float sum = 0.0f;
	int total = 0;

	float* p = new float[max - min + 1];
	memset(p, 0.0f, max - min + 1);

	for (int i = min; i < max; i++)
	{
		total += histogram[i];
	}

	for (int i = min + 1; i < max; i++)
	{
		int t = i - min;
		p[t] = static_cast<float>(histogram[i]) / total;
		if (p[t] != 0.0f)
		{
			hn += p[t] * std::log(p[t]);
		}

		sum += static_cast<float>((t - 1) * p[t]);
	}
	hn = -hn;

	float max_value = 0.0f;
	int threshold = 0;

	float omega_1 = 0;
	float mu_k = 0.0f;
	float ps = 0.0f;
	float hs = 0.0f;

	for (int i = min + 1; i < max; i++)
	{
		int t = i - min;
		omega_1 += p[t];
		float omega_2 = 1.0f - omega_1;
		mu_k += static_cast<float>((t - 1) * histogram[i]) / total;
		float mu_1 = mu_k / omega_1;
		float mu_2 = (sum - mu_k) / omega_2;
		ps += p[t];

		if (p[t] != 0.0f)
		{
			hs -= p[t] * std::log(p[t]);
		}

		float psi = std::log(ps * (1.0f - ps)) + hs / ps + (hn - hs) / (1.0f - ps);
		float current = psi * (omega_1 * (mu_1 * mu_1) + omega_2 * (mu_2 * mu_2));

		if (current > max_value)
		{
			max_value = current;
			threshold = i - 1;
		}
	}

	delete[] p;

	qDebug() << "end CW3Classification::EntropyOtsuThresholder :" << timer.elapsed() << "ms";

	return threshold;
}

int CW3Classification::ValleyEmphasisThresholder(const int* histogram, int min, int max)
{
	qDebug() << "start CW3Classification::ValleyEmphasisThresholder";

	QElapsedTimer timer;
	timer.start();

	int threshold = 0;
	float sum = 0.0f;
	int number_of_pixels = 0.0;
	float sum_b = 0.0f;
	int w_b = 0;
	int w_f = 0;
	float max_value = 0.0f;

	for (int i = min; i < max; i++)
	{
		sum += (float)(i * histogram[i]);
		number_of_pixels += histogram[i];
	}

	for (int i = min; i < max; i++)
	{
		w_b += histogram[i];

		if (w_b == 0)
		{
			continue;
		}

		w_f = number_of_pixels - w_b;

		if (w_f == 0)
		{
			break;
		}

		sum_b += (float)(histogram[i] * i);

		float m_b = sum_b / w_b;
		float m_f = (sum - sum_b) / w_f;

		float p = static_cast<float>(histogram[i]) / static_cast<float>(number_of_pixels);

		float between = (1.0f - p) * (float)w_b * (float)w_f * (m_b - m_f) * (m_b - m_f);

		if (between > max_value)
		{
			max_value = between;
			threshold = i;
		}
	}

	qDebug() << "end CW3Classification::ValleyEmphasisThresholder :" << timer.elapsed() << "ms";

	return threshold;
}

#include <algorithm>
std::vector<double> CW3Classification::OtsuFilterMulti(const int* histogram, int histogram_size, int bins, int levels)
{
	unsigned int num_elem = 0;
	std::vector<double> threshold;
	std::vector<double> q;
	std::vector<double> mu;
	std::vector<double> muk;
	std::vector<uint> binv;
	threshold.resize(levels, 0.0);
	q.resize(levels + 1, 0.0);
	mu.resize(levels + 1, 0.0);
	muk.resize(levels + 1, 0.0);
	binv.resize(levels, 0);

	if (levels <= 1)
	{
		return threshold;
	}

	for (int i = 0; i < histogram_size; ++i)
	{
		num_elem += histogram[i];
	}

	double maxval = histogram_size - 1;
	double minval = 0;
	double odelta = (maxval - abs(minval)) / bins;     // distance between histogram bins

	std::vector<double> oval;
	oval.resize(bins, 0.0);
	double mt = 0, variance = 0.0, bestVariance = 0.0;

	for (int ii = 0; ii < bins; ii++)
	{
		oval[ii] = (double)odelta*ii + (double)odelta*0.5;  // centers of histogram bins
		mt += (double)ii*((double)histogram[ii]) / (double)num_elem;
	}

	for (int ii = 0; ii < levels; ii++)
	{
		binv[ii] = ii;
	}

	double sq, smuk;
	int nComb;

	nComb = Combinations(bins, levels);
	std::vector<bool> v(bins);
	std::fill(v.begin(), v.begin() + levels, true);

	std::vector<std::vector<uint>> ibin; // indices from combinations will be stored here
	for (int i = 0; i < nComb; ++i)
	{
		std::vector<uint> temp;
		temp.resize(levels, 0);
		ibin.push_back(temp);
	}

	int cc = 0;
	int ci = 0;
	do
	{
		for (int i = 0; i < bins; ++i)
		{
			if (ci == levels) ci = 0;
			if (v[i])
			{
				ibin[cc][ci] = i;
				ci++;
			}
		}
		cc++;
	} while (std::prev_permutation(v.begin(), v.end()));

	std::vector<uint> lastIndex;
	lastIndex.resize(levels, 0);

	// Perform operations on pre-calculated indices
	for (int ii = 0; ii < nComb; ii++)
	{
		for (int jj = 0; jj < levels; jj++)
		{
			smuk = 0;
			sq = 0;
			if (lastIndex[jj] != ibin[ii][jj] || ii == 0)
			{
				q[jj] += double(histogram[ibin[ii][jj]]) / (double)num_elem;
				muk[jj] += ibin[ii][jj] * (double(histogram[ibin[ii][jj]])) / (double)num_elem;
				mu[jj] = muk[jj] / q[jj];
				q[jj + 1] = 0.0;
				muk[jj + 1] = 0.0;

				if (jj > 0)
				{
					for (int kk = 0; kk <= jj; kk++)
					{
						sq += q[kk];
						smuk += muk[kk];
					}
					q[jj + 1] = 1 - sq;
					muk[jj + 1] = mt - smuk;
					mu[jj + 1] = muk[jj + 1] / q[jj + 1];
				}
				if (jj > 0 && jj < (levels - 1))
				{
					q[jj + 1] = 0.0;
					muk[jj + 1] = 0.0;
				}

				lastIndex[jj] = ibin[ii][jj];
			}
		}

		variance = 0.0;
		for (int jj = 0; jj <= levels; jj++)
		{
			variance += q[jj] * (mu[jj] - mt)*(mu[jj] - mt);
		}

		if (variance > bestVariance)
		{
			bestVariance = variance;
			for (int jj = 0; jj < levels; jj++)
			{
				threshold[jj] = oval[ibin[ii][jj]];
			}
		}
	}

	qDebug() << "Optimized thresholds: ";
	for (int jj = 0; jj < levels; jj++)
	{
		qDebug() << threshold[jj];
	}

	return threshold;
}

int CW3Classification::Combinations(int n, int r)
{
	if (r > n) return 0;
	if (r * 2 > n) r = n - r;
	if (r == 0) return 1;

	int ret = n;
	for (int i = 2; i <= r; ++i)
	{
		ret *= (n - i + 1);
		ret /= i;
	}
	return ret;
}

inline std::vector<qreal> BuildTables(const int* histogram, int histogram_size)
{
	// Create cumulative sum tables.
	QVector<quint64> P(histogram_size + 1);
	QVector<quint64> S(histogram_size + 1);
	P[0] = 0;
	S[0] = 0;

	quint64 sum_P = 0;
	quint64 sum_S = 0;

	for (int i = 0; i < histogram_size; i++)
	{
		sum_P += quint64(histogram[i]);
		sum_S += quint64(i * histogram[i]);
		P[i + 1] = sum_P;
		S[i + 1] = sum_S;
	}

	// Calculate the between-class variance for the interval u-v
#if 1
	std::vector<qreal> H;
	H.resize(qPow(histogram_size, 2), 0.0f);
#else
	QVector<qreal> H(qPow(histogram_size, 2), 0.0f);
#endif

	for (int u = 0; u < histogram_size; u++)
	{
		qreal *h_line = H.data() + (u * histogram_size);

		for (int v = u + 1; v < histogram_size; v++)
		{
			h_line[v] = qPow(S[v] - S[u], 2) / (P[v] - P[u]);
		}
	}

	return H;
}

void ForLoop(qreal* max_sum,
	QVector<int>* thresholds,
	const std::vector<qreal>& H,
	int u,
	int vmax,
	int level,
	int levels,
	QVector<int>* index)
{
	int classes = index->size() - 1;

	for (int i = u; i < vmax; i++)
	{
		(*index)[level] = i;

		if (level + 1 >= classes)
		{
			// Reached the end of the for loop.

			// Calculate the quadratic sum of al intervals.
			qreal sum = 0.0;

			for (int c = 0; c < classes; c++)
			{
				int u = index->at(c);
				int v = index->at(c + 1);
				sum += H[v + u * levels];
			}

			if (*max_sum < sum)
			{
				// Return calculated threshold.
				*thresholds = index->mid(1, thresholds->size());
				*max_sum = sum;
			}
		}
		else
			// Start a new for loop level, one position after current one.
			ForLoop(max_sum,
				thresholds,
				H,
				i + 1,
				vmax + 1,
				level + 1,
				levels,
				index);
	}
}

std::vector<int> CW3Classification::MultiThreshold(const int* histogram, int histogram_size, int levels, int min)
{
	int classes = levels;
	qreal max_sum = 0.;
	QVector<int> thresholds(classes - 1, 0);
	std::vector<qreal> H = BuildTables(histogram, histogram_size);
	QVector<int> index(classes + 1);
	index[0] = 0;
	index[index.size() - 1] = histogram_size - 1;

	ForLoop(&max_sum,
		&thresholds,
		H,
		1,
		histogram_size - classes + 1,
		1,
		histogram_size, &index);

	std::vector<int> results;
	for (int i = 0; i < thresholds.size(); ++i)
	{
		qDebug() << i << "thresholds :" << thresholds.at(i);
		results.push_back(thresholds.at(i));
	}

	return results;
}
