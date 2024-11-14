#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <fstream>

class Interface {
public:
    explicit Interface(const std::string& fileName);
    void logMessage(const std::string& message);
    void logError(const std::string& error, bool isCritical);
private:
    std::string logFileName;
};

#endif // INTERFACE_H

