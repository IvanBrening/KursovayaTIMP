#include "ConnectToBase.h"
#include "Error.h"
#include <fstream>
#include <sstream>
#include <openssl/sha.h>
#include <iomanip>
#include <algorithm>
#include <iostream>

/**
 * @file ConnectToBase.cpp
 * @brief Содержит реализацию класса ConnectToBase.
 * @author Бренинг Иван
 * @date 13.12.24
 * @version 1.0
 */

/**
 * @brief Аутентифицирует пользователя по логину, соли и хешу пароля.
 *
 * @param login Логин пользователя.
 * @param salt Соль, используемая при хешировании.
 * @param clientHash Хеш пароля, полученный от клиента.
 * @param dbFileName Имя файла базы данных, содержащей логины и пароли пользователей.
 * @return `true`, если аутентификация прошла успешно, `false` в противном случае.
 *
 * @note Функция считывает файл базы данных построчно.
 * @note Функция использует `hashPassword` для вычисления хеша пароля и `compareHashes` для сравнения хешей.
 */
bool ConnectToBase::authenticateUser(const std::string& login, const std::string& salt, const std::string& clientHash, const std::string& dbFileName) {
    std::ifstream dbFile(dbFileName);
    if (!dbFile.is_open()) {
        Error::logError("Cannot open database file.", true);
        return false;
    }

    std::string dbLogin, dbPassword;
    while (dbFile >> dbLogin >> dbPassword) {
        if (dbLogin == login) {
            std::string serverHash = hashPassword(dbPassword, salt);
            return compareHashes(serverHash, clientHash);
        }
    }
    return false; // Если не нашли пользователя
}

/**
 * @brief Вычисляет SHA1 хеш пароля с добавлением соли.
 *
 * @param password Пароль, который необходимо хешировать.
 * @param salt Соль, добавляемая к паролю перед хешированием.
 * @return Строка, представляющая собой SHA1 хеш пароля с солью.
 *
 * @note Функция использует библиотеку OpenSSL для вычисления SHA1 хеша.
 */
std::string ConnectToBase::hashPassword(const std::string& password, const std::string& salt) {
    std::string data = salt + password;
    unsigned char result[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(data.c_str()), data.size(), result);

    std::stringstream hashStream;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        hashStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(result[i]);
    }
    return hashStream.str();
}

/**
 * @brief Сравнивает два хеша, приведенных к нижнему регистру.
 *
 * @param serverHash Хеш, сгенерированный на сервере.
 * @param clientHash Хеш, полученный от клиента.
 * @return `true`, если хеши равны, `false` в противном случае.
 */
bool ConnectToBase::compareHashes(const std::string& serverHash, const std::string& clientHash) {
    std::string clientHashLower = clientHash;
    std::string serverHashLower = serverHash;
    std::transform(clientHashLower.begin(), clientHashLower.end(), clientHashLower.begin(), ::tolower);
    std::transform(serverHashLower.begin(), serverHashLower.end(), serverHashLower.begin(), ::tolower);
    return serverHashLower == clientHashLower;
}
