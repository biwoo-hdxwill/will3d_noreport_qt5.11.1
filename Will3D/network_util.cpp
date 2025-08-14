#include "network_util.h"
#include <QToolButton> 
#include <QtConcurrent>
#include <QMessageBox>

#include <QtSql/qsqldatabase.h>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include "../SubProjects/Engine/Common/Common/global_preferences.h"
#include "../SubProjects/Engine/Common/Common/language_pack.h"
#include "../SubProjects/Engine/Common/Common/W3Define.h"
#include "../SubProjects/Engine/Common/Common/W3MessageBox.h"
#include "../SubProjects/Engine/Common/Common/sw_info.h"

#include "SplashScreen.h"

network_util::network_util(QObject *parent)
	: QObject(parent)
{
  registryKey = "HKEY_CURRENT_USER\\SOFTWARE\\Wow6432Node\\Microsoft\\TempData";
  registryValue = "MicrosoftUID";
  demoDays = 30;
 
  current_download_file_path_ = QDir::currentPath() + "/Will3D_Implant_22.0.0.exe";

  implant_copy_watcher_ = new QFutureWatcher<void>;
  progressDialog_ = new QProgressDialog;
  network_manager_ = new QNetworkAccessManager();
  
  connect(implant_copy_watcher_, SIGNAL(finished()), this, SLOT(onWatcherFinished()));
  connect(this, SIGNAL(sigSetProgress(int)), this, SLOT(onSetProgress(int)));
}

network_util::~network_util()
{
  if(progressDialog_) {
    progressDialog_->setValue(100);
    progressDialog_->close();
    progressDialog_->deleteLater();
    progressDialog_ = nullptr;
  }

  if (implant_copy_watcher_) {
    implant_copy_watcher_->deleteLater();
    implant_copy_watcher_ = nullptr;
  }

  if (setup_process_) {
    setup_process_->deleteLater();
    setup_process_ = nullptr;
  }

  if (network_manager_)
  {
    network_manager_->deleteLater();
    network_manager_ = nullptr;
  }

  if (network_reply_)
  {
    network_reply_->deleteLater();
    network_reply_ = nullptr;
  }
}

////Get hardware ID
//QString network_util::getHardwareID() {
//  QProcess process;
//  process.start("wmic csproduct get UUID");
//  process.waitForFinished();
//
//  QString output = process.readAllStandardOutput().trimmed();
//  QString errorOutput = process.readAllStandardError().trimmed(); // 오류 메시지 확인
//
//  if (!errorOutput.isEmpty()) {
//    qDebug() << "wmic 실행 오류:" << errorOutput;
//    return QString();
//  }
//
//  QStringList lines = output.split("\n");
//  lines.removeAll("");  // 빈 문자열 제거
//  if (lines.size() > 1) {
//    return lines[1].trimmed(); // UUID 값 반환
//  }
//
//  return QString();
//}
//
//// hardware ID + DateTime
//QString network_util::generateLicenseKey(const QString& hwID, const QDate& startDate) {
//  QByteArray data = (hwID + startDate.toString("yyyy-MM-dd")).toUtf8();
//  return QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex();
//}
//
void network_util::getWill3DSetup(QString readFilePath, QString outScriptPath)
{
  readFilePath_ = readFilePath;
  outScriptPath_ = outScriptPath;

  progressDialog_->setLabelText("Checking for updates");
  progressDialog_->setWindowTitle("Update");


  if (networkCheck() == false) {
   
    runSplash();
    this->deleteLater();
  }
  else
  {
    //s3 version info read
    QString version_url = "https://hdxwill.com/download/Will3D_version.ini";
    QString download_path = QDir::currentPath() + "/Will3D_version.ini";
    downloadVersion(SettingFileType::VERSION_INI, version_url, download_path);
  }


}


