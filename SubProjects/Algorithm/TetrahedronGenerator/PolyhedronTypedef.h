/**********************************************************************************************//**
 * @file	PolyhedronTypedef.h
 *
 * @brief	기본적인 mesh자료형에 대한 cgal의 include와 typdef
 **************************************************************************************************/

#pragma once
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/Polyhedron_3.h>
#include "TetrahedronGenerator_global.h"
/* kernel, polyhedron, halfedge */
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Polyhedron_3<K>				Polyhedron;
typedef Polyhedron::HalfedgeDS				HalfedgeDS;
