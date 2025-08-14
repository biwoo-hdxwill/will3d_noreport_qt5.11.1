#include <Qt3DCore/QAbstractAspect>

#include <QtWidgets/QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QTextCodec>
#include <QSettings>
// IPC 170120
#include <QLocalServer>
#include <QLocalSocket>
#include <QCryptographicHash>
#include <QProcess>
#include <QDateTime>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtNetwork>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

#include "../SubProjects/Engine/Common/Common/global_preferences.h"
#include "../SubProjects/Engine/Common/Common/W3Theme.h"
#include "../SubProjects/Engine/Common/Common/W3Logger.h"
#include "../SubProjects/Engine/Common/Common/W3Memory.h"
#include "network_util.h"
#include "SplashScreen.h"
#if defined(_WIN32)
#include "../SubProjects/Engine/Common/Common/sw_info.h"
#endif

#include "../SubProjects/Engine/Common/Common/language_pack.h"
#include "hasp_api.h"
#ifdef WILL3D_VIEWER
// 20250210 LIN
#include <QToolButton> // for messagebox button
#include <QUrl>
#include <QDesktopServices>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QProgressDialog>
#include <QProcess>
#include <QDir>
#include <QDirIterator>
#include <QTimer>
#include <QApplication>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QSettings>

// DB관련
#include <QtSql/qsqldatabase.h>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include "../SubProjects/Engine/Common/Common/W3Define.h"
#include "../SubProjects/Engine/Common/Common/W3MessageBox.h"
#include "../SubProjects/Engine/Common/Common/W3Enum.h"
#endif

// smseo : GPU Selection code
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

