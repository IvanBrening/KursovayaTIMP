#include <UnitTest++/UnitTest++.h>
#include "Interface.h"
#include "Error.h"
#include "Calculator.h"
#include "ConnectToBase.h"
#include "ClientCommunicate.h"
#include <fstream>
#include <string>
#include <cstdio>

// Функция для проверки наличия строки в файле лога
bool checkLogContains(const std::string& fileName, const std::string& searchString) {
    std::ifstream logFile(fileName);
    if (!logFile.is_open()) return false;

    std::string line;
    while (std::getline(logFile, line)) {
        if (line.find(searchString) != std::string::npos) {
            return true;
        }
    }
    return false;
}

struct LogFileSetup {
    const std::string logFileName = "test_log.txt";
    
    LogFileSetup() {
        std::remove(logFileName.c_str());
    }

    ~LogFileSetup() {
        std::remove(logFileName.c_str());
    }
};

// Тестирование Interface: Логирование сообщений
TEST_FIXTURE(LogFileSetup, Interface_LogMessage) {
    Interface logger(logFileName);
    logger.logMessage("Test info message");
    CHECK(checkLogContains(logFileName, "[INFO] Test info message"));
}

// Тестирование Interface: Логирование ошибок
TEST_FIXTURE(LogFileSetup, Interface_LogError) {
    Interface logger(logFileName);
    logger.logError("Test error", false);
    CHECK(checkLogContains(logFileName, "[ERROR] Test error"));
}

// Тестирование Interface: Критическая ошибка
TEST_FIXTURE(LogFileSetup, Interface_LogCriticalError) {
    Interface logger(logFileName);
    logger.logError("Test critical error", true);
    CHECK(checkLogContains(logFileName, "[CRITICAL ERROR] Test critical error"));
}

// Тестирование Error: Некритичная ошибка
TEST_FIXTURE(LogFileSetup, Error_ReportNonCritical) {
    Interface logger(logFileName);
    Error errorHandler(&logger);
    errorHandler.report("Non-critical error", false);
    CHECK(checkLogContains(logFileName, "[ERROR] Non-critical error"));
}

// Тестирование Calculator: Нормальная работа
TEST(Calculator_SumOfSquares) {
    Calculator calculator;
    std::vector<uint16_t> values = {3, 4, 5};
    CHECK_EQUAL(50, calculator.sumOfSquares(values));
}

// Тестирование Calculator: Переполнение
TEST(Calculator_SumOfSquares_Overflow) {
    Calculator calculator;
    std::vector<uint16_t> values = {30000, 40000, 50000};
    CHECK_EQUAL(1, calculator.sumOfSquares(values));  // Ожидаем 1 при переполнении
}

// Тестирование ConnectToBase: Успешная аутентификация
TEST_FIXTURE(LogFileSetup, ConnectToBase_AuthenticateSuccess) {
    std::ofstream dbFile("user_db.txt");
    dbFile << "user P@ssW0rd" << std::endl;
    dbFile.close();

    ConnectToBase db("user_db.txt");
    std::string salt = "0000000000000000";
    std::string correctHash = db.hashPassword("P@ssW0rd", salt);
    CHECK(db.authenticate("user", salt, correctHash));
    std::remove("user_db.txt");
}

// Тестирование ConnectToBase: Неверный логин
TEST_FIXTURE(LogFileSetup, ConnectToBase_Authenticate_LoginNotFound) {
    std::ofstream dbFile("user_db.txt");
    dbFile << "user P@ssW0rd" << std::endl;
    dbFile.close();

    ConnectToBase db("user_db.txt");
    std::string salt = "0000000000000000";
    CHECK(!db.authenticate("wronguser", salt, "incorrectHash"));
    std::remove("user_db.txt");
}

// Тестирование ConnectToBase: Неверный пароль
TEST_FIXTURE(LogFileSetup, ConnectToBase_Authenticate_WrongPassword) {
    std::ofstream dbFile("user_db.txt");
    dbFile << "user P@ssW0rd" << std::endl;
    dbFile.close();

    ConnectToBase db("user_db.txt");
    std::string salt = "0000000000000000";
    std::string wrongHash = "1111111111111111111111111111111111111110";
    CHECK(!db.authenticate("user", salt, wrongHash));
    std::remove("user_db.txt");
}

// Тестирование ConnectToBase: Пустой логин или пароль
TEST(ConnectToBase_EmptyUserOrPassword) {
    ConnectToBase db("user_db.txt");
    std::string salt = "0000000000000000";
    CHECK(!db.authenticate("", salt, ""));
    CHECK(!db.authenticate("user", salt, ""));
    CHECK(!db.authenticate("", salt, "hash"));
}

// Тестирование ConnectToBase: Слишком длинный логин или пароль
TEST(ConnectToBase_LongUserOrPassword) {
    ConnectToBase db("user_db.txt");
    std::string salt = "0000000000000000";
    std::string longUser = std::string(256, 'u');
    std::string longPassword = std::string(256, 'p');
    CHECK(!db.authenticate(longUser, salt, longPassword));
}

// Тестирование ClientCommunicate: Успешное парсирование сообщения
TEST(ClientCommunicate_ParseMessage_Success) {
    ClientCommunicate comm;
    std::string login, salt, hash;
    std::string testData = "user00000000000000001111111111111111111111111111111111111111";
    bool result = comm.parseMessage(testData, login, salt, hash);
    CHECK(result);
    CHECK_EQUAL("user", login);
    CHECK_EQUAL("0000000000000000", salt);
    CHECK_EQUAL("1111111111111111111111111111111111111111", hash);
}

// Тестирование ClientCommunicate: Ошибка при коротком сообщении
TEST(ClientCommunicate_ParseMessage_Failure_ShortMessage) {
    ClientCommunicate comm;
    std::string login, salt, hash;
    CHECK(!comm.parseMessage("user0123456", login, salt, hash));
}

// Тестирование ClientCommunicate: Ошибка при отсутствии соли
TEST(ClientCommunicate_ParseMessage_Failure_NoSalt) {
    ClientCommunicate comm;
    std::string login, salt, hash;
    CHECK(!comm.parseMessage("user", login, salt, hash));
}

// Тестирование ClientCommunicate: Ошибка при отсутствии хеша
TEST(ClientCommunicate_ParseMessage_Failure_NoHash) {
    ClientCommunicate comm;
    std::string login, salt, hash;
    CHECK(!comm.parseMessage("user0000000000000000", login, salt, hash));
}

int main() {
    return UnitTest::RunAllTests();  // Запуск всех тестов
}


