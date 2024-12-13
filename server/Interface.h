#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <sstream>

class Interface {
public:
    static void printUsage();
    static void logError(const std::string& logFileName, const std::string& message, bool isCritical);
    static void logMessage(const std::string& logFileName, const std::string& message);
    static int runServer(int argc, char* argv[]);

    // Возвращает результат разбора строки, запишет ошибки в буффер, если они есть
    static int getParseResult(int argc, char** argv, std::stringstream& buffer);

private:
    static std::string logFileName;
    static std::string userDbFileName;
    static int port;

    static int parseCommandLine(int argc, char* argv[]);

    static int startServer();
};

#endif
