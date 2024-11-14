#include "Calculator.h"
#include <limits>

uint16_t Calculator::sumOfSquares(const std::vector<uint16_t>& values) {
    uint32_t sum = 0;
    for (auto value : values) {
        uint32_t square = static_cast<uint32_t>(value) * value;
        if (sum > std::numeric_limits<uint32_t>::max() - square) {
            return 1; // Переполнение
        }
        sum += square;
    }
    return sum > std::numeric_limits<uint16_t>::max() ? 1 : static_cast<uint16_t>(sum);
}
