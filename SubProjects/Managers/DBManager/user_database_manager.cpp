#include "user_database_manager.h"

#include "../../Engine/Common/Common/W3Define.h"
#include "../../Engine/Common/Common/W3Logger.h"
#include "../../Engine/Common/Common/W3Memory.h"
#include "../../Engine/Common/Common/W3MessageBox.h"
#include "../../Engine/Common/Common/language_pack.h"
#include <Engine/Common/Common/global_preferences.h>

#include <QSqlQueryModel>
#include <QSqlError>
#include <QSqlRecord>
#include <QCryptographicHash>
#include <QByteArray>
#include <QTime>
#include <QString>

namespace {
	const QString kConnectionName("User");
}

UserDatabaseManager* UserDatabaseManager::instance_ = nullptr;

UserDatabaseManager::UserDatabaseManager() {
	int port = GlobalPreferences::GetInstance()->preferences_.general.database.port;
	//int odbc_version = GlobalPreferences::GetInstance()->preferences_.general.database.version;

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

	driver_name.remove(" (32 bit)");
	driver_name.remove(" (64 bit)");

	QTime time = QTime::currentTime();
	qsrand((uint)time.msec());

	//db_ = QSqlDatabase::database(kConnectionName);
#if defined(__APPLE__)
	db_ = QSqlDatabase::addDatabase("QMYSQL", kConnectionName);
	db_.setHostName("127.0.0.1");
	db_.setPort(port);
	db_.setDatabaseName("Will3D");
	db_.setUserName("root");
#else
	db_ = QSqlDatabase::addDatabase("QODBC", kConnectionName);
#if MARIA_DB
#if 1
	QString db_connection_string;
	db_connection_string = QString("DRIVER={%1};SERVER=127.0.0.1;DATABASE=Will3D;Port=%2;UID=root;PWD=2002;WSID=.;").arg(driver_name).arg(port);
	common::Logger::instance()->Print(common::LogType::INF, QString("ODBC version : %1").arg(driver_name).toStdString());
	db_.setDatabaseName(db_connection_string);
#else
	db_ = QSqlDatabase::addDatabase("QMYSQL", kConnectionName);
	db_.setHostName("127.0.0.1");
	db_.setPort(port);
	db_.setDatabaseName("Will3D");
	db_.setUserName("root");
	db_.setPassword("2002");
#endif
#else
	db.setDatabaseName("DRIVER={SQL Server};SERVER=127.0.0.1\\HDXWILL2014;DATABASE=Will3D;Port=1433;UID=sa;PWD=2002;WSID=.;");
#endif
#endif

	if (!db_.open()) {
		PrintErrorWill3DDB();
	} else {
		common::Logger::instance()->Print(common::LogType::INF, "Open database");
		db_.close();
	}
}

UserDatabaseManager::~UserDatabaseManager() {
	if (db_.isOpen())
		db_.close();

	db_ = QSqlDatabase();
	db_.removeDatabase(kConnectionName);
}

QString UserDatabaseManager::Hash(const QString& source, const int &iter) {
	QString input = source;
	QByteArray ba(input.toStdString().c_str());

#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
	QCryptographicHash::Algorithm method = QCryptographicHash::Keccak_512;
#else
	QCryptographicHash::Algorithm method = QCryptographicHash::Sha3_512;
#endif

	for (int i = 0; i < iter; i++)
	{
		ba = QCryptographicHash::hash(ba, method);
	}

	return QString(ba.toHex());
}

QString UserDatabaseManager::Encrypt(const QString& source) {
	int length = source.length();
	char* encrypted = new char[length];
	memset(encrypted, 0, sizeof(char) * length);
	qsnprintf(encrypted, sizeof(encrypted), "%s", source.toUtf8().constData());

	QByteArray ba(encrypted);
	QString result = QString(ba.toHex());
	SAFE_DELETE_ARRAY(encrypted);

	return result;
}

QString UserDatabaseManager::Decrypt(const QString& source) {
	QByteArray ba(source.toUtf8().constData());
	QString fromHex = QString(QByteArray::fromHex(ba));

	int length = fromHex.length();
	char* decrypted = new char[length];
	memset(decrypted, 0, sizeof(char) * length);
	qsnprintf(decrypted, sizeof(decrypted), "%s", fromHex.toUtf8().constData());

	QString result = QString::fromLocal8Bit(decrypted);
	SAFE_DELETE_ARRAY(decrypted);

	return result;
}

QString UserDatabaseManager::GenerateRandomPassword(const int length) {
	static const char parts[] =
		"0123456789"
		"abcdefghijklmnopqrstuvwxyz";

	char* password = new char[length];
	memset(password, 0, sizeof(char) * length);
	for (int i = 0; i < length; i++) {
		password[i] = parts[qrand() % (sizeof(parts) - 1)];
	}

	QString result = QString::fromLocal8Bit(password);
	SAFE_DELETE_ARRAY(password);

	return result;
}

