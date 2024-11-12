#include "Calculator.h"
#include <limits>

uint16_t Calculator::sumOfSquares(const std::vector<uint16_t>& values) {
    uint32_t result = 0;
    for (uint16_t value : values) {
        uint32_t square = static_cast<uint32_t>(value) * value;
        if (result + square < result) return 1;  // Upward overflow
        result += square;
    }
    return (result > std::numeric_limits<uint16_t>::max()) ? 65535 : static_cast<uint16_t>(result);
}

