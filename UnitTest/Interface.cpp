#include "Interface.h"

Interface::Interface(const std::string& fileName) : logFileName(fileName) {}

void Interface::logMessage(const std::string& message) {
    std::ofstream logFile(logFileName, std::ios::app);
    logFile << "[INFO] " << message << std::endl;
}

void Interface::logError(const std::string& error, bool isCritical) {
    std::ofstream logFile(logFileName, std::ios::app);
    if (isCritical) {
        logFile << "[CRITICAL ERROR] " << error << std::endl;
    } else {
        logFile << "[ERROR] " << error << std::endl;
    }
}
