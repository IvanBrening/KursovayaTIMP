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

// Класс для вычислений
class Calculator {
public:
    uint16_t processVectors(int socket) {
        uint32_t numberOfVectors;

        // Получаем количество векторов
        recv(socket, &numberOfVectors, sizeof(uint32_t), 0);
        numberOfVectors = ntohl(numberOfVectors); // Преобразуем в сетевой порядок

        for (uint32_t i = 0; i < numberOfVectors; ++i) {
            uint32_t vectorSize;

            // Получаем размер вектора
            recv(socket, &vectorSize, sizeof(uint32_t), 0);
            vectorSize = ntohl(vectorSize); // Преобразуем в сетевой порядок

            std::vector<uint16_t> vector(vectorSize);

            // Получаем значения вектора
            recv(socket, vector.data(), vectorSize * sizeof(uint16_t), 0);

            uint32_t sumOfSquares = 0;
            bool overflow = false;

            for (const auto& value : vector) {
                // Проверяем на переполнение
                if (sumOfSquares > (UINT16_MAX - value * value)) {
                    overflow = true; // Устанавливаем флаг переполнения
                    break;
                }
                sumOfSquares += value * value;
            }

            // Формируем результат
            uint32_t result;
            if (overflow) {
                result = 1; // Переполнение вверх
            } else if (sumOfSquares > UINT16_MAX) {
                result = UINT16_MAX; // Переполнение вниз
            } else {
                result = static_cast<uint32_t>(sumOfSquares);
            }

            // Отправляем результат обратно клиенту (4 байта, uint32_t)
            uint32_t networkResult = htonl(result); // Преобразуем в сетевой порядок
            send(socket, &networkResult, sizeof(uint32_t), 0); // Отправляем результат
        }

        return 0;
    }
};

// Класс для подключения к базе данных
class DatabaseConnector {
public:
    DatabaseConnector(const std::string& dbFileName) : dbFileName(dbFileName) {}

    bool authenticateUser(const std::string& login, const std::string& salt, const std::string& clientHash) {
        std::ifstream dbFile(dbFileName);
        if (!dbFile.is_open()) {
            std::cerr << "Не удалось открыть базу данных пользователей." << std::endl;
            return false;
        }

        std::string dbLogin, dbPassword;
        while (dbFile >> dbLogin >> dbPassword) {
            if (dbLogin == login) {
                std::string serverHash = hashPassword(dbPassword, salt);
                return compareHashes(clientHash, serverHash);
            }
        }
        dbFile.close();
        return false;
    }

private:
    std::string dbFileName;

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

    bool compareHashes(const std::string& clientHash, const std::string& serverHash) {
        std::string clientHashLower = clientHash;
        std::transform(clientHashLower.begin(), clientHashLower.end(), clientHashLower.begin(), ::tolower);
        
        std::string serverHashLower = serverHash;
        std::transform(serverHashLower.begin(), serverHashLower.end(), serverHashLower.begin(), ::tolower);

        return serverHashLower == clientHashLower;
    }
};

// Класс для интерфейса сервера
class ServerInterface {
public:
    void printUsage() const {
        std::cout << "Использование: ./server <log_file> <user_db> <порт (если не 22852)>\n";
    }
void logError(const std::string& logFileName, const std::string& message, bool isCritical) const {
        std::ofstream logFile(logFileName, std::ios::app);
        if (logFile.is_open()) {
            std::time_t currentTime = std::time(nullptr);
            logFile << std::put_time(std::localtime(&currentTime), "%Y-%m-%d %H:%M:%S")
                    << " - " << (isCritical ? "Критическая" : "Не критическая") << " ошибка: "
                    << message << std::endl;
            logFile.close();
        }
    }
};

// Класс для взаимодействия с клиентом
class ClientCommunicator {
public:
    ClientCommunicator(const std::string& userDbFileName, const std::string& logFileName)
        : dbConnector(userDbFileName), serverInterface(), logFileName(logFileName) {}

    void communicate(int socket) {
        char buffer[256] = {0};
        read(socket, buffer, 255);

        std::string receivedData(buffer);
        int saltLength = 16;
        int hashLength = 40;
        int loginLength = receivedData.size() - saltLength - hashLength;

        std::string login = receivedData.substr(0, loginLength);
        std::string salt = receivedData.substr(loginLength, saltLength);
        std::string clientHash = receivedData.substr(loginLength + saltLength, hashLength);

        if (dbConnector.authenticateUser(login, salt, clientHash)) {
            send(socket, "OK", 2, 0);
            Calculator calc;
            calc.processVectors(socket);
        } else {
            serverInterface.logError(logFileName, "Ошибка аутентификации пользователя " + login, false);
            send(socket, "ERR", 3, 0);
        }
    }

private:
    DatabaseConnector dbConnector;
    ServerInterface serverInterface;
    std::string logFileName; // Добавлено поле для имени файла лога
};

class Server {
public:
    Server(int port, const std::string& logFileName, const std::string& userDbFileName) 
        : port(port), logFileName(logFileName), userDbFileName(userDbFileName) {}

    void start() {
        setupServer();
        listenForConnections();
    }

private:
    int port;
    std::string logFileName;
    std::string userDbFileName;

    void setupServer() {
        int opt = 1;
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            logError("Ошибка создания сокета", true);
        }

        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            logError("Ошибка привязки", true);
        }

        if (listen(server_fd, 3) < 0) {
            logError("Ошибка прослушивания", true);
        }

        std::cout << "Сервер запущен и слушает на порту " << port << std::endl;
    }

    void listenForConnections() {
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        int new_socket;

        while ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) >= 0) {
            ClientCommunicator clientComm(userDbFileName, logFileName);
            clientComm.communicate(new_socket);
            close(new_socket);
        }

        close(server_fd);
    }

    void logError(const std::string& message, bool isCritical) const {
        ServerInterface().logError(logFileName, message, isCritical);
    }

    int server_fd;
};

int main(int argc, char* argv[]) {
    ServerInterface interface;
    if (argc < 3 || (argc == 2 && std::string(argv[1]) == "-h")) {
        interface.printUsage();
        return 1;
    }

    std::string logFileName = argv[1];
    std::string userDbFileName = argv[2];
    int port = (argc == 4) ? std::stoi(argv[3]) : 22852;

    Server server(port, logFileName, userDbFileName);
    server.start();

    return 0;
}
