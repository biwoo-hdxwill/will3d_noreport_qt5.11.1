#pragma once
#pragma message("Util/Core/IOParser_v3.h visited")
#include <sstream>
#include <exception>
#include <string>
#include <functional>
#include <fstream>
#include <vector>
#include <regex>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif

#include "Logger.hpp"
#include "util_global.h"
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class	IOParser_v3
///
/// @brief	new version IO tool
///////////////////////////////////////////////////////////////////////////////////////////////////
class UTIL_EXPORT IOParser_v3 {
public: /* public static members */
	static std::string SEP; ///< OS path seperator. windows=\, mac=/, linux=/
public: /* public methods */
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// @ file manage methods
	// @openFile: get FILE pointer. fpath: file path with file name, mode: "r" or "w".
	// @openOutputStream: get ofstream. fpath: file path with file name.
	// @openInputStream: get ifstream. fpath: file path with file name.
	// @closeFile: close FILE pointer.
	// @ls: linux ls command. given fdir, return files: files in the fdir, dirs: directories in the fdir.
	// @mkdir: linux mkdir command. fdir: folder path with folder name. the method makes the folder fdir.
	// @fullfile: matlab fullfile function. join given string with OS path seperator (ex: \). if given string contain seperator (ex: \) at the end,
	//			it will be automatically removed and joined.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	static FILE* openFile(const std::string& fpath, const std::string& mode);
	static std::ofstream openOutputStream(const std::string& fpath);
	static std::ifstream openInputStream(const std::string& fpath);
	static void closeFile(FILE* fid);
	static bool ls(std::vector<std::string>& files, std::vector<std::string>& dirs, const std::string& fdir);
	static bool mkdir(const std::wstring& fdirw);
	static bool mkdir(const std::string& fdir);
	static std::string fullfile(const std::vector<std::string>& vfpath);
	static std::string fullfile(const std::string& fpath1);
	static std::string fullfile(const std::string& fpath1, const std::string& fpath2);
	static std::string fullfile(const std::string& fpath1, const std::string& fpath2, const std::string& fpath3);
	static std::string fullfile(const std::string& fpath1, const std::string& fpath2, const std::string& fpath3, const std::string& fpath4);
	static std::string fullfile(const std::string& fpath1, const std::string& fpath2, const std::string& fpath3, const std::string& fpath4, const std::string& fpath5);
public: /* publie methods */
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// @ 3d points
	// @writeASCIIPoints: write text file with given points. (ex: 0 0\n1 1\n2 2)
	// @readASCIIPoints: read text file and return points as vector.
	// @writeBytePoints: write raw file with given points.
	// @readBytePoints: read raw file and return points as vector.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	static bool writeASCIIPoints(const std::vector<glm::vec2>& points, const std::string& fpath);
	static bool writeASCIIPoints(const std::vector<glm::vec3>& points, const std::string& fpath);
	static bool writeASCIIPoints(const std::vector<glm::vec4>& points, const std::string& fpath);
	static bool readASCIIPoints(std::vector<glm::vec2>& points, const std::string& fpath);
	static bool readASCIIPoints(std::vector<glm::vec3>& points, const std::string& fpath);
	static bool readASCIIPoints(std::vector<glm::vec4>& points, const std::string& fpath);
	static bool writeBytePoints(const std::vector<glm::vec2>& points, const std::string& fpath);
	static bool writeBytePoints(const std::vector<glm::vec3>& points, const std::string& fpath);
	static bool writeBytePoints(const std::vector<glm::vec4>& points, const std::string& fpath);
	static bool readBytePoints(std::vector<glm::vec2>& points, const std::string& fpath);
	static bool readBytePoints(std::vector<glm::vec3>& points, const std::string& fpath);
	static bool readBytePoints(std::vector<glm::vec4>& points, const std::string& fpath);

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// @ triangle idx
	// @writeASCIITriangles: write text file with given triangles. (ex: 0 1 2\n0 2 3\n3 4 5)
	// @readASCIITriangles: read text file and return triangles as vector of vector.
	// @writeByteTriangles: write raw file with given triangles.
	// @readByteTriangles: read raw file and return triangles as vector of vector.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	static bool writeASCIITriangles(const std::vector<std::vector<int>>& tris, const std::string& fpath);
	static bool readASCIITriangles(std::vector<std::vector<int>>& tris, const std::string& fpath);
	static bool writeByteTriangles(const std::vector<std::vector<int>>& tris, const std::string& fpath);
	static bool readByteTriangles(std::vector<std::vector<int>>& tris, const std::string& fpath);

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// @ tetrahedron idx
	// @writeASCIITetrahedrons: write text file with given tetrahedrons. (ex: 0 1 2 3\n0 2 4 5\n0 3 5 6)
	// @readASCIITetrahedrons: read text file and return tetrahedrons as vector of vector.
	// @writeByteTetrahedrons: write text file with given tetrahedrons.
	// @readByteTetrahedrons: read raw file and return tetrahedrons as vector of vector.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	static bool writeASCIITetrahedrons(const std::vector<std::vector<int>>& tetras, const std::string& fpath);
	static bool readASCIITetrahedrons(std::vector<std::vector<int>>& tetras, const std::string& fpath);
	static bool writeByteTetrahedrons(const std::vector<std::vector<int>>& tetras, const std::string& fpath);
	static bool readByteTetrahedrons(std::vector<std::vector<int>>& tetras, const std::string& fpath);
public: /* public methods */
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// @ raw data IO methods
	// <class InternalDataType>: type of elements saved in c++ vector data.
	// <class OuterDataType>: type of elements saved in raw file.
	// @writeRaw1d: write raw file with given data 1d vector. fpath: file path with file name.
	// @writeRaw2d: write raw file with given data 1d vector. nx: virtual x-dim of data vector. ny: virtual y-dim of data vector. fpath: file path with file name.
	// @writeRaw3d: write raw files with given data 1d vector. nx: virtual x-dim of data vector. ny: virtual y-dim of data vector. nz: virtual z-dim of data vector. fdir: folder path with folder name.
	// @readRaw1d: read raw file and return data 1d vector. nx: virtual x-dim of data vector. fpath: file path with file name.
	// @readRaw2d: read raw file and return data 1d vector. nx: virtual x-dim of data vector. ny: virtual y-dim of data vector. fpath: file path with file name.
	// @readRaw3d: read raw files and return data 1d vector. nx: virtual x-dim of data vector. ny: virtual y-dim of data vector. nz: virtual z-dim of data vector. fdir: folder path with folder name.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	template<class InternalDataType, class OuterDataType>
	static bool writeRaw1d(const std::vector<InternalDataType>& data, const std::string& fpath);