const QString registryKey = "HKEY_CURRENT_USER\\SOFTWARE\\Wow6432Node\\Microsoft\\TempData";
const QString registryValue = "MicrosoftUID";
const int demoDays = 30;
#ifndef WILL3D_VIEWER
hasp_status_t CheckFeatureID(uint feature_id) {
  const hasp_feature_t feature = feature_id;

  hasp_handle_t handle = HASP_INVALID_HANDLE_VALUE;
  hasp_status_t status;

  unsigned char vendor_code[] =
    "R24a9rlqD985/vf6Fk7TNx/NzFRuIlA+9t78mEFCvIKyHmHto1jZixXVnGl7I88mgeAQxD/XGddDfqlM"
    "3HhFNm0gdtHdMo951f5XMF6hSuUgWboVF4fFT6lv9YF/QxqDSA6gOIXa8GsCm3p7JW/eyTxSTlUnVgi8"
    "qRVMXbndzvRJ813HycSc85ZNSe2J5fALA/00gJi6oDKy26S0YzjcKlr2myzpdFS9HDiFNx1MvM8KzYQ/"
    "EVjQeQ8BdhNkutKLGaF2cQ3yy8RBrPgX0VqEnEKUX9wwJSizpPngv7xuPkLjNSghMzyBEhB2jWN4aUK3"
    "MwYngMN05/dvUr8oBLY9pXOOiP0qtfNk6QRGqBzRNJ0HU2VLv72m8MlwemM7StKrdjSAGexXFS+paN7W"
    "d+PmSuKM2A8ngcxnkSBnh1zQdRrvrOgvNnIAewDFS1OLaIhsHfjzjmAZaMw66TF3dRD9j8IV6Du7nqeU"
    "YdWbM4m/HFNjY1PnuibpuSeRPHLZMJyUP0gVy7lq6tFBXE+AU8+qc20WY6RC7BkyG9UeIheXShFEUBRB"
    "67jx21EbZ+uuNx7KKuR5aGmmbpBEdp/K0GxGUtNX2wlxufRr+MAUqFMtXB9zUenN91bKRPKidQUOwXtT"
    "wxk1GNxQIjczE+dH6v2cYZJ107YTGp3XBgmYv9JwboBOZsDixDtqzOyvervSbhOLDn/6YYHLYHi4yQdX"
    "02BJzwHTGxNPP8B9R9xGum0sp2UneDQPg/vIFmyLcVYxsJVuK/UkgxLOuPIuFdVOtFwLFjZjWVNjb5z5"
    "eCN2El7wBjgJc0pLLjZ3G1EV1X9kmWJAkdsx+M5184SiiFG5NVnNRig9oNekkTBo1SGkec59w0POPrT5"
    "MIVOgNKo//wJVtpihmyE9hRDycEEygI3nDJylKHd2mV6FldPizJjqO9fVvEqF6YEebngoFyoXbvgrjH8"
    "yqlaPTObmRADbrkvkXYYsQ==";

  /*"T1YgIJURgQNG9ccQqB8RmjrPShYT12VbBRfhMJDfkiptALQ+4sUQTNiL28y0yovKVTuARR7oBGq6C84X"
  "2i+yLD/0CqxZXIaR6eZXBNWORdjHh5FgmHkruvYl86xB9xTij70tNVd6jmf/vnRvcR8Iot7dbvYgTso5"
  "LUNKbfoZTIpfYiWhgQbNUMPK9XdDUNCnzwy289sABtJtpZwOuS3cQrNl6kJ9obpi11xjChRDYdqAtZ4x"
  "vfh9hrLieNFCQrLSep88LDuLthfLUsKGvl+SGo2trpEq8IfQTw7GlES54VzGaPfx6swV8DVACDQ8C/gi"
  "0uvMUHTtn3j/3xdvUEvCWaRLAugG6FHBjmhJWUOmlGNgy+E42Yd6qdcj0umq2urzI2fN3ftNN0lG557O"
  "iAzIT1UXEyNjK3ETW0sMs/nlQVuRmPI9G4+PcXIoaWnLS+r7EdxQqNFIlpr0/aiRpt0KfJN2Oixzqr6Z"
  "lAbPw2+JeoXFnJlp8TxGBe8/cP65XVNY32wYf0YL2cSpTYl6VyhXTbSifDBzQLojbhSNFkqWS26wRYM6"
  "3HiJJSEjHoc4XO/jJK69s8n52iGks/GJNXgrZvnIs/xMTiEC9+lpvNryQ58yjiL/oYVn9bJiR5SbrB1A"
  "FqSMXAG8YpUISxoOFVN2OnH6izOxwYGf/Qa6yoajaW9AAtqx9KdqT8/iDFZmyk/EI2fGQq4bb3SFeZl+"
  "YHhk1WRgpcikBucbOdfMyb1aT4uPTizxojlQvYb52AVe52vhRS6hj0HL/I5T6xXVhYq4ip4zUjlwl5FP"
  "E3DcVGKJwkDRdeojCDUNNCcjJczARkcNsUVKgQDXqfK1vtQGZbzfFruQn10K+pzGV38BzDcGb8XRLZvu"
  "nBy0a40LoohZmDjjtolODK29Ipz7yy/gOwJJjqMK804q2x+QUjE+tkiZKAmQm6gxGqM3tZHtjbPhp61P"
  "HVDokjII10oMBO4uwrLL2Q==";*/

  status = hasp_login(feature, vendor_code, &handle);

  /* check if operation was successful */
  if (status != HASP_STATUS_OK) {
    switch (status) {
    case HASP_FEATURE_NOT_FOUND:
      break;
    case HASP_HASP_NOT_FOUND:
      break;
    case HASP_OLD_DRIVER:
      break;
    case HASP_NO_DRIVER:
      break;
    case HASP_INV_VCODE:
      break;
    case HASP_FEATURE_TYPE_NOT_IMPL:
      break;
    case HASP_TMOF:
      break;
    case HASP_TS_DETECTED:
      break;
    default:
      break;
    }
  }

  return status;
}
#endif

//Get hardware ID
QString getHardwareID() {
  QProcess process;
  process.start("wmic csproduct get UUID");
  process.waitForFinished();

  QString output = process.readAllStandardOutput().trimmed();
  QString errorOutput = process.readAllStandardError().trimmed(); // 오류 메시지 확인

  if (!errorOutput.isEmpty()) {
    qDebug() << "wmic 실행 오류:" << errorOutput;
    return QString();
  }

  QStringList lines = output.split("\n");
  lines.removeAll("");  // 빈 문자열 제거
  if (lines.size() > 1) {
    return lines[1].trimmed(); // UUID 값 반환
  }

  return QString();
}