void network_util::getImplantSetup(QString readFilePath, QString outScriptPath)
{
  QSettings settingsViewer("Will3D.ini", QSettings::IniFormat);
  int use_implant = settingsViewer.value("IMPLANT/use_implant_in_viewer", 1).toInt();

  if (use_implant)
  {
    // implant db있는 지에 따라 db설치여부를 결정
  // 20250210 LIN Viewer 실행경로에 Implant폴더 없거나 안에 있는 products 흠결이 있는 경우에
  //"C:/Will3D/에 있는 Implant폴더를 실행경로로 copy함
  // copy끝나고 main thread이어서 진행한다
    int port = GlobalPreferences::GetInstance()->preferences_.general.database.port;

    QSettings odbc_settings("C:/Windows/ODBCINST.INI", QSettings::IniFormat);
    QStringList odbc_group_list = odbc_settings.childGroups();
    QString driver_name = "";
    for (int i = 0; i < odbc_group_list.size(); ++i)
    {
      if (odbc_group_list.at(i).indexOf("MariaDB ODBC") == 0)
      {
        driver_name = odbc_group_list.at(i);
      }
    }
    bool is_mariadb = false;
    if (!driver_name.isEmpty())
    {
      is_mariadb = true;
    }

    driver_name.remove(" (32 bit)");
    driver_name.remove(" (64 bit)");
    bool is_implantdb = false;
#if defined(__APPLE__)
    m_db = QSqlDatabase::addDatabase("QMYSQL", "DBM");
    m_db.setHostName("127.0.0.1");
    m_db.setPort(port);
    m_db.setDatabaseName("Will3D");
    m_db.setUserName("root");
#else
    QSqlDatabase m_db = QSqlDatabase::addDatabase("QODBC", "DBM");
#if MARIA_DB
    m_db.setDatabaseName(QString("DRIVER={%1};SERVER=127.0.0.1;DATABASE=Will3D;Port=%2;UID=root;PWD=2002;WSID=.;").arg(driver_name).arg(port));
#else
    m_db.setDatabaseName("DRIVER={SQL Server};SERVER=127.0.0.1\\HDXWILL2014;DATABASE=Will3D;Port=1433;UID=sa;PWD=2002;WSID=.;");
#endif
#endif
    if (m_db.open())
    {
      QSqlQuery query(m_db);
#if MARIA_DB
      query.exec("SHOW DATABASES;");
#else
      query.exec("SELECT name FROM sys.databases;");
#endif
      while (query.next())
      {
        QString dbName = query.value(0).toString();
        if (dbName == QString("implant"))
        {
          is_implantdb = true;
        }
      }
      m_db.close();
    }
    else
    {
      // qDebug() << "DB Error: " << m_db.lastError().text();
      is_implantdb = false;
    }

    QString download_url = "https://hdxwill.com/download/Will3D_Implant_Setup.exe";
    QString download_path = QDir::currentPath() + "/Will3D_Implant_Setup.exe";
    readFilePath_ = readFilePath;
    outScriptPath_ = outScriptPath;

    if (is_implantdb && is_mariadb)
    {
      QString appDir = "C:/Will3D";
      QString implant_folder = appDir + "/Implant";

      QDir dir(implant_folder);
      QStringList folders = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
      int implant_folder_count = folders.size();


      int ini_implant_count = settingsViewer.value("IMPLANT/using_implant_count", 17).toInt();
      // 현재 버전의 Implant 수
      if (implant_folder_count < ini_implant_count || !dir.exists())
      {
        downloadAndSetupImplant(SettingFileType::IMPLANTDB, download_url, download_path);
      }
      else
      {
        copyImplantFolderWithCallback();

      }
    }
    else if (!is_implantdb && is_mariadb)// implant설치X, Will3D설치O
    {
      downloadAndSetupImplant(SettingFileType::IMPLANTDB, download_url, download_path);
    }
    else if (!is_implantdb && !is_mariadb)// Implant설치X, Will3D 설치X
    {
      downloadAndSetupImplant(SettingFileType::MARIADB_AND_IMPLANTDB, download_url, download_path);
    }

  }

}

