/*=========================================================================

File:			class Common
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-11-21
Last date:		2016-04-26

Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/

#include "common.h"

#include <fstream>
#include <sstream>

#if defined(__APPLE__)
#include </usr/local/Cellar/llvm/5.0.0/lib/clang/5.0.0/include/omp.h>
#else
#include <omp.h>
#endif

#include <QDebug>
#include <QElapsedTimer>

#include "W3Matrix.h"
#include "W3Memory.h"
#include "W3Vector.h"

Common::Common() {}

Common::~Common() {}

void Common::generateCubicSpline(std::vector<QPointF>& lstControlPoints,
	std::vector<QPointF>& vecPointList,
	int nLineSeg)
{
	qDebug() << "start Common::generateCubicSpline";

	int num_thread = omp_get_max_threads();
	omp_set_num_threads(num_thread);

	QElapsedTimer timer;
	timer.start();

	int nNumOfCtrlPt = lstControlPoints.size();

	qDebug() << "nNumOfCtrlPt :" << nNumOfCtrlPt;

	///////////////////////////////////////////
	//	Natural Cubic Spline Matrix
	///////////////////////////////////////////
	double matEntry[3] = { 1.0 / 6.0, -2.0 / 6.0, 1.0 / 6.0 };
	double matEntry2[3] = { 1.0 / 6.0, 4.0 / 6.0, 1.0 / 6.0 };

	float** fMatrix =
		SAFE_ALLOC_VOLUME(float, nNumOfCtrlPt + 2, nNumOfCtrlPt + 2);

#pragma omp parallel for
	for (int i = 0; i < nNumOfCtrlPt + 2; i++)
	{
		memset(fMatrix[i], 0, sizeof(float) * (nNumOfCtrlPt + 2));
	}

#pragma omp parallel for
	for (int i = 0; i < nNumOfCtrlPt + 2; ++i)
	{
		for (int j = 0; j < nNumOfCtrlPt + 2; ++j)
		{
			if (i == 0)
			{
				if (j - 3 < 0) fMatrix[i][j] = matEntry[j];
			}
			else if (i == nNumOfCtrlPt + 1)
			{
				if (nNumOfCtrlPt - j - 2 < 0)
					fMatrix[i][j] = matEntry[j - nNumOfCtrlPt + 1];
			}
			else
			{
				if (j >= i - 1 && j <= i - 1 + 2) fMatrix[i][j] = matEntry2[j - i + 1];
			}
		}
	}

	///////////////////////////////////////////
	// Solve Linear System
	///////////////////////////////////////////
	CW3Matrix A;  // matrixN A;
	A.SetSize(nNumOfCtrlPt + 2, nNumOfCtrlPt + 2);

#pragma omp parallel for
	for (int i = 0; i < nNumOfCtrlPt + 2; ++i)
	{
		for (int j = 0; j < nNumOfCtrlPt + 2; ++j)
		{
			A.SetValue(i, j, fMatrix[i][j]);
		}
	}

	double* px = SAFE_ALLOC_1D(double, nNumOfCtrlPt + 2);
	double* py = SAFE_ALLOC_1D(double, nNumOfCtrlPt + 2);
	double* bx = SAFE_ALLOC_1D(double, nNumOfCtrlPt + 2);
	double* by = SAFE_ALLOC_1D(double, nNumOfCtrlPt + 2);

#pragma omp parallel for
	for (int i = 0; i < nNumOfCtrlPt; ++i)
	{
		px[i + 1] = lstControlPoints.at(i).x();
		py[i + 1] = lstControlPoints.at(i).y();
	}

	px[0] = 0.0;
	py[0] = 0.0;
	px[nNumOfCtrlPt + 1] = 0.0;
	py[nNumOfCtrlPt + 1] = 0.0;

	CW3Vector pxVec;
	pxVec.SetSize(nNumOfCtrlPt + 2);
	pxVec.SetValue(px);

	CW3Vector bxVec;
	bxVec.SetSize(nNumOfCtrlPt + 2);
	bxVec.Solve(A, pxVec);

	CW3Vector pyVec;
	pyVec.SetSize(nNumOfCtrlPt + 2);
	pyVec.SetValue(py);

	CW3Vector byVec;
	byVec.SetSize(nNumOfCtrlPt + 2);
	byVec.Solve(A, pyVec);

	for (int i = 0; i < nNumOfCtrlPt + 2; ++i)
	{
		bx[i] = bxVec[i];
		by[i] = byVec[i];
	}

	double sampling_rate = 1.0 / nLineSeg;
	vecPointList.resize((nNumOfCtrlPt - 1) * nLineSeg + 1);

#pragma omp parallel for
	for (int i = 0; i < nNumOfCtrlPt - 1; ++i)
	{
		const double bx1 = bx[i];
		const double bx2 = bx[i + 1];
		const double bx3 = bx[i + 2];
		const double bx4 = bx[i + 3];
		const double by1 = by[i];
		const double by2 = by[i + 1];
		const double by3 = by[i + 2];
		const double by4 = by[i + 3];

		for (int j = 0; j < nLineSeg; ++j)
		{
			double pow_j_sampling_3 = pow(j * sampling_rate, 3.0);
			double pow_j_sampling_2 = pow(j * sampling_rate, 2.0);
			double one_pow_j_sampling_3 = pow(1.0 - j * sampling_rate, 3.0);
			QPointF temp;
			temp.setX(
				(bx1 * one_pow_j_sampling_3 / 6.0 +
					bx2 * (3.0 * pow_j_sampling_3 - 6.0 * pow_j_sampling_2 + 4.0) / 6.0 +
					bx3 * (-3.0 * pow_j_sampling_3 + 3.0 * pow_j_sampling_2 + 3.0 * j * sampling_rate + 1.0) / 6.0 +
					bx4 * pow_j_sampling_3 / 6.0));
			temp.setY(
				(by1 * one_pow_j_sampling_3 / 6.0 +
					by2 * (3.0 * pow_j_sampling_3 - 6.0 * pow_j_sampling_2 + 4.0) / 6.0 +
					by3 * (-3.0 * pow_j_sampling_3 + 3.0 * pow_j_sampling_2 + 3.0 * j * sampling_rate + 1.0) / 6.0 +
					by4 * pow_j_sampling_3 / 6.0));
			vecPointList[i * nLineSeg + j] = (temp);
			if (i == nNumOfCtrlPt - 2 && j == nLineSeg - 1)
			{
				pow_j_sampling_3 = pow((j + 1) * sampling_rate, 3.0);
				pow_j_sampling_2 = pow((j + 1) * sampling_rate, 2.0);
				one_pow_j_sampling_3 = pow(1.0 - (j + 1) * sampling_rate, 3.0);
				temp.setX((bx[i + 0] * one_pow_j_sampling_3 / 6.0 +
					bx[i + 1] *
					(3.0 * pow_j_sampling_3 - 6.0 * pow_j_sampling_2 + 4.0) /
					6.0 +
					bx[i + 2] *
					(-3.0 * pow_j_sampling_3 + 3.0 * pow_j_sampling_2 +
						3.0 * (j + 1) * sampling_rate + 1.0) /
					6.0 +
					bx[i + 3] * pow_j_sampling_3 / 6.0));
				temp.setY((by[i + 0] * one_pow_j_sampling_3 / 6.0 +
					by[i + 1] *
					(3.0 * pow_j_sampling_3 - 6.0 * pow_j_sampling_2 + 4.0) /
					6.0 +
					by[i + 2] *
					(-3.0 * pow_j_sampling_3 + 3.0 * pow_j_sampling_2 +
						3.0 * (j + 1) * sampling_rate + 1.0) /
					6.0 +
					by[i + 3] * pow_j_sampling_3 / 6.0));
				vecPointList[i * nLineSeg + j + 1] = (temp);
			}
		}
	}

	SAFE_DELETE_VOLUME(fMatrix, nNumOfCtrlPt + 2);
	SAFE_DELETE_ARRAY(px);
	SAFE_DELETE_ARRAY(bx);
	SAFE_DELETE_ARRAY(py);
	SAFE_DELETE_ARRAY(by);

	qDebug() << "end Common::generateCubicSpline :" << timer.elapsed() << "ms";
}
void Common::generateCubicSpline(std::vector<glm::vec3>& lstControlPoints,
	std::vector<glm::vec3>& vecPointList,
	int nLineSeg)
{
	vecPointList.clear();
	int nNumOfCtrlPt = lstControlPoints.size();
	int i, j;
	///////////////////////////////////////////
	//	Natural Cubic Spline Matrix
	///////////////////////////////////////////
	double matEntry[3] = { 1 / 6.f, -2 / 6.f, 1 / 6.f };
	double matEntry2[3] = { 1 / 6.f, 4 / 6.f, 1 / 6.f };

	double** fMatrix = new double*[nNumOfCtrlPt + 2];
	for (i = 0; i < nNumOfCtrlPt + 2; i++)
	{
		fMatrix[i] = new double[nNumOfCtrlPt + 2];
		memset(fMatrix[i], 0, sizeof(double) * (nNumOfCtrlPt + 2));
	}

	for (i = 0; i < nNumOfCtrlPt + 2; i++)
	{
		for (j = 0; j < nNumOfCtrlPt + 2; j++)
		{
			if (i == 0)
			{
				if (j - 3 < 0) fMatrix[i][j] = matEntry[j];
			}
			else if (i == nNumOfCtrlPt + 1)
			{
				if (nNumOfCtrlPt - j - 2 < 0)
					fMatrix[i][j] = matEntry[j - nNumOfCtrlPt + 1];
			}
			else
			{
				if (j >= i - 1 && j <= i - 1 + 2) fMatrix[i][j] = matEntry2[j - i + 1];
			}
		}
	}

	///////////////////////////////////////////
	// Solve Linear System
	///////////////////////////////////////////
	CW3Matrix A;  // matrixN A;
	A.SetSize(nNumOfCtrlPt + 2, nNumOfCtrlPt + 2);
	for (i = 0; i < nNumOfCtrlPt + 2; i++)
	{
		for (j = 0; j < nNumOfCtrlPt + 2; j++)
		{
			A.SetValue(i, j, fMatrix[i][j]);
		}
	}

	double* px = new double[nNumOfCtrlPt + 2];
	double* bx = new double[nNumOfCtrlPt + 2];
	double* py = new double[nNumOfCtrlPt + 2];
	double* by = new double[nNumOfCtrlPt + 2];
	double* pz = new double[nNumOfCtrlPt + 2];
	double* bz = new double[nNumOfCtrlPt + 2];

	for (i = 0; i < nNumOfCtrlPt; i++)
	{
		px[i + 1] = (double)(lstControlPoints[i].x);
		py[i + 1] = (double)(lstControlPoints[i].y);
		pz[i + 1] = (double)(lstControlPoints[i].z);
	}
	px[0] = 0;
	py[0] = 0;
	pz[0] = 0;
	px[nNumOfCtrlPt + 1] = 0;
	py[nNumOfCtrlPt + 1] = 0;
	pz[nNumOfCtrlPt + 1] = 0;

	CW3Vector pxVec;
	pxVec.SetSize(nNumOfCtrlPt + 2);
	pxVec.SetValue(px);

	CW3Vector pyVec;
	pyVec.SetSize(nNumOfCtrlPt + 2);
	pyVec.SetValue(py);

	CW3Vector pzVec;
	pzVec.SetSize(nNumOfCtrlPt + 2);
	pzVec.SetValue(pz);

	CW3Vector bxVec;
	bxVec.SetSize(nNumOfCtrlPt + 2);
	bxVec.Solve(A, pxVec);
	for (i = 0; i < nNumOfCtrlPt + 2; i++)
	{
		bx[i] = bxVec[i];
	}

	CW3Vector byVec;
	byVec.SetSize(nNumOfCtrlPt + 2);
	byVec.Solve(A, pyVec);
	for (i = 0; i < nNumOfCtrlPt + 2; i++)
	{
		by[i] = byVec[i];
	}

	CW3Vector bzVec;
	bzVec.SetSize(nNumOfCtrlPt + 2);
	bzVec.Solve(A, pzVec);
	for (i = 0; i < nNumOfCtrlPt + 2; i++)
	{
		bz[i] = bzVec[i];
	}

	///////////////////////////////////////////
	//	Generate 3D Curve Point
	///////////////////////////////////////////

	float sampling_rate = (float)1 / nLineSeg;

	for (i = 0; i < nNumOfCtrlPt - 1; i++)
	{
		for (j = 0; j < nLineSeg; j++)
		{
			float pow_j_sampling_3 = pow(j * sampling_rate, 3.0f);
			float pow_j_sampling_2 = pow(j * sampling_rate, 2.0f);
			float one_pow_j_sampling_3 = pow(1.0f - j * sampling_rate, 3.0f);
			glm::vec3 temp;
			temp.x =
				((bx[i] * one_pow_j_sampling_3 / 6.0f +
					bx[i + 1] *
					(3.0f * pow_j_sampling_3 - 6.0f * pow_j_sampling_2 + 4.0f) /
					6.0f +
					bx[i + 2] *
					(-3.0f * pow_j_sampling_3 + 3.0f * pow_j_sampling_2 +
						3.0f * j * sampling_rate + 1.0f) /
					6.0f +
					bx[i + 3] * pow_j_sampling_3 / 6.0f));
			temp.y =
				((by[i + 0] * pow(1 - j * sampling_rate, 3.0f) / 6.0f +
					by[i + 1] *
					(3.0f * pow_j_sampling_3 - 6.0f * pow_j_sampling_2 + 4.0f) /
					6.0f +
					by[i + 2] *
					(-3.0f * pow_j_sampling_3 + 3.0f * pow_j_sampling_2 +
						3.0f * j * sampling_rate + 1.0f) /
					6.0f +
					by[i + 3] * pow_j_sampling_3 / 6.0f));
			temp.z =
				((bz[i + 0] * one_pow_j_sampling_3 / 6.0f +
					bz[i + 1] *
					(3.0f * pow_j_sampling_3 - 6.0f * pow_j_sampling_2 + 4.0f) /
					6.0f +
					bz[i + 2] *
					(-3.0f * pow_j_sampling_3 + 3.0f * pow_j_sampling_2 +
						3.0f * j * sampling_rate + 1.0f) /
					6.0f +
					bz[i + 3] * pow_j_sampling_3 / 6.0f));
			vecPointList.push_back(temp);
			if (i == nNumOfCtrlPt - 2 && j == nLineSeg - 1)
			{
				pow_j_sampling_3 = pow((j + 1) * sampling_rate, 3.0f);
				pow_j_sampling_2 = pow((j + 1) * sampling_rate, 2.0f);
				one_pow_j_sampling_3 = pow(1.0f - (j + 1) * sampling_rate, 3.0f);
				glm::vec3 temp;
				temp.x =
					((bx[i + 0] * one_pow_j_sampling_3 / 6.0f +
						bx[i + 1] *
						(3.0f * pow_j_sampling_3 - 6.0f * pow_j_sampling_2 + 4.0f) /
						6.0f +
						bx[i + 2] *
						(-3.0f * pow_j_sampling_3 + 3.0f * pow_j_sampling_2 +
							3.0f * (j + 1) * sampling_rate + 1.0f) /
						6.0f +
						bx[i + 3] * pow_j_sampling_3 / 6.0f));
				temp.y =
					((by[i + 0] * one_pow_j_sampling_3 / 6.0f +
						by[i + 1] *
						(3.0f * pow_j_sampling_3 - 6.0f * pow_j_sampling_2 + 4.0f) /
						6.0f +
						by[i + 2] *
						(-3.0f * pow_j_sampling_3 + 3.0f * pow_j_sampling_2 +
							3.0f * (j + 1) * sampling_rate + 1.0f) /
						6.0f +
						by[i + 3] * pow_j_sampling_3 / 6.0f));
				temp.z =
					((bz[i + 0] * pow(1 - (j + 1) * sampling_rate, 3.0f) / 6.0f +
						bz[i + 1] *
						(3.0f * pow_j_sampling_3 - 6.0f * pow_j_sampling_2 + 4.0f) /
						6.0f +
						bz[i + 2] *
						(-3.0f * pow_j_sampling_3 + 3.0f * pow_j_sampling_2 +
							3.0f * (j + 1) * sampling_rate + 1.0f) /
						6.0f +
						bz[i + 3] * pow_j_sampling_3 / 6.0f));
				vecPointList.push_back(temp);
			}
		}
	}

	for (i = 0; i < nNumOfCtrlPt + 2; i++) SAFE_DELETE_ARRAY(fMatrix[i]);
	SAFE_DELETE_ARRAY(fMatrix);
	SAFE_DELETE_ARRAY(px);
	SAFE_DELETE_ARRAY(bx);
	SAFE_DELETE_ARRAY(py);
	SAFE_DELETE_ARRAY(by);
	SAFE_DELETE_ARRAY(pz);
	SAFE_DELETE_ARRAY(bz);
}

int Common::projPointToSpline(const std::vector<glm::vec3> lstSpline,
	const glm::vec3 point,
	const glm::vec3 planeVector,
	const float projRange)
{
	if (lstSpline.size() < 2) return -1;

	std::vector<glm::vec3> pts = lstSpline;

	glm::vec3 tail = pts.back() * 2.0f - pts.at(pts.size() - 2);
	pts.push_back(tail);

	float fMinDist = std::numeric_limits<float>::max();

	int nSplineIdx;
	for (int i = 0; i < (int)pts.size() - 1; i++)
	{
		glm::vec3 v1 = pts.at(i);
		glm::vec3 v2 = pts.at(i + 1);

		glm::vec3 rightVector = glm::normalize(v2 - v1);
		glm::vec3 upVector = glm::normalize(glm::cross(planeVector, rightVector));

		glm::vec3 vec = point - pts.at(i);
		float fUpDist = glm::dot(vec, upVector);
		glm::vec3 ptProj = vec - upVector * (fUpDist);

		if (abs(fUpDist) > projRange) continue;

		float fProjDist = glm::length(ptProj);

		if (fProjDist < fMinDist)
		{
			fMinDist = fProjDist;
			nSplineIdx = i;
		}
	}

	return nSplineIdx;
}

void Common::equidistanceSpline(std::vector<glm::vec3>& out,
	const std::vector<glm::vec3>& in)
{
	out.clear();
	out.push_back(in.front());

	for (int i = 1; i < in.size(); i++)
	{
		glm::vec3 p1 = out.back();
		glm::vec3 p2 = in.at(i);
		glm::vec3 vec = p2 - p1;

		int len = static_cast<int>(glm::length(vec));

		vec = glm::normalize(vec);

		for (int j = 0; j < len; j++)
		{
			out.push_back(p1 + vec * float(j + 1));
		}
	}
}

void Common::equidistanceSpline(std::vector<QPointF>& out,
	const std::vector<QPointF>& in)
{
	out.clear();
	out.push_back(in.front());

	for (int i = 1; i < in.size(); i++)
	{
		QPointF p1 = out.back();
		QPointF p2 = in.at(i);
		QPointF vec = p2 - p1;

		double len = sqrt(vec.x() * vec.x() + vec.y() * vec.y());
		int int_len = (int)len;

		vec = vec / len;

		for (int j = 0; j < int_len; j++)
		{
			out.push_back(p1 + vec * float(j + 1));
		}
	}
}
void Common::SmoothingHisto(int* histo, int histo_size,
	std::vector<int>& smooth_histo, int histo_bin,
	int seg_histo, bool is_sqrt_histo)
{
	qDebug() << "start Common::SmoothingHisto";

	QElapsedTimer timer;
	timer.start();

#if 1
	// bin 구간에서 max를 찾아서 인덱스와 값을 curve point로 설정.
	std::vector<QPointF> curve_points;
	GetBinningCurve(histo, histo_size, histo_bin, curve_points, is_sqrt_histo);

	// curve point로 spline을 구함.
	std::vector<QPointF> spline;
	Common::generateCubicSpline(curve_points, spline, seg_histo);

	// spline을 histogram index 축의 크기 1만큼씩 sampling한다.
	std::vector<QPointF> histo_curve;
	SamplingSpline(spline, histo_curve);

	// sampling한 histogram curve를 smoothing histogram으로 설정.
	smooth_histo.resize(histo_size, 0);

	const int histo_size_m1 = histo_size - 1;
	for (const auto& elem : histo_curve)
	{
		int x = (int)elem.x();
		if (x >= histo_size) x = histo_size_m1;
		if (x < 0) x = 0;
		smooth_histo[x] = std::max(0, static_cast<int>(elem.y()));
	}
#else
	smooth_histo.resize(histo_size, 0);
	for (int i = 0; i < histo_size; ++i)
	{
		smooth_histo[i] = histo[i];
	}
#endif

	qDebug() << "end Common::SmoothingHisto :" << timer.elapsed() << "ms";
}

void Common::GetBinningCurve(int* histo, int histo_size, int histo_bin,
	std::vector<QPointF>& curve_points, bool is_sqrt)
{
	qDebug() << "start Common::GetBinningCurve";

	QElapsedTimer timer;
	timer.start();

#if 0
	if (is_sqrt)
	{
		for (int i = 1; i < histo_size - histo_bin; i += histo_bin)
		{
			std::vector<int> median_nums;
			median_nums.reserve(histo_bin);
			for (int k = 0; k < histo_bin; k++)
			{
				median_nums.push_back(static_cast<int>(sqrtf(static_cast<float>(histo[i + k]))));
			}
			std::sort(median_nums.begin(), median_nums.end());
			int mid_idx = median_nums.size() / 2;

			curve_points.push_back(QPointF(
				static_cast<double>(i), static_cast<double>(median_nums[mid_idx])));
		}
	}
	else
	{
		for (int i = 1; i < histo_size - histo_bin; i += histo_bin)
		{
			std::vector<int> median_nums;
			median_nums.reserve(histo_bin);
			for (int k = 0; k < histo_bin; k++)
			{
				median_nums.push_back(histo[i + k]);
			}
			std::sort(median_nums.begin(), median_nums.end());
			int mid_idx = median_nums.size() / 2;

			curve_points.push_back(
				QPointF(static_cast<double>(i), static_cast<double>(median_nums[mid_idx])));
		}
	}
#else
		for (int i = 0; i < histo_size - histo_bin; i += histo_bin)
		{
			std::vector<int> median_nums;
			median_nums.reserve(histo_bin);
			int sum = 0;
			for (int k = 0; k < histo_bin; k++)
			{
				int val = is_sqrt ? sqrt(histo[i + k]) : histo[i + k];
				median_nums.push_back(val);
				sum += val;
			}
			std::sort(median_nums.begin(), median_nums.end());
			int mid_idx = median_nums.size() / 2;

			int mean = sum / median_nums.size();

			curve_points.push_back(QPointF(static_cast<double>(i), mean));
		}
#endif

	qDebug() << "end Common::GetBinningCurve :" << timer.elapsed() << "ms";
}

void Common::SamplingSpline(const std::vector<QPointF>& spline,
	std::vector<QPointF>& out_sampled_spline)
{
	qDebug() << "start Common::SamplingSpline";

	QElapsedTimer timer;
	timer.start();

	if (spline.size() < 2) return;

	out_sampled_spline.push_back(spline.front());

	std::vector<QPointF> spline_temp = spline;
	QPointF tail =
		spline_temp.back() * 2.0 - spline_temp[spline_temp.size() - 2];
	spline_temp.push_back(tail);

	for (int i = 1; i < (int)spline_temp.size() - 1; i++)
	{
		QPointF p0 = out_sampled_spline.back();
		QPointF p1 = spline_temp[i];

		QPointF v = p1 - p0;
		int xLen = (int)((v.x()));

		if (xLen < 1) continue;

		float length = sqrt(v.x() * v.x() + v.y() * v.y());

		QPointF stepVec = QPointF(v.x() / length, v.y() / length);
		float fStep = abs(1.0 / stepVec.x());

		stepVec.setY(stepVec.y() * fStep);
		stepVec.setX(1.0);

		for (int j = 0; j < xLen; j++)
			out_sampled_spline.push_back(p0 + stepVec * ((double)j + 1.0));
	}

	qDebug() << "end Common::SamplingSpline :" << timer.elapsed() << "ms";
}

std::vector<std::string> Common::ReadCSV(const QString& file_path)
{
	std::ifstream file(file_path.toStdString().c_str());

	std::stringstream ss;
	bool inquotes = false;
	std::vector<std::string> row;

	while (file.good())
	{
		char c = file.get();
		if (!inquotes && c == '"')
		{
			inquotes = true;
		}
		else if (inquotes && c == '"')
		{
			if (file.peek() == '"')
			{
				ss << (char)file.get();
			}
			else
			{
				inquotes = false;
			}
		}
		else if (!inquotes && c == ',')
		{
			row.push_back(ss.str());
			ss.str("");
		}
		else if (!inquotes && (c == '\r' || c == '\n'))
		{
			if (file.peek() == '\n')
			{
				file.get();
			}

			row.push_back(ss.str());
			return row;
		}
		else
		{
			ss << c;
		}
	}

	return row;
}
