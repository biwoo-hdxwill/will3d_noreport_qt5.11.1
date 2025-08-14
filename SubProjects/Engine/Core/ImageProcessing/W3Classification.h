#pragma once
/*=========================================================================

File:			class CW3Classification
Language:		C++11
Library:		Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2017-02-14
Last modify:	2017-09-28

=========================================================================*/
#include <vector>
#include "imageprocessing_global.h"

class CW3Image3D;
class QPointF;

class IMAGEPROCESSING_EXPORT CW3Classification
{
public:
  static void ThresholderVolume(const CW3Image3D& vol, int* thd_air_tissue, int* thd_tissue_bone, int* thd_bone_teeth);

private:
  static int GetAirThreshold(const CW3Image3D& vol, const std::vector<ushort>& mip_data, const std::vector<int>& mip_histo);
  static int SerchHistoValleyOnTheLeft(int start_pos, std::vector<int>& histo);
  static int SerchHistoValleyOnTheRight(int start_pos, std::vector<int>& histo);
  static int GetTeethThreshold(const CW3Image3D& vol);
  static int GetPeakCntInHistogram(const std::vector<int>& histo, int start, int end, int peak_bin);
  static int GetTissueThreshold(const CW3Image3D& vol, int thd_air_tissue, int thd_bone_teeth);

  static void SetAirMaskData(const ushort& vol_min, const std::vector<ushort>& mip_data, const std::vector<int>& mip_histo, std::vector<uchar>& out_air_mask);
  static float GetAirRegionAvgMIP(const std::vector<ushort>& mip_data, const std::vector<uchar> air_mask);

  static void GenerateSagittalMIP(const CW3Image3D& vol, std::vector<ushort>& out_mip_data, std::vector<int>& out_mip_histo);
  static void SagittalMIPData(const CW3Image3D & vol, std::vector<ushort>& out_mip_data, ushort& out_mip_max);

  static int OtsuThresholder(const int* histogram, int min, int max);
  static int EntropyOtsuThresholder(const int* histogram, int min, int max);
  static int ValleyEmphasisThresholder(const int* histogram, int min, int max);

  static std::vector<double> OtsuFilterMulti(const int* histogram, int histogram_size, int bins, int levels);
  static int Combinations(int n, int r);

  static std::vector<int> MultiThreshold(const int* histogram, int histogram_size, int levels, int min = 0);
};
