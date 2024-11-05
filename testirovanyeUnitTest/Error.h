#ifndef ERROR_H
#define ERROR_H

#include <iostream>

class Error {
public:
    static void logError(const std::string& message, bool isCritical = false);
};

#endif // ERROR_H

