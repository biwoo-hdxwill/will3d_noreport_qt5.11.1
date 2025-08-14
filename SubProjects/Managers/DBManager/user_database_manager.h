#pragma once

/*=========================================================================

File:			class NewUserDialog
Language:		C++11
Library:        Qt 5.8.0
Author:			Jung Dae Gun
First date:		2018-05-29
Last modify:	2016-05-29

=========================================================================*/

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QMutex>

#include "dbmanager_global.h"

class QSqlQueryModel;

struct User {
	enum class UserType { UsertType_User, Admin };

	int index = 0;
	QString username;
	QString password;
	QString name;
	QString email;
	QString phone;
	UserType type = UserType::UsertType_User;
};

class DBMANAGER_EXPORT UserDatabaseManager {
public:
	UserDatabaseManager();
	~UserDatabaseManager();

	static UserDatabaseManager* getInstance() {
		static QMutex mutex;
		mutex.lock();
		if (!instance_) {
			instance_ = new UserDatabaseManager();
			atexit(destroy);
		}
		mutex.unlock();
		return instance_;
	}

	static void destroy() {
		static QMutex mutex;
		mutex.lock();
		if (instance_) {
			delete instance_;
			instance_ = nullptr;
		}
		mutex.unlock();
	}

	QList<User> GetUserList();
	bool Login(const QString& username, const QString& password, const int type, const bool auto_login);
	bool AutoLogin(const QString& username);
	QString FindUsername(const QString& email);
	QString RequestNewPassword(const QString& username);
	bool AddNewUser(const QString& username, const QString& password, const QString& name, const QString& email, const QString& phone, const int type);
	bool UpdateUser(const QString& username, const QString& password, const QString& name, const QString& email, const QString& phone, const int type);
	bool DeleteUser(const QString& username);
	bool DeleteUser(const int index);
	User GetUser(int index);
	inline const QList<User> user_list() const { return user_list_; };

private:
	static QString Hash(const QString& source, const int &iter = 3);
	static QString Encrypt(const QString& source);
	static QString Decrypt(const QString& source);
	static QString GenerateRandomPassword(const int length = 6);

	void PrintErrorWill3DDB();
	bool CheckSqlError(const QSqlQueryModel& sql_query_model);
	bool InitAutoLogin();
	bool SetAutoLogin(const QString& username);

private:
	static UserDatabaseManager* instance_;
	
	QList<User> user_list_;
	QSqlDatabase db_;
};