	template<class InternalDataType, class OuterDataType>
	static bool writeRaw2d(const std::vector<InternalDataType>& data, int nx, int ny, const std::string& fpath);

	template<class InternalDataType, class OuterDataType>
	static bool writeRaw2d(
		const std::vector<InternalDataType>& data, int nx, int ny, const std::string& fpath,
		std::function<int(int, int)> idxInner, ///< data(ix, iy) is saved at data[idxInner(ix, iy)]
		std::function<void(int&, int&, int)> idxOuter); ///< rawfile(ix, iy) will be saved to rawfile[i], where idxOuter(ix, iy, i). and assign: rawfile(ix, iy) = data(ix, iy).

	template<class InternalDataType, class OuterDataType>
	static bool writeRaw3d(std::vector<InternalDataType>& data, int nx, int ny, int nz, const std::string& fdir);

	template<class InternalDataType, class OuterDataType>
	static bool writeRaw3d(
		std::vector<InternalDataType>& data, int nx, int ny, int nz, const std::string& fdir,
		std::function<int(int, int, int)> idxInner,  ///< data(ix, iy, iz) is saved at data[idxInner(ix, iy, iz)]
		std::function<void(int&, int&, int)> idxOuter); ///< rawfile[iz](ix, iy) will be saved to rawfile[iz][i], where idxOuter(ix, iy, i). and assign: rawfile[iz](ix, iy) = data(ix, iy, iz).

	template<class InternalDataType, class OuterDataType>
	static bool readRaw1d(std::vector<InternalDataType>& data, int nx, const std::string& fpath);

	template<class InternalDataType, class OuterDataType>
	static bool readRaw2d(
		std::vector<InternalDataType>& data, int nx, int ny, const std::string& fpath);

