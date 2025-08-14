#pragma once

#include <string>

#include "common_global.h"

#define DEVELOP_MODE 0

namespace common
{
	/**********************************************************************************************//**
	 * @enum	LogType
	 *
	 * @brief	Values that represent elog types.
	 * 			DBG : Debug, Debug에 관련된 함수 호출과 기타 디버그를 하기 위한 로그 메세지
	 * 			CMD : Command, 다른 process 에서 날아오는 명령어에 관한 로그 메세지
	 * 			INF : Info, 일반적인 사용, 하드웨어 정보, 메모리에 관한 로그 메세지
	 * 			WRN : Warning, 아직 오류는 아니지만 발생 가능한 문제
	 * 			ERR : Error, 오류를 일으킨 문제에 대한 로그 메세지
	 **************************************************************************************************/
	enum class LogType
	{
		DBG,
		CMD,
		INF,
		WRN,
		ERR
	};

	/**********************************************************************************************//**
	 * @enum	LogMode
	 *
	 * @brief	Values that represent elog Options.
	 * 			DEFAULT : default mode. 일반적인 프로그램 사용 시 발생하는 로그들을 출력하는 모드
	 * 					  CMD, INF, WRN, ERR가 출력된다
	 * 			DEBUG : debug mode. 프로그램 사용에 관련된 거의 모든 로그들을 출력하는 모드
	 * 					default mode 에서 출력되는 로그들과, 추가로 DBG가 출력된다
	 **************************************************************************************************/
	enum class LogMode : int
	{
		DEFAULT = 0,
		DEBUG = 1
	};

	/**********************************************************************************************//**
	 * @class	Logger
	 *
	 * @brief	로그파일을 작성하는 클래스로 Singlton pattern 에 따라 작성됨.
	 * 			메인 어플리케이션 시작 시 생성되고, 종료시에 소멸됨.
	 *
	 * [사용 방법]
	 * 			시작 시 :
	 * 				common::CWLogger pLogger = common::CWLogger::instance();
	 * 				pLogger->Init();
	 *
	 * 			호출 후 사용 방법 :
	 * 				1. 일반 로그 출력
	 * 					common::CWLogger pLogger = common::Logger::instance();
	 * 					pLogger->Print(common::LogType, "Log Message");
	 * 				2. 디버깅용 로그 출력
	 * 					common::CWLogger pLogger = common::Logger::instance();
	 * 					pLogger->PrintDebugMode("Module Name", Log Message");
	 *
	 * 			로그 경로 변경 :
	 * 				common::CWLogger pLogger = common::Logger::instance();
	 * 				pLogger->ChangeLogPath("New Log Path");
	 *
	 * 			로그 모드 변경 :
	 * 				common::CWLogger pLogger = common::Logger::instance();
	 * 				pLogger->ChangeLogMode(common::LogMode);
	 *
	 * 			로그파일 크기 변경 :
	 * 				common::CWLogger pLogger = common::Logger::instance();
	 * 				pLogger->ChangeFileSize(nSize);
	 * 			종료 시 :
	 * 				common::CWLogger pLogger = common::CWLogger::instance();
	 * 				pLogger->SaveCurrentSettings();
	 *
	 *
	 * @author	Seo Seok Man
	 * @date	2017-06-07
	 **************************************************************************************************/
	class COMMON_EXPORT Logger
	{
	private:
		Logger();
		Logger(const Logger& other);

	public:
		~Logger();

		/**********************************************************************************************//**
		 * @fn	void Logger::Init(void);
		 *
		 * @brief	LoadPath()를 불러와 Logger를 초기화
		 *
		 * @author	Seo Seok Man
		 * @date	2017-06-09
		 **************************************************************************************************/
		void Init();

		/**********************************************************************************************//**
		 * @fn	static Logger* Logger::getInstance(void);
		 *
		 * @brief	Gets the Sigleton instance.
		 *
		 * @author	Seo Seok Man
		 * @date	2017-06-09
		 *
		 * @return	Singleton instance.
		 **************************************************************************************************/
		static Logger* instance(void);

		/**********************************************************************************************//**
		 * @fn	void Logger::Print(const LogMode& eMode, const std::string& strLog);
		 *
		 * @brief	로그 프린팅 모드에 맞게, 로그를 파일에 프린팅함.
		 *
		 * @author	Seo Seok Man
		 * @date	2017-06-09
		 *
		 * @param	eType 	log type. LogType 정의 참조
		 * @param	strLog	log file에 프린트 될 로그의 내용
		 **************************************************************************************************/
		void Print(const LogType& eType, const std::string& strLog);

		void PrintAndAssert(const LogType& type, const std::string& txt);

		/**********************************************************************************************//**
		 * @fn	void Logger::PrintDebugMode(const LogType& eType, const std::string& strModName, const std::string& strLog);
		 *
		 * @brief	디버그 모드로 로그를 프린팅 함.
		 *
		 * @author	Seo Seok Man
		 * @date	2017-06-09
		 *
		 * @param	strModName	Name of the current module.
		 * @param	strLog	  	log file에 프린트 될 로그의 내용
		 **************************************************************************************************/
		void PrintDebugMode(const std::string& strModName, const std::string& strLog = "");

		/**********************************************************************************************//**
		 * @fn	void Logger::SaveCurrentSettings(void);
		 *
		 * @brief	Saves the current log information.
		 * 			Main Application이 종료될 때(소멸자 등) 이 함수를 호출해서, 마지막 로그 세팅을 저장해야 함.
		 *
		 * @author	Seo Seok Man
		 * @date	2017-06-09
		 **************************************************************************************************/
		void SaveCurrentSettings(void);

