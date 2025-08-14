#pragma once

/*=========================================================================

File:		Heap Memory Segment Macros.
Language:	C++11
Library:	Standard C++ Library

=========================================================================*/
#include <iostream>
#include <assert.h>
#define __forceinline __inline

namespace W3
{
	/*
	inline function for allocation of memory.
	used by MACRO "SAFE_ALLOC_1D".
	notes:
	terminates current process if exception occurs.
	*/
	extern "C++"
		template <typename TYPE>
    __forceinline TYPE* allocate_1D(size_t size){
		TYPE *p;
		try{
			p = new TYPE[size];
		}
		catch (std::bad_alloc){
			std::cerr << "Bad Allocations" << std::endl;
			assert(false);
#if defined(__APPLE__) || defined(MACOSX)
            exit(EXIT_FAILURE);
#else
            _exit(EXIT_FAILURE);
#endif
		}
		return p;
	}
	/*
	inline function for allocation of memory.
	used by MACRO "SAFE_ALLOC_2D".
	notes:
	terminates current process if exception occurs.
	*/
	extern "C++"
		template <typename TYPE>
    __forceinline TYPE* allocate_2D(size_t row, size_t col){
		TYPE *p;
		try{
			p = new TYPE[row*col];
		}
		catch (std::bad_alloc){
			std::cerr << "Bad Allocations" << std::endl;
#if defined(__APPLE__) || defined(MACOSX)
            exit(EXIT_FAILURE);
#else
            _exit(EXIT_FAILURE);
#endif
		}
		return p;
	}
	/*
	inline function for allocation of memory.
	used by MACRO "SAFE_ALLOC_VOLUME".
	notes:
	terminates current process if exception occurs.
	*/
	extern "C++"
		template <typename TYPE>
    __forceinline TYPE** allocate_volume(size_t depth, size_t szSlice){
		TYPE **p;
		try{
			p = new TYPE*[depth];
			for (int i = 0; i<depth; i++)
				p[i] = new TYPE[szSlice];
		}
		catch (std::bad_alloc){
			std::cerr << "Bad Allocations" << std::endl;
#if defined(__APPLE__) || defined(MACOSX)
            exit(EXIT_FAILURE);
#else
            _exit(EXIT_FAILURE);
#endif
		}
		return p;
	}

	// by jdk 510622
	/*
	inline function for allocation of memory.
	used by MACRO "P_SAFE_ALLOC_1D".
	notes:
	terminates current process if exception occurs.
	*/
	extern "C++"
		template <typename TYPE>
    __forceinline void p_allocate_1D(TYPE **p, size_t size)
	{
		if (*p)
		{
			std::cerr << "Not initialized with nullptr" << std::endl;
		}

		try
		{
			*p = new TYPE[size];
		}
		catch (std::bad_alloc)
		{
			std::cerr << "Bad Allocations" << std::endl;
#if defined(__APPLE__) || defined(MACOSX)
            exit(EXIT_FAILURE);
#else
            _exit(EXIT_FAILURE);
#endif
		}
	}
	/*
	inline function for allocation of memory.
	used by MACRO "P_SAFE_ALLOC_2D".
	notes:
	terminates current process if exception occurs.
	*/
	extern "C++"
		template <typename TYPE>
    __forceinline void p_allocate_2D(TYPE **p, size_t row, size_t col)
	{

		if (*p)
		{
			std::cerr << "Not initialized with nullptr" << std::endl;
		}

		try
		{
			*p = new TYPE[row * col];
		}
		catch (std::bad_alloc)
		{
			std::cerr << "Bad Allocations" << std::endl;
#if defined(__APPLE__) || defined(MACOSX)
            exit(EXIT_FAILURE);
#else
            _exit(EXIT_FAILURE);
#endif
		}
	}
	/*
	inline function for allocation of memory.
	used by MACRO "P_SAFE_ALLOC_VOLUME".
	notes:
	terminates current process if exception occurs.
	*/
	extern "C++"
		template <typename TYPE>
    __forceinline void p_allocate_volume(TYPE ***p, size_t depth, size_t szSlice)
	{

		if (*p)
		{
			std::cerr << "Not initialized with nullptr" << std::endl;
		}

		try
		{
			*p = new TYPE*[depth];
			for (int i = 0; i < depth; i++)
				(*p)[i] = new TYPE[szSlice];
		}
		catch (std::bad_alloc)
		{
			std::cerr << "Bad Allocations" << std::endl;
#if defined(__APPLE__) || defined(MACOSX)
            exit(EXIT_FAILURE);
#else
            _exit(EXIT_FAILURE);
#endif
		}
	}
	// jdk end
}

/*
MACRO : safe allocation of memory.
1D & 2D
*/
#define SAFE_ALLOC_1D( TYPE, size )		W3::allocate_1D<TYPE>((size))
#define SAFE_ALLOC_2D( TYPE, row, col)	W3::allocate_2D<TYPE>((row), (col))
/*
MACRO : safe allocation of memory.
"Volume allocation"
INPUT : double pointer
*/
#define SAFE_ALLOC_VOLUME( TYPE, depth, szSlice )	W3::allocate_volume<TYPE>((depth), (szSlice))
/*
MACRO : safe deallocation of memory.
*/
#define SAFE_DELETE_OBJECT(p)	{ if(p) delete(p); p=nullptr; }
#define SAFE_DELETE_ARRAY(p)	{ if(p) delete[](p); p=nullptr; }
#define SAFE_DELETE_VOLUME(p, depth)	{ if(p) {for (int i=0;i<depth;i++)	SAFE_DELETE_ARRAY(p[i]); SAFE_DELETE_ARRAY(p);} }
#define SAFE_DELETE_LATER(p) { if(p) p->deleteLater(); }

// by jdk 150622
/*
MACRO : safe allocation of memory.
1D & 2D
*/
#define P_SAFE_ALLOC_1D( P, TYPE, size )	W3::p_allocate_1D<TYPE>((P), (size))
#define P_SAFE_ALLOC_2D( P, TYPE, row, col)	W3::p_allocate_2D<TYPE>((P), (row), (col))
/*
MACRO : safe allocation of memory.
"Volume allocation"
INPUT : triple pointer
*/
#define P_SAFE_ALLOC_VOLUME( P, TYPE, depth, szSlice )	W3::p_allocate_volume<TYPE>((P), (depth), (szSlice))
// jdk end
