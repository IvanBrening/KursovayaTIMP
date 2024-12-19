#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <climits>
#include <cstring>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <iomanip>
#include <openssl/sha.h>

#include "Calculator.h"
#include "ConnectToBase.h"
#include "Interface.h"
#include "Error.h"
#include "ClientCommunicate.h"

#include <UnitTest++/UnitTest++.h>

// Фикстура для тестов, связанных с файлами
struct FileTestFixture {
    const std::string TEST_DB_FILE = "test_db.txt";
    const std::string TEST_LOG_FILE = "test_log.txt";

    FileTestFixture() {
        // Создание тестовой базы данных
        std::ofstream dbFile(TEST_DB_FILE);
        if (dbFile.is_open()) {
            dbFile << "user1 password123\n";
            dbFile << "user2 securepass\n";
            dbFile << "longuser" + std::string(256, 'a') + " longpass" + std::string(256, 'b') + "\n";
            dbFile.close();
        }
    }

    ~FileTestFixture() {
        // Удаление тестовой базы данных и лог-файла
        std::remove(TEST_DB_FILE.c_str());
        std::remove(TEST_LOG_FILE.c_str());
    }
};

// Тесты для модуля логирования (Interface)
TEST_FIXTURE(FileTestFixture, LogInfoMessage) {
    const std::string message = "This is an informational message.";
    Interface::logMessage(TEST_LOG_FILE, message);

    std::ifstream logFile(TEST_LOG_FILE);
    std::string line;
    std::getline(logFile, line);
    logFile.close();

    CHECK(line.find(message) != std::string::npos);
}

TEST_FIXTURE(FileTestFixture, LogError) {
    const std::string message = "This is an error message.";
    Interface::logError(TEST_LOG_FILE, message, false);

    std::ifstream logFile(TEST_LOG_FILE);
    std::string line;
    std::getline(logFile, line);
    logFile.close();

    CHECK(line.find(message) != std::string::npos);
    CHECK(line.find("Non-critical") != std::string::npos);
}

TEST_FIXTURE(FileTestFixture, LogCriticalError) {
    const std::string message = "This is a critical error message.";
    Interface::logError(TEST_LOG_FILE, message, true);

    std::ifstream logFile(TEST_LOG_FILE);
    std::string line;
    std::getline(logFile, line);
    logFile.close();

    CHECK(line.find(message) != std::string::npos);
    CHECK(line.find("Critical") != std::string::npos);
}

// Тесты для модуля вызова ошибок (Error)
TEST(HandleError) {
    const std::string message = "This is a test error.";

    // Перенаправляем stderr в буфер
    std::stringstream buffer;
    std::streambuf *old = std::cerr.rdbuf(buffer.rdbuf());

    Error::logError(message);

    // Восстанавливаем stderr
    std::cerr.rdbuf(old);

    CHECK(buffer.str().find(message) != std::string::npos);
}

// Тесты для модуля взаимодействия с базой данных (ConnectToBase)
TEST_FIXTURE(FileTestFixture, SuccessfulAuthentication) {
    ConnectToBase db;
    std::string salt = "salt";
    std::string password = "password123";
    std::string clientHash = "ca74ec8f894b777b36a6f5ffbf54f7f6c1ae352f";

    CHECK(db.authenticateUser("user1", salt, clientHash, TEST_DB_FILE));
}

TEST_FIXTURE(FileTestFixture, InvalidLogin) {
    ConnectToBase db;
    CHECK(!db.authenticateUser("invaliduser", "salt", "hash", TEST_DB_FILE));
}

TEST_FIXTURE(FileTestFixture, InvalidPassword) {
    ConnectToBase db;
    CHECK(!db.authenticateUser("user1", "salt", "wronghash", TEST_DB_FILE));
}

TEST_FIXTURE(FileTestFixture, EmptyLoginOrPassword) {
    ConnectToBase db;
    CHECK(!db.authenticateUser("", "salt", "hash", TEST_DB_FILE));
    CHECK(!db.authenticateUser("user1", "salt", "", TEST_DB_FILE));
}

TEST_FIXTURE(FileTestFixture, TooLongLoginOrPassword) {
    ConnectToBase db;
    std::string longString(300, 'a');
    CHECK(!db.authenticateUser(longString, "salt", "hash", TEST_DB_FILE));
    CHECK(!db.authenticateUser("user1", "salt", longString, TEST_DB_FILE));
}

