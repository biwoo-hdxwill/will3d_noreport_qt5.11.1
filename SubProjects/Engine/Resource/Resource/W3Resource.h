#pragma once
/*=========================================================================

File:		class CW3Resource
Language:	C++11
Library:	Standard C++ Library

=========================================================================*/
/*
	* Resource Data Structure.
	* Descriptions.
		- Super class of : 
			CW3Image2D, CW3Image3D, CW3Mask
*/
#include "resource_global.h"
class RESOURCE_EXPORT CW3Resource
{
public:
	explicit CW3Resource();
	virtual ~CW3Resource(void); // Virtual Destructor. (for subclasses memory release)

public:
};
