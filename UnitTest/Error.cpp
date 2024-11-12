#include "Error.h"

Error::Error(Interface* logger) : logger(logger) {}

void Error::report(const std::string& errorMessage, bool critical) {
    if (logger) {
        logger->logError(errorMessage, critical);
    }
}