// Тесты для Calculator
TEST(CalculatorTest_CalculateSumOfSquares_DirectInput) {
    std::vector<uint16_t> vec = {1, 2, 3};
    CHECK_EQUAL(14, Calculator::calculateSumOfSquares(vec));

    std::vector<uint16_t> vec2 = {UINT16_MAX, 2};
    CHECK_EQUAL(65535, Calculator::calculateSumOfSquares(vec2));

    std::vector<uint16_t> emptyVec;
    CHECK_EQUAL(0, Calculator::calculateSumOfSquares(emptyVec));
}

// Тесты для ClientCommunicate
TEST_FIXTURE(FileTestFixture, ParseValidMessage) {
    std::string message = "user1somesalt12345678e5fc2f79d56d7dd9497959d856679d78f0858c9a";
    ParsedMessage parsed = ClientCommunicate::parseMessage(message);

    CHECK_EQUAL("user1", parsed.login);
    CHECK_EQUAL("somesalt12345678", parsed.salt);
    CHECK_EQUAL("e5fc2f79d56d7dd9497959d856679d78f0858c9a", parsed.hash);
}

TEST_FIXTURE(FileTestFixture, ParseInvalidMessage) {
    std::string message = "tooshort";
    ParsedMessage parsed = ClientCommunicate::parseMessage(message);
    CHECK(parsed.login.empty());
    CHECK(parsed.salt.empty());
    CHECK(parsed.hash.empty());
}

// Тесты для разбора командной строки
TEST(CommandLineParsing_ValidCommandLine) {
    std::stringstream buffer;
    char* argv[] = {(char*)"./server", (char*)"-l", (char*)"log.txt", (char*)"-b", (char*)"users.db", (char*)"-p", (char*)"12345", nullptr};
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CHECK_EQUAL(0, Interface::getParseResult(argc, argv, buffer));
    CHECK(buffer.str().empty());
}

TEST(CommandLineParsing_InvalidPort) {
    std::stringstream buffer;
    char* argv[] = {(char*)"./server", (char*)"-l", (char*)"log.txt", (char*)"-b", (char*)"users.db", (char*)"-p", (char*)"abc", nullptr};
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CHECK_EQUAL(-1, Interface::getParseResult(argc, argv, buffer));
    CHECK(buffer.str().find("Critical Error: Invalid port value") != std::string::npos);
}

TEST(CommandLineParsing_MissingArgument) {
    std::stringstream buffer;

    char* argv[] = {(char*)"./server", (char*)"-l", (char*)"-b", (char*)"users.db", nullptr};
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CHECK_EQUAL(1, Interface::getParseResult(argc, argv, buffer));
    CHECK(buffer.str().find("Usage: ./server -l log_file -b user_data_base [-p port (default 33333)]") != std::string::npos);
}

TEST(CommandLineParsing_InvalidFlag) {
    std::stringstream buffer;

    char* argv[] = {(char*)"./server", (char*)"-x", (char*)"log.txt", (char*)"-b", (char*)"users.db", nullptr};
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CHECK_EQUAL(1, Interface::getParseResult(argc, argv, buffer));
    CHECK(buffer.str().find("Usage: ./server -l log_file -b user_data_base [-p port (default 33333)]") != std::string::npos);
}

TEST(CommandLineParsing_NoArguments) {
    std::stringstream buffer;

    char* argv[] = {(char*)"./server", nullptr};
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CHECK_EQUAL(1, Interface::getParseResult(argc, argv, buffer));
    CHECK(buffer.str().find("Usage: ./server -l log_file -b user_data_base [-p port (default 33333)]") != std::string::npos);
}

TEST(CommandLineParsing_OutOfRangePort) {
    std::stringstream buffer;
    char* argv[] = {(char*)"./server", (char*)"-l", (char*)"log.txt", (char*)"-b", (char*)"users.db", (char*)"-p", (char*)"70000", nullptr};
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CHECK_EQUAL(-1, Interface::getParseResult(argc, argv, buffer));
    CHECK(buffer.str().find("Critical Error: Port value") != std::string::npos);
}


int main(int argc, char** argv) {
    return UnitTest::RunAllTests();
}
