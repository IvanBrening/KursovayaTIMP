#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <iomanip>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <arpa/inet.h>
#include <random>
#include <openssl/rand.h>

// Генерация соли
std::string generateSalt() {
    uint64_t saltValue;
    RAND_bytes(reinterpret_cast<unsigned char*>(&saltValue), sizeof(saltValue));
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << saltValue;
    return ss.str();
}

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

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <login> <password>" << std::endl;
        return 1;
    }

    std::string login = argv[1];
    std::string password = argv[2];

    // Генерация соли и вычисление хэша
    std::string salt = generateSalt();
    std::string hash = hashPassword(password, salt);

    // Создание сокета
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Ошибка создания сокета" << std::endl;
        return 1;
    }

    sockaddr_in servAddr{};
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(22852);  // Порт по умолчанию
    servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Локальный сервер

    if (connect(sock, reinterpret_cast<sockaddr*>(&servAddr), sizeof(servAddr)) < 0) {
        std::cerr << "Ошибка подключения" << std::endl;
        return 1;
    }

    // Отправка данных для аутентификации
    std::stringstream msgStream;
    msgStream << login << " " << salt << " " << hash;
    send(sock, msgStream.str().c_str(), msgStream.str().length(), 0);

    // Чтение ответа от сервера
    char buffer[3] = {0};
    read(sock, buffer, 3);
    if (strcmp(buffer, "OK") == 0) {
        std::cout << "Аутентификация прошла успешно!" << std::endl;

        // Отправка векторов
        uint32_t N = 4; // Количество векторов
        send(sock, &N, sizeof(N), 0); // Отправка количества векторов

        for (uint32_t i = 0; i < N; ++i) {
            std::vector<uint16_t> vector(4);
            // Генерация случайных значений для вектора
            for (uint16_t &val : vector) {
                val = rand() % 100; // Случайные значения от 0 до 99
            }

            uint32_t S = vector.size(); // Размер вектора
            send(sock, &S, sizeof(S), 0); // Отправка размера вектора
            send(sock, vector.data(), S * sizeof(uint16_t), 0); // Отправка значений вектора
        }

        // Чтение результатов от сервера
        std::vector<uint16_t> results(N);
        read(sock, results.data(), N * sizeof(uint16_t));

        std::cout << "Результаты:" << std::endl;
        for (uint32_t i = 0; i < N; ++i) {
            std::cout << "Сумма квадратов вектора " << i + 1 << ": " << results[i] << std::endl;
        }
    } else {
        std::cerr << "Ошибка аутентификации!" << std::endl;
    }

    close(sock);
    return 0;
}
