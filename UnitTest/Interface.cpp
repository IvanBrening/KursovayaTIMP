#include "Interface.h"

Interface::Interface(const std::string& logFileName) {
    logFile.open(logFileName, std::ios::app);
}

void Interface::logMessage(const std::string& message) {
    if (logFile.is_open()) {
        logFile << "[INFO] " << message << std::endl;
    }
}

void Interface::logError(const std::string& errorMessage, bool critical) {
    if (logFile.is_open()) {
        logFile << (critical ? "[CRITICAL ERROR] " : "[ERROR] ") << errorMessage << std::endl;
    }
}

