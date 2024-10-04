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
class ConnectToBase {
public:
    bool authenticateUser(const std::string& login, const std::string& salt, const std::string& clientHash, const std::string& dbFileName) {
        std::ifstream dbFile(dbFileName);
        if (!dbFile.is_open()) {
            std::cerr << "Не удалось открыть базу данных пользователей." << std::endl;
            return false;
        }

        std::string dbLogin, dbPassword;
        while (dbFile >> dbLogin >> dbPassword) {
            if (dbLogin == login) {
                std::string serverHash = hashPassword(dbPassword, salt);

                // Преобразуем хэши в нижний регистр
                std::string clientHashLower = clientHash;
                std::transform(clientHashLower.begin(), clientHashLower.end(), clientHashLower.begin(), ::tolower);
                std::string serverHashLower = serverHash;
                std::transform(serverHashLower.begin(), serverHashLower.end(), serverHashLower.begin(), ::tolower);

                return serverHashLower == clientHashLower;
            }
        }
        dbFile.close();
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
};
// Класс для интерфейса сервера
class Interface {
public:
    void printUsage() {
        std::cout << "Использование: ./server <log_file> <user_db> <порт (если не 22852)>\n";
    }
    
    void logError(const std::string& logFileName, const std::string& message, bool isCritical) {
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
class ClientCommunicate {
public:
    void communicate(int socket, const std::string& userDbFileName, const std::string& logFileName) {
        char buffer[256] = {0};
        read(socket, buffer, 255);

        std::string receivedData(buffer);

        int saltLength = 16;
        int hashLength = 40;
        int loginLength = receivedData.size() - saltLength - hashLength;

        std::string login = receivedData.substr(0, loginLength);
        std::string salt = receivedData.substr(loginLength, saltLength);
        std::string clientHash = receivedData.substr(loginLength + saltLength, hashLength);

        ConnectToBase dbConnection;
        Interface interface;

        if (dbConnection.authenticateUser(login, salt, clientHash, userDbFileName)) {
            send(socket, "OK", 2, 0);
            Calculator calc;
            calc.processVectors(socket);
        } else {
            interface.logError(logFileName, "Ошибка аутентификации пользователя " + login, false);
            send(socket, "ERR", 3, 0);
        }
    }
};

int main(int argc, char* argv[]) {
    Interface interface;
    if (argc < 3 || (argc == 2 && std::string(argv[1]) == "-h")) {
        interface.printUsage();
        return 1;
    }

    std::string logFileName = argv[1];
    std::string userDbFileName = argv[2];
    int port = (argc == 4) ? std::stoi(argv[3]) : 22852;

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        interface.logError(logFileName, "Ошибка создания сокета", true);
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        interface.logError(logFileName, "Ошибка setsockopt", true);
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        interface.logError(logFileName, "Ошибка привязки", true);
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        interface.logError(logFileName, "Ошибка прослушивания", true);
        return -1;
    }

    std::cout << "Сервер запущен и слушает на порту " << port << std::endl;

    while ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) >= 0) {
        ClientCommunicate clientComm;
        clientComm.communicate(new_socket, userDbFileName, logFileName);
        close(new_socket);
    }

    close(server_fd);
    return 0;
}
