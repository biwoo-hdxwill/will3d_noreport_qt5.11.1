#include "W3ResourceFactory.h"
#include "W3Resource.h"
#include "W3Image2D.h"
#include "W3Image3D.h"
#include "W3Mask.h"
#include "W3Mesh.h"
#include <cassert>

CW3ResourceFactory::CW3ResourceFactory(void)
{
}


CW3ResourceFactory::~CW3ResourceFactory(void)
{
}

CW3Resource* CW3ResourceFactory::createResource(ERESOURCE_TYPE eType, CW3Resource *parent)
{
	CW3Resource *pRes = nullptr;

	switch( eType )
	{
	case ERESOURCE_TYPE::BASE:
		/*pRes = new CW3Resource(eType);
		pRes->setParent(parent);
		return pRes;*/
		std::cout<<"creating base resource type..."<<std::endl;
		assert(true);
		break;
	
	case ERESOURCE_TYPE::IMAGE2D:
		pRes = new CW3Image2D();
		pRes->setParent(parent);
		break;

	case ERESOURCE_TYPE::IMAGE3D:
		assert( parent != nullptr );
		pRes = new CW3Image3D(parent->width(), parent->height(), parent->depth());
		pRes->setParent(parent);
		break;

	case ERESOURCE_TYPE::MASK:
		assert( parent != nullptr );
		assert( parent->resourceType() == ERESOURCE_TYPE::IMAGE3D );
		pRes = new CW3Mask(static_cast<CW3Image3D*>(parent));
		break;

	case ERESOURCE_TYPE::MESH:
		pRes = new CW3Mesh();
		pRes->setParent(parent);
		break;

	default:
		std::cout<<"creating illegal resource type."<<std::endl;
		assert(true);
	}

	return pRes;
}
