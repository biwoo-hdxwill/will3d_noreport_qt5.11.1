#include "Logger.hpp"
Logger lg;
/* 컴파일 시간을 위해 template이 아닌 모든 것들은 다 여기로 옮기자. */
void Logger::_init() {
	m_mode = NORMAL;
	pushLevel(0);
	m_uptoLevel = 10;
#if USE_COLOR
	setDefaultColor(rlutil::GREY);
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// @write
///////////////////////////////////////////////////////////////////////////////////////////////////
/* use explicit specification */
template<>
void Logger::write<std::runtime_error>(std::stringstream& ss, const std::runtime_error& e) {
	ss << e.what();
}
template<>
void Logger::write<std::runtime_error>(const std::runtime_error& t) {
#if USE_COLOR
	pushColor(rlutil::LIGHTRED);
#endif
	*m_out << t.what();
#if USE_COLOR
	popColor();
#endif
}
template<>
void Logger::write<float>(const float& t) {
#if USE_COLOR
	pushColor(rlutil::WHITE);
#endif
	*m_out << t;
#if USE_COLOR
	popColor();
#endif
}
template<>
void Logger::write<double>(const double& t) {
#if USE_COLOR
	pushColor(rlutil::WHITE);
#endif
	*m_out << t;
#if USE_COLOR
	popColor();
#endif
}
template<>
void Logger::write<int>(const int& t) {
#if USE_COLOR
	pushColor(rlutil::WHITE);
#endif
	*m_out << t;
#if USE_COLOR
	popColor();
#endif
}
//template<>
//void write<std::string>(const std::string& t) {
//	*out << t;
//}
//void write(const char* t) {
//	*out << t;
//}
///////////////////////////////////////////////////////////////////////////////////////////////////
// @constr
///////////////////////////////////////////////////////////////////////////////////////////////////
Logger::Logger() {
	_init();
	m_out = std::shared_ptr<std::ostream>(&std::cout, [](std::ostream* ptr) { /* do nothing */ });
}
Logger::Logger(const std::string& fpath) {
	_init();
	try {
		std::ofstream* fid = new std::ofstream(fpath);
		if (!(*fid)) {
			delete fid;
			throw std::runtime_error("invalid fid");
		}
		m_out = std::shared_ptr<std::ostream>(fid, [](std::ostream* ptr) {
			auto obj = dynamic_cast<std::ofstream*>(ptr);
			obj->close();
			delete ptr;
		});
	} catch (std::runtime_error& e) {
		*this << "Logger::Logger: " << e.what() << std::endl;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// @color
///////////////////////////////////////////////////////////////////////////////////////////////////
#if USE_COLOR
void Logger::setDefaultColor(int color) {
	m_colorStack.clear();
	pushColor(color);
}
void Logger::pushColor(int color) {
	m_colorStack.push_back(color);
	rlutil::setColor(color);
}
void Logger::peekColor() {
	try {
		if (m_colorStack.size() < 1) {
			throw std::runtime_error("m_colorStack.size < 1");
		}
		rlutil::setColor(m_colorStack.back());
	} catch (std::runtime_error& e) {
		*this << "Logger::peekColor: " << e << std::endl;
	}
}
void Logger::popColor() {
	try {
		if (m_colorStack.size() < 2) {
			throw std::runtime_error("m_colorStack.size < 2");
		}
		m_colorStack.pop_back();
		rlutil::setColor(m_colorStack.back());
	} catch (std::runtime_error& e) {
		*this << "Logger::popColor: " << e << std::endl;
	}
}
void Logger::pushLevel(int level) {
	m_curLevelStack.push_back(level);
}
int Logger::peekLevel() {
	try {
		if (m_curLevelStack.size() < 1) {
			throw std::runtime_error("not enough level in stack");
		}
		return m_curLevelStack.back();
	} catch (std::runtime_error& e) {
		*this << "Logger::peekLevel: " << e << std::endl;
		return 0;
	}
}
void Logger::popLevel() {
	try {
		if (m_curLevelStack.size() < 2) {
			throw std::runtime_error("not enough level in stack");
		}
		m_curLevelStack.pop_back();
	} catch (std::runtime_error& e) {
		*this << "Logger::popLevel: " << e << std::endl;
	}
}

#endif
///////////////////////////////////////////////////////////////////////////////////////////////////
// @operator()
///////////////////////////////////////////////////////////////////////////////////////////////////
Logger& Logger::operator()(int level) {
	pushLevel(level);
	return *this;
}
Logger& Logger::operator()() {
	popLevel();
	return *this;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// @operator<<
///////////////////////////////////////////////////////////////////////////////////////////////////
//Logger& operator << (Logger& logger, Logger::StandardManipulator smanip) {
//	try {
//		if (logger.peekLevel() <= logger.m_uptoLevel) {
//			if (smanip == std::endl) {
//				switch (logger.m_mode) {
//				case Logger::NORMAL:
//					smanip(*logger.m_out);
//					logger.m_mode = Logger::IND;
//					break;
//				case Logger::IND:
//					smanip(*logger.m_out);
//					logger.m_mode = Logger::IND;
//					break;
//				case Logger::TIC:
//					logger.tic(logger.m_ticoutBuf.str());
//					logger.m_ticoutBuf.str("");
//					logger.m_mode = Logger::IND;
//					break;
//				case Logger::TOC:
//#if USE_COLOR
//					logger.pushColor(rlutil::YELLOW);
//#endif
//					logger.write(logger.ind());
//					logger.write(logger.m_tocoutBuf.str());
//					smanip(*logger.m_out);
//#if USE_COLOR
//					logger.popColor();
//#endif
//					logger.m_tocoutBuf.str("");
//					logger.m_mode = Logger::IND;
//					break;
//				}
//			}
//			else {
//				smanip(*logger.m_out);
//			}
//		}
//	}
//	catch (std::runtime_error& e) {
//		logger << "Logger::operator<< " << e.what() << std::endl;
//	}
//	return logger;
//}
///////////////////////////////////////////////////////////////////////////////////////////////////
// @string
///////////////////////////////////////////////////////////////////////////////////////////////////
std::string Logger::tab(int n) {
	std::stringstream ss;
	for (int i = 0; i < n; i++) {
		ss << "   ";
	}
	return ss.str();
}
std::string Logger::fill(int n) {
	std::stringstream ss;
	for (int i = 0; i < n; i++) {
		ss << "## ";
	}
	return ss.str();
}
std::string Logger::bar(int n) {
	std::stringstream ss;
	for (int i = 0; i < n; i++) {
		ss << "   ";
	}
	ss << "[" << n << "] ";
	return ss.str();
}
std::string Logger::ind(int n) {
	return bar(m_timer_starts.size() + n);
}
std::string Logger::ind() {
	return ind(0);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// @tic toc ms
///////////////////////////////////////////////////////////////////////////////////////////////////
void Logger::tic(const std::string tag) {
	if (peekLevel() <= m_uptoLevel) {
		m_timer_starts.push_back(std::chrono::steady_clock::now());
		m_timer_tags.push_back(tag);
#if USE_COLOR
		pushColor(rlutil::YELLOW);
#endif
		write(ind(-1));
		write("[start] ");
		write(tag);
		write(":\n");
#if USE_COLOR
		popColor();
#endif
		m_mode = IND;
	}
}
Logger& Logger::ticout() {
	if (peekLevel() <= m_uptoLevel) {
		m_mode = TIC;
	}
	return *this;
}
Logger& Logger::tocout() {
	if (peekLevel() <= m_uptoLevel) {
		m_mode = TOC;
	}
	return *this;
}
void Logger::toc() {
	try {
		if (peekLevel() <= m_uptoLevel) {
			if (m_timer_starts.empty()) {
				throw std::runtime_error("dont call tic yet");
			}
#if USE_COLOR
			pushColor(rlutil::YELLOW);
#endif
			write(ind(-1));
			write("[done] ");
			write(m_timer_tags.back());
			write(": ");
			write(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_timer_starts.back()).count());
			m_timer_starts.pop_back();
			m_timer_tags.pop_back();
			write(" ms");
			write('\n');
#if USE_COLOR
			popColor();
#endif
			m_mode = IND;
		}
	} catch (std::runtime_error& e) {
		*this << "Logger::toc: " << e.what() << std::endl;
	}
}
std::string Logger::ms() {
	try {
		if (peekLevel() <= m_uptoLevel) {
			if (m_timer_starts.empty()) {
				throw std::runtime_error("dont call tic yet");
			}
			std::stringstream ss;
			ss << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_timer_starts.back()).count() << " ms";
			m_timer_starts.pop_back();
			m_timer_tags.pop_back();
			return ss.str();
		}
		return " lg.ms(): print level is off now";
	} catch (std::runtime_error& e) {
		*this << "Logger::toc: " << e.what() << std::endl;
		return "nan ms";
	}
}
