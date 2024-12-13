#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <vector>
#include <cstdint>

class Calculator {
public:
    uint16_t processVectors(int socket);

    // Функция для вычисления суммы квадратов вектора
    static uint32_t calculateSumOfSquares(const std::vector<uint16_t>& vec);

private:
    
};

#endif