bool network_util::networkCheck() {
  QNetworkAccessManager manager;
  QNetworkRequest request(QUrl("http://1.1.1.1"));
  request.setHeader(QNetworkRequest::UserAgentHeader, "QtApp");

  QNetworkReply *reply = manager.get(request);
  int timeoutMs = 3000;
  QTimer timer;
  QEventLoop loop;

  // 타임아웃 시 loop 종료
  timer.setSingleShot(true);
  QObject::connect(&timer, &QTimer::timeout, [&]() {
    reply->abort();  // reply 중단
    loop.quit();
  });
  
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  timer.start(timeoutMs);  // 타이머 시작
  loop.exec(); // 여기서 블로킹 (요청이 끝날 때까지 기다림)

  if (reply->error() != QNetworkReply::NoError) {
    reply->deleteLater();
    return false; 
  }
  
  return true;
}


bool network_util::fetchTimeFromAPI(QDate &date) {
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

void network_util::downloadAndSetupWill3D(SettingFileType type, QString download_url, QString download_path)
{
  CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_96(), CW3MessageBox::Warning);
  message_box.m_btnOK->setText(lang::LanguagePack::txt_install());
  message_box.m_btnCancel->hide();
  message_box.resize(470,200);
  setup_type_ = type;


  if (message_box.exec() == QDialog::Accepted)
  {
    current_download_file_path_ = download_path;

    progressDialog_->setLabelText(lang::LanguagePack::txt_downloading());
    progressDialog_->setWindowTitle(lang::LanguagePack::txt_download_progress());
    progressDialog_->setCancelButtonText(nullptr);
    progressDialog_->setRange(0, 100);
    progressDialog_->setModal(true);


    downloadURL(download_url, download_path);

    progressDialog_->exec();
  }
  else
  {
    this->deleteLater();
    exit(0);
  }
}

void network_util::downloadAndSetupImplant(SettingFileType type, QString download_url, QString download_path)
{
  CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_94(), CW3MessageBox::Information);
  message_box.m_btnOK->setText("Install");
  message_box.m_btnCancel->setText(lang::LanguagePack::txt_cancel());
  setup_type_ = type;


  if (message_box.exec() == QDialog::Accepted)
  {

    if (networkCheck() == false) {
      CW3MessageBox message_box("Will3D", "Please connect to the Internet and try again", CW3MessageBox::Critical);
      message_box.m_btnCancel->hide();
      message_box.exec();

      exit(0);
      return;
    }

    current_download_file_path_ = download_path;

    progressDialog_->setLabelText("Downloading...");
    progressDialog_->setWindowTitle("Download Progress");
    progressDialog_->setCancelButtonText(nullptr);
    progressDialog_->setRange(0, 100);
    progressDialog_->setModal(true);


    downloadURL(download_url, download_path);

    progressDialog_->exec();
  }
  else
  {
    copyImplantFolderWithCallback();
    //splash
  }
}

void network_util::downloadVersion(SettingFileType type, QString download_url, QString download_path)
{
  setup_type_ = type;
  current_download_file_path_ = download_path;
  downloadURL(download_url, download_path);
}

void network_util::downloadURL(QString download_url, QString download_path)
{

  save_file_.setFileName(download_path);
  if (save_file_.exists())
    save_file_.remove();

  if (save_file_.open(QIODevice::ReadWrite) == false) {
    qDebug() << "Download file open fail" << endl;
  }

  network_reply_ = network_manager_->get(QNetworkRequest(QUrl(download_url)));

  connect(network_reply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onNetworkError(QNetworkReply::NetworkError)));
  connect(network_reply_, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onDownloadProgress(qint64, qint64)));
  connect(network_reply_, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
  connect(network_reply_, SIGNAL(finished()), this, SLOT(onReplyFinished()));;
}

