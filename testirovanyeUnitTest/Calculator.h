// Calculator.h
#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <vector>
#include <cstdint>

class Calculator {
public:
    uint32_t calculateVectorSum(const std::vector<uint16_t>& vector);
    uint16_t processVectors(int socket); // Добавьте это объявление, если он должен быть в классе
};

#endif // CALCULATOR_H

