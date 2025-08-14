#include "W3NerveMask.h"
#include "W3Image3D.h"

#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/common.h"
#include "../../Common/Common/W3Math.h"

CW3NerveMask::CW3NerveMask(void) :
	CW3Resource(ERESOURCE_TYPE::MASK)
{
	m_ppData = nullptr;

	m_nDataWidth = m_nDataHeight = m_nDataDepth = 0;
	m_nVolWidth = m_nVolHeight = m_nVolDepth = 0;
	m_fPixelSpacing = m_fSliceSpacing = 0.f;

	m_fNerveRadius = NERVE_RADIUS_DEFAULT;

	for (W3INT i = 0; i < ENERVE_STATE::nerve_both_on; i++)
	{
		m_lstNervePoints.push_back(new std::vector<CW3Vector3D>);
		m_lstNerveDir.push_back(new std::vector<CW3Vector3D>);
		m_lstNerveROI_Start.push_back(new std::vector<CW3Vector3D>);
		m_lstNerveROI_End.push_back(new std::vector<CW3Vector3D>);
	}
}

CW3NerveMask::CW3NerveMask(CW3Image3D* pVol) :
	CW3Resource(ERESOURCE_TYPE::MASK),
	m_nVolWidth(pVol->width()),
	m_nVolHeight(pVol->height()),
	m_nVolDepth(pVol->depth()),
	m_fPixelSpacing(pVol->pixelSpacing()),
	m_fSliceSpacing(pVol->sliceSpacing())
{
	m_ppData = nullptr;

	m_nDataWidth = m_nDataHeight = m_nDataDepth = 0;

	m_fNerveRadius = NERVE_RADIUS_DEFAULT;

	for (W3INT i = 0; i < ENERVE_STATE::nerve_both_on; i++)
	{
		m_lstNervePoints.push_back(new std::vector<CW3Vector3D>);
		m_lstNerveDir.push_back(new std::vector<CW3Vector3D>);
		m_lstNerveROI_Start.push_back(new std::vector<CW3Vector3D>);
		m_lstNerveROI_End.push_back(new std::vector<CW3Vector3D>);
	}
	
	//createBuffer(m_nVolWidth, m_nVolHeight, m_nVolDepth); //bit 연산자 추가시 변경

}

CW3NerveMask::~CW3NerveMask(void)
{
	this->deleteBuffer();

	while (m_lstNervePoints.size())
	{
		auto iter = m_lstNervePoints.begin();
		SAFE_DELETE_OBJECT(*iter);
		m_lstNervePoints.erase(iter);
	}
	while (m_lstNerveDir.size())
	{
		auto iter = m_lstNerveDir.begin();
		SAFE_DELETE_OBJECT(*iter);
		m_lstNerveDir.erase(iter);
	}
	while (m_lstNerveROI_Start.size())
	{
		auto iter = m_lstNerveROI_Start.begin();
		SAFE_DELETE_OBJECT(*iter);
		m_lstNerveROI_Start.erase(iter);
	}
	while (m_lstNerveROI_End.size())
	{
		auto iter = m_lstNerveROI_End.begin();
		SAFE_DELETE_OBJECT(*iter);
		m_lstNerveROI_End.erase(iter);
	}

}


///////////////////////////////////////////////////////////////////////////////////////////////
// public functions
///////////////////////////////////////////////////////////////////////////////////////////////
void CW3NerveMask::clear(void)
{
	for (W3INT i = 0; i < ENERVE_STATE::nerve_both_on; i++)
		this->clearNerveList((ENERVE_STATE)i);

	resetBuffer();
}

void CW3NerveMask::deleteBuffer(void)
{
	if (m_ppData)
		SAFE_DELETE_VOLUME(m_ppData, m_nDataDepth);

}

//bit연산자로 변경시 사이즈 변경
W3UCHAR** CW3NerveMask::createBuffer(W3INT width, W3INT height, W3INT depth)
{
	if (isValid())
		deleteBuffer();

	W3INT szSlice = width*height;

	m_nDataWidth = width;
	m_nDataHeight = height;
	m_nDataDepth = depth;

	m_ppData = SAFE_ALLOC_VOLUME(W3UCHAR, m_nDataDepth, szSlice);
	for (int i = 0; i < m_nDataDepth; i++)
		std::memset(m_ppData[i], 0, szSlice);
	return m_ppData;
}

void CW3NerveMask::resetBuffer(void)
{
	if (!isValid()) return;

	W3INT szSlice = m_nDataWidth*m_nDataHeight;

	for (int i = 0; i < m_nDataDepth; i++)
		std::memset(m_ppData[i], 0, szSlice);
}
void CW3NerveMask::setNervePoints(const std::vector<CW3Vector3D> nervePoints, const ENERVE_STATE state)
{
	
	m_lstNervePoints.at(state)->clear();
	m_lstNervePoints.at(state)->assign(nervePoints.begin(), nervePoints.end());

	updateNerveMask(state);
}