// hardware ID + DateTime
QString generateLicenseKey(const QString& hwID, const QDate& startDate) {
  QByteArray data = (hwID + startDate.toString("yyyy-MM-dd")).toUtf8();
  return QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex();
}

bool fetchTimeFromAPI(QDate &date) {
  QNetworkAccessManager manager;
  QNetworkRequest request(QUrl("https://www.timeapi.io/api/Time/current/zone?timeZone=UTC"));
  request.setHeader(QNetworkRequest::UserAgentHeader, "QtApp");

  QNetworkReply *reply = manager.get(request);

  
  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec(); // 여기서 블로킹 (요청이 끝날 때까지 기다림)

  // 요청 완료 후 결과 확인
  if (reply->error() != QNetworkReply::NoError) {
    qDebug() << "HTTP 요청 실패:" << reply->errorString();
    reply->deleteLater();
    return false; // 빈 JSON 반환
  }

  // JSON 응답 파싱
  QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll());
  QJsonObject jsonObj = jsonDoc.object();

  reply->deleteLater();

  if (!jsonObj.isEmpty()) {
    qDebug() << "NTP 시간:" << jsonObj["year"].toInt() << "-"
      << jsonObj["month"].toInt() << "-"
      << jsonObj["day"].toInt();
  }

  date.setDate(jsonObj["year"].toInt(), jsonObj["month"].toInt(), jsonObj["day"].toInt());

  return date.isValid();

}

// 데모 기간 체크
bool isDemoExpired() {
  QSettings settings(registryKey, QSettings::NativeFormat);
  QVariant storedLicense = settings.value(registryValue);

  QString hwID = getHardwareID();
  if (hwID.isEmpty()) return true; // 하드웨어 ID 가져오기 실패 시 실행 차단

  QDate serverDate;
  if (fetchTimeFromAPI(serverDate) == false)
    return true;

  //serverDate = QDate(2025, 1, 1);
  //QDate serverDate = getHTTPTime();
  if (!serverDate.isValid()) 
    return true; // 서버 시간을 못 가져오면 실행 차단

  if (storedLicense.isValid()) {
    QString existingLicense = storedLicense.toString();
    for (int i = 0; i <= demoDays; i++) {
      QDate testDate = serverDate.addDays(-i);
      QString testLicense = generateLicenseKey(hwID, testDate);
      if (existingLicense == testLicense) return false; // 유효한 라이선스
    }
    return true; // 기간 초과 또는 변조됨
  }
  else {
    
    settings.setValue(registryValue, generateLicenseKey(hwID, serverDate));
    settings.sync();
  }
  return false;
}

