#pragma once
/*=========================================================================

File:		class CW3ResourceFactory
Language:	C++11
Library:	Qt 5.2.1, Standard C++ Library

=========================================================================*/

#include "resource_global.h"
#include "../../Common/Common/W3Enum.h"

class CW3Resource;

/*
	class CW3ResourceFactory description.

		- used at W3ResourceModel.
		- encapsulate resource creating code. (make generic to resource type)
*/
class RESOURCE_EXPORT CW3ResourceFactory
{
public:
	CW3ResourceFactory(void);
	~CW3ResourceFactory(void);

public:
	CW3Resource* createResource(ERESOURCE_TYPE, CW3Resource *parent = nullptr);
};

