#include <UnitTest++/UnitTest++.h>
#include "Interface.h"
#include "Error.h"
#include "Calculator.h"
#include "ConnectToBase.h"
#include "ClientCommunicate.h"
#include <fstream>
#include <string>
#include <cstdio>  // Для удаления файла

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
        // Удаляем файл лога перед тестами, если он существует
        std::remove(logFileName.c_str());
    }

    ~LogFileSetup() {
        // Удаляем файл лога после тестов
        std::remove(logFileName.c_str());
    }
};

// Тестирование логирования сообщений
TEST_FIXTURE(LogFileSetup, Interface_LogMessage) {
    Interface logger(logFileName);
    logger.logMessage("Test info message");

    CHECK(checkLogContains(logFileName, "[INFO] Test info message"));
}

// Тестирование логирования ошибок
TEST_FIXTURE(LogFileSetup, Interface_LogError) {
    Interface logger(logFileName);
    logger.logError("Test error", false);

    CHECK(checkLogContains(logFileName, "[ERROR] Test error"));
}

// Тестирование критических ошибок
TEST_FIXTURE(LogFileSetup, Error_ReportCritical) {
    Interface logger(logFileName);
    Error errorHandler(&logger);
    errorHandler.report("Critical error occurred", true);

    CHECK(checkLogContains(logFileName, "[CRITICAL ERROR] Critical error occurred"));
}

// Тестирование некритичных ошибок
TEST_FIXTURE(LogFileSetup, Error_ReportNonCritical) {
    Interface logger(logFileName);
    Error errorHandler(&logger);
    errorHandler.report("Non-critical error", false);

    CHECK(checkLogContains(logFileName, "[ERROR] Non-critical error"));
}

// Тест вычисления суммы квадратов чисел
TEST(Calculator_SumOfSquares) {
    Calculator calculator;
    std::vector<uint16_t> values = {3, 4, 5};
    CHECK_EQUAL(50, calculator.sumOfSquares(values));
}

// Тест на переполнение при вычислении суммы квадратов
TEST(Calculator_SumOfSquares_Overflow) {
    Calculator calculator;
    std::vector<uint16_t> values = {30000, 40000, 50000};
    CHECK_EQUAL(1, calculator.sumOfSquares(values));  // Ожидаем 1 при переполнении
}

// Тест аутентификации пользователя (успешный случай)
TEST_FIXTURE(LogFileSetup, ConnectToBase_AuthenticateSuccess) {
    // Создаем файл базы данных с нужными данными
    std::ofstream dbFile("user_db.txt");
    dbFile << "user P@ssW0rd" << std::endl;
    dbFile.close();

    ConnectToBase db("user_db.txt");  // Правильный путь
    std::string salt = "0000000000000000";
    std::string correctHash = db.hashPassword("P@ssW0rd", salt);  // Хэш пароля с солью
    CHECK(db.authenticate("user", salt, correctHash));  // Проверяем успешную аутентификацию

    std::remove("user_db.txt");  // Удаляем файл базы данных после теста
}

// Тест на неуспешное подключение к базе данных (неправильный путь)
TEST_FIXTURE(LogFileSetup, ConnectToBase_Authenticate_Failure_InvalidPath) {
    ConnectToBase db("invalid_db.txt");  // Неправильный путь
    std::string salt = "0000000000000000";
    CHECK(!db.authenticate("user", salt, "1111111111111111111111111111111111111110"));  // Ожидаем неудачную аутентификацию
}

// Тест парсинга сообщения для правильного разделения логина, соли и хеша
TEST(ClientCommunicate_ParseMessage_Success) {
    ClientCommunicate comm;
    std::string login, salt, hash;
    
    // Строка с ожидаемыми данными (60 символов)
    std::string testData = "user00000000000000001111111111111111111111111111111111111111";  // 60 символов

    bool result = comm.parseMessage(testData, login, salt, hash);

    CHECK(result);  // Проверка успешности разбора
    CHECK_EQUAL("user", login);  // Проверка логина
    CHECK_EQUAL("0000000000000000", salt);  // Проверка соли
    CHECK_EQUAL("1111111111111111111111111111111111111111", hash);  // Проверка хэша
}

// Тест на ошибку парсинга (слишком короткое сообщение)
TEST(ClientCommunicate_ParseMessage_Failure_ShortMessage) {
    ClientCommunicate comm;
    std::string login, salt, hash;
    CHECK(!comm.parseMessage("user0123456", login, salt, hash));  // Сообщение слишком короткое для корректного разбора
}

// Тест на ошибку парсинга (отсутствие соли)
TEST(ClientCommunicate_ParseMessage_Failure_NoSalt) {
    ClientCommunicate comm;
    std::string login, salt, hash;
    CHECK(!comm.parseMessage("user1111111111111111111111111111111111111111", login, salt, hash));  // Отсутствие соли в сообщении
}

// Тест аутентификации пользователя, если логин не найден в базе данных
TEST_FIXTURE(LogFileSetup, ConnectToBase_Authenticate_LoginNotFound) {
    std::ofstream dbFile("user_db.txt");
    dbFile << "user P@ssW0rd" << std::endl;
    dbFile.close();

    ConnectToBase db("user_db.txt");
    std::string salt = "0000000000000000";
    std::string incorrectLogin = "wronguser";
    CHECK(!db.authenticate(incorrectLogin, salt, "incorrectHash"));  // Логин не найден в базе

    std::remove("user_db.txt");  // Удаляем файл базы данных после теста
}

// Тест аутентификации пользователя с неверным хешом
TEST_FIXTURE(LogFileSetup, ConnectToBase_Authenticate_WrongHash) {
    std::ofstream dbFile("user_db.txt");
    dbFile << "user P@ssW0rd" << std::endl;
    dbFile.close();

    ConnectToBase db("user_db.txt");
    std::string salt = "0000000000000000";
    std::string wrongHash = "1111111111111111111111111111111111111110";
    CHECK(!db.authenticate("user", salt, wrongHash));  // Неверный хеш пароля

    std::remove("user_db.txt");  // Удаляем файл базы данных после теста
}

int main() {
    return UnitTest::RunAllTests();  // Запуск всех тестов
}


