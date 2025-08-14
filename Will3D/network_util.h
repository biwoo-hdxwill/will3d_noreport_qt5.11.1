#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtNetwork>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QSettings>
#include <QProgressDialog>
#include "../SubProjects/Engine/Common/Common/W3Enum.h"


class network_util : public QObject
{
	Q_OBJECT

public:
	network_util(QObject *parent=nullptr);
	~network_util();
	void getImplantSetup(QString readFilePath, QString outScriptPath);
	void getWill3DSetup(QString readFilePath, QString outScriptPath);

private:
	enum {};

	QString registryKey;
	QString registryValue;
	QString readFilePath_;
	QString outScriptPath_;
	QString implant_version_;

	int demoDays;
	QNetworkReply *network_reply_ = nullptr;;
	QNetworkAccessManager *network_manager_ = nullptr;;
	QProgressDialog *progressDialog_ = nullptr;;
	QFile save_file_;
	SettingFileType setup_type_;

	QFutureWatcher<void> *implant_copy_watcher_ = nullptr;
	QString current_download_file_path_;
	QProcess* setup_process_ = nullptr;

private:
	//QString getHardwareID();
	//QString generateLicenseKey(const QString& hwID, const QDate& startDate);
	bool fetchTimeFromAPI(QDate &date);
	//bool isDemoExpired();
	//void CopyDirectory(const QString &source_dir, const QString &target_dir, int &progress, int totalFiles);
	void copyImplantFolderWithCallback();
	void handleDownloadError(QNetworkReply::NetworkError code, QNetworkReply *reply);
	void downloadAndSetupImplant(SettingFileType type, QString download_url, QString download_path);
	void downloadAndSetupWill3D(SettingFileType type, QString download_url, QString download_path);
	void downloadVersion(SettingFileType type, QString download_url, QString download_path);
	void downloadURL(QString download_url, QString download_path);
	void copyDirectory(const QString &source_dir, const QString &target_dir, int &progress, int totalFiles);
	bool isNewVersion(VersionCheckType type);
	void runSplash();
	void runSetupFile();
	bool networkCheck();

public slots:
	void onNetworkError(QNetworkReply::NetworkError code);
	void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void onReadyRead();
	void onReplyFinished();
	void onProcessStateChanged(QProcess::ProcessState newState);
	void onWatcherFinished();
	void onSetProgress(int value);

signals:
	void sigSetProgress(int);

};
