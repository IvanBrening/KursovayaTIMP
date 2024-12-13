#include "ClientCommunicate.h"
#include "ConnectToBase.h"
#include "Calculator.h"
#include "Interface.h"
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

ParsedMessage ClientCommunicate::parseMessage(const std::string& message) {
    ParsedMessage parsed;
    std::string::size_type saltLength = 16;
    std::string::size_type hashLength = 40;
    std::string::size_type loginLength = message.size() - saltLength - hashLength;

    if (loginLength <= 0 || message.size() < saltLength + hashLength) {
        return parsed; // Возвращаем пустую структуру в случае ошибки
    }
    
    parsed.login = message.substr(0, loginLength);
    parsed.salt = message.substr(loginLength, saltLength);
    parsed.hash = message.substr(loginLength + saltLength, hashLength);

    return parsed;
}

void ClientCommunicate::communicate(int socket, const std::string& userDbFileName, const std::string& logFileName) {
    char buffer[256] = {0};
    if (recv(socket, buffer, sizeof(buffer) - 1, 0) <= 0) {
        return; // Ошибка при получении данных
    }
    buffer[sizeof(buffer) - 1] = '\0';

    std::string receivedData(buffer);

    ParsedMessage parsed = parseMessage(receivedData);

    if (parsed.login.empty()) {
        Interface::logError(logFileName, "Failed to parse message from client", false);
        send(socket, "ERR", 3, 0);
        return;
    }

    ConnectToBase dbConnection;

    // Аутентификация пользователя
    if (dbConnection.authenticateUser(parsed.login, parsed.salt, parsed.hash, userDbFileName)) {
        send(socket, "OK", 2, 0); // Отправляем сообщение об успешной аутентификации
        Interface::logMessage(logFileName, "User " + parsed.login + " authenticated successfully.");
        Calculator calc;
        if (calc.processVectors(socket) != 0) {
            Interface::logError(logFileName, "Error processing vectors.", false);
        }
    } else {
        Interface::logError(logFileName, "Authentication failed for user: " + parsed.login, false);
        send(socket, "ERR", 3, 0); // Отправляем сообщение о неудачной аутентификации
    }
}
