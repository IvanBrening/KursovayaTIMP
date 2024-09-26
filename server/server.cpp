#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <arpa/inet.h>
#include <iomanip>

// Хеширование пароля с солью
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

// Аутентификация пользователя
bool authenticateUser(const std::string& login, const std::string& salt, const std::string& clientHash, const std::string& dbFileName) {
    std::ifstream dbFile(dbFileName);
    if (!dbFile.is_open()) {
        std::cerr << "Не удалось открыть базу данных пользователей." << std::endl;
        return false;
    }

    std::string dbLogin, dbHash;
    while (dbFile >> dbLogin >> dbHash) {
        if (login == dbLogin) {
            std::string computedHash = hashPassword(dbHash, salt);
            if (computedHash == clientHash) {
                dbFile.close();
                return true;
            }
        }
    }
    dbFile.close();
    std::cerr << "Несоответствие логина!" << std::endl;
    return false;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Использование: " << argv[0] << " <log_file> <user_db> [port]" << std::endl;
        return 1;
    }

    std::string logFileName = argv[1];
    std::string userDbFileName = argv[2];
    int port = (argc == 4) ? std::stoi(argv[3]) : 22852;

    // Создание сокета
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Ошибка создания сокета" << std::endl;
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "Ошибка setsockopt" << std::endl;
        return -1;
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Ошибка привязки" << std::endl;
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        std::cerr << "Ошибка прослушивания" << std::endl;
        return -1;
    }

    std::cout << "Сервер запущен и слушает на порту " << port << std::endl;

    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Ошибка подключения" << std::endl;
            continue;
        }

        char buffer[1024] = {0};
        read(new_socket, buffer, 1024);
        std::istringstream iss(buffer);
        std::string login, salt, clientHash;
        iss >> login >> salt >> clientHash;

        if (authenticateUser(login, salt, clientHash, userDbFileName)) {
            const char* success = "OK";
            send(new_socket, success, strlen(success), 0);
            std::cout << "Аутентификация успешна!" << std::endl;

            // Обработка векторов
            uint32_t N;
            read(new_socket, &N, sizeof(N)); // Чтение количества векторов

            std::vector<uint16_t> results(N); // Вектор для хранения результатов

            for (uint32_t i = 0; i < N; ++i) {
                uint32_t S;
                read(new_socket, &S, sizeof(S)); // Чтение размера вектора

                std::vector<uint16_t> vector(S);
                read(new_socket, vector.data(), S * sizeof(uint16_t)); // Чтение значений вектора
// Вычисление суммы квадратов
                uint32_t sumOfSquares = 0;
                for (const auto& val : vector) {
                    uint32_t temp = sumOfSquares + static_cast<uint32_t>(val * val);
                    if (temp < sumOfSquares) { // Переполнение
                        sumOfSquares = (sumOfSquares > 65535) ? 1 : 65535;
                        break;
                    }
                    sumOfSquares = temp;
                }

                results[i] = sumOfSquares; // Сохраняем результат для каждого вектора
            }

            // Отправка результатов
            send(new_socket, results.data(), results.size() * sizeof(uint16_t), 0);
        } else {
            const char* failure = "FAIL";
            send(new_socket, failure, strlen(failure), 0);
            std::cerr << "Ошибка аутентификации!" << std::endl;
        }

        close(new_socket);
    }

    return 0;
}
