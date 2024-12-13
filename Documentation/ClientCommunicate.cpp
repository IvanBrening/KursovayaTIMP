#include "ClientCommunicate.h"
#include "ConnectToBase.h"
#include "Calculator.h"
#include "Interface.h"
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

/**
 * @file ClientCommunicate.cpp
 * @brief Содержит реализацию класса ClientCommunicate.
 * @author Бренинг Иван
 * @date 13.12.24
 * @version 1.0
 */

/**
 * @brief Парсит сообщение, полученное от клиента.
 *
 * Функция извлекает из сообщения логин, соль и хеш пароля.
 *
 * @param message Сообщение от клиента в формате `<логин><соль><хеш>`.
 *                Длина соли - 16 символов, длина хеша - 40 символов.
 * @return Структура `ParsedMessage`, содержащая логин, соль и хеш.
 *         В случае ошибки парсинга возвращается пустая структура.
 *
 * @note Функция использует фиксированные длины для соли и хеша.
 * @note Функция не проверяет корректность формата сообщения.
 */
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

/**
 * @brief Осуществляет взаимодействие с клиентом.
 *
 * Функция получает сообщение от клиента, парсит его, аутентифицирует пользователя
 * и, в случае успеха, обрабатывает данные от клиента.
 *
 * @param socket Сокет для взаимодействия с клиентом.
 * @param userDbFileName Имя файла базы данных пользователей.
 * @param logFileName Имя файла для записи логов.
 *
 * @note Функция использует классы `ConnectToBase`, `Calculator` и `Interface` для выполнения соответствующих задач.
 * @note Функция не обрабатывает ошибки отправки данных клиенту.
 * @note Функция не закрывает соединение с клиентом.
 */
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
