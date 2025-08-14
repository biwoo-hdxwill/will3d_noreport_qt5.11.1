#include "W3Logger.h"

/* standard C++ Libs */
#include <fstream>
#include <iostream>
#include <assert.h>

/* Qt Libs */
#include <qdatetime.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qsettings.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <QDebug>

namespace
{
	const std::string	kFileName = "WILL3DLog.txt";
	const int			kDefaultFileSize = 40;
	const std::string	kDefaultPath = "./Logs"; // 읽어온 경로가 잘못되었을 때 지정해 줄 기본 로그의 경로
	const QString		kSectionKeyPath = "LOG/path";
	const QString		kSectionKeySize = "LOG/size";
	const std::string	kMsgEmpty = "";
	/** @brief	The section key : .ini 파일에서 log mode의 구분자.
	0 = default mode
	1 = debug mode
	*/
	const QString		kSectionKeyMode = "LOG/mode";

	const size_t		kByteToMB = 1000 * 1000;
}

namespace common
{
	/*
	*  헤더에서 선언한 static 멤버들을 다시한전 선언한다.
	* 이는 static 멤버를 일반 멤버함수처럼 참조할 수 있도록 한다(?)
	*/
	bool Logger::initialized_ = false;
	Logger* Logger::instance_ = nullptr;

	Logger::Logger() {}

	Logger::~Logger(void)
	{
		initialized_ = false;
		this->SaveCurrentSettings();
	}

	void Logger::Init(void)
	{
		LoadPath();
	}

	Logger* Logger::instance(void)
	{
		if (!instance_)
		{
			instance_ = new Logger();
			initialized_ = true;
		}
		return instance_;
	}

	void Logger::Print(const LogType & eType, const std::string & log)
	{
		if (!IsValidLogType(eType))
			return;

		this->Print(eType, kMsgEmpty, log);
	}

	void Logger::PrintAndAssert(const LogType & type, const std::string & txt)
	{
		this->Print(type, txt);
		assert(false);
	}

	void Logger::PrintDebugMode(const std::string & module_name, const std::string& log)
	{
		if (!this->IsDebugMode())
			return;

		this->Print(LogType::DBG, module_name, log);
	}

	void Logger::Print(const LogType& eType, const std::string& module_name, const std::string& log)
	{
		const std::string& strDT = this->getCurrentDateTime();
		const std::string& strType = this->GetTypeText(eType);

		std::fstream os;
		os.open(full_path_, std::ios_base::out | std::ios_base::app);
		if (!os.is_open())
		{
			std::cout << "log file open error" << std::endl;
			return;
		}
		if (module_name.size() == 0)
		{
			os << strDT << strType << log << std::endl;
#if DEVELOP_MODE
			std::cout << strDT << strType << log << std::endl;
#else
#ifdef COMMON_DBG_MODE
			std::cout << strDT << strType << log << std::endl;
#endif
#endif
		}
		else
		{
			os << strDT << strType << "[" << module_name << "] " << log << std::endl;
#if DEVELOP_MODE
			std::cout << strDT << strType << "[" << module_name << "] " << log << std::endl;
#else
#ifdef COMMON_DBG_MODE
			std::cout << strDT << strType << "[" << module_name << "] " << log << std::endl;
#endif
#endif
		}

		os.close();

		size_t byteFileSize = this->CurrentFileSize();
		if (byteFileSize > file_size_ * 1000 * 1000)
			this->SplitFile();
	}

	void Logger::SaveCurrentSettings(void)
	{
		std::cout << "main application finished" << std::endl;
		std::cout << "save current log information" << std::endl;

		int mode = print_mode_ == LogMode::DEFAULT ? 0 : 1;

		QSettings settings("Will3D.ini", QSettings::IniFormat);
		settings.setIniCodec(QTextCodec::codecForName("UTF-8"));
		settings.setValue(kSectionKeyPath, QString::fromLocal8Bit(folder_path_.c_str()));
		settings.setValue(kSectionKeySize, file_size_);
		settings.setValue(kSectionKeyMode, mode);
	}

