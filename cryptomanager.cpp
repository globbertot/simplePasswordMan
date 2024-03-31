#include "cryptomanager.h"

cryptoManager::cryptoManager() {
    db = QSqlDatabase::addDatabase("QSQLITE", "crypto");
    db.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/simplePassMan.db");
    db.open();

    QSqlQuery build(db);build.prepare("CREATE TABLE IF NOT EXISTS magic(lost TEXT);");if (!build.exec()) { qDebug() << "FATAL ERROR - " << build.lastError().text();return; }

    QSqlQuery checkIfExists(db);checkIfExists.prepare("SELECT COUNT(*) FROM magic");
    if (!checkIfExists.exec() || !checkIfExists.next()) { qDebug() << "CANNOT ACCESS DATABASE - " << checkIfExists.lastError().text();return; }

    if (checkIfExists.value(0).toInt() == 0) {
        // Key doesnt exist, thus create it
        if (RAND_bytes(key, EVP_MAX_KEY_LENGTH) != 1){
            char tmp[256];ERR_error_string_n(ERR_get_error(), tmp, sizeof(tmp));std::string err = "FATAL KEY ERROR - " + std::string(tmp);
            qDebug() << QString::fromStdString(err);
            exit(-1);
        }
        QByteArray simpleKey(reinterpret_cast<const char*>(key), static_cast<int>(EVP_MAX_KEY_LENGTH));
        QSqlQuery create(db);
        create.prepare("INSERT INTO magic (lost) VALUES (:s);");

        create.bindValue(":s", base64Encode(simpleKey));

        if (!create.exec()) {
            qDebug() << "FATAL KEY ISSUE - " << create.lastError().text();return;
        }
        db.close();
    } else {
        // Grab the found key
        QSqlQuery grab(db);
        grab.prepare("SELECT lost FROM magic");

        if (grab.exec() && grab.next()) {
            QByteArray keyDecoded = base64Decode(grab.value(0).toString().toUtf8());
            memcpy(key, keyDecoded.constData(), qMin((size_t)EVP_MAX_KEY_LENGTH, (size_t)keyDecoded.length()));

            db.close();
        } else {
            qDebug() << "FATAL ISSUE CANT GRAB KEY - " << grab.lastError().text();db.close();return;
        }
    }
}

std::string cryptoManager::encryptDecrypt(const std::string& in, bool encrypt) {
    QByteArray inputBytes;
    if (encrypt) {
        inputBytes = addPKCS7Padding(QByteArray::fromStdString(in), EVP_CIPHER_block_size(EVP_aes_128_ecb()));
    } else {
        inputBytes = base64Decode(QByteArray::fromStdString(in));
    }

    const unsigned char* simpleIn = reinterpret_cast<const unsigned char*>(inputBytes.constData());
    const unsigned char* simpleKey = reinterpret_cast<const unsigned char*>(key);

    QByteArray outBytes(inputBytes.length() + EVP_MAX_BLOCK_LENGTH, 0);
    unsigned char* simpleOut = reinterpret_cast<unsigned char*>(outBytes.data());
    int resultLength = 0;
    int L = 0;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_set_padding(ctx, 1);

    if (!ctx) {
        char tmp[256];ERR_error_string_n(ERR_get_error(), tmp, sizeof(tmp));std::string err = "ERROR - " + std::string(tmp);
        EVP_CIPHER_CTX_free(ctx);
        return err;
    }

    EVP_CIPHER* cipher;
    if (encrypt) {
        if (!EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), nullptr, simpleKey, nullptr)) {
            char tmp[256];ERR_error_string_n(ERR_get_error(), tmp, sizeof(tmp));std::string err = "ERROR - " + std::string(tmp);
            EVP_CIPHER_CTX_free(ctx);
            return err;
        }
        if (!EVP_EncryptUpdate(ctx, simpleOut, &L, simpleIn, inputBytes.length())) {
            char tmp[256];ERR_error_string_n(ERR_get_error(), tmp, sizeof(tmp));std::string err = "ERROR - " + std::string(tmp);
            EVP_CIPHER_CTX_free(ctx);
            return err;
        }
    } else {
        if (!EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), nullptr, simpleKey, nullptr)) {
            char tmp[256];ERR_error_string_n(ERR_get_error(), tmp, sizeof(tmp));std::string err = "ERROR - " + std::string(tmp);
            EVP_CIPHER_CTX_free(ctx);
            return err;
        }
        if (!EVP_DecryptUpdate(ctx, simpleOut, &L, simpleIn, inputBytes.length())) {
            char tmp[256];ERR_error_string_n(ERR_get_error(), tmp, sizeof(tmp));std::string err = "ERROR - " + std::string(tmp);
            EVP_CIPHER_CTX_free(ctx);
            return err;
        }
    }

    resultLength += L;

    if (encrypt) {
        if (!EVP_EncryptFinal(ctx, simpleOut + L, &L)) {
            char tmp[256];ERR_error_string_n(ERR_get_error(), tmp, sizeof(tmp));std::string err = "ERROR - " + std::string(tmp);
            EVP_CIPHER_CTX_free(ctx);
            return err;
        }
    } else {
        if (!EVP_DecryptFinal(ctx, simpleOut + L, &L)) {
            char tmp[256];ERR_error_string_n(ERR_get_error(), tmp, sizeof(tmp));std::string err = "ERROR - " + std::string(tmp);
            EVP_CIPHER_CTX_free(ctx);
            return err;
        }
    }

    resultLength += L;

    outBytes.resize(resultLength);
    EVP_CIPHER_CTX_free(ctx);

    if (encrypt) {
        return base64Encode(outBytes).toStdString();
    } else {
        QByteArray decryptedBytes = removePKCS7Padding(outBytes);

        return std::string(decryptedBytes.constData(), decryptedBytes.length());
    }
}
QByteArray cryptoManager::base64Encode(const QByteArray& input) {
    return input.toBase64();
}

QByteArray cryptoManager::base64Decode(const QByteArray& input) {
    return QByteArray::fromBase64(input);
}

QByteArray cryptoManager::addPKCS7Padding(const QByteArray& in, int blockSize) {
    QByteArray padded = in;
    int paddingSize = blockSize - (in.length() % blockSize);
    padded.append(paddingSize, static_cast<char>(paddingSize));
    return padded;
}

QByteArray cryptoManager::removePKCS7Padding(const QByteArray& input) {
    int paddingSize = static_cast<int>(input.at(input.length() - 1));
    return input.left(input.length() - paddingSize);
}
