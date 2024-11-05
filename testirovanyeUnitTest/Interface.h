#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>

class Interface {
public:
    static void printUsage();
    static void logError(const std::string& logFileName, const std::string& message, bool isCritical);
    static void logMessage(const std::string& logFileName, const std::string& message);
};

#endif // INTERFACE_H

