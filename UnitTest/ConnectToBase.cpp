#include "ConnectToBase.h"
#include <fstream>
#include <sstream>
#include <openssl/sha.h> // Подключаем для SHA-1
#include <iomanip>

// Конструктор принимает имя файла базы данных
ConnectToBase::ConnectToBase(const std::string& dbFileName) : dbFileName(dbFileName) {}

// Метод для хэширования пароля с использованием соли
std::string ConnectToBase::hashPassword(const std::string& password, const std::string& salt) {
    std::string combined = salt + password; // Соединяем соль и пароль

    unsigned char hash[SHA_DIGEST_LENGTH]; // Массив для хэша
    SHA1(reinterpret_cast<const unsigned char*>(combined.c_str()), combined.size(), hash);

    std::ostringstream oss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return oss.str();
}

// Метод для аутентификации пользователя
bool ConnectToBase::authenticate(const std::string& login, const std::string& salt, const std::string& hash) {
    std::ifstream dbFile(dbFileName);
    if (!dbFile.is_open()) return false;

    std::string dbLogin, dbPassword;
    while (dbFile >> dbLogin >> dbPassword) {
        if (dbLogin == login) {
            // Сравниваем хэш пароля с сохраненным значением
            return hashPassword(dbPassword, salt) == hash;
        }
    }
    return false; // Логин не найден в базе данных
}

