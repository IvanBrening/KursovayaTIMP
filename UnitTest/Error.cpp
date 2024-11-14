#include "Error.h"

Error::Error(Interface* logger) : logger(logger) {}

void Error::report(const std::string& message, bool isCritical) {
    logger->logError(message, isCritical);
}
