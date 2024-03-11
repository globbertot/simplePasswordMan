#include "dbmanager.h"

dbManager::dbManager(QObject *parent) : QObject(parent), passPolicySets("globbertot", "simplePassMan") {
    db = QSqlDatabase::addDatabase("QSQLITE", "main");
    QDir().mkpath(dbPath);
    db.setDatabaseName(dbPath+"/simplePassMan.db");
    db.open();

    resetSettings();

    QSqlQuery createDatabase(db);
    createDatabase.prepare("CREATE TABLE IF NOT EXISTS data (service TEXT, password TEXT);");

    if (!createDatabase.exec()) {
        qDebug() << "Error initializing database:" << createDatabase.lastError().text();return;
    }
}

dbManager::~dbManager() { db.close(); }

void dbManager::resetSettings() {
    passPolicy["maxPassLength"] = passPolicySets.value("maxPassLength", 28).toInt();passPolicySets.setValue("maxPassLength", 28);
    passPolicy["minPassLength"] = passPolicySets.value("minPassLength", 16).toInt();passPolicySets.setValue("minPassLength", 16);
    passPolicy["minLetters"] = passPolicySets.value("minLetters", 2).toInt();passPolicySets.setValue("minLetters", 2);
    passPolicy["minNumbers"] = passPolicySets.value("minNumbers", 2).toInt();passPolicySets.setValue("minNumbers", 2);
    passPolicy["minSymbols"] = passPolicySets.value("minSymbols", 2).toInt();passPolicySets.setValue("minSymbols", 2);
    passPolicy["maxSymbolsRepeat"] = passPolicySets.value("maxSymbolsRepeat", 6).toInt();passPolicySets.setValue("maxSymbolsRepeat", 6);
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

std::string dbManager::generatePass() {
    std::random_device rd;std::mt19937 gen(rd());
    std::string pass = "";
    int lettersC = 0;int numsC = 0;int symbolsC = 0;
    std::string alphabet = "abcdefghijklmnopqrstuvwxyz";
    std::string numbers = "0123456789";
    std::string symbols = "!@#$%&*";

    int length = std::uniform_int_distribution<int>(passPolicy["minPassLength"], passPolicy["maxPassLength"])(gen);

    for (int i=0;i<length;++i) {
        int category = gen() % 3; // 0 for letter || 1 for number || 2 for symbol

        if (category == 0) {
            // Add a letter
            pass += (gen() % 2 == 0) ? std::tolower((alphabet[gen() % alphabet.size()])) : std::toupper((alphabet[gen() % alphabet.size()]));
            lettersC++;
        } else if (category == 1) {
            // Add a number
            pass += numbers[gen() % numbers.size()];
            numsC++;
        } else {
            // Add a symbol
            if (symbolsC >= passPolicy["maxSymbolsRepeat"]) { continue; }
            pass += symbols[gen() % symbols.size()];
            symbolsC++;
        }
    }

    if (lettersC < passPolicy["minLetters"]) {
        for (int i=lettersC;i<passPolicy["minLetters"];++i) {
            pass += (gen() % 2 == 0) ? std::tolower((alphabet[gen() % alphabet.size()])) : std::toupper((alphabet[gen() % alphabet.size()]));
        }
    }

    if (numsC < passPolicy["minNumbers"]) {
        for (int i=numsC;i<passPolicy["minNumbers"];++i) {
            pass += numbers[gen() % numbers.size()];
        }
    }

    if (symbolsC < passPolicy["minSymbols"]) {
        for (int i=symbolsC;i<passPolicy["minSymbols"];++i) {
            pass += symbols[gen() % symbols.size()];
        }
    }

    return pass;
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
