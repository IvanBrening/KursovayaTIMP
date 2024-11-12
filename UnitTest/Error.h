#ifndef ERROR_H
#define ERROR_H

#include <string>
#include "Interface.h"

class Error {
public:
    Error(Interface* logger);
    void report(const std::string& errorMessage, bool critical = false);
private:
    Interface* logger;
};

#endif // ERROR_H