	template<class InternalDataType, class OuterDataType>
	static bool readRaw2d(
		std::vector<InternalDataType>& data, int nx, int ny, const std::string& fpath,
		std::function<int(int, int)> idxInner, ///< data(ix, iy) will be saved to data[idxInner(ix, iy)]
		std::function<void(int&, int&, int)> idxOuter); ///< rawfile(ix, iy) is saved at rawfile[i], where idxOuter(ix, iy, i). and assign: data(ix, iy) = rawfile(ix, iy).

	template<class InternalDataType, class OuterDataType>
	static bool readRaw3d(std::vector<InternalDataType>& data, int nx, int ny, int nz, const std::string& fdir, const std::string& reg = ".*\\d{4,}.*\\.raw");

	template<class InternalDataType, class OuterDataType>
	static bool readRaw3d(
		std::vector<InternalDataType>& data, int nx, int ny, int nz, const std::string& fdir, const std::string& reg,
		std::function<int(int, int, int)> idxInner, ///< data(ix, iy, iz) will be saved to data[idxInner(ix, iy, iz)]
		std::function<void(int&, int&, int)> idxOuter); ///< rawfile[iz](ix, iy) is saved at rawfile[iz][i], where idxOuter(ix, iy, i). and assign: data(ix, iy, iz) = rawfile[iz](ix, iy).
};
///////////////////////////////////////////////////////////////////////////////////////////////////
// @ basic methods
///////////////////////////////////////////////////////////////////////////////////////////////////
UTIL_EXPORT std::string fullfile(const std::vector<std::string>& vfpath);
UTIL_EXPORT std::string fullfile(const std::string& fpath1);
UTIL_EXPORT std::string fullfile(const std::string& fpath1, const std::string& fpath2);
UTIL_EXPORT std::string fullfile(const std::string& fpath1, const std::string& fpath2, const std::string& fpath3);
UTIL_EXPORT std::string fullfile(const std::string& fpath1, const std::string& fpath2, const std::string& fpath3, const std::string& fpath4);
UTIL_EXPORT std::string fullfile(const std::string& fpath1, const std::string& fpath2, const std::string& fpath3, const std::string& fpath4, const std::string& fpath5);
///////////////////////////////////////////////////////////////////////////////////////////////////
// @ raw data IO methods implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

