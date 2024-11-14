#ifndef ERROR_H
#define ERROR_H

#include <string>
#include "Interface.h"

class Error {
public:
    explicit Error(Interface* logger);
    void report(const std::string& message, bool isCritical);
private:
    Interface* logger;
};

#endif // ERROR_H

