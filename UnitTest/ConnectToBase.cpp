#include "ConnectToBase.h"
#include <fstream>
#include <sstream>
#include <openssl/sha.h>  // Подключение библиотеки OpenSSL для SHA-1

ConnectToBase::ConnectToBase(const std::string& dbFileName) {
    loadDatabase(dbFileName);  // Загружаем базу данных из файла
}

void ConnectToBase::loadDatabase(const std::string& dbFileName) {
    std::ifstream dbFile(dbFileName);
    std::string line;
    while (std::getline(dbFile, line)) {
        std::istringstream iss(line);
        std::string login, password;
        if (iss >> login >> password) {
            users[login] = password;  // Сохраняем логин и пароль в базе данных
        }
    }
}

std::string ConnectToBase::hashPassword(const std::string& password, const std::string& salt) {
    std::string saltedPassword = salt + password;
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(saltedPassword.c_str()), saltedPassword.size(), hash);
    
    std::ostringstream oss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        oss << std::hex << (hash[i] >> 4) << (hash[i] & 0xF);
    }
    return oss.str();
}

bool ConnectToBase::authenticate(const std::string& login, const std::string& salt, const std::string& clientHash) {
    auto it = users.find(login);
    if (it == users.end()) return false;  // Логин не найден

    std::string computedHash = hashPassword(it->second, salt);  // Хэшируем пароль с солью
    return computedHash == clientHash;  // Сравниваем хэш с клиентским
}