void network_util::copyDirectory(const QString &source_dir, const QString &target_dir, int &progress, int totalFiles)
{
  struct DirPair {
    QString srcDir;
    QString dstDir;
  };

  QStack<DirPair> stack;
  stack.push({ source_dir, target_dir });
  int progressinc = 0;
  int progressValue = 0;

 
  while (!stack.isEmpty())
  {
    DirPair current = stack.pop();

    QDir source(current.srcDir);
    QDir target(current.dstDir);

    if (!target.exists())
    {
      target.mkpath(".");
    }
    
    QStringList entries = source.entryList(QDir::NoDotAndDotDot | QDir::AllEntries);

    for (const QString &entry : entries)
    {
      QString srcPath = source.absoluteFilePath(entry);
      QString dstPath = target.absoluteFilePath(entry);
      QFileInfo fileInfo(srcPath);

      if (fileInfo.isDir())
      {
        // 디렉터리는 나중에 처리하므로 스택에 push
        stack.push({ srcPath, dstPath });
      }
      else
      {
        if (QFile::exists(dstPath))
        {
          QFile::remove(dstPath);
        }
        QFile::copy(srcPath, dstPath);

        progressValue = static_cast<int>((progressinc * 100) / totalFiles);
        sigSetProgress(progressValue);
        progressinc++;
      }

      if (progressDialog_ && progressDialog_->wasCanceled())
        return;

    }
  }
}

void network_util::copyImplantFolderWithCallback()
{
  qDebug() << "enter copy implant folder with call back func ";
  QString appDir = "C:/Will3D";
  QString implant_folder = appDir + "/Implant";
  
  progressDialog_->setLabelText("Copy Implant Folder...");
  progressDialog_->setWindowTitle("Copy Progress");
  progressDialog_->setCancelButtonText("Cancel");
  progressDialog_->setRange(0, 100);
  progressDialog_->setModal(true);
  progressDialog_->setValue(0);
  progressDialog_->show();
 
  QFuture<void> future = QtConcurrent::run([this, implant_folder]()
  {
    QString target_path = QCoreApplication::applicationDirPath();
    QString target_implant_folder = QDir(target_path).absoluteFilePath("Implant");

    int curr_products_count = 0;
    int target_products_count = 0;

    QDirIterator it(implant_folder, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
      it.next();
      curr_products_count++;
    }

    QDirIterator target_it(target_implant_folder, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (target_it.hasNext())
    {
      target_it.next();
      target_products_count++;
    }

    if (curr_products_count > target_products_count)
    {
      int progress = 0;
      copyDirectory(implant_folder, target_implant_folder, progress, curr_products_count);
    } 
  });

  implant_copy_watcher_->setFuture(future);
  //future.waitForFinished();
  //implant_copy_watcher_->deleteLater();
}

void network_util::handleDownloadError(QNetworkReply::NetworkError code, QNetworkReply *reply)
{
  qDebug() << "Download error occurred. Error code:" << code;
  QMessageBox::critical(nullptr, "Download Error", "Failed to download the file: " + reply->errorString());
}





void network_util::onNetworkError(QNetworkReply::NetworkError)
{
  /*CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_98(), CW3MessageBox::Warning);
  message_box.m_btnCancel->hide();
  message_box.exec();*/

  // QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (network_reply_) {
    network_reply_->deleteLater();
  }
}

void network_util::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
  if (bytesTotal > 0)
  {
    int progress = static_cast<int>((bytesReceived * 80) / bytesTotal);
    if (progressDialog_) // nullptr 체크
    {
      progressDialog_->setValue(progress);

      if (progressDialog_->wasCanceled())
      {
        network_reply_->abort();  // 다운로드 중단
        //exit(0);
      }
    }
  }
}
void network_util::onReadyRead()
{
 // QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!network_reply_)
    return;

  if (save_file_.write(network_reply_->readAll()) < 0) {
    network_reply_->abort();  // 다운로드 중단
  }
}

//download finish
void network_util::onReplyFinished()
{
  //QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  save_file_.close();

  if (!network_reply_)
    return;

  qDebug() << network_reply_->error();
  if (network_reply_->error() == QNetworkReply::NoError) {

    // QFile file; 이 클래스의 멤버여야 합니다

    if (progressDialog_)
      progressDialog_->setValue(80);

    //Version check아니면
    if (setup_type_ < SettingFileType::VERSION_INI)
      runSetupFile();
    else
    {
      QString version_url, download_path;
      //Will3D 신규버전있으면 
      if (isNewVersion(VersionCheckType::WILL3D_VER))
      {
        version_url = "https://hdxwill.com/download/Will3D_Setup.exe";
        download_path = QDir::currentPath() + "/Will3D_Setup.exe";
        downloadAndSetupWill3D(SettingFileType::WILL3D_SETUP, version_url, download_path);
      }
      //Implant만 신규버전 있으면
      else if (isNewVersion(VersionCheckType::IMPLANT_VER))
      {
        version_url = "https://hdxwill.com/download/Will3D_Implant_Setup.exe";
        download_path = QDir::currentPath() + "/Will3D_Implant_Setup.exe";
        downloadAndSetupWill3D(SettingFileType::WILL3D_IMPLANT, version_url, download_path);
      }
      else
      {
        runSplash();
        this->deleteLater();
      }
    }
  }
  else {
    CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_99(), CW3MessageBox::Warning);
    message_box.m_btnCancel->hide();
    message_box.exec();
    this->deleteLater();
    exit(0);
  }
}