QList<User> UserDatabaseManager::GetUserList() {
	if (!db_.open()) {
		PrintErrorWill3DDB();
		return user_list_;
	}

	QSqlQueryModel sql_query_model;
	QString query = QString("select * from user order by type desc");
	sql_query_model.setQuery(query, db_);

	QSqlError error = sql_query_model.lastError();
	if (error.type() != QSqlError::ErrorType::NoError) {
		QString err = sql_query_model.lastError().text();
		common::Logger::instance()->Print(common::LogType::INF, std::string("UserDatabaseManager::CheckSqlError : ") + err.toStdString());
		CW3MessageBox message_box("Will3D", err, CW3MessageBox::Critical, nullptr, CW3Dialog::Theme::Light);
		message_box.exec();

		QSqlDatabase::database(kConnectionName).close();
	}

	if (!CheckSqlError(sql_query_model)) {
		db_.close();
		return user_list_;
	}

	user_list_.clear();

	for (int i = 0; i < sql_query_model.rowCount(); i++) {
		User user;
		user.username = sql_query_model.record(i).value("username").toString();
		//user.password = sql_query_model.record(i).value("password").toString();
		user.name = sql_query_model.record(i).value("name").toString();
		user.email = sql_query_model.record(i).value("email").toString();
		user.phone = sql_query_model.record(i).value("phone").toString();
		user.type = static_cast<User::UserType>(sql_query_model.record(i).value("type").toInt());

		user_list_.append(user);
	}

	sql_query_model.clear();
	db_.close();

	return user_list_;
}

bool UserDatabaseManager::Login(const QString& username, const QString& password, const int type, const bool auto_login) {
	if (!InitAutoLogin())
		return false;

	if (!db_.open()) {
		PrintErrorWill3DDB();
		return false;
	}

	QString hash = UserDatabaseManager::Hash(password);

	QSqlQueryModel sql_query_model;
	QString query = QString("select * from user where username='%1' and password='%2'")
		.arg(username)
		.arg(hash);
	if (type == static_cast<int>(User::UserType::Admin)) {
		query += QString(" and type=%1")
			.arg(type);
	}
	sql_query_model.setQuery(query, db_);

	QSqlError error = sql_query_model.lastError();
	if (error.type() != QSqlError::ErrorType::NoError) {
		QString err = sql_query_model.lastError().text();
		common::Logger::instance()->Print(common::LogType::INF, std::string("UserDatabaseManager::CheckSqlError : ") + err.toStdString());
		CW3MessageBox message_box("Will3D", err, CW3MessageBox::Critical, nullptr, CW3Dialog::Theme::Light);
		message_box.exec();

		QSqlDatabase::database(kConnectionName).close();
	}

	if (!CheckSqlError(sql_query_model)) {
		db_.close();
		return false;
	}

	int count = sql_query_model.rowCount();
	sql_query_model.clear();
	db_.close();

	if (count > 0) {
		if (auto_login)
			if (!SetAutoLogin(username))
				return false;

		return true;
	} else {
		return false;
	}
}

bool UserDatabaseManager::AutoLogin(const QString& username) {
	if (!db_.open()) {
		PrintErrorWill3DDB();
		return false;
	}

	QSqlQueryModel sql_query_model;
	QString query = QString("select * from user where username='%1' and auto_login=1")
		.arg(username);
	sql_query_model.setQuery(query, db_);

	QSqlError error = sql_query_model.lastError();
	if (error.type() != QSqlError::ErrorType::NoError) {
		QString err = sql_query_model.lastError().text();
		common::Logger::instance()->Print(common::LogType::INF, std::string("UserDatabaseManager::CheckSqlError : ") + err.toStdString());
		CW3MessageBox message_box("Will3D", err, CW3MessageBox::Critical, nullptr, CW3Dialog::Theme::Light);
		message_box.exec();

		QSqlDatabase::database(kConnectionName).close();
	}

	if (!CheckSqlError(sql_query_model)) {
		db_.close();
		return false;
	}

	int count = sql_query_model.rowCount();
	sql_query_model.clear();
	db_.close();

	if (count > 0)
		return true;
	else
		return false;
}

QString UserDatabaseManager::FindUsername(const QString& email) {
	QString result;

	if (!db_.open()) {
		PrintErrorWill3DDB();
		return result;
	}

	QSqlQueryModel sql_query_model;
	QString query = QString("select username from user where email='%1' and type=1")
		.arg(email);
	sql_query_model.setQuery(query, db_);

	QSqlError error = sql_query_model.lastError();
	if (error.type() != QSqlError::ErrorType::NoError) {
		QString err = sql_query_model.lastError().text();
		common::Logger::instance()->Print(common::LogType::INF, std::string("UserDatabaseManager::CheckSqlError : ") + err.toStdString());
		CW3MessageBox message_box("Will3D", err, CW3MessageBox::Critical, nullptr, CW3Dialog::Theme::Light);
		message_box.exec();

		QSqlDatabase::database(kConnectionName).close();
	}

	if (!CheckSqlError(sql_query_model)) {
		db_.close();
		return result;
	}

	if (sql_query_model.rowCount() > 0)
		result = sql_query_model.record(0).value("username").toString();

	sql_query_model.clear();
	db_.close();

	return result;
}

