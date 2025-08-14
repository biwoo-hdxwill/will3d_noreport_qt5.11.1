#pragma once
/*=========================================================================

File:			class CW3NerveMask
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-10-08
Modify Date:	2015-10-08
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include "resource_global.h"

#include "W3Image3D.h"
#include "../../Common/Common/W3Vector3D.h"
#include "../../Common/Common/W3Types.h"
#include "W3MaskException.h"
#include "W3Resource.h"


class RESOURCE_EXPORT CW3NerveMask : public CW3Resource
{
public:
	CW3NerveMask(void);
	CW3NerveMask(CW3Image3D* pVol);
	~CW3NerveMask(void);
	
public:
	void clear(void);
	void deleteBuffer(void);
	void resetBuffer();
	W3UCHAR**  createBuffer(W3INT width, W3INT height, W3INT depth);

	void setNervePoints(const std::vector<CW3Vector3D> nervePoints, const ENERVE_STATE state);
	void updateNerveMask(const ENERVE_STATE state);

	inline W3UCHAR** getData(void) const { return m_ppData; }
	inline W3FLOAT	getNerveRadius(void) const { return m_fNerveRadius; }
	inline W3INT	getDataWidth(void) const	{ return m_nDataWidth; }
	inline W3INT	getDataHeight(void) const	{ return m_nDataHeight; }
	inline W3INT	getDataDepth(void) const	{ return m_nDataDepth; }

	inline W3INT	getVolWidth(void) const	{ return m_nVolWidth; }
	inline W3INT	getVolHeight(void) const	{ return m_nVolHeight; }
	inline W3INT	getVolDepth(void) const	{ return m_nVolDepth; }

	inline W3FLOAT	getPixelSpacing(void) const	{ return m_fPixelSpacing; }
	inline W3FLOAT	getSliceSpacing(void) const	{ return m_fSliceSpacing; }

	
	void getNerveListPoints( std::vector<CW3Vector3D>& points);
	void getNerveListDir( std::vector<CW3Vector3D>& points);
	void getNerveListROI_Start( std::vector<CW3Vector3D>& points);
	void getNerveListROI_End( std::vector<CW3Vector3D>& points);

	inline std::vector<CW3Vector3D>& getNerveListPoints(ENERVE_STATE state) const{ return *m_lstNervePoints.at(state); }
	inline std::vector<CW3Vector3D>& getNerveListDir(ENERVE_STATE state) const{ return *m_lstNerveDir.at(state); }
	inline std::vector<CW3Vector3D>& getNerveListROI_Start(ENERVE_STATE state) const{ return *m_lstNerveROI_Start.at(state); }
	inline std::vector<CW3Vector3D>& getNerveListROI_End(ENERVE_STATE state) const{ return *m_lstNerveROI_End.at(state); }
	
private:

	void fillNerveList(const ENERVE_STATE state);
	void clearNerveList(const ENERVE_STATE state);
	inline W3BOOL	isValid(void)	const	{ return (m_ppData != nullptr) ? true : false; }


private:
	W3UCHAR**	m_ppData;

	W3INT	    m_nDataWidth;
	W3INT		m_nDataHeight;
	W3INT		m_nDataDepth;

	W3INT		m_nVolWidth;
	W3INT		m_nVolHeight;
	W3INT		m_nVolDepth;
	W3FLOAT		m_fPixelSpacing;
	W3FLOAT		m_fSliceSpacing;
	W3FLOAT		m_fSpacingZ;
	W3FLOAT		m_fNerveRadius;
	

	

	QList<std::vector<CW3Vector3D>*> m_lstNervePoints;
	QList<std::vector<CW3Vector3D>*> m_lstNerveDir;
	QList<std::vector<CW3Vector3D>*> m_lstNerveROI_Start;
	QList<std::vector<CW3Vector3D>*> m_lstNerveROI_End;
	QList<W3BOOL>					 m_lbUpdatedNerve;
};
