#include "dbmanager.h"

dbManager::dbManager(QObject *parent) : QObject(parent), passPolicySets("globbertot", "simplePassMan") {
    db = QSqlDatabase::addDatabase("QSQLITE", "main");
    db.setDatabaseName("simplePassMan.db");
    if (!db.open()) {
        qDebug() << "UNABLE TO START DATABASE - " << db.lastError().text();
        return;
    }
    resetSettings();

    QSqlQuery createDatabase(db);
    createDatabase.prepare("CREATE TABLE IF NOT EXISTS data (service TEXT, password TEXT);");

    if (!createDatabase.exec()) {
        qDebug() << "Error initializing database:" << createDatabase.lastError().text();return;
    }
}

dbManager::~dbManager() { db.close(); }

void dbManager::resetSettings() {
    passPolicy["maxPassLength"] = passPolicySets.value("maxPassLength", 18).toInt();passPolicySets.setValue("maxPassLength", 18);
    passPolicy["minPassLength"] = passPolicySets.value("minPassLength", 8).toInt();passPolicySets.setValue("minPassLength", 8);
    passPolicy["minLetters"] = passPolicySets.value("minLetters", 1).toInt();passPolicySets.setValue("minLetters", 1);
    passPolicy["minNumbers"] = passPolicySets.value("minNumbers", 1).toInt();passPolicySets.setValue("minNumbers", 1);
    passPolicy["minSymbols"] = passPolicySets.value("minSymbols", 1).toInt();passPolicySets.setValue("minSymbols", 1);
    passPolicy["maxSymbolsRepeat"] = passPolicySets.value("maxSymbolsRepeat", 3).toInt();passPolicySets.setValue("maxSymbolsRepeat", 3);
}

bool dbManager::exists(const std::string& table, const std::string& item, const std::string& columnToFind) {
    CM.base64Decode(QByteArray::fromStdString(item));
    QSqlQuery e(db);e.prepare("SELECT COUNT(*) FROM " + QString::fromStdString(table) + " WHERE "+QString::fromStdString(columnToFind)+" = :s;");
    e.bindValue(":s", QString::fromStdString(item));

    if (e.exec() && e.next()) {
        if (e.value(0).toInt() > 0) { return true; }
        else { return false; }
    }
    return false;
}

bool dbManager::checkPassword(std::string pass) {
    if (pass.length() > passPolicy["maxPassLength"] || pass.length() < passPolicy["minPassLength"]) { emit databaseActionCompleted(false, "CHECKPASS", "Password length not valid, either too long or too small.");return false; }

    if (pass.find(' ') != std::string::npos) { emit databaseActionCompleted(false, "CHECKPASS", "Password cant contain spaces.");return false; }
    int lettersCount = 0;int numCount = 0;
    std::unordered_map<char, int> symbols;
    for (int i=0;i<pass.length();++i) {
        char C = pass[i];

        if (std::isalpha(C)) { lettersCount++;continue; }
        if (std::isdigit(C)) { numCount++;continue; }

        if (symbols[C] >= passPolicy["maxSymbolsRepeat"]) { emit databaseActionCompleted(false, "CHECKPASS", "Password cant contain the same symbol "+std::to_string(passPolicy["maxSymbolRepeat"])+" times");return false; }
        symbols[C]++;
    }

    if (lettersCount < passPolicy["minLetters"]) { emit databaseActionCompleted(false, "CHECKPASS", "Password must contain more than "+std::to_string(passPolicy["minLetters"])+" letters");return false; }
    if (numCount < passPolicy["minNumbers"]) { emit databaseActionCompleted(false, "CHECKPASS", "Password must contain more than "+std::to_string(passPolicy["minNumbers"])+" numbers");return false; }
    if (passPolicy["minSymbols"] != -1 && symbols.size() < passPolicy["minSymbols"]) { emit databaseActionCompleted(false, "CHECKPASS", "Password must contain more than "+std::to_string(passPolicy["minSymbols"])+" symbols");return false; }

    return true;
}

void dbManager::create(std::string service, std::string password) {
    bool s = exists("data", service, "service");
    if (!checkPassword(password)) { return; }

    if (s) { emit databaseActionCompleted(false, "CREATE", "Already exists");return; }

    std::string encryptedPass = CM.encryptDecrypt(password, true);
    QSqlQuery create(db);create.prepare("INSERT INTO data (service, password) VALUES (:s, :p);");
    create.bindValue(":s", QString::fromStdString(service));
    create.bindValue(":p", QString::fromStdString(encryptedPass));

    if (create.exec()) {
        emit databaseActionCompleted(true, "CREATE");
        return;
    }

    emit databaseActionCompleted(false, "CREATE", "SQL ERROR: "+create.lastError().text().toStdString());
    return;
}

void dbManager::search(std::string service) {
    QSqlQuery action(db);action.prepare("SELECT password FROM data WHERE service = :s;");
    action.bindValue(":s", QString::fromStdString(service));

    if (action.exec()) {
        if (action.next()) {
            if (!action.value(0).toString().isNull()) {
                QList<std::string> labels;
                std::string decryptedPass = CM.encryptDecrypt(action.value(0).toString().toStdString(), false);
                labels.push_back(decryptedPass);
                emit databaseActionCompleted(true, "SEARCH", "", labels);return;
            }
        } else {
            emit databaseActionCompleted(false, "SEARCH", "Service does not exist");return;
        }
    }
    emit databaseActionCompleted(false, "SEARCH", "SQL ERROR: " + action.lastError().text().toStdString());return;
}

void dbManager::_delete(std::string service) {
    bool s = exists("data", service, "service");
    if (!s) { emit databaseActionCompleted(false, "DELETE", "Does not exist");return; }

    QSqlQuery del(db);del.prepare("DELETE FROM data WHERE service = :s;");
    del.bindValue(":s", QString::fromStdString(service));

    if (del.exec()) {
        if (del.numRowsAffected() > 0) { emit databaseActionCompleted(true, "DELETE");return; }
    }
    emit databaseActionCompleted(false, "DELETE", "SQL ERROR: "+del.lastError().text().toStdString());return;
}

void dbManager::showAll() {
    QSqlQuery action(db);action.prepare("SELECT service FROM data");
    QList<std::string> services;

    if (action.exec()) {
        while (action.next()) {
            services.push_back(action.value(0).toString().toStdString());
        }
        emit databaseActionCompleted(true, "showAll", "", services);
        return;
    }
    emit databaseActionCompleted(false, "showAll", "SQL ERROR: "+action.lastError().text().toStdString());
}

void dbManager::updatePassword(std::string service, std::string password) {
    if (!checkPassword(password)) { return; };
    bool s = exists("data", service, "service");
    std::string encryptedPass = CM.encryptDecrypt(password, true);

    if (!s) { emit databaseActionCompleted(false, "UPDATEPASS", "Does not exist.");return; }

    QSqlQuery action(db);action.prepare("UPDATE data SET password = :p WHERE service = :s;");

    action.bindValue(":p", QString::fromStdString(encryptedPass));
    action.bindValue(":s", QString::fromStdString(service));

    if (action.exec()) {
        emit databaseActionCompleted(true, "UPDATEPASS");return;
    }
    emit databaseActionCompleted(false, "UPDATEPASS", "SQL ERROR: "+action.lastError().text().toStdString());
}

void dbManager::updatePassPolicy(std::string setting, int val) {
    if (passPolicy.find(setting) == passPolicy.end()) { emit databaseActionCompleted(false, "UPDATEPOLICY", "Unknown variable detected");return; }

    passPolicy[setting] = val;
    passPolicySets.setValue(setting, passPolicy[setting]);
}
