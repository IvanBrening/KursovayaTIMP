#include "ClientCommunicate.h"  // Добавьте этот заголовочный файл, если его нет
#include <string>

bool ClientCommunicate::parseMessage(const std::string& receivedData, std::string& login, std::string& salt, std::string& clientHash) {
    int saltLength = 16;  // Длина соли
    int hashLength = 40;  // Длина хэша

    if (receivedData.size() < static_cast<size_t>(saltLength + hashLength)) {
        return false;
    }

    // Длина логина вычисляется как остаток от длины строки
    int loginLength = receivedData.size() - saltLength - hashLength;

    // Разделение строки на логин, соль и хэш
    login = receivedData.substr(0, loginLength);
    salt = receivedData.substr(loginLength, saltLength);
    clientHash = receivedData.substr(loginLength + saltLength, hashLength);

    return true;
}