template<class InternalDataType, class OuterDataType>
bool IOParser_v3::writeRaw1d(const std::vector<InternalDataType>& data, const std::string& fpath) {
	try {
		FILE* fid = openFile(fpath, "wb");
		int nx = data.size();
		/* conversion buf */
		std::vector<OuterDataType> buf(nx);
		for (int i = 0; i < nx; i++) {
			buf[i] = static_cast<OuterDataType>(data[i]);
		}
		/* fwrite */
		int cnt = fwrite(buf.data(), sizeof(OuterDataType), nx, fid);
		/* err chk */
		if (cnt != nx) {
			std::stringstream ss;
			ss << "# of pixel in " << fpath << "(=" << cnt << ") != size(=" << nx << ")";
			throw std::runtime_error(ss.str().c_str());
		}
		/* close file */
		closeFile(fid);
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::writeRaw1d: " << e << std::endl;
		return false;
	}
}
template<class InternalDataType, class OuterDataType>
bool IOParser_v3::writeRaw2d(const std::vector<InternalDataType>& data, int nx, int ny, const std::string& fpath) {
	try {
		return writeRaw2d<InternalDataType, OuterDataType>(data, nx, ny, fpath,
			[nx, ny](int ix, int iy)->int {return nx*iy + ix; },
			[nx, ny](int& ix, int& iy, int i) {
			ix = i % nx;
			iy = i / nx;
		}
		);
	} catch (std::runtime_error& e) {
		lg << "IOParser::writeRaw2d: " << e << std::endl;
		return false;
	}
}
template<class InternalDataType, class OuterDataType>
bool IOParser_v3::writeRaw2d(
	const std::vector<InternalDataType>& data, int nx, int ny, const std::string& fpath,
	std::function<int(int, int)> idxInner,
	std::function<void(int&, int&, int)> idxOuter) {
	try {
		FILE* fid = openFile(fpath, "wb");
		/* conversion buf */
		int size = nx*ny;
		std::vector<OuterDataType> buf(nx*ny);
		int ix, iy;
		for (int i = 0; i < size; i++) {
			idxOuter(ix, iy, i);
			buf[i] = static_cast<OuterDataType>(data[idxInner(ix, iy)]);
		}
		/* fwrite */
		int cnt = fwrite(buf.data(), sizeof(OuterDataType), nx*ny, fid);
		/* err chk */
		if (cnt != size) {
			std::stringstream ss;
			ss << "# of pixel in " << fpath << "(=" << cnt << ") != nx*ny(=" << nx*ny << ")";
			throw std::runtime_error(ss.str().c_str());
		}
		/* close file */
		closeFile(fid);
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::writeRaw2d: " << e << std::endl;
		return false;
	}
}
template<class InternalDataType, class OuterDataType>
bool IOParser_v3::writeRaw3d(std::vector<InternalDataType>& data, int nx, int ny, int nz, const std::string& fdir) {
	try {
		return writeRaw3d<InternalDataType, OuterDataType>(data, nx, ny, nz, fdir,
			[nx, ny, nz](int ix, int iy, int iz)->int {return nx*ny*iz + nx*iy + ix; },
			[nx, ny, nz](int& ix, int& iy, int i) {
			ix = i % nx;
			iy = i / nx;
		}
		);
	} catch (std::runtime_error& e) {
		lg << "IOParser::writeRaw3d: " << e << std::endl;
		return false;
	}
}
template<class InternalDataType, class OuterDataType>
bool IOParser_v3::writeRaw3d(
	std::vector<InternalDataType>& data, int nx, int ny, int nz, const std::string& fdir,
	std::function<int(int, int, int)> idxInner,
	std::function<void(int&, int&, int)> idxOuter) {
	try {
		/* file name gen */
		std::stringstream ss;
		auto l_fname = [&ss](int i)->std::string {
			ss.str("");
			ss << std::setfill('0') << std::setw(4) << i << ".raw";
			return ss.str();
		};
		/* folder make */
		if (!mkdir(fdir)) {
			std::stringstream ss;
			ss << "dir=" << fdir << " cannot be made";
			throw std::runtime_error(ss.str().c_str());
		}
		/* conversion buf and output */
		std::vector<OuterDataType> slice(nx*ny);
		data.resize(nx*ny*nz, InternalDataType(0));
		for (int iz = 0; iz < nz; iz++) {
			std::string fpath = fullfile(fdir, l_fname(iz));
			FILE* fid = openFile(fpath, "wb");
			int size = nx*ny;
			for (int i = 0; i < size; i++) {
				int ix, iy;
				idxOuter(ix, iy, i);
				slice[i] = static_cast<OuterDataType>(data[idxInner(ix, iy, iz)]);
			}
			int cnt = fwrite(slice.data(), sizeof(OuterDataType), nx*ny, fid);
			if (cnt != nx*ny) {
				std::stringstream ss;
				ss << "# of pixel in " << fpath << "(=" << cnt << ") != nx*ny(=" << nx*ny << ")";
				throw std::runtime_error(ss.str().c_str());
			}
			closeFile(fid);
		}
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::writeRaw3d: " << e << std::endl;
		return false;
	}
}
template<class InternalDataType, class OuterDataType>
bool IOParser_v3::readRaw1d(std::vector<InternalDataType>& data, int nx, const std::string& fpath) {
	try {
		FILE* fid = openFile(fpath, "rb");
		/* conversion buf */
		std::vector<OuterDataType> buf(nx);
		/* fread */
		int cnt = fread(buf.data(), sizeof(OuterDataType), nx, fid);
		/* err chk */
		if (cnt != nx) {
			std::stringstream ss;
			ss << "# of pixel in " << fpath << "(=" << cnt << ") != nx(=" << nx << ")";
			throw std::runtime_error(ss.str().c_str());
		}
		/* output */
		data.resize(nx);
		for (int i = 0; i < nx; i++) {
			data[i] = static_cast<InternalDataType>(buf[i]);
		}
		/* close file */
		closeFile(fid);
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::readRaw1d: " << e << std::endl;
		return false;
	}
}
template<class InternalDataType, class OuterDataType>
bool IOParser_v3::readRaw2d(std::vector<InternalDataType>& data, int nx, int ny, const std::string& fpath) {
	try {
		return readRaw2d<InternalDataType, OuterDataType>(data, nx, ny, fpath,
			[nx, ny](int ix, int iy)->int {return nx*iy + ix; },
			[nx, ny](int& ix, int& iy, int i) {
			ix = i % nx;
			iy = i / nx;
		}
		);
	} catch (std::runtime_error& e) {
		lg << "IOParser::readRaw2d: " << e << std::endl;
		return false;
	}
}
template<class InternalDataType, class OuterDataType>
bool IOParser_v3::readRaw2d(
	std::vector<InternalDataType>& data, int nx, int ny, const std::string& fpath,
	std::function<int(int, int)> idxInner,
	std::function<void(int&, int&, int)> idxOuter) {
	try {
		FILE* fid = openFile(fpath, "rb");
		/* conversion buf */
		std::vector<OuterDataType> slice(nx*ny);
		/* fread */
		int cnt = fread(slice.data(), sizeof(OuterDataType), nx*ny, fid);
		/* err chk */
		if (cnt != nx*ny) {
			std::stringstream ss;
			ss << "# of pixel in " << fpath << "(=" << cnt << ") != nx*ny(=" << nx*ny << ")";
			throw std::runtime_error(ss.str().c_str());
		}
		/* output */
		data.resize(nx*ny);
		for (int i = 0; i < nx*ny; i++) {
			int ix;
			int iy;
			idxOuter(ix, iy, i);
			data[idxInner(ix, iy)] = static_cast<InternalDataType>(slice[i]);
		}
		/* close file */
		closeFile(fid);
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::readRaw2d: " << e << std::endl;
		return false;
	}
}
template<class InternalDataType, class OuterDataType>
bool IOParser_v3::readRaw3d(std::vector<InternalDataType>& data, int nx, int ny, int nz, const std::string& fdir, const std::string& reg) {
	try {
		return readRaw3d<InternalDataType, OuterDataType>(data, nx, ny, nz, fdir, reg,
			[nx, ny, nz](int ix, int iy, int iz)->int {return nx*ny*iz + nx*iy + ix; },
			[nx, ny, nz](int& ix, int& iy, int i) {
			ix = i % nx;
			iy = i / nx;
		}
		);
	} catch (std::runtime_error& e) {
		lg << "IOParser::readRaw3d: " << e << std::endl;
		return false;
	}
	return true;
}
template<class InternalDataType, class OuterDataType>
bool IOParser_v3::readRaw3d(
	std::vector<InternalDataType>& data, int nx, int ny, int nz, const std::string& fdir, const std::string& reg,
	std::function<int(int, int, int)> idxInner,
	std::function<void(int&, int&, int)> idxOuter) {
	try {
		/* find all files */
		std::vector<std::string> files;
		std::vector<std::string> dirs;
		ls(files, dirs, fdir);
		/* find files match */
		std::vector<std::string> filesMatch;
		std::smatch m;
		for (const auto& file : files) {
			if (std::regex_match(file, m, std::regex(reg))) {
				filesMatch.push_back(m.str());
			}
		}
		/* err chk */
		if (filesMatch.size() != nz) {
			std::stringstream ss;
			ss << "# of slice file(= " << filesMatch.size() << ") is inconsistant to data nz(= " << nz << ")";
			throw std::runtime_error(ss.str().c_str());
		}
		/* sort data by file name */
		std::sort(filesMatch.begin(), filesMatch.end());
		/* conversion buf and output */
		std::vector<OuterDataType> slice(nx*ny);
		data.resize(nx*ny*nz, 0);
		for (int iz = 0; iz < nz; iz++) {
			const auto& file = filesMatch[iz];
			FILE* fid = openFile(fullfile(fdir, file), "rb");
			int cnt = fread(slice.data(), sizeof(OuterDataType), nx*ny, fid);
			if (cnt != nx*ny) {
				std::stringstream ss;
				ss << "# of pixel in " << file << "(=" << cnt << ") != nx*ny(=" << nx*ny << ")";
				throw std::runtime_error(ss.str().c_str());
			}
			for (int i = 0; i < nx*ny; i++) {
				int ix;
				int iy;
				idxOuter(ix, iy, i);
				data[idxInner(ix, iy, iz)] = static_cast<InternalDataType>(slice[i]);
			}
			closeFile(fid);
		}
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::readRaw3d: " << e << std::endl;
		return false;
	}
}
