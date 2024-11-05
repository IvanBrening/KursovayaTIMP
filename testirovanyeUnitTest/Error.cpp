#include "Error.h"

void Error::logError(const std::string& message, bool isCritical) {
    std::cerr << (isCritical ? "Critical Error: " : "Error: ") << message << std::endl;
}

