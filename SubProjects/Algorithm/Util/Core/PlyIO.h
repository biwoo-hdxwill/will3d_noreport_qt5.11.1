#pragma once
#include <string>
#include <vector>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif
#include "util_global.h"
class UTIL_EXPORT PlyIO {
public:
	enum ply_file_type {
		ascii, 
		binary_little_endian, 
		binary_big_endian // not supported yet
	};
// #pragma pack(push, 1), #pragma pack(pop) usage
// http://stackoverflow.com/questions/22112171/alignment-of-struct-didnt-work-with-pragma-pack
// http://javawoo.tistory.com/30
#pragma pack(push, 1)
	class PlyTriangle {
	public:
		unsigned char n; // if #pragma pack dont exist, n will be aligned to 4 byte.
		int a;
		int b;
		int c;
	};
#pragma pack(pop)
	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	static bool PlyIO::ply_write( 
	/// 	const std::vector<glm::vec3>& points,
	/// 	const std::vector<std::vector<int>>& triangles, 
	/// 	const std::string& fpath,
	/// 	ply_file_type format);
	///
	/// @brief	write ply file
	///
	/// @author	Hosan
	/// @date	2016-07-04
	///
	/// @param	points   	
	/// @param	triangles	
	/// @param	fpath	 	full file path
	/// @param	format		use ascii or binary_little_endian
	///
	/// @return	true if it succeeds, false if it fails.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	static bool ply_write(
		const std::vector<glm::vec3>& points,
		const std::vector<std::vector<int>>& triangles,
		const std::string& fpath,
		ply_file_type format
	);
};