	void Logger::ChangeLogPath(void)
	{
		QString current_path = folder_path_.size() == 0 ?
			QString::fromLocal8Bit(kDefaultPath.c_str()) :
			QString::fromLocal8Bit(folder_path_.c_str());

		QString	folder_name = QFileDialog::getExistingDirectory(nullptr,
			"Select Log Folder",
			current_path,
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
		if (folder_name.size() == 0)
			return;

		folder_path_ = folder_name.toLocal8Bit();

		this->VerifyFolderPath();
		this->IsValidLogFile();
	}

	inline const bool Logger::IsDebugMode(void) const
	{
#ifdef COMMON_DBG_MODE
		return true; //  smseo : debug mode 일 경우에는 모든 로그를 조건 검사 없이 항상 출력함
#endif
		return (print_mode_ == LogMode::DEBUG) ? true : false;
	}

	void Logger::LoadPath(void)
	{
		QSettings settings("Will3D.ini", QSettings::IniFormat);
		settings.setIniCodec(QTextCodec::codecForName("UTF-8"));

		QString path = settings.value(kSectionKeyPath).toString();
		int file_size = settings.value(kSectionKeySize).toInt();
		int print_mode = settings.value(kSectionKeyMode).toInt();

		if (path.size() > 0)
		{
			QFileInfo fi(path);
			folder_path_ = (fi.absoluteFilePath()).toLocal8Bit();
		}
		else
		{
			QFileInfo fi(QString::fromLocal8Bit(kDefaultPath.c_str()));
			folder_path_ = (fi.absoluteFilePath()).toLocal8Bit();
		}

		qDebug() << "folder_path_ :" << folder_path_.c_str();

		file_size_ = file_size > 0 ? file_size : kDefaultFileSize;
		print_mode_ = print_mode == 0 ? LogMode::DEFAULT : LogMode::DEBUG;

		this->VerifyFolderPath();
		this->IsValidLogFile();
	}

	void Logger::VerifyFolderPath(void)
	{
		QDir dir(QString::fromLocal8Bit(folder_path_.c_str()));
		if (!dir.exists())
		{
			if (!dir.mkpath(dir.absolutePath()))
			{
				std::cout << "wrong path, set to default path" << std::endl;
				QDir defaultDir(QString::fromLocal8Bit(kDefaultPath.c_str()));
				if (!defaultDir.exists())
					defaultDir.mkpath(defaultDir.absolutePath());
				folder_path_ = defaultDir.absolutePath().toStdString();
			}
		}
		//std::cout << "selected log folder : " << folder_path_ << std::endl;
	}

	const bool Logger::IsValidLogFile(void)
	{
		const std::string strFullPath = folder_path_ + "/" + kFileName;
		std::fstream fstream;
		fstream.open(strFullPath);

		if (!fstream.is_open())
		{
			fstream.open(strFullPath, std::fstream::out);

			if (!fstream.is_open())
			{
				return false;
			}
		}
		fstream.close();

		full_path_ = strFullPath;
		return true;
	}

	const bool Logger::IsValidLogType(const LogType & eLogType)
	{
#ifdef COMMON_DBG_MODE
		return true; //  smseo : debug mode 일 경우에는 모든 로그를 조건 검사 없이 항상 출력함
#endif
		if (print_mode_ == LogMode::DEFAULT)
		{
			if (eLogType == LogType::DBG)
			{
				return false;
			}
		}
		return true;
	}

	const std::string Logger::GetTypeText(const LogType & eLogType)
	{
		std::string logTxt;
		switch (eLogType)
		{
		case LogType::DBG:
			logTxt = "[DBG] ";
			break;
		case LogType::CMD:
			logTxt = "[CMD] ";
			break;
		case LogType::INF:
			logTxt = "[INF] ";
			break;
		case LogType::WRN:
			logTxt = "[WRN] ";
			break;
		case LogType::ERR:
			logTxt = "[ERR] ";
			break;
		default:
			break;
		}
		return logTxt;
	}

	const std::string Logger::getCurrentDateTime(void)
	{
		QDateTime dt = QDateTime::currentDateTime();
		QString strDT = dt.toString("[yyyy-MM-dd hh:mm:ss.zzz] ");
		return strDT.toStdString();
	}

	const std::string Logger::CurrentDateTime(void)
	{
		QDateTime date_time = QDateTime::currentDateTime();
		QString date_time_text = date_time.toString("[yyyy-MM-dd hh:mm:ss.zzz] ");
		return date_time_text.toStdString();
	}

	const size_t Logger::CurrentFileSize(void)
	{
		// file 크기를 얻어오기 위하여 length 구해서 byte 단위로 리턴
		std::ifstream is;
		is.open(full_path_, std::ios::binary);
		is.seekg(0, std::ios::end);
		return is.tellg();
	}

	void Logger::SplitFile(void)
	{
		QDateTime dt = QDateTime::currentDateTime();
		QString qstrDT = dt.toString("[yyyy-MM-dd]") + "WILL3DLog.txt";
		std::string strFileName = qstrDT.toStdString();
		std::string strOldLogFullPath = folder_path_ + "/" + strFileName;

		auto ret = rename(full_path_.c_str(), strOldLogFullPath.c_str());
		std::cout << "change success : " << ret << std::endl;
	}
} // namespace log
