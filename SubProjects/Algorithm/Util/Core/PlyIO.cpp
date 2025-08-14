#include "PlyIO.h"
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <exception>
#include <vector>
using namespace std;

bool PlyIO::ply_write(
	const std::vector<glm::vec3>& points,
	const std::vector<std::vector<int>>& triangles,
	const std::string& fpath,
	ply_file_type format
) {
	try {
		FILE* fid = nullptr;
		bool result = false;
#if defined(__APPLE__)
		fid = fopen(fpath.c_str(), "wb");
		result = (fid) ? true : false;
#else
		result = !fopen_s(&fid, fpath.c_str(), "wb");
#endif
		if (!result) {
			stringstream ss;
			ss << "file (" << fpath << ") open err";
			throw runtime_error(ss.str().c_str());
		}
		stringstream header;
		string format_name;
		switch (format) {
		case ascii:
			format_name = "ascii 1.0";
			break;
		case binary_little_endian:
			format_name = "binary_little_endian 1.0";
			break;
		case binary_big_endian:
			format_name = "asciibinary_big_endian 1.0";
			break;
		default:
			throw runtime_error("unknown ply file format");
		}
		header
			<< "ply" << endl
			<< "format " << format_name << endl
			<< "element vertex " << points.size() << endl
			<< "property float x" << endl
			<< "property float y" << endl
			<< "property float z" << endl
			<< "element face " << triangles.size() << endl
			<< "property list uchar int vertex_indices" << endl
			<< "end_header" << endl;
		fprintf(fid, "%s", header.str().c_str());

		if (format == ascii) {
			for (const auto& p : points) {
				fprintf(fid, "%f %f %f\n", p.x, p.y, p.z);
			}
			for (const auto& t : triangles) {
				fprintf(fid, "%d %d %d %d\n", 3, t[0], t[1], t[2]);
			}
		} else if (format == binary_little_endian) {
			fwrite(points.data(), sizeof(glm::vec3), points.size(), fid);
			vector<PlyTriangle> buf(triangles.size());
			for (int i = 0; i < triangles.size(); i++) {
				const auto& t = triangles[i];
				auto& b = buf[i];
				b.n = 3;
				b.a = t[0];
				b.b = t[1];
				b.c = t[2];
			}

			fwrite(buf.data(), sizeof(PlyTriangle), triangles.size(), fid);
			if (fclose(fid) != 0) {
				stringstream ss;
				ss << "file (" << fpath << ") close err";
				throw runtime_error(ss.str().c_str());
			}
		} else if (format == binary_big_endian) {
			throw runtime_error("binary_big_ending is not supported yet");
		} else {
			throw runtime_error("unknown ply file format");
		}

		return true;
	} catch (runtime_error& e) {
		cout << "PlyIO::ply_write: " << e.what() << endl;
		return false;
	}
}
