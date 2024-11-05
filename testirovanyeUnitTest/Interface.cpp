#include "Interface.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>

void Interface::printUsage() {
    std::cout << "Usage: ./server -l log_file -b user_db [-p port (default 22852)]\n";
}

void Interface::logError(const std::string& logFileName, const std::string& message, bool isCritical) {
    std::ofstream logFile(logFileName, std::ios::app);
    if (logFile.is_open()) {
        std::time_t currentTime = std::time(nullptr);
        logFile << std::put_time(std::localtime(&currentTime), "%Y-%m-%d %H:%M:%S") << " - " << (isCritical ? "Critical" : "Non-critical") << " error: " << message << std::endl;
    }
}

void Interface::logMessage(const std::string& logFileName, const std::string& message) {
    std::ofstream logFile(logFileName, std::ios::app);
    if (logFile.is_open()) {
        std::time_t currentTime = std::time(nullptr);
        logFile << std::put_time(std::localtime(&currentTime), "%Y-%m-%d %H:%M:%S") << " - Info: " << message << std::endl;
    }
}

