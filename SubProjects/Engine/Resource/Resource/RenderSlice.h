#pragma once
/*=========================================================================

File:		structure RENDERSLICE
Language:	C++11
Library:	Standard C++ Library

=========================================================================*/
#include "../../Common/Common/W3Types.h"
#include "../../Common/Common/W3Memory.h"
/*
	RGBA data structure.
*/
struct byte4
{
	W3UCHAR	b;	// blue.
	W3UCHAR	g;	// green.
	W3UCHAR	r;	// red.
	W3UCHAR	a;	// alpha.
};

/*
	temporary structure for VR module.

	MUST BE REPLACED BY OTHER DATA STRUCTURE.
*/
struct RENDERSLICE{
	W3UINT		_iVolSizeX;		// width
	W3UINT		_iVolSizeY;		// height
	byte4*		_pData;			// Data read from RAW.

	RENDERSLICE(){
		_iVolSizeX = _iVolSizeY = 0;
		_pData = nullptr;
	}
	~RENDERSLICE(){
		CleanUp();
	}

	void SetResolution(W3UINT res){
		CleanUp();
		_iVolSizeX = res;
		_iVolSizeY = res;
		_pData = SAFE_ALLOC_1D(byte4, res*res);
	}

	void CleanUp(void){
		SAFE_DELETE_ARRAY(_pData);
	}
	W3INT SizeSlice(void){
		return _iVolSizeX*_iVolSizeY;
	}
};
