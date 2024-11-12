#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <fstream>

class Interface {
public:
    explicit Interface(const std::string& logFileName);
    void logMessage(const std::string& message);
    void logError(const std::string& errorMessage, bool critical);
private:
    std::ofstream logFile;
};

#endif // INTERFACE_H

