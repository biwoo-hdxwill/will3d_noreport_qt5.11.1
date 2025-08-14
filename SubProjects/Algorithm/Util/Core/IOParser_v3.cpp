#pragma message("Util/Core/IOParser_v3.cpp visited")
#include "IOParser_v3.h"
#include <locale>
#include <codecvt>
#if defined(_WIN32)
#include <Util/dirent/dirent.h>
#endif
using namespace std;
///////////////////////////////////////////////////////////////////////////////////////////////////
// @ static members
///////////////////////////////////////////////////////////////////////////////////////////////////
std::string IOParser_v3::SEP = "\\"; ///< OS path seperator. for windows case
///////////////////////////////////////////////////////////////////////////////////////////////////
// @ file manage methods
///////////////////////////////////////////////////////////////////////////////////////////////////
FILE* IOParser_v3::openFile(const std::string& fpath, const std::string& mode) {
	FILE* fid = nullptr;
	bool result = false;
#if defined(__APPLE__)
	fid = fopen(fpath.c_str(), mode.c_str());
	result = (fid) ? true : false;
#else
	result = !fopen_s(&fid, fpath.c_str(), mode.c_str());
#endif
	if (result) {
		return fid;
	} else {
		stringstream ss;
		ss << "fpath=" << fpath << " file open err. mode=" << mode;
		throw runtime_error(ss.str().c_str());
		return nullptr;
	}
}
std::ofstream IOParser_v3::openOutputStream(const std::string& fpath) {
	ofstream out(fpath);
	if (!out) {
		stringstream ss;
		ss << "fpath=" << fpath << " ofstream open err.";
		throw runtime_error(ss.str().c_str());
	} else {
		return out;
	}
}
std::ifstream IOParser_v3::openInputStream(const std::string& fpath) {
	ifstream in(fpath);
	if (!in) {
		stringstream ss;
		ss << "fpath=" << fpath << " ifstream open err";
		throw runtime_error(ss.str().c_str());
	} else {
		return in;
	}
}
void IOParser_v3::closeFile(FILE* fid) {
	if (fid && fclose(fid) != 0) {
		throw runtime_error("file cannot be closed");
	}
}
bool IOParser_v3::ls(std::vector<std::string>& files, std::vector<std::string>& dirs, const std::string& fdir) {
#if defined(_WIN32)
	try {
		DIR* dir = opendir(fdir.c_str());
		files.clear();
		dirs.clear();
		if (dir) {
			dirent* ent;
			while (ent = readdir(dir)) {
				switch (ent->d_type) {
				case DT_REG:
					files.push_back(ent->d_name);
					break;
				case DT_DIR:
					dirs.push_back(ent->d_name);
					break;
				default:
					files.push_back(ent->d_name);
					break;
				}
			}
			closedir(dir);
		} else {
			stringstream ss;
			ss << "dir=" << fdir << " cant be found";
			throw std::runtime_error(ss.str().c_str());
		}
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::ls: " << e << endl;
		return false;
	}
#else
	return false;
#endif
}
bool IOParser_v3::mkdir(const std::wstring& fdirw) {
#if defined(_WIN32)
	try {
		if (CreateDirectory(fdirw.c_str(), NULL)) {
			return true;
		} else if (ERROR_ALREADY_EXISTS == GetLastError()) {
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
			lg << "IOParser::mkdir: dir=" << conv.to_bytes(fdirw) << " already exists" << endl;
			return true;
		} else {
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
			stringstream ss;
			ss << "making of dir=" << conv.to_bytes(fdirw) << " is failed";
			throw std::runtime_error(ss.str().c_str());
		}
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::mkdir: " << e << endl;
		return false;
	}
#else
	return false;
#endif
}
bool IOParser_v3::mkdir(const std::string& fdir) {
	try {
		bool res = true;
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
		wstring fdirw = conv.from_bytes(fdir);
		res = res && mkdir(fdirw);
		if (!res) {
			throw runtime_error("there is some err");
		}
		return true;
	} catch (runtime_error& e) {
		lg << "IOParser::mkdir: " << e << endl;
		return false;
	}
}
std::string IOParser_v3::fullfile(const std::vector<std::string>& vfpath) {
	stringstream ss;
	smatch m;
	for (int i = 0; i < vfpath.size(); i++) {
		smatch m;
		if (i == 0) {
			ss << vfpath[i];
		} else {
			stringstream ssreg;
			ssreg << "(.*)";
			if (SEP == "\\") {
				ssreg << "\\\\";
			} else {
				ssreg << SEP;
			}
			ssreg << "$";

			if (regex_search(vfpath[i - 1], m, regex(ssreg.str()))) {
				ss << vfpath[i];
			} else {
				ss << SEP << vfpath[i];
			}
		}
	}
	return ss.str();
}
std::string IOParser_v3::fullfile(const std::string& fpath1) {
	vector<string> vfpath;
	vfpath.push_back(fpath1);
	return fullfile(vfpath);
}
std::string IOParser_v3::fullfile(const std::string& fpath1, const std::string& fpath2) {
	vector<string> vfpath;
	vfpath.push_back(fpath1);
	vfpath.push_back(fpath2);
	return fullfile(vfpath);
}
std::string IOParser_v3::fullfile(const std::string& fpath1, const std::string& fpath2, const std::string& fpath3) {
	vector<string> vfpath;
	vfpath.push_back(fpath1);
	vfpath.push_back(fpath2);
	vfpath.push_back(fpath3);
	return fullfile(vfpath);
}
std::string IOParser_v3::fullfile(const std::string& fpath1, const std::string& fpath2, const std::string& fpath3, const std::string& fpath4) {
	vector<string> vfpath;
	vfpath.push_back(fpath1);
	vfpath.push_back(fpath2);
	vfpath.push_back(fpath3);
	vfpath.push_back(fpath4);
	return fullfile(vfpath);
}
std::string IOParser_v3::fullfile(const std::string& fpath1, const std::string& fpath2, const std::string& fpath3, const std::string& fpath4, const std::string& fpath5) {
	vector<string> vfpath;
	vfpath.push_back(fpath1);
	vfpath.push_back(fpath2);
	vfpath.push_back(fpath3);
	vfpath.push_back(fpath4);
	vfpath.push_back(fpath5);
	return fullfile(vfpath);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// @ 3d points
///////////////////////////////////////////////////////////////////////////////////////////////////
bool IOParser_v3::writeASCIIPoints(const std::vector<glm::vec2>& points, const std::string& fpath) {
	try {
		ofstream out = openOutputStream(fpath);
		for (const auto& p : points) {
			out << p[0] << "\t" << p[1] << endl;
		}
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::writeASCIIPoints: " << e << endl;
		return false;
	}
}
bool IOParser_v3::writeASCIIPoints(const std::vector<glm::vec3>& points, const std::string& fpath) {
	try {
		ofstream out = openOutputStream(fpath);
		for (const auto& p : points) {
			out << p[0] << "\t" << p[1] << "\t" << p[2] << endl;
		}
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::writeASCIIPoints: " << e << endl;
		return false;
	}
}
bool IOParser_v3::writeASCIIPoints(const std::vector<glm::vec4>& points, const std::string& fpath) {
	try {
		ofstream out = openOutputStream(fpath);
		for (const auto& p : points) {
			out << p[0] << "\t" << p[1] << "\t" << p[2] << "\t" << p[3] << endl;
		}
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::writeASCIIPoints: " << e << endl;
		return false;
	}
}
bool IOParser_v3::readASCIIPoints(std::vector<glm::vec2>& points, const std::string& fpath) {
	try {
		ifstream in = openInputStream(fpath);
		points.clear();
		for (string line; std::getline(in, line);) {
			stringstream ss(line);
			vector<string> toks;
			string tok;
			while (ss >> tok) {
				toks.push_back(tok);
			}
			if (toks.size() >= 2) {
				points.push_back(glm::vec2(stof(toks[0]), stof(toks[1])));
			}
		}
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::readASCIIPoints: " << e << endl;
		return false;
	}
}
bool IOParser_v3::readASCIIPoints(std::vector<glm::vec3>& points, const std::string& fpath) {
	try {
		ifstream in = openInputStream(fpath);
		points.clear();
		for (string line; std::getline(in, line);) {
			stringstream ss(line);
			vector<string> toks;
			string tok;
			while (ss >> tok) {
				toks.push_back(tok);
			}
			if (toks.size() >= 3) {
				points.push_back(glm::vec3(stof(toks[0]), stof(toks[1]), stof(toks[2])));
			}
		}
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::readASCIIPoints: " << e << endl;
		return false;
	}
}
bool IOParser_v3::readASCIIPoints(std::vector<glm::vec4>& points, const std::string& fpath) {
	try {
		ifstream in = openInputStream(fpath);
		points.clear();
		for (string line; std::getline(in, line);) {
			stringstream ss(line);
			vector<string> toks;
			string tok;
			while (ss >> tok) {
				toks.push_back(tok);
			}
			if (toks.size() >= 4) {
				points.push_back(glm::vec4(stof(toks[0]), stof(toks[1]), stof(toks[2]), stof(toks[3])));
			}
		}
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::readASCIIPoints: " << e << endl;
		return false;
	}
}
bool IOParser_v3::writeBytePoints(const std::vector<glm::vec2>& points, const std::string& fpath) {
	try {
		FILE* fid = openFile(fpath, "wb");
		vector<float> buf(2 * points.size());
		for (int i = 0; i < points.size(); i++) {
			buf[2 * i] = points[i][0];
			buf[2 * i + 1] = points[i][1];
		}
		int cnt = fwrite(buf.data(), sizeof(float), 2 * points.size(), fid);
		if (cnt != 2 * points.size()) {
			throw std::runtime_error("all points were not written");
		}
		closeFile(fid);
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::writeBytePoints: " << e << endl;
		return false;
	}
}
bool IOParser_v3::writeBytePoints(const std::vector<glm::vec3>& points, const std::string& fpath) {
	try {
		FILE* fid = openFile(fpath, "wb");
		vector<float> buf(3 * points.size());
		for (int i = 0; i < points.size(); i++) {
			buf[3 * i] = points[i][0];
			buf[3 * i + 1] = points[i][1];
			buf[3 * i + 2] = points[i][2];
		}
		int cnt = fwrite(buf.data(), sizeof(float), 3 * points.size(), fid);
		if (cnt != 3 * points.size()) {
			throw std::runtime_error("all points were not written");
		}
		closeFile(fid);
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::writeBytePoints: " << e << endl;
		return false;
	}
}
bool IOParser_v3::writeBytePoints(const std::vector<glm::vec4>& points, const std::string& fpath) {
	try {
		FILE* fid = openFile(fpath, "wb");
		vector<float> buf(4 * points.size());
		for (int i = 0; i < points.size(); i++) {
			buf[4 * i] = points[i][0];
			buf[4 * i + 1] = points[i][1];
			buf[4 * i + 2] = points[i][2];
			buf[4 * i + 3] = points[i][3];
		}
		int cnt = fwrite(buf.data(), sizeof(float), 4 * points.size(), fid);
		if (cnt != 4 * points.size()) {
			throw std::runtime_error("all points were not written");
		}
		closeFile(fid);
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::writeBytePoints: " << e << endl;
		return false;
	}
}
bool IOParser_v3::readBytePoints(std::vector<glm::vec2>& points, const std::string& fpath) {
	try {
		FILE* fid = openFile(fpath, "rb");
		points.clear();
		int BUF_SIZE = 2 * 1024 * 1024;
		vector<float> buf(BUF_SIZE);
		while (int cnt = fread(buf.data(), sizeof(float), BUF_SIZE, fid)) {
			int iter = cnt / 2;
			for (int i = 0; i < iter; i++) {
				points.push_back(glm::vec2(buf[2 * i], buf[2 * i + 1]));
			}
		}
		fclose(fid);
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::readBytePoints: " << e.what() << endl;
		return false;
	}
}
bool IOParser_v3::readBytePoints(std::vector<glm::vec3>& points, const std::string& fpath) {
	try {
		FILE* fid = openFile(fpath, "rb");
		points.clear();
		int BUF_SIZE = 3 * 1024 * 1024;
		vector<float> buf(BUF_SIZE);
		while (int cnt = fread(buf.data(), sizeof(float), BUF_SIZE, fid)) {
			int iter = cnt / 3;
			for (int i = 0; i < iter; i++) {
				points.push_back(glm::vec3(buf[3 * i], buf[3 * i + 1], buf[3 * i + 2]));
			}
		}
		fclose(fid);
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::readBytePoints: " << e << endl;
		return false;
	}
}
bool IOParser_v3::readBytePoints(std::vector<glm::vec4>& points, const std::string& fpath) {
	try {
		FILE* fid = openFile(fpath, "rb");
		points.clear();
		int BUF_SIZE = 4 * 1024 * 1024;
		vector<float> buf(BUF_SIZE);
		while (int cnt = fread(buf.data(), sizeof(float), BUF_SIZE, fid)) {
			int iter = cnt / 4;
			for (int i = 0; i < iter; i++) {
				points.push_back(glm::vec4(buf[4 * i], buf[4 * i + 1], buf[4 * i + 2], buf[4 * i + 3]));
			}
		}
		fclose(fid);
		return true;
	} catch (std::runtime_error& e) {
		cout << "IOParser::readBytePoints: " << e.what() << endl;
		return false;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// @ triangle idx
///////////////////////////////////////////////////////////////////////////////////////////////////
bool IOParser_v3::writeASCIITriangles(const std::vector<std::vector<int>>& tris, const std::string& fpath) {
	try {
		ofstream out = openOutputStream(fpath);
		for (const auto& tri : tris) {
			for (int i : tri) {
				if (tri.size() != 3) {
					throw std::runtime_error("tri size is not 3");
				}
				out << i << "\t";
			}
			out << endl;
		}
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::writeASCIITriangles: " << e << endl;
		return false;
	}
}

bool IOParser_v3::readASCIITriangles(std::vector<std::vector<int>>& tris, const std::string& fpath) {
	try {
		ifstream in = openInputStream(fpath);
		tris.clear();
		for (string line; std::getline(in, line);) {
			stringstream ss(line);
			vector<string> toks;
			string tok;
			while (ss >> tok) {
				toks.push_back(tok);
			}
			if (toks.size() >= 3) {
				vector<int> temp(3);
				temp[0] = stoi(toks[0]);
				temp[1] = stoi(toks[1]);
				temp[2] = stoi(toks[2]);
				tris.push_back(temp);
			}
		}
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::readASCIITriangles: " << e << endl;
		return false;
	}
}
bool IOParser_v3::writeByteTriangles(const std::vector<std::vector<int>>& tris, const std::string& fpath) {
	try {
		FILE* fid = openFile(fpath, "wb");
		vector<int> buf(3 * tris.size());
		for (int i = 0; i < tris.size(); i++) {
			if (tris[i].size() < 3) {
				stringstream ss;
				ss << "tris[" << i << "].size < 3 // tris[" << i << "].size=" << tris[i].size() << endl;
				throw std::runtime_error(ss.str().c_str());
			}
			buf[3 * i] = tris[i][0];
			buf[3 * i + 1] = tris[i][1];
			buf[3 * i + 2] = tris[i][2];
		}
		int cnt = fwrite(buf.data(), sizeof(int), 3 * tris.size(), fid);
		if (cnt != 3 * tris.size()) {
			throw std::runtime_error("all idx were not written");
		}
		closeFile(fid);
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::writeByteTriangles: " << e << endl;
		return false;
	}
}
bool IOParser_v3::readByteTriangles(std::vector<std::vector<int>>& tris, const std::string& fpath) {
	try {
		FILE* fid = openFile(fpath, "rb");
		tris.clear();
		int BUF_SIZE = 3 * 1024 * 1024;
		vector<int> buf(BUF_SIZE);
		vector<int> temp(3);
		while (int cnt = fread(buf.data(), sizeof(int), BUF_SIZE, fid)) {
			int iter = cnt / 3;
			for (int i = 0; i < iter; i++) {
				temp[0] = buf[3 * i];
				temp[1] = buf[3 * i + 1];
				temp[2] = buf[3 * i + 2];
				tris.push_back(temp);
			}
		}
		fclose(fid);
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::readByteTriangles: " << e << endl;
		return false;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// @ tetrahedron idx
///////////////////////////////////////////////////////////////////////////////////////////////////
bool IOParser_v3::writeASCIITetrahedrons(const std::vector<std::vector<int>>& tetras, const std::string& fpath) {
	try {
		ofstream out = openOutputStream(fpath);
		for (const auto& tetra : tetras) {
			if (tetra.size() != 4) {
				throw std::runtime_error("tetra size is not 4");
			}
			for (int i : tetra) {
				out << i << "\t";
			}
			out << endl;
		}
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::writeASCIITetrahedrons: " << e << endl;
		return false;
	}
}
bool IOParser_v3::readASCIITetrahedrons(std::vector<std::vector<int>>& tetras, const std::string& fpath) {
	try {
		ifstream in = openInputStream(fpath);
		tetras.clear();
		vector<int> temp(4);
		for (string line; std::getline(in, line);) {
			stringstream ss(line);
			vector<string> toks;
			string tok;
			while (ss >> tok) {
				toks.push_back(tok);
			}
			if (toks.size() >= 4) {
				temp[0] = stoi(toks[0]);
				temp[1] = stoi(toks[1]);
				temp[2] = stoi(toks[2]);
				temp[3] = stoi(toks[3]);
				tetras.push_back(temp);
			}
		}
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::readASCIITetrahedrons: " << e << endl;
		return false;
	}
}
bool IOParser_v3::writeByteTetrahedrons(const std::vector<std::vector<int>>& tetras, const std::string& fpath) {
	try {
		FILE* fid = openFile(fpath, "wb");
		vector<int> buf(4 * tetras.size());
		for (int i = 0; i < tetras.size(); i++) {
			if (tetras[i].size() < 4) {
				stringstream ss;
				ss << "tetras[" << i << "].size < 4 // tetras[" << i << "].size=" << tetras[i].size() << endl;
				throw std::runtime_error(ss.str().c_str());
			}
			buf[4 * i] = tetras[i][0];
			buf[4 * i + 1] = tetras[i][1];
			buf[4 * i + 2] = tetras[i][2];
			buf[4 * i + 3] = tetras[i][3];
		}
		int cnt = fwrite(buf.data(), sizeof(int), 4 * tetras.size(), fid);
		if (cnt != 4 * tetras.size()) {
			throw std::runtime_error("all idx were not written");
		}
		closeFile(fid);
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::writeByteTetrahedron: " << e << endl;
		return false;
	}
}
bool IOParser_v3::readByteTetrahedrons(std::vector<std::vector<int>>& tetras, const std::string& fpath) {
	try {
		FILE* fid = openFile(fpath, "rb");
		tetras.clear();
		int BUF_SIZE = 4 * 1024 * 1024;
		vector<int> temp(4);
		vector<int> buf(BUF_SIZE);
		while (int cnt = fread(buf.data(), sizeof(int), BUF_SIZE, fid)) {
			int iter = cnt / 4;
			for (int i = 0; i < iter; i++) {
				temp[0] = buf[4 * i];
				temp[1] = buf[4 * i + 1];
				temp[2] = buf[4 * i + 2];
				temp[3] = buf[4 * i + 3];
				tetras.push_back(temp);
			}
		}
		fclose(fid);
		return true;
	} catch (std::runtime_error& e) {
		lg << "IOParser::readByteTetrahedron: " << e << endl;
		return false;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// @ file manage methods
///////////////////////////////////////////////////////////////////////////////////////////////////
std::string fullfile(const std::vector<std::string>& vfpath) {
	return IOParser_v3::fullfile(vfpath);
}
std::string fullfile(const std::string& fpath1) {
	return IOParser_v3::fullfile(fpath1);
}
std::string fullfile(const std::string& fpath1, const std::string& fpath2) {
	return IOParser_v3::fullfile(fpath1, fpath2);
}
std::string fullfile(const std::string& fpath1, const std::string& fpath2, const std::string& fpath3) {
	return IOParser_v3::fullfile(fpath1, fpath2, fpath3);
}
std::string fullfile(const std::string& fpath1, const std::string& fpath2, const std::string& fpath3, const std::string& fpath4) {
	return IOParser_v3::fullfile(fpath1, fpath2, fpath3, fpath4);
}
std::string fullfile(const std::string& fpath1, const std::string& fpath2, const std::string& fpath3, const std::string& fpath4, const std::string& fpath5) {
	return IOParser_v3::fullfile(fpath1, fpath2, fpath3, fpath4, fpath5);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// @raw data IO methods
///////////////////////////////////////////////////////////////////////////////////////////////////
/* template implementation in header file */