QString UserDatabaseManager::RequestNewPassword(const QString& username) {
	QString new_password = GenerateRandomPassword();

	if (!db_.open()) {
		PrintErrorWill3DDB();
		return QString();
	}

	QSqlQueryModel sql_query_model;
	QString query = QString("update user set password='%1' where username='%2'")
		.arg(UserDatabaseManager::Hash(new_password))
		.arg(username);

	sql_query_model.setQuery(query, db_);

	if (!CheckSqlError(sql_query_model)) {
		db_.close();
		return QString();
	}

	db_.close();

	return new_password;
}

bool UserDatabaseManager::AddNewUser(const QString& username, const QString& password, const QString& name, const QString& email, const QString& phone, const int type) {
	if (!db_.open()) {
		PrintErrorWill3DDB();
		return false;
	}

	QString hash = UserDatabaseManager::Hash(password);

	QSqlQueryModel sql_query_model;
	QString query = QString("insert into user values('%1', '%2', N'%3', '%4', '%5', %6, 0)")
		.arg(username)
		.arg(hash)
		.arg(name)
		.arg(email)
		.arg(phone)
		.arg(type);

	sql_query_model.setQuery(query, db_);

	if (!CheckSqlError(sql_query_model)) {
		db_.close();
		return false;
	}

	db_.close();

	return true;
}

bool UserDatabaseManager::UpdateUser(const QString& username, const QString& password, const QString& name, const QString& email, const QString& phone, const int type) {
	if (!db_.open()) {
		PrintErrorWill3DDB();
		return false;
	}

	QString hash = UserDatabaseManager::Hash(password);

	QSqlQueryModel sql_query_model;
	QString query;
	if (password.isEmpty()) {
		query = QString("update user set name=N'%2', email='%3', phone='%4', type=%5 where username='%1'")
			.arg(username)
			.arg(name)
			.arg(email)
			.arg(phone)
			.arg(type);
	} else {
		query = QString("update user set password='%2', name=N'%3', email='%4', phone='%5', type=%6 where username='%1'")
			.arg(username)
			.arg(hash)
			.arg(name)
			.arg(email)
			.arg(phone)
			.arg(type);
	}

	sql_query_model.setQuery(query, db_);

	if (!CheckSqlError(sql_query_model)) {
		db_.close();
		return false;
	}

	db_.close();

	return true;
}

bool UserDatabaseManager::DeleteUser(const QString& username) {
	if (!db_.open()) {
		PrintErrorWill3DDB();
		return false;
	}

	QSqlQueryModel sql_query_model;
	QString query = QString("delete from user where username='%1'")
		.arg(username);

	sql_query_model.setQuery(query, db_);

	if (!CheckSqlError(sql_query_model)) {
		db_.close();
		return false;
	}

	db_.close();

	return true;
}

bool UserDatabaseManager::DeleteUser(const int index) {
	return DeleteUser(user_list_.at(index).username);
}

User UserDatabaseManager::GetUser(int index) {
	if (user_list_.size() > index)
		return user_list_.at(index);
	else
		return User();
}

void UserDatabaseManager::PrintErrorWill3DDB() {
	common::Logger::instance()->Print(common::LogType::ERR, "UserDatabaseManager::PrintErrorWill3DDB : Can't open database. (" + db_.lastError().text().toStdString() + ")");
	CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_07(), CW3MessageBox::Critical, nullptr, CW3Dialog::Theme::Light);
	message_box.exec();
}

bool UserDatabaseManager::CheckSqlError(const QSqlQueryModel& sql_query_model) {
	QSqlError error = sql_query_model.lastError();
	if (error.type() != QSqlError::ErrorType::NoError) {
		QString err = sql_query_model.lastError().text();
		common::Logger::instance()->Print(common::LogType::INF, std::string("UserDatabaseManager::CheckSqlError : ") + err.toStdString());
		CW3MessageBox message_box("Will3D", err, CW3MessageBox::Critical, nullptr, CW3Dialog::Theme::Light);
		message_box.exec();

		QSqlDatabase::database(kConnectionName).close();
		return false;
	}
	return true;
}

bool UserDatabaseManager::InitAutoLogin() {
	if (!db_.open()) {
		PrintErrorWill3DDB();
		return false;
	}

	QSqlQueryModel sql_query_model;
	QString query = QString("update user set auto_login=1");

	sql_query_model.setQuery(query, db_);

	if (!CheckSqlError(sql_query_model)) {
		db_.close();
		return false;
	}

	db_.close();

	return true;
}

bool UserDatabaseManager::SetAutoLogin(const QString& username) {
	if (!db_.open()) {
		PrintErrorWill3DDB();
		return false;
	}

	QSqlQueryModel sql_query_model;
	QString query = QString("update user set auto_login=1 where username='%1'")
		.arg(username);

	sql_query_model.setQuery(query, db_);

	if (!CheckSqlError(sql_query_model)) {
		db_.close();
		return false;
	}

	db_.close();

	return true;
}