#ifdef WILL3D_VIEWER
//void CopyDirectory(const QString &source_dir, const QString &target_dir, int &progress, int totalFiles, QProgressDialog *progressDialog)
//{
//	QDir source(source_dir);
//	QDir target(target_dir);
//
//	if (!target.exists())
//	{
//		target.mkpath(target.absolutePath());
//	}
//
//	QStringList entries = source.entryList(QDir::NoDotAndDotDot | QDir::AllEntries);
//	for (const QString &entry : entries)
//	{
//		QString srcPath = source.absoluteFilePath(entry);
//		QString destPath = target.absoluteFilePath(entry);
//		QFileInfo fileInfo(srcPath);
//
//		if (fileInfo.isDir())
//		{
//			CopyDirectory(srcPath, destPath, progress, totalFiles, progressDialog);
//		}
//		else
//		{
//			if (QFile::exists(destPath))
//			{
//				QFile::remove(destPath);
//			}
//			QFile::copy(srcPath, destPath);
//		}
//
//		progress++;
//		int progressValue = static_cast<int>((progress * 100) / totalFiles);
//		progressDialog->setValue(progressValue);
//
//		if (progressDialog->wasCanceled())
//		{
//			return;
//		}
//	}
//}
//
//void CopyImplantFolderWithCallback(std::function<void()> callback)
//{
//	qDebug() << "enter copy implant folder with call back func ";
//	QString appDir = "C:/Will3D";
//	QString implant_folder = appDir + "/Implant";
//	QProgressDialog *progressDialog = new QProgressDialog;
//	progressDialog->setLabelText("Copy Implant Folder...");
//	progressDialog->setWindowTitle("Copy Progress");
//	progressDialog->setCancelButtonText("Cancel");
//	progressDialog->setRange(0, 100);
//	progressDialog->setModal(true);
//	progressDialog->setValue(0);
//	progressDialog->show();
//
//	QFutureWatcher<void> *watcher = new QFutureWatcher<void>;
//
//	QObject::connect(watcher, &QFutureWatcher<void>::finished, [progressDialog, watcher, callback]()
//									 {
//		progressDialog->setValue(100);
//		progressDialog->close();
//		watcher->deleteLater();
//		progressDialog->deleteLater();
//		if (callback) {
//			callback(); 
//		} });
//
//	QFuture<void> future = QtConcurrent::run([implant_folder, progressDialog]()
//																					 {
//																						 QString target_path = QCoreApplication::applicationDirPath();
//																						 QString target_implant_folder = QDir(target_path).absoluteFilePath("Implant");
//
//																						 int curr_products_count = 0;
//																						 int target_products_count = 0;
//
//																						 QDirIterator it(implant_folder, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
//																						 while (it.hasNext())
//																						 {
//																							 it.next();
//																							 curr_products_count++;
//																						 }
//
//																						 QDirIterator target_it(target_implant_folder, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
//																						 while (target_it.hasNext())
//																						 {
//																							 target_it.next();
//																							 target_products_count++;
//																						 }
//
//																						 if (curr_products_count > target_products_count)
//																						 {
//																							 int progress = 0;
//																							 CopyDirectory(implant_folder, target_implant_folder, progress, curr_products_count, progressDialog);
//																						 } });
//
//	watcher->setFuture(future);
//}
//
//void handleDownloadError(QNetworkReply::NetworkError code, QNetworkReply *reply)
//{
//  qDebug() << "Download error occurred. Error code:" << code;
//  QMessageBox::critical(nullptr, "Download Error", "Failed to download the file: " + reply->errorString());
//}
//
//
//void DownloadAndSetSettingFile(QString readFilePath, QString outScriptPath, SettingFileType type)
//{
//	CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_94(), CW3MessageBox::Question);
//	message_box.m_btnOK->setText("Install");
//	message_box.m_btnCancel->setText("Cancel");
//
//	if (message_box.exec() == QDialog::Accepted)
//	{
//    QDate online_check;
//    if (fetchTimeFromAPI(online_check)==false) {
//      CW3MessageBox message_box("Will3D", "Please connect to the Internet and try again", CW3MessageBox::Warning);
//      message_box.m_btnCancel->hide();
//      message_box.exec();
//
//      exit(0);
//      return;
//    }
//
//		QProgressDialog *progressDialog = new QProgressDialog;
//		progressDialog->setLabelText("Downloading...");
//		progressDialog->setWindowTitle("Download Progress");
//		progressDialog->setCancelButtonText(nullptr);
//		progressDialog->setRange(0, 100);
//		progressDialog->setModal(true);
//
//		QNetworkAccessManager *manager = new QNetworkAccessManager();
//		QNetworkReply *reply = manager->get(QNetworkRequest(QUrl("https://hdxwill.com/download/Will3D_Implant_22.0.0.exe")));
//
//    QString savePath = QDir::currentPath() + "/Will3D_Implant_22.0.0.exe";
//    QFile file(savePath);
//    if (file.open(QIODevice::WriteOnly) ==false) {
//      qDebug() << "Download file open fail" <<endl;
//    }
//
//   
//    QObject::connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
//      [reply](QNetworkReply::NetworkError) {
//      CW3MessageBox message_box("Will3D", "The internet connection has been lost. Please check and try again.", CW3MessageBox::Warning);
//      message_box.m_btnCancel->hide();
//      message_box.exec();
//      reply->deleteLater();
//    });
//
//		QObject::connect(reply, &QNetworkReply::downloadProgress,
//										 [progressDialog](qint64 bytesReceived, qint64 bytesTotal)
//										 {           
//											 if (bytesTotal > 0)
//											 {
//												 int progress = static_cast<int>((bytesReceived * 80) / bytesTotal);
//												 progressDialog->setValue(progress);
//											 }
//										 });
//
//    QObject::connect(reply, &QNetworkReply::readyRead, [&]() {
//      if (file.write(reply->readAll()) < 0) {
//      
//        reply->abort();  // 다운로드 중단
//      }
//    });
//
//
//		QObject::connect(reply, &QNetworkReply::finished, [&]()
//										 {
//
//      qDebug() << reply->error();
//			if (reply->error() == QNetworkReply::NoError) {
//				QString savePath = QDir::currentPath() + "/Will3D_Implant_22.0.0.exe";
//				//QFile file(savePath);
//				//if (file.open(QIODevice::WriteOnly)) 
//        {
//					//file.write(reply->readAll());
//					file.close();
//					progressDialog->setValue(80);
//					QProcess *process = new QProcess;
//					QStringList arguments;
//					arguments << "-Command" << "Start-Process -FilePath '" + savePath + "' -Verb runAs -Wait" << QString::number(static_cast<int>(type));
//
//					process->setProgram("powershell");
//					process->setArguments(arguments);
//
//					QObject::connect(process, &QProcess::stateChanged, [=](QProcess::ProcessState newState) {
//						if (newState == QProcess::NotRunning) { 
//							if (progressDialog) {
//								progressDialog->setValue(100);
//								progressDialog->close();
//							}
//							CopyImplantFolderWithCallback([=]() {
//								CSplashScreen *SplashScreen = new CSplashScreen(readFilePath, outScriptPath);
//								SplashScreen->show();
//								});
//
//							process->deleteLater();
//							progressDialog->deleteLater();
//							manager->deleteLater();
//							reply->deleteLater();
//						}
//						});
//
//
//					process->start();
//					if (!process->waitForStarted()) {
//						qDebug() << "Failed to start process. Error:" << process->errorString();
//					}
//					else {
//						progressDialog->setLabelText("Installation Implant...");
//						progressDialog->setValue(85);
//						qDebug() << "Process started successfully!";
//					}
//				}
//				//else 
//        //{
//				//	qDebug() << "Failed to save file";
//				//}
//			}
//			else {
//        CW3MessageBox message_box("Will3D", "Please connect to the Internet and try again", CW3MessageBox::Warning);
//        message_box.m_btnCancel->hide();
//        message_box.exec();
//
//        exit(0);
//        return;
//			} });
//
//		progressDialog->exec();
//	}
//	else
//	{
//		CopyImplantFolderWithCallback([&]()
//																	{
//			CSplashScreen *SplashScreen = new CSplashScreen(readFilePath, outScriptPath);
//			SplashScreen->show(); });
//	}
//}

