#include "Calculator.h"
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>
#include <limits>
#include <algorithm>
#include <limits>
#include <sys/socket.h>


uint32_t Calculator::calculateVectorSum(const std::vector<uint16_t>& vector) {
    uint32_t sum = 0;
    for (uint16_t value : vector) {
        if (std::numeric_limits<uint32_t>::max() - sum < static_cast<uint32_t>(value) * value) {
            return std::numeric_limits<uint16_t>::max(); // Возврат максимума 16-битного числа
        }
        sum += static_cast<uint32_t>(value) * value;
    }
    return sum;
}

uint16_t Calculator::processVectors(int socket) {
    uint32_t numberOfVectors;
    // Получаем количество векторов от клиента
    if (recv(socket, &numberOfVectors, sizeof(uint32_t), 0) <= 0) {
        return -1; // Ошибка при получении
    }
    numberOfVectors = ntohl(numberOfVectors);
    for (uint32_t i = 0; i < numberOfVectors; ++i) {
        uint32_t vectorSize;
        // Получаем размер вектора
        if (recv(socket, &vectorSize, sizeof(uint32_t), 0) <= 0) {
            return -1; // Ошибка при получении
        }
        vectorSize = ntohl(vectorSize);
        std::vector<uint16_t> vector(vectorSize);
        // Получаем значения вектора
        if (recv(socket, vector.data(), vectorSize * sizeof(uint16_t), 0) <= 0) {
            return -1; // Ошибка при получении
        }
        uint32_t sumOfSquares = 0;
        bool overflowUp = false;
        for (const auto& value : vector) {
            // Проверка на переполнение вверх
            if (sumOfSquares > (static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) - value * value)) {
                overflowUp = true;
                break; // Прерываем, если произойдет переполнение
            }
            sumOfSquares += value * value;
        }
        // Логика для определения результата в зависимости от переполнения
        uint32_t result;
        if (overflowUp) {
            result = 1; // Переполнение вверх
        } else if (sumOfSquares > UINT16_MAX) {
            result = 65535; // Переполнение вниз
        } else {
            result = static_cast<uint32_t>(sumOfSquares);
        }
        uint32_t networkResult = htonl(result);
        send(socket, &networkResult, sizeof(uint32_t), 0); // Отправляем результат обратно клиенту
    }
    return 0;
}

