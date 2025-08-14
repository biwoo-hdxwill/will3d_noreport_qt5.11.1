#pragma once

#define DEF_KERNEL				13
#define DEF_SCALE				2
#define DEF_SCALEFACTOR			.4f
#define	DEF_EPS					.0001f
#define DEF_THRESH_PIXELSIZE	.3f

typedef	short voltype;
typedef	unsigned short usvoltype;

//========= Constant Global Variable ===========//
const	int		gconst_iCountofMaxLight = 8;
const	int		gconst_iCountofMaxMaterial = 8;
const	double	gconst_HALFPI = 1.57079632679489661923;
const	double	gconst_dPI = 3.14159265358979323846;
const	double	gconst_d2PI = 6.28318530717958647692;
const	double	gconst_dPIover180 = 0.01745329251994329576;
const	double	gconst_d1Radian = 57.2957795130823208768;


template<typename T>
inline void SafeNew(T*& x)
{
	if (x)
		delete(x);

	x = new T;
}

template<typename T>
inline bool SafeNews(T*& x, const int a)
{
	if (x)
		delete[](x);

	x = new T[a];
	std::memset(x, 0, sizeof(T)*a);

	if (x == nullptr) return false;
	return true;
}

template<typename T>
inline bool SafeNew2D(T**& x, const int wh, const int d)
{
	x = new T*[d];
	if (!x) return false;

	for (int i = 0; i < d; i++)
	{
		x[i] = new T[wh];
		std::memset(x[i], 0, sizeof(T)*wh);
	}
	if (!x[d - 1]) return false;

	return true;
}

inline bool GetIdx1DTo3D(int idx, int w, int h, int d, int* x, int* y, int* z)
{
	int wh = w*h;
	*z = idx / wh;
	int xy = idx - (*z)*wh;
	*y = xy / w;
	*x = xy - (*y)*w;

	return true;
}
inline bool GetIdx3DTo1D(int x, int y, int z, int w, int h, int d, int* idx)
{
	*idx = x + y*w + z*w*h;
	return true;
}

inline bool GetIdx1DTo2D(int idx, int w, int h, int d, int* xy, int* z)
{
	int wh = w*h;
	*z = idx / wh;
	*xy = idx - (*z)*wh;

	return true;
}
inline bool GetIdx2DTo1D(int xy, int z, int w, int h, int d, int* idx)
{
	*idx = xy + z*w*h;
	return true;
}
inline bool GetIdx3DTo2D(int x, int y, int z, int w, int* idxxy, int* idxz)
{
	*idxz = z;
	*idxxy = x + y*w;

	return true;
}

inline bool GetIdx2DTo3D(int idxxy, int idxz, int w, int* x, int* y, int* z)
{
	*z = idxz;
	*y = idxxy / w;
	*x = idxxy - (*y)*w;

	return true;
}

template<typename T>
inline bool CompareEq(T x, T tmin, T tmax)
{
	if ((x >= tmin) && (x <= tmax)) return true;
	return false;
}

template<class T>
inline void SWAP(T &a, T &b)
{
	T dum = a;
	a = b;
	b = dum;
}

template <typename T> 
inline const T& Max(const T& a, const T& b) { return (a >= b) ? a : b; }

template <typename T>
inline const T& Min(const T& a, const T& b) { return (a <= b) ? a : b; }