#endif



int main(int argc, char *argv[])
{
 
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
	QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
#if 1
#if defined(_WIN32)
	SetProcessDPIAware();
	// QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
#else
#if 1
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#else
	QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
#endif
#endif

#if defined(__APPLE__)
	QSurfaceFormat format;
	format.setVersion(4, 1);
	format.setSamples(4);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setRenderableType(QSurfaceFormat::OpenGL);

	QSurfaceFormat::setDefaultFormat(format);
#endif

	QApplication a(argc, argv);
	QDir::setCurrent(QCoreApplication::applicationDirPath());

#ifndef WILL3D_VIEWER
  //hasp_status_t feature_index = CheckFeatureID(50);

  //if (feature_index != HASP_STATUS_OK)
  //{
  //  qDebug() << "lockkey not exist";
  //  if (isDemoExpired()) 
  //  {
  //    QMessageBox msg;
  //    msg.setText("The demo version has expired. \r\nPlease connect the USB key.");
  //    msg.exec();
  //    return 0;
  //  }
  //}
#endif

 
	using common::Logger;
	auto logger = Logger::instance();
	logger->Init();
	logger->Print(common::LogType::INF, "  ");
	logger->Print(common::LogType::INF, "Will3D - start");
#if defined(_WIN32)
	sw_info::SWInfo::printSWVersion();
	sw_info::SWInfo::printCPUInfo();
	sw_info::SWInfo::printCPUMemoryStatus();
#endif

	lang::LanguagePack::Init();

	// thyoo 160324
	CW3Theme::getInstance()->setAppTheme(&a);

	QString readFilePath, outScriptPath, moduleName;

	//QTextCodec *codec = QTextCodec::codecForName("eucKR");
	//QTextCodec::setCodecForLocale(codec);
  QLocale locale = QLocale::system();  // 시스템 로케일 확인
  QString language = QLocale::languageToString(locale.language());
  QString country = QLocale::countryToString(locale.country());


  QTextCodec *codec = QTextCodec::codecForName("eucKR");

  if (locale.language() == QLocale::Korean && locale.country() == QLocale::SouthKorea) {
    // 한국어 Windows라면 EUC-KR 사용
    codec = QTextCodec::codecForName("eucKR");
  }
  else {
    // 그 외는 UTF-8 기본
    codec = QTextCodec::codecForName("UTF-8");
  }


  QTextCodec::setCodecForLocale(codec);

  

#ifdef _DEBUG
#ifndef WILL3D_VIEWER
#if 1
	readFilePath = codec->toUnicode("F:\\Work\\[00]3DProject\\SampleDCM\\LeeSeungYeol_200x200\\[sample]send_ct_from_willmaster_to_will3d.txt");
	// readFilePath = codec->toUnicode("G:\\SIDEX02\\read_script.txt");
	// readFilePath = codec->toUnicode("G:\\오류영상\\산본엘치과_프로젝트_안열림\\20200611154232095_0449\\492920200828180042.w3d");
	// readFilePath = codec->toUnicode("F:\\Work\\[00]3DProject\\SampleDCM\\3DPhoto_Dicom\\고덕원과장_Dicom\\[sample]send_ct_from_willmaster_to_will3d.txt");
	// readFilePath = codec->toUnicode("F:\\Work\\[00]3DProject\\SampleDCM\\3DPhoto_Dicom\\고덕원과장_Dicom\\[sample]send_onlytrd_from_willmaster_to_will3d.txt");
	// readFilePath = codec->toUnicode("F:\\Work\\[00]3DProject\\SampleDCM\\3DPhoto_Dicom\\고덕원과장_Dicom\\[sample]send_ct+trd_from_willmaster_to_will3d.txt");
	// outScriptPath = codec->toUnicode("C:\\Will-Master\\TempDown\\20180808\\20180813\\CT3D\\ProjectOut-105111.txt");
	outScriptPath = codec->toUnicode("F:\\20180808-173928_out.txt");
#endif
#endif
#else
	if (argc == 2)
	{
		QString argument = codec->toUnicode(argv[1]);
		if (argument.contains(".w3d", Qt::CaseInsensitive)) // project file path
		{
			readFilePath = argument;
		}
	}
#endif

	for (int i = 0; i < argc; i++)
	{
		QString argument = codec->toUnicode(argv[i]);

		QByteArray ba = argument.toLocal8Bit();
		std::string stdArgument = ba.constData();
		// printf("%s\r\n", stdArgument.c_str());
		logger->Print(common::LogType::INF, stdArgument);

		if (argument.at(0) != '/')
		{
			continue;
		}

		QStringList listArgument = argument.split(':');
		if (listArgument.size() < 2)
		{
			continue;
		}

		QString command = listArgument.at(0);
		QString value = argument.right(argument.size() - 1 - command.size());
		if (command.compare("/read", Qt::CaseInsensitive) == 0) // ct image list sctipt path
		{
			readFilePath = value;
		}
		else if (command.compare("/project", Qt::CaseInsensitive) == 0) // project file path
		{
			readFilePath = value;
		}
		else if (command.compare("/out", Qt::CaseInsensitive) == 0) // output file sctipt path
		{
			outScriptPath = value;
		}
		else if (command.compare("/module", Qt::CaseInsensitive) == 0) // not use
		{
			moduleName = value;
		}
	}

	// readFilePath = codec->toUnicode("D:\\_dataset\\master\\20180626-135046.txt");
	// outScriptPath = codec->toUnicode("D:\\_dataset\\master\\20180626-135046_out.txt");

	FILE *file = nullptr;

	QByteArray baRead = readFilePath.toLocal8Bit();
	std::string stdReadFilePath = baRead.constData();
	QByteArray baOut = outScriptPath.toLocal8Bit();
	std::string stdOutScriptPath = baOut.constData();

	bool result = false;
#if defined(__APPLE__)
	file_ = fopen(stdReadFilePath.c_str(), "rt");
	result = (file_) ? true : false;
#else
	result = !fopen_s(&file, stdReadFilePath.c_str(), "rt");
#endif

	if (!result)
	{
		// printf("Read script : %s not found.\r\n", stdReadFilePath.c_str());
		logger->Print(common::LogType::WRN, std::string("Read script : ") + stdReadFilePath + std::string(" not found."));
		readFilePath = "";
		outScriptPath = "";
	}
	else
	{
		fclose(file);

		result = false;
#if defined(__APPLE__)
		file_ = fopen(stdOutScriptPath.c_str(), "rt");
		result = (file_) ? true : false;
#else
		result = !fopen_s(&file, stdOutScriptPath.c_str(), "rt");
#endif

		if (!result)
		{
			// printf("Out script : %s not found.\r\n", stdOutScriptPath.c_str());
			logger->Print(common::LogType::WRN, std::string("Out script : ") + stdOutScriptPath + std::string(" not found."));
			outScriptPath = "";
		}
		else
		{
			fclose(file);
		}
	}

	QSettings settings(GlobalPreferences::GetInstance()->ini_path(), QSettings::IniFormat);
	bool singleton_process = settings.value("PROCESS/singleton", true).toBool();

	if (singleton_process)
	{
#if defined(_WIN32)
		// IPC 170120 send argv to executed will3d
		HWND hwnd = FindWindow(L"Qt5QWindowIcon", L"Will3D");
		if (hwnd) // will3d 가 이미 실행중
		{
			// 포커스 이동
			if (argc > 1) // 외부에서 호출 시 이미 실행중인 will3d로 argv만 전달
			{
				QLocalSocket *socket = new QLocalSocket();

				socket->abort();
				socket->connectToServer("Will3D");
				if (socket->waitForConnected())
				{
					QByteArray block;
					QDataStream out(&block, QIODevice::WriteOnly);
					out.setVersion(QDataStream::Qt_5_8);
					out << (unsigned short)0;
					out << readFilePath;
					out << outScriptPath;
					out << moduleName;
					out << (unsigned short)(block.size() - sizeof(unsigned short));

					socket->write(block);
					socket->flush();
				}

				if (socket->waitForBytesWritten())
				{
					SAFE_DELETE_OBJECT(socket);
				}
			}

			exit(0);
		}
#endif
	}

  
 
#ifdef WILL3D_VIEWER

  network_util *networkutil = new network_util();
  networkutil->getImplantSetup(readFilePath, outScriptPath);
 


//  QSettings settingsViewer("Will3D.ini", QSettings::IniFormat);
//  int use_implant = settingsViewer.value("IMPLANT/use_implant_in_viewer", 1).toInt();
//
//  if (use_implant)
//  {
//    // implant db있는 지에 따라 db설치여부를 결정
//  // 20250210 LIN Viewer 실행경로에 Implant폴더 없거나 안에 있는 products 흠결이 있는 경우에
//  //"C:/Will3D/에 있는 Implant폴더를 실행경로로 copy함
//  // copy끝나고 main thread이어서 진행한다
//    int port = GlobalPreferences::GetInstance()->preferences_.general.database.port;
//
//    QSettings odbc_settings("C:/Windows/ODBCINST.INI", QSettings::IniFormat);
//    QStringList odbc_group_list = odbc_settings.childGroups();
//    QString driver_name = "";
//    for (int i = 0; i < odbc_group_list.size(); ++i)
//    {
//      if (odbc_group_list.at(i).indexOf("MariaDB ODBC") == 0)
//      {
//        driver_name = odbc_group_list.at(i);
//      }
//    }
//    bool is_mariadb = false;
//    if (!driver_name.isEmpty())
//    {
//      is_mariadb = true;
//    }
//
//    driver_name.remove(" (32 bit)");
//    driver_name.remove(" (64 bit)");
//    bool is_implantdb = false;
//#if defined(__APPLE__)
//    m_db = QSqlDatabase::addDatabase("QMYSQL", "DBM");
//    m_db.setHostName("127.0.0.1");
//    m_db.setPort(port);
//    m_db.setDatabaseName("Will3D");
//    m_db.setUserName("root");
//#else
//    QSqlDatabase m_db = QSqlDatabase::addDatabase("QODBC", "DBM");
//#if MARIA_DB
//    m_db.setDatabaseName(QString("DRIVER={%1};SERVER=127.0.0.1;DATABASE=Will3D;Port=%2;UID=root;PWD=2002;WSID=.;").arg(driver_name).arg(port));
//#else
//    m_db.setDatabaseName("DRIVER={SQL Server};SERVER=127.0.0.1\\HDXWILL2014;DATABASE=Will3D;Port=1433;UID=sa;PWD=2002;WSID=.;");
//#endif
//#endif
//    if (m_db.open())
//    {
//      QSqlQuery query(m_db);
//#if MARIA_DB
//      query.exec("SHOW DATABASES;");
//#else
//      query.exec("SELECT name FROM sys.databases;");
//#endif
//      while (query.next())
//      {
//        QString dbName = query.value(0).toString();
//        if (dbName == QString("implant"))
//        {
//          is_implantdb = true;
//        }
//      }
//      m_db.close();
//    }
//    else
//    {
//      // qDebug() << "DB Error: " << m_db.lastError().text();
//      is_implantdb = false;
//    }
//
//
//    if (is_implantdb && is_mariadb)
//    {
//      QString appDir = "C:/Will3D";
//      QString implant_folder = appDir + "/Implant";
//
//      QDir dir(implant_folder);
//      QStringList folders = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
//      int implant_folder_count = folders.size();
//
//
//      int ini_implant_count = settingsViewer.value("IMPLANT/using_implant_count", 17).toInt();
//      // 현재 버전의 Implant 수
//      if (implant_folder_count < ini_implant_count || !dir.exists())
//      {
//        DownloadAndSetSettingFile(readFilePath, outScriptPath, SettingFileType::IMPLANTDB);
//      }
//      else
//      {
//        CopyImplantFolderWithCallback([&]()
//        {
//          CSplashScreen *SplashScreen = new CSplashScreen(readFilePath, outScriptPath);
//          SplashScreen->show(); });
//      }
//    }
//    else if (!is_implantdb && is_mariadb)// implant설치X, Will3D설치O
//    {
//      DownloadAndSetSettingFile(readFilePath, outScriptPath, SettingFileType::IMPLANTDB);
//    }
//    else if (!is_implantdb && !is_mariadb)// Implant설치X, Will3D 설치X
//    {
//      DownloadAndSetSettingFile(readFilePath, outScriptPath, SettingFileType::MARIADB_AND_IMPLANTDB);
//    }
//
//  }
//  else
//  {
//
//    	CSplashScreen *SplashScreen = new CSplashScreen(readFilePath, outScriptPath);
//			SplashScreen->show();
//  }
// 
//
//	
//	//===
#else
network_util *networkutil = new network_util();
networkutil->getWill3DSetup(readFilePath, outScriptPath);

#endif

   // CSplashScreen SplashScreen(readFilePath, outScriptPath);
   // SplashScreen.show();
  
	return a.exec();
}
