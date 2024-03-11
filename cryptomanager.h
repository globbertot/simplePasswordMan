#ifndef CRYPTOMANAGER_H
#define CRYPTOMANAGER_H

#include <QCryptographicHash>
#include <QFile>
#include <QDebug>
#include <QStandardPaths>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>

class cryptoManager
{
public:
    cryptoManager();

    // main guy
    std::string encryptDecrypt(const std::string& in, bool encrypt);
    QByteArray base64Decode(const QByteArray& input);

private:
    QSqlDatabase db;
    unsigned char key[EVP_MAX_KEY_LENGTH];

    // small helpers
    QByteArray addPKCS7Padding(const QByteArray& input, int blockSize);
    QByteArray removePKCS7Padding(const QByteArray& input);

    QByteArray base64Encode(const QByteArray& input);
};

#endif // CRYPTOMANAGER_H