		/**********************************************************************************************//**
		 * @fn	void Logger::ChangeLogPath(void);
		 *
		 * @brief	사용자가 로그 파일의 경로를 변경하고자 할 때 호출하는 함수.
		 *
		 * @author	Seo Seok Man
		 * @date	2017-06-09
		 **************************************************************************************************/
		void ChangeLogPath(void);

		/**********************************************************************************************//**
		 * @fn	void Logger::ChangeLogMode(const LogMode& eMode);
		 *
		 * @brief	Change log mode.
		 *
		 * @author	Seo Seok Man
		 * @date	2017-06-09
		 *
		 * @param	log mode
		 **************************************************************************************************/
		inline void ChangeLogMode(const LogMode& eMode) { print_mode_ = eMode; }

		/**********************************************************************************************//**
		 * @fn	inline void Logger::ChangeFileSize(const int nSize)
		 *
		 * @brief	Change log size.
		 *
		 * @author	Seo Seok Man
		 * @date	2017-06-09
		 *
		 * @param	nSize	The size.
		 **************************************************************************************************/
		inline void ChangeFileSize(const int nSize) { file_size_ = nSize; }

		/**********************************************************************************************//**
		 * @fn	inline const bool IsDebugMode(void) const
		 *
		 * @brief	debug mode 일 때, log print를 하지 않고 넘어가기 위해 현재 모드를 체크하는 함수.
		 *
		 * @author	Seo Seok Man
		 * @date	2017-06-30
		 *
		 * @return	A const bool.
		 **************************************************************************************************/
		inline const bool IsDebugMode(void) const;

	private:

		/**********************************************************************************************//**
		 * @fn	void Logger::LoadPath(void);
		 *
		 * @brief	Loads saved log path.
		 * 			WILL3d.ini 파일에 저장되어 있는 로그파일의 저장 경로를 불러온다.
		 *
		 * @author	Seo Seok Man
		 * @date	2017-06-09
		 **************************************************************************************************/
		void LoadPath(void);

		/**********************************************************************************************//**
		 * @fn	void Logger::Print(const LogType& eType, const std::string& strLog, const std::string& strModName);
		 *
		 * @brief	Print log.
		 *
		 * @author	Seo Seok Man
		 * @date	2017-06-09
		 *
		 * @param	eType	  	The type.
		 * @param	strLog	  	The log.
		 * @param	strModName	Name of the modifier.
		 **************************************************************************************************/
		void Print(const LogType& eType, const std::string& strModName, const std::string& strLog);

		/**********************************************************************************************//**
		 * @fn	bool Logger::isFolderPath(void);
		 *
		 * @brief	현재 가지고 있는 폴더 경로가 정상적인 경로인지 판단한다.
		 * 			만약 비정상적인 경로면 기본 로그 경로로 세팅한다.
		 *
		 * @author	Seo Seok Man
		 * @date	2017-06-09
		 **************************************************************************************************/
		void VerifyFolderPath(void);

		/**********************************************************************************************//**
		 * @fn	bool Logger::openLogFile(void);
		 *
		 * @brief	Opens log file.
		 *
		 * @author	Seo Seok Man
		 * @date	2017-06-09
		 *
		 * @return	로그 파일 열기 테스트를 진행하여, 로그파일이 열리지 않으면 false 를 리턴한다.
		 **************************************************************************************************/
		const bool IsValidLogFile(void);

		/**********************************************************************************************//**
		 * @fn	bool Logger::verifyLogType(const LogType& eLogType);
		 *
		 * @brief	입력으로 들어온 로그 타입이 현재 로그 출력 모드에 적합한지 판단.
		 *
		 * @author	Seo Seok Man
		 * @date	2017-06-09
		 *
		 * @param	eLogType	Type of the log.
		 *
		 * @return	로그 출력 모드가 LogMode::MD_D 일 때, LOG_TYPE 이 LOG_V, LOG_D이면 부적합 판단하여 리턴
		 **************************************************************************************************/
		const bool IsValidLogType(const LogType& eLogType);

		/**********************************************************************************************//**
		 * @fn	const std::string& Logger::GetTypeText(const LogType& eLogType);
		 *
		 * @brief	log type 으로 그에 맞는 string을 리턴함
		 *
		 * @author	Seo Seok Man
		 * @date	2017-06-09
		 *
		 * @param	eLogType	Type of the log.
		 *
		 * @return	The log text.
		 **************************************************************************************************/
		const std::string GetTypeText(const LogType& eLogType);

		/**********************************************************************************************//**
		 * @fn	const std::string Logger::getCurrentDateTime(void);
		 *
		 * @brief	Gets current date time.
		 *
		 * @author	Seo Seok Man
		 * @date	2017-06-09
		 *
		 * @return	The current date time.
		 **************************************************************************************************/
		const std::string getCurrentDateTime(void);

		/**********************************************************************************************//**
		 * @fn	const size_t Logger::CurrentFileSize(void);
		 *
		 * @brief	Gets current log file size.
		 *
		 * @author	Seo Seok Man
		 * @date	2017-06-09
		 *
		 * @return	The current log file size.
		 **************************************************************************************************/
		const size_t CurrentFileSize(void);
		const std::string CurrentDateTime(void);

		void SplitFile(void);

	private:
		static Logger*	instance_;
		static bool		initialized_; // Singleton instance 생성 여부

		LogMode			print_mode_ = LogMode::DEFAULT;

		int				file_size_ = 0;
		std::string		folder_path_ = std::string();
		std::string		full_path_ = std::string();
	};
} // namespace log
