#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
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

// Класс для обработки ошибок
class Error {
public:
    static void logError(const std::string& message, bool isCritical = false) {
        std::cerr << (isCritical ? "Critical Error: " : "Error: ") << message << std::endl;
    }
};

// Класс для вычислений
class Calculator {
public:
    uint16_t processVectors(int socket) {
        uint32_t numberOfVectors;

        if (recv(socket, &numberOfVectors, sizeof(uint32_t), 0) <= 0) {
            return -1;
        }
        numberOfVectors = ntohl(numberOfVectors);

        for (uint32_t i = 0; i < numberOfVectors; ++i) {
            uint32_t vectorSize;

            if (recv(socket, &vectorSize, sizeof(uint32_t), 0) <= 0) {
                return -1;
            }
            vectorSize = ntohl(vectorSize);

            std::vector<uint16_t> vector(vectorSize);
            if (recv(socket, vector.data(), vectorSize * sizeof(uint16_t), 0) <= 0) {
                return -1;
            }

            uint32_t sumOfSquares = 0;
            bool overflow = false;

            for (const auto& value : vector) {
                if (sumOfSquares > (UINT16_MAX - value * value)) {
                    overflow = true;
                    break;
                }
                sumOfSquares += value * value;
            }

            uint32_t result;
            if (overflow) {
                result = 1;
            } else if (sumOfSquares > UINT16_MAX) {
                result = UINT16_MAX;
            } else {
                result = static_cast<uint32_t>(sumOfSquares);
            }

            uint32_t networkResult = htonl(result);
            send(socket, &networkResult, sizeof(uint32_t), 0);
        }

        return 0;
    }
};

// Класс для подключения к базе данных
class ConnectToBase {
public:
    bool authenticateUser(const std::string& login, const std::string& salt, const std::string& clientHash, const std::string& dbFileName) {
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
        return false;
    }

private:
    std::string hashPassword(const std::string& password, const std::string& salt) {
        std::string data = salt + password;
        unsigned char result[SHA_DIGEST_LENGTH];
        SHA1(reinterpret_cast<const unsigned char*>(data.c_str()), data.size(), result);

        std::stringstream hashStream;
        for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
            hashStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(result[i]);
        }
        return hashStream.str();
    }

    bool compareHashes(const std::string& serverHash, const std::string& clientHash) {
        std::string clientHashLower = clientHash;
        std::string serverHashLower = serverHash;
        std::transform(clientHashLower.begin(), clientHashLower.end(), clientHashLower.begin(), ::tolower);
        std::transform(serverHashLower.begin(), serverHashLower.end(), serverHashLower.begin(), ::tolower);
        return serverHashLower == clientHashLower;
    }
};

// Класс для работы с интерфейсом (логирование, использование)
class Interface {
public:
    static void printUsage() {
        std::cout << "Usage: ./server -l log_file -b user_db [-p port (default 22852)]\n";
    }
static void logError(const std::string& logFileName, const std::string& message, bool isCritical) {
        std::ofstream logFile(logFileName, std::ios::app);
        if (logFile.is_open()) {
            std::time_t currentTime = std::time(nullptr);
            logFile << std::put_time(std::localtime(&currentTime), "%Y-%m-%d %H:%M:%S")
                    << " - " << (isCritical ? "Critical" : "Non-critical") << " error: "
                    << message << std::endl;
        }
    }

    static void logMessage(const std::string& logFileName, const std::string& message) {
        std::ofstream logFile(logFileName, std::ios::app);
        if (logFile.is_open()) {
            std::time_t currentTime = std::time(nullptr);
            logFile << std::put_time(std::localtime(&currentTime), "%Y-%m-%d %H:%M:%S")
                    << " - Info: " << message << std::endl;
        }
    }
};

// Класс для взаимодействия с клиентом
class ClientCommunicate {
public:
    void communicate(int socket, const std::string& userDbFileName, const std::string& logFileName) {
        char buffer[256] = {0};
        if (recv(socket, buffer, sizeof(buffer) - 1, 0) <= 0) {
            return;
        }
        buffer[sizeof(buffer) - 1] = '\0';

        std::string receivedData(buffer);
        int saltLength = 16;
        int hashLength = 40;
        int loginLength = receivedData.size() - saltLength - hashLength;

        std::string login = receivedData.substr(0, loginLength);
        std::string salt = receivedData.substr(loginLength, saltLength);
        std::string clientHash = receivedData.substr(loginLength + saltLength, hashLength);

        ConnectToBase dbConnection;

        if (dbConnection.authenticateUser(login, salt, clientHash, userDbFileName)) {
            send(socket, "OK", 2, 0);
            Interface::logMessage(logFileName, "User " + login + " authenticated successfully.");
            Calculator calc;
            if (calc.processVectors(socket) < 0) {
                Interface::logError(logFileName, "Error processing vectors.", false);
            }
        } else {
            Interface::logError(logFileName, "Authentication failed for user: " + login, false);
            send(socket, "ERR", 3, 0);
        }
    }
};

// Основная программа
int main(int argc, char* argv[]) {
    if (argc < 5) {
        Interface::printUsage();
        return 1;
    }

    std::string logFile;
    std::string userDb;
    int port = 22852;  // Значение порта по умолчанию

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-l") {
            logFile = argv[++i];
        } else if (std::string(argv[i]) == "-b") {
            userDb = argv[++i];
        } else if (std::string(argv[i]) == "-p") {
            port = std::stoi(argv[++i]);
        } else {
            Interface::printUsage();
            return 1;
        }
    }

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Создание сокета
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        Interface::logError(logFile, "Socket creation error", true);
        return -1;
    }

    // Установка параметров сокета
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        Interface::logError(logFile, "Setsockopt error", true);
        return -1;
    }

    // Указание параметров адреса
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Привязка сокета к адресу
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        Interface::logError(logFile, "Bind error", true);
        return -1;
    }

    // Начало прослушивания
    if (listen(server_fd, 3) < 0) {
        Interface::logError(logFile, "Listen error", true);
        return -1;
    }

    std::cout << "Server started on port " << port << std::endl;
    Interface::logMessage(logFile, "Server started on port " + std::to_string(port));
// Основной цикл ожидания подключения клиентов
    while (true) {
        std::cout << "Waiting for a client..." << std::endl;
        Interface::logMessage(logFile, "Waiting for a client...");

        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            Interface::logError(logFile, "Accept error", true);
            continue;
        }

        std::cout << "Client connected!" << std::endl;
        ClientCommunicate clientComm;
        clientComm.communicate(new_socket, userDb, logFile);

        close(new_socket);
    }

    close(server_fd);
    return 0;
}