void CW3NerveMask::updateNerveMask(const ENERVE_STATE state)
{
	if (!isValid())
	{
		W3INT width = getVolWidth();
		W3INT height = getVolHeight();
		W3INT depth = getVolDepth();

		if (width <= 0 || height <= 0 || depth <= 0)
			throw CW3MaskException(CW3MaskException::EID::INVALID_SIZE, "Invalid volume size");

		createBuffer(width, height, depth);
	}
	
	this->resetBuffer();

	for (W3INT idx = 0; idx < ENERVE_STATE::nerve_both_on; idx++)
		this->fillNerveList((ENERVE_STATE)idx);
}

void CW3NerveMask::getNerveListPoints( std::vector<CW3Vector3D>& points)
{
	points.clear();
	for (W3INT idx = 0; idx < ENERVE_STATE::nerve_both_on; idx++)
		points.insert(points.begin(), m_lstNervePoints.at(idx)->begin(), m_lstNervePoints.at(idx)->end());
}
void CW3NerveMask::getNerveListDir( std::vector<CW3Vector3D>& points)
{
	points.clear();
	for (W3INT idx = 0; idx < ENERVE_STATE::nerve_both_on; idx++)
		points.insert(points.begin(), m_lstNerveDir.at(idx)->begin(), m_lstNerveDir.at(idx)->end());
}
void CW3NerveMask::getNerveListROI_Start( std::vector<CW3Vector3D>& points)
{
	points.clear();
	for (W3INT idx = 0; idx < ENERVE_STATE::nerve_both_on; idx++)
		points.insert(points.begin(), m_lstNerveROI_Start.at(idx)->begin(), m_lstNerveROI_Start.at(idx)->end());
}
void CW3NerveMask::getNerveListROI_End( std::vector<CW3Vector3D>& points)
{
	points.clear();
	for (W3INT idx = 0; idx < ENERVE_STATE::nerve_both_on; idx++)
		points.insert(points.begin(), m_lstNerveROI_End.at(idx)->begin(), m_lstNerveROI_End.at(idx)->end());
}

///////////////////////////////////////////////////////////////////////////////////////////////
// private functions
///////////////////////////////////////////////////////////////////////////////////////////////

void CW3NerveMask::fillNerveList(const ENERVE_STATE state)
{

	std::vector<CW3Vector3D>& nervePoints = *m_lstNervePoints.at(state);

	m_lstNerveDir.at(state)->clear();
	m_lstNerveROI_Start.at(state)->clear();
	m_lstNerveROI_End.at(state)->clear();

	if (nervePoints.size() < 2)
		return;

	W3UCHAR** data = this->getData();
	W3FLOAT rad = this->getNerveRadius();

	W3INT width = this->getVolWidth(); //thyoo 비트연산자 변경

	CW3Vector3D tail = nervePoints.back() * 2 - nervePoints.at(nervePoints.size() - 2);

	nervePoints.push_back(tail);

	for (W3INT i = 0; i < nervePoints.size() - 1; i++)
	{
		CW3Vector3D x2 = nervePoints.at(i + 1);
		CW3Vector3D x1 = nervePoints.at(i);
	
	
		CW3Vector3D p1, p2;
		p1 = CW3Vector3D(W3::MIN(x1.x(), x2.x()), W3::MIN(x1.y(), x2.y()), W3::MIN(x1.z(), x2.z()));
		p2 = CW3Vector3D(W3::MAX(x1.x(), x2.x()), W3::MAX(x1.y(), x2.y()), W3::MAX(x1.z(), x2.z()));
	
		CW3Vector3D p1_ = p1 - CW3Vector3D(rad, rad, rad);
		CW3Vector3D p2_ = p2 + CW3Vector3D(rad, rad, rad);
	
		CW3Vector3D dir = (x2 - x1).normalized();
	
		for (W3INT iterZ = (W3INT)(p1_.z()); iterZ <= (W3INT)(p2_.z()); iterZ++)
		for (W3INT iterY = (W3INT)(p1_.y()); iterY <= (W3INT)(p2_.y()); iterY++)
		for (W3INT iterX = (W3INT)(p1_.x()); iterX <= (W3INT)(p2_.x()); iterX++)
		{
			if ((iterX >= 0 && iterX < m_nDataWidth) &&
				(iterY >= 0 && iterY < m_nDataHeight) &&
				(iterZ >= 0 && iterZ < m_nDataDepth))
			data[iterZ][iterY*width + iterX] = 255; //thyoo 비트연산자 변경
		}
	
		m_lstNerveDir.at(state)->push_back(dir);
		m_lstNerveROI_Start.at(state)->push_back(p1_);
		m_lstNerveROI_End.at(state)->push_back(p2_);
	}
	nervePoints.pop_back();
}

void CW3NerveMask::clearNerveList(const ENERVE_STATE state)
{
	m_lstNervePoints.at(state)->clear();
	m_lstNerveDir.at(state)->clear();
	m_lstNerveROI_Start.at(state)->clear();
	m_lstNerveROI_End.at(state)->clear();
}