void network_util::onProcessStateChanged(QProcess::ProcessState newState)
{
  
  if (newState == QProcess::NotRunning)
  {
    if (progressDialog_) {
      progressDialog_->setValue(100);
      progressDialog_->hide();
    }

    //Only Will3D Viewer
    if (setup_type_ < SettingFileType::WILL3D_SETUP)
      copyImplantFolderWithCallback();
  }
   
}

void network_util::onWatcherFinished()
{

  runSplash();

  if (progressDialog_) {
    progressDialog_->setValue(100);
    progressDialog_->close();
    progressDialog_->deleteLater();
    progressDialog_ = nullptr;
  }

  if (implant_copy_watcher_) {
    implant_copy_watcher_->deleteLater();
    implant_copy_watcher_ = nullptr;
  }

  if (setup_process_) {
    setup_process_->deleteLater();
    setup_process_ = nullptr;
  }

  if (network_manager_)
  {
    network_manager_->deleteLater();
    network_manager_ = nullptr;
  }

  if (network_reply_)
  {
    network_reply_->deleteLater();
    network_reply_ = nullptr;
  }
   
  this->deleteLater();
}

void network_util::onSetProgress(int value)
{
  progressDialog_->setValue(value);
}

void network_util::runSplash()
{
  CSplashScreen *SplashScreen = new CSplashScreen(readFilePath_, outScriptPath_);
  SplashScreen->show();
}

void network_util::runSetupFile()
{
 
  setup_process_ = new QProcess(this);
  QStringList arguments;
  arguments << "-Command" << "Start-Process -FilePath '" + current_download_file_path_ + "' -Verb runAs -Wait" << QString::number(static_cast<int>(setup_type_));

  setup_process_->setProgram("powershell");
  setup_process_->setArguments(arguments);

  // 시그널 연결을 위해 process를 멤버로 저장하거나 sender로 캐스팅해서 사용
  connect(setup_process_, SIGNAL(stateChanged(QProcess::ProcessState)),
    this, SLOT(onProcessStateChanged(QProcess::ProcessState)));

  setup_process_->start();

  if (!setup_process_->waitForStarted()) {
    qDebug() << "Failed to start process. Error:" << setup_process_->errorString();
  }
  else {
    if (progressDialog_) {
      progressDialog_->setLabelText("Installation...");
      progressDialog_->setValue(85);
    }
    qDebug() << "Process started successfully!";
  }
}

bool network_util::isNewVersion(VersionCheckType type)
{
  QString version_ini_path = QDir::currentPath() + "/Will3D_version.ini";
  QString will3d_ini_path = QDir::currentPath() + "/Will3D.ini";
  if (type == VersionCheckType::WILL3D_VER)
  {
    QString latest_version = GlobalPreferences::GetInstance()->GetINIData("Version", "latest_version", "0", version_ini_path).toString();
    return sw_info::SWInfo::IsNewerVersion(latest_version.toUtf8().constData());
  }
  else
  {
    int impplant_latest_version = GlobalPreferences::GetInstance()->GetINIData("Version", "Implant_latest_version", "0", version_ini_path).toInt();
    
    int implant_cur_ver = GlobalPreferences::GetInstance()->GetINIData("IMPLANT", "Version", "0", will3d_ini_path).toInt();

    if (impplant_latest_version > implant_cur_ver)
      return true;
  }
  

  return false;
}

