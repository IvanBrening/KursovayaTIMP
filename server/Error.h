#ifndef ERROR_H
#define ERROR_H

#include <string>

class Error {
public:
    static void logError(const std::string& message, bool isCritical = false);
};

#endif
