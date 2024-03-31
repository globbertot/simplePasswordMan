#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QByteArray>
#include "cryptomanager.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QList>
#include <QLabel>
#include <QStandardPaths>
#include <random>

class dbManager : public QObject
{
    Q_OBJECT
public:
    dbManager(QObject *parent = nullptr);
    ~dbManager();

    // Main variables will be customizable in the future
    std::map<std::string, int> passPolicy = {
        {"maxPassLength", 28},
        {"minPassLength", 16},
        {"minLetters", 2},
        {"minNumbers", 2},
        {"minSymbols", 2},
        {"maxSymbolsRepeat", 6},
    };
    QSettings passPolicySets;
    const QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    bool bMKExists = false;

    // Core logic
    void create(std::string service, std::string password);
    void _delete(std::string service);
    void search(std::string service);
    void showAll();
    void updatePassword(std::string service, std::string password);
    void updatePassPolicy(std::string setting, int val);
    void MKCreator(std::string MK);

    // Helper functions
    bool exists(const std::string& table, const std::string& item, const std::string& columnToFind);
    bool checkPassword(std::string pass);
    std::string generatePass();
    void resetSettings();
signals:
    void databaseActionCompleted(bool success, std::string action, std::string errMsg = "", QList<std::string> results = {}, std::string stylesheet = "");
private:

    QSqlDatabase db;
    cryptoManager CM;
};

#endif // DBMANAGER_H
