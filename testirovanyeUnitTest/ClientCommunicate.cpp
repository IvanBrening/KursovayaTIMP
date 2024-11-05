#include "ClientCommunicate.h"
#include <cstring>
#include <unistd.h>
#include "ConnectToBase.h"
#include "Calculator.h"
#include "Error.h"
#include "Interface.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h> // Для функции close


void ClientCommunicate::communicate(int socket, const std::string& userDbFileName, const std::string& logFileName) {
    char buffer[256] = {0};
    if (recv(socket, buffer, sizeof(buffer) - 1, 0) <= 0) {
        return; // Ошибка при получении данных
    }
    buffer[sizeof(buffer) - 1] = '\0';
    std::string receivedData(buffer);
    int saltLength = 16; // Длина соли
    int hashLength = 40; // Длина хэша
    int loginLength = receivedData.size() - saltLength - hashLength;

    std::string login = receivedData.substr(0, loginLength); // Извлекаем логин
    std::string salt = receivedData.substr(loginLength, saltLength); // Извлекаем соль
    std::string clientHash = receivedData.substr(loginLength + saltLength, hashLength); // Извлекаем хэш клиента

    ConnectToBase db;
    if (db.authenticateUser(login, salt, clientHash, userDbFileName)) {
        Interface::logMessage(logFileName, "User authenticated: " + login);
        send(socket, "Authentication successful", strlen("Authentication successful"), 0);
        Calculator calc;
        if (calc.processVectors(socket) < 0) {
            Interface::logError(logFileName, "Error processing vectors", false);
        }
    } else {
        Interface::logError(logFileName, "Authentication failed for user: " + login, true);
        send(socket, "Authentication failed", strlen("Authentication failed"), 0);
    }
}

