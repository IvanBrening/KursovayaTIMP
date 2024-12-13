#include "Interface.h"
#include "ClientCommunicate.h"
#include "Error.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <arpa/inet.h>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <stdexcept>
#include <cstring>

std::string Interface::logFileName = "";
std::string Interface::userDbFileName = "";
int Interface::port = 33333;

void Interface::printUsage() {
    std::cout << "Usage: ./server -l log_file -b user_data_base [-p port (default 33333)]" << std::endl;
}

void Interface::logError(const std::string& logFileName, const std::string& message, bool isCritical) {
    std::ofstream logFile(logFileName, std::ios::app);
    if (logFile.is_open()) {
        std::time_t currentTime = std::time(nullptr);
        logFile << std::put_time(std::localtime(&currentTime), "%Y-%m-%d %H:%M:%S")
                << " - " << (isCritical ? "Critical" : "Non-critical") << " error: "
                << message << std::endl;
    }
}

void Interface::logMessage(const std::string& logFileName, const std::string& message) {
    std::ofstream logFile(logFileName, std::ios::app);
    if (logFile.is_open()) {
        std::time_t currentTime = std::time(nullptr);
        logFile << std::put_time(std::localtime(&currentTime), "%Y-%m-%d %H:%M:%S")
                << " - Info: " << message << std::endl;
    }
}

int Interface::parseCommandLine(int argc, char* argv[]) {
    Interface::logFileName = "";
    Interface::userDbFileName = "";
    Interface::port = 33333;

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-l") {
            if (i + 1 < argc) {
                Interface::logFileName = argv[++i];
            } else {
                Interface::printUsage();
                return 1;
            }
        } else if (std::string(argv[i]) == "-b") {
            if (i + 1 < argc) {
                Interface::userDbFileName = argv[++i];
            } else {
                Interface::printUsage();
                return 1;
            }
        } else if (std::string(argv[i]) == "-p") {
            if (i + 1 < argc) {
                try {
                    Interface::port = std::stoi(argv[++i]);
                    if (Interface::port <= 0 || Interface::port > 65535) {
                        throw std::out_of_range("Port out of range");
                    }
                } catch (const std::invalid_argument&) {
                    std::cerr << "Critical Error: Invalid port value \"" << argv[i]
                              << "\". Please specify a valid port number (1-65535)." << std::endl;
                    return -1;
                } catch (const std::out_of_range&) {
                    std::cerr << "Critical Error: Port value \"" << argv[i]
                              << "\" is out of range. Please specify a valid port number (1-65535)." << std::endl;
                    return -1;
                }
            } else {
                Interface::printUsage();
                return 1;
            }
        } else if (argv[i][0] == '-') {
            Interface::printUsage();
            return 1;
        }
    }

    if (Interface::logFileName.empty() || Interface::userDbFileName.empty()) {
        Interface::printUsage();
        return 1;
    }

    return 0;
}

int Interface::startServer() {
    // Проверка наличия лог-файла
    std::ofstream logTest(Interface::logFileName, std::ios::app);
    if (!logTest.is_open()) {
        std::cerr << "Critical Error: Cannot open log file: " << Interface::logFileName << std::endl;
        return -1; // Ошибка при доступе к лог-файлу
    }
    logTest.close(); // Закрываем файл после проверки

    // Проверка наличия базы данных
    std::ifstream dbFile(Interface::userDbFileName);
    if (!dbFile.is_open()) {
        std::cerr << "Critical Error: Cannot open database file: " << Interface::userDbFileName << std::endl;
        Interface::logError(Interface::logFileName, "Cannot open database file: " + Interface::userDbFileName, true);
        return -1; // Ошибка при открытии базы данных
    }
    dbFile.close(); // Закрываем файл базы данных, так как проверка завершена

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Создание сокета
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        Interface::logError(Interface::logFileName, "Socket creation error", true);
        std::cerr << "Socket creation error" << std::endl;
        return -1; // Ошибка создания сокета
    }

    // Установка параметров сокета
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        Interface::logError(Interface::logFileName, "Setsockopt error", true);
        std::cerr << "Setsockopt error" << std::endl;
        return -1; // Ошибка установки параметров
    }

    // Указание параметров адреса
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(Interface::port);

    // Привязка сокета к адресу
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        Interface::logError(Interface::logFileName, "Bind error", true);
        std::cerr << "Bind error" << std::endl;
        return -1; // Ошибка привязки
    }

    // Начало прослушивания
    if (listen(server_fd, 3) < 0) {
        Interface::logError(Interface::logFileName, "Listen error", true);
        std::cerr << "Listen error" << std::endl;
        return -1; // Ошибка начала прослушивания
    }

    std::cout << "Server started on port " << Interface::port << std::endl;
    Interface::logMessage(Interface::logFileName, "Server started on port " + std::to_string(Interface::port));

    // Основной цикл ожидания подключения клиентов
    while (true) {
        std::cout << "Waiting for a client..." << std::endl;
        Interface::logMessage(Interface::logFileName, "Waiting for a client...");

        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            Interface::logError(Interface::logFileName, "Accept error", true);
            continue; // Ошибка принятия соединения
        }

        std::cout << "Client connected!" << std::endl;
        ClientCommunicate clientComm;
        clientComm.communicate(new_socket, Interface::userDbFileName, Interface::logFileName); // Взаимодействие с клиентом

        close(new_socket); // Закрытие соединения с клиентом
    }

    close(server_fd); // Закрытие сервера
    return 0;
}

int Interface::runServer(int argc, char* argv[]) {
    int parseResult = Interface::parseCommandLine(argc, argv);
    if (parseResult != 0) {
        return parseResult;
    }

    return Interface::startServer();
}

int Interface::getParseResult(int argc, char** argv, std::stringstream& buffer) {
    // Перенаправляем stderr в буфер
    std::streambuf *old_cerr = std::cerr.rdbuf(buffer.rdbuf());
    std::streambuf *old_cout = std::cout.rdbuf(buffer.rdbuf());
    int result = parseCommandLine(argc, argv);
    // Восстанавливаем stderr
    std::cerr.rdbuf(old_cerr);
    std::cout.rdbuf(old_cout);
    return result;
}
