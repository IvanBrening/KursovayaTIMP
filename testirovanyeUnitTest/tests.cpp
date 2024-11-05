#include <UnitTest++/UnitTest++.h>
#include "Error.h"
#include "Calculator.h"
#include "ConnectToBase.h"
#include "Interface.h"
#include "ClientCommunicate.h"
#include <vector>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <limits>
#include <algorithm>

// Mock функция для работы с сокетами
class MockSocket {
public:
    MockSocket() {
        socketFd = socket(AF_INET, SOCK_STREAM, 0);
        if (socketFd < 0) {
            throw std::runtime_error("Failed to create socket");
        }
        sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY; 
        serverAddr.sin_port = htons(0); 

        if (bind(socketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            throw std::runtime_error("Failed to bind socket");
        }

        if (listen(socketFd, 1) < 0) {
            throw std::runtime_error("Failed to listen on socket");
        }

        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        clientSocketFd = accept(socketFd, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientSocketFd < 0) {
            throw std::runtime_error("Failed to accept connection");
        }
    }

    ~MockSocket() {
        close(clientSocketFd);
        close(socketFd);
    }

    void sendData(const std::string& data) {
        if (send(clientSocketFd, data.c_str(), data.size(), 0) < 0) {
            throw std::runtime_error("Failed to send data");
        }
    }

    std::string receiveData(size_t size) {
        char buffer[256];
        ssize_t bytesReceived = recv(clientSocketFd, buffer, size, 0);
        if (bytesReceived < 0) {
            throw std::runtime_error("Failed to receive data");
        }
        return std::string(buffer, bytesReceived);
    }

    int getSocket() const {
        return clientSocketFd;
    }

private:
    int socketFd;
    int clientSocketFd;
}; // Добавленная точка с запятой

SUITE(ErrorTests) {
    TEST(ErrorLogging) {
        std::ostringstream logStream;
        std::streambuf* oldCerrBuffer = std::cerr.rdbuf(logStream.rdbuf());

        Error::logError("Test error message", false);

        std::cerr.rdbuf(oldCerrBuffer); // Возвращаем оригинальный буфер
        CHECK(logStream.str().find("Error: Test error message") != std::string::npos);
    }
}

SUITE(CalculatorTests) {
    TEST(VectorSumNoOverflow) {
        Calculator calc;
        std::vector<uint16_t> vector = {3, 4}; // Вектор с данными

        // Ожидаем, что результат будет равен 25 (3^2 + 4^2)
        uint32_t expectedResult = 25;
        uint32_t receivedResult = calc.calculateVectorSum(vector); // Убедитесь, что метод существует

        CHECK_EQUAL(expectedResult, receivedResult);
    }

    TEST(VectorSumOverflow) {
        Calculator calc;
        std::vector<uint16_t> vector = {65535, 65535}; // Вектор с данными

        // Ожидаем, что будет возвращено значение максимума 16-битного числа
        uint32_t expectedResult = std::numeric_limits<uint16_t>::max();
        uint32_t receivedResult = calc.calculateVectorSum(vector); // Убедитесь, что метод существует

        CHECK_EQUAL(expectedResult, receivedResult);
    }
}

SUITE(ConnectToBaseTests) {
    TEST(UserAuthenticationSuccess) {
        std::ofstream dbFile("test_db.txt");
        dbFile << "testUser " << ConnectToBase().hashPassword("testPassword123", "testSalt") << "\n";
        dbFile.close();

        ConnectToBase db;
        bool authenticated = db.authenticateUser("testUser", "testSalt", "testPassword123", "test_db.txt");
        CHECK(authenticated);

        remove("test_db.txt"); // Удаляем тестовый файл
    }

    TEST(UserAuthenticationFailure) {
        std::ofstream dbFile("test_db.txt");
        dbFile << "testUser " << ConnectToBase().hashPassword("testPassword123", "testSalt") << "\n";
        dbFile.close();

        ConnectToBase db;
        bool authenticated = db.authenticateUser("testUser", "wrongSalt", "testPassword123", "test_db.txt");
        CHECK(!authenticated);

        remove("test_db.txt"); // Удаляем тестовый файл
    }
}

int main(int argc, char** argv) {
    return UnitTest::RunAllTests();
}


