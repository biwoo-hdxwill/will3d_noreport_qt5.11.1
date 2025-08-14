/**********************************************************************************************//**
 * @file	C3t3Typedef.h
 *
 * @brief	tetrahedron generator를 위한 cgal의 include와 typedef.
 **************************************************************************************************/

#pragma once
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Mesh_triangulation_3.h>
#include <CGAL/Mesh_complex_3_in_triangulation_3.h>
#include <CGAL/Mesh_criteria_3.h>
#include <CGAL/Implicit_mesh_domain_3.h>
#include <CGAL/make_mesh_3.h>

#include <CGAL/Polyhedral_mesh_domain_3.h>
#include <CGAL/refine_mesh_3.h>
#include "PolyhedronTypedef.h"
#include "TetrahedronGenerator_global.h"

/* Kernel and element */
typedef K::FT FT;
typedef K::Point_3 Point;
typedef FT(Function)(const Point&);
/* Domain */
typedef CGAL::Implicit_mesh_domain_3<Function, K> Mesh_domain_implicit;
typedef CGAL::Polyhedral_mesh_domain_3<Polyhedron, K> Mesh_domain_polyhedral;
/* Triangulation */
#ifdef CGAL_CONCURRENT_MESH_3
typedef CGAL::Mesh_triangulation_3<
	Mesh_domain_implicit,
	CGAL::Kernel_traits<Mesh_domain_implicit>::Kernel, // Same as sequential
	CGAL::Parallel_tag                        // Tag to activate parallelism
>::type Tr_implicit;
typedef CGAL::Mesh_triangulation_3<
	Mesh_domain_polyhedral,
	CGAL::Kernel_traits<Mesh_domain_polyhedral>::Kernel, // Same as sequential
	CGAL::Parallel_tag                        // Tag to activate parallelism
>::type Tr_polyhedral;
#else
typedef CGAL::Mesh_triangulation_3<Mesh_domain_implicit>::type Tr_implicit;
typedef CGAL::Mesh_triangulation_3<Mesh_domain_polyhedral>::type Tr_polyhedral;
#endif
typedef CGAL::Mesh_complex_3_in_triangulation_3<Tr_implicit> C3t3_implicit;
typedef CGAL::Mesh_complex_3_in_triangulation_3<Tr_polyhedral> C3t3_polyhedral;
