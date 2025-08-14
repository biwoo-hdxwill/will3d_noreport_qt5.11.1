#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <vector>
#include <memory>
#include <exception>
#include <iomanip>
#define USE_COLOR 1
#if USE_COLOR
#include "../rlutil/rlutil.h" // http://stackoverflow.com/questions/29574849/how-to-change-text-color-and-console-color-in-codeblocks
#include "util_global.h"
#endif
class UTIL_EXPORT Logger {
protected: /* protected members */
	enum Mode {
		NORMAL, IND, TIC, TOC
	};

	Mode m_mode;
	std::stringstream m_ticoutBuf;
	std::stringstream m_tocoutBuf;
	std::vector<int> m_colorStack;
	std::vector<std::chrono::steady_clock::time_point> m_timer_starts;
	std::vector<std::string> m_timer_tags;

	typedef std::basic_ostream<char, std::char_traits<char>> CoutType;
	typedef CoutType& (*StandardManipulator)(CoutType&);
public: /* public members */
	int m_uptoLevel;
	std::vector<int> m_curLevelStack;
	std::shared_ptr<std::ostream> m_out;
private: /* private methods */
	void _init();
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// @write
	///////////////////////////////////////////////////////////////////////////////////////////////////
	template<class T>
	void write(const T& t);
	template<class T>
	void write(std::stringstream& ss, const T& t);
public: /* public methods */
///////////////////////////////////////////////////////////////////////////////////////////////////
// @constr
///////////////////////////////////////////////////////////////////////////////////////////////////
	Logger();
	Logger(const std::string& fpath);
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// @color
	///////////////////////////////////////////////////////////////////////////////////////////////////
#if USE_COLOR
	void setDefaultColor(int color);
	void pushColor(int color);
	void peekColor();
	void popColor();
	void pushLevel(int level);
	int peekLevel();
	void popLevel();
#endif
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// @operator()
	///////////////////////////////////////////////////////////////////////////////////////////////////
	Logger& operator()(int level);
	Logger& operator()();
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// @operator<<
	///////////////////////////////////////////////////////////////////////////////////////////////////
	template<class T>
	friend Logger& operator<<(Logger& logger, const T& t);
	friend Logger& operator << (Logger& logger, Logger::StandardManipulator smanip) {
		try {
			if (logger.peekLevel() <= logger.m_uptoLevel) {
                if (smanip == (Logger::StandardManipulator)std::endl) {
					switch (logger.m_mode) {
					case Logger::NORMAL:
						smanip(*logger.m_out);
						logger.m_mode = Logger::IND;
						break;
					case Logger::IND:
						smanip(*logger.m_out);
						logger.m_mode = Logger::IND;
						break;
					case Logger::TIC:
						logger.tic(logger.m_ticoutBuf.str());
						logger.m_ticoutBuf.str("");
						logger.m_mode = Logger::IND;
						break;
					case Logger::TOC:
#if USE_COLOR
						logger.pushColor(rlutil::YELLOW);
#endif
						logger.write(logger.ind());
						logger.write(logger.m_tocoutBuf.str());
						smanip(*logger.m_out);
#if USE_COLOR
						logger.popColor();
#endif
						logger.m_tocoutBuf.str("");
						logger.m_mode = Logger::IND;
						break;
					}
				} else {
					smanip(*logger.m_out);
				}
			}
		} catch (std::runtime_error& e) {
			logger << "Logger::operator<< " << e.what() << std::endl;
		}
		return logger;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// @string
	///////////////////////////////////////////////////////////////////////////////////////////////////
	std::string tab(int n);
	std::string fill(int n);
	std::string bar(int n);
	std::string ind(int n);
	std::string ind();
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// @tic toc ms
	///////////////////////////////////////////////////////////////////////////////////////////////////
	void tic(const std::string tag);
	void toc();
	Logger& ticout();
	Logger& tocout();
	std::string ms();
};

extern UTIL_EXPORT Logger lg;

///////////////////////////////////////////////////////////////////////////////////////////////////
// @write
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
void Logger::write(const T& t) {
	*m_out << t;
}
template<> UTIL_EXPORT void Logger::write<std::runtime_error>(std::stringstream& ss, const std::runtime_error& e);
template<> UTIL_EXPORT void Logger::write<std::runtime_error>(const std::runtime_error& t);
template<> UTIL_EXPORT void Logger::write<float>(const float& t);
template<> UTIL_EXPORT void Logger::write<double>(const double& t);
template<> UTIL_EXPORT void Logger::write<int>(const int& t);

template<class T>
void Logger::write(std::stringstream& ss, const T& t) {
	ss << t;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// @operator<<
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
Logger& operator<<(Logger& logger, const T& t) {
	if (logger.peekLevel() <= logger.m_uptoLevel) {
		switch (logger.m_mode) {
		case Logger::NORMAL:
			logger.write(t);
			break;
		case Logger::IND:
			logger.write(logger.ind());
			logger.write(t);
			logger.m_mode = Logger::NORMAL;
			break;
		case Logger::TIC:
			logger.write(logger.m_ticoutBuf, t);
			break;
		case Logger::TOC:
			logger.write(logger.m_tocoutBuf, t);
			break;
		}
	}
	return logger;
}