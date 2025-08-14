#pragma once
/*=========================================================================

File:			surface_axes_item_geometry.h
Language:		C++11
Library:		Qt 5.8.0, Standard C++ Library
Author:			JUNG DAE GUN
First Date:		2019-01-23
Last Modify:	2019-01-23

Copyright (c) 2018~2019 All rights reserved by HDXWILL.

=========================================================================*/

#include "uiglobjects_global.h"

#include <QMutex>

class CW3VBOSTL;

class UIGLOBJECTS_EXPORT SurfaceAxesItemGeometry
{
public:
	static SurfaceAxesItemGeometry* GetInstance()
	{
		static QMutex mutex;
		mutex.lock();
		if (!instance_)
		{
			instance_ = new SurfaceAxesItemGeometry();
			atexit(Destroy);
		}
		mutex.unlock();
		return instance_;
	}

	inline CW3VBOSTL* arrow() { return arrow_; }
	inline CW3VBOSTL* torus() { return torus_; }

protected:
	SurfaceAxesItemGeometry();
	virtual ~SurfaceAxesItemGeometry();

private:
	static void Destroy()
	{
		static QMutex mutex;
		mutex.lock();
		if (instance_)
		{
			delete instance_;
			instance_ = nullptr;
		}
		mutex.unlock();
	}

private:
	static SurfaceAxesItemGeometry* instance_;

	CW3VBOSTL* arrow_ = nullptr;
	CW3VBOSTL* torus_ = nullptr;
};
