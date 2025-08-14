#pragma once
/*=========================================================================

File:		class CW3Mask
Language:	C++11
Library:	Qt 5.2.1, Standard C++ Library

=========================================================================*/

#include "resource_global.h"
#include "W3Resource.h"
#include "../../Common/Common/W3Types.h"
#include "W3Image3D.h"
#include "W3MaskException.h"

#include <qcolor.h>
class RESOURCE_EXPORT CW3Mask : public CW3Resource
{
public:
	CW3Mask(void);
	CW3Mask(CW3Image3D* pParent);
	virtual ~CW3Mask(void) override;

public:
	// public functions.
	void cleanUp(void);
	void resetBuffer();
	W3UCHAR**  createBuffer(W3INT width, W3INT height, W3INT depth);

	inline W3BOOL	isValid(void)	const	{ return m_ppcData ? true : false;}
	inline W3BOOL	isVisible(void) const	{ return m_bVisible ? true : false; }
	inline CW3Image3D* getParent(void) const { return m_pParent; }
	inline W3UINT	width(void) const	{ return m_nWidth; }
	inline W3UINT	height(void) const	{ return m_nHeight; }
	inline W3UINT	depth(void) const	{ return m_nDepth; }
	inline W3FLOAT	pixelSpacing(void) const	{ return m_fPixelSpacing; }
	inline W3FLOAT	sliceSpacing(void) const	{ return m_fSliceSpacing; }
	inline QColor	foregroundColor(void) const	{ return m_foregroundColor; }
	inline QColor	backgroundColor(void) const	{ return m_backgroundColor; }

	inline void		setParent(CW3Image3D* pParent)		{ m_pParent = pParent; }
	inline void		setVisible(W3BOOL visible)			{ m_bVisible = visible; }
	inline void		setForegroundColor(QColor color)	{ m_foregroundColor = color; }
	inline void		setBackgroundColor(QColor color)	{ m_backgroundColor = color; }
	inline void		setAlpha(W3INT alpha)				{ m_foregroundColor.setAlpha(alpha); }
	inline void		setPixelSpacing(const W3FLOAT pSpacing)	{ m_fPixelSpacing = pSpacing; }
	inline void		setSliceSpacing(const W3FLOAT sSpacing) { m_fSliceSpacing = sSpacing; }
	inline void		setValue(W3INT x, W3INT y, W3INT z, W3UCHAR nVal) { m_ppcData[z][y*m_nWidth + x] = nVal; }
	inline void		setValue(W3INT xy, W3INT z, W3UCHAR nVal) { m_ppcData[z][xy] = nVal; }
	inline W3UCHAR**	constData(void) const { return (!this->isValid()) ? nullptr : m_ppcData; }
	inline W3UCHAR*		constData(W3INT index) const {
		if(!this->isValid())			throw CW3MaskException(CW3MaskException::EID::BUFFER_NULLPTR, "Buffer is nullptr");
		if(index<0 || index>=m_nDepth)	throw CW3MaskException(CW3MaskException::EID::INDEX_OUT_OF_RANGE, "Index out of range");
		return m_ppcData[index];}
	inline W3UCHAR**	data(void) { return (!this->isValid()) ? nullptr : m_ppcData; }
	inline W3UCHAR*	data(W3INT index) {
		if(!this->isValid())			throw CW3MaskException(CW3MaskException::EID::BUFFER_NULLPTR, "Buffer is nullptr");
		if(index<0 || index>=m_nDepth)	throw CW3MaskException(CW3MaskException::EID::INDEX_OUT_OF_RANGE, "Index out of range");
		return m_ppcData[index];}


	void copy(W3BYTE** ppSrc);
	void erosion(W3INT nSESize);
	void dilation(W3INT nSESize);
	void leaveonlyone();

private:
	// private functions.
	void allocBuffer(void);

private:
	// private member fields.
	CW3Image3D* m_pParent;
	W3BOOL		m_bVisible;
	W3UINT		m_nWidth;
    W3UINT		m_nHeight;
    W3UINT		m_nDepth;
	W3UCHAR**	m_ppcData;
    W3FLOAT		m_fPixelSpacing;	// pixel spacing. (on-plane spacing)
	W3FLOAT		m_fSliceSpacing;	// slice spacing. (Z-spacing)
	QColor		m_foregroundColor;
    QColor		m_backgroundColor;

	W3BYTE		m_bytImageValue;


//	W3USHORT				m_ushMaxLabel;
//	W3USHORT**				m_ppLabelToImage;
//	std::vector<W3USHORT**>	m_vecppLabelToMask;

public:
//	inline	W3BOOL incLabel()	{if (m_ushMaxLabel < 65535) { m_ushMaxLabel++; return true; } return false;}
//	inline	W3BOOL decLabel()	{if (m_ushMaxLabel > 0)		{ m_ushMaxLabel--; return true; } return false;}
//	inline	W3UCHAR getLabel()	{return m_ushMaxLabel;}
//	void setLabel(W3INT x, W3INT y, W3INT z);
//	void eraseLabel(W3UCHAR label);
//	void eraseMaxLabel();	
};
