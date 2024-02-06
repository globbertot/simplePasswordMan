#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QByteArray>
#include "cryptomanager.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QSettings>
#include <QDebug>
#include <QList>
#include <QLabel>

class dbManager : public QObject
{
    Q_OBJECT
public:
    dbManager(QObject *parent = nullptr);
    ~dbManager();

    // Main variables will be customizable in the future
    std::map<std::string, int> passPolicy = {
        {"maxPassLength", 18},
        {"minPassLength", 8},
        {"minLetters", 1},
        {"minNumbers", 1},
        {"minSymbols", 1},
        {"maxSymbolsRepeat", 3},
    };
    QSettings passPolicySets;

    // Core logic
    void create(std::string service, std::string password);
    void _delete(std::string service);
    void search(std::string service);
    void showAll();
    void updatePassword(std::string service, std::string password);
    void updatePassPolicy(std::string setting, int val);

    // Helper functions
    bool exists(const std::string& table, const std::string& item, const std::string& columnToFind);
    bool checkPassword(std::string pass);
    void resetSettings();
signals:
    void databaseActionCompleted(bool success, std::string action, std::string errMsg = "", QList<std::string> results = {});
private:

    QSqlDatabase db;
    cryptoManager CM;
};

#endif // DBMANAGER_H
