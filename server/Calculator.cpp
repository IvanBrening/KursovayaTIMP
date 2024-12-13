#include "Calculator.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <climits>
#include <iostream>

uint32_t Calculator::calculateSumOfSquares(const std::vector<uint16_t>& vec) {
    uint32_t sumOfSquares = 0;
    bool overflowUp = false;

    for (const auto& value : vec) {
        // Проверка на переполнение вверх
        if (sumOfSquares > static_cast<uint32_t>(UINT16_MAX) - static_cast<uint32_t>(value * value)) {
            overflowUp = true;
            break; // Прерываем, если произойдет переполнение
        }
        sumOfSquares += value * value;
    }
    if (overflowUp) {
        return 65535; // Переполнение вверх
    } else {
        return sumOfSquares;
    }
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

        for (size_t i = 0; i < vector.size(); ++i) {
            vector[i] = ntohs(vector[i]);
        }

        uint32_t result = calculateSumOfSquares(vector);

        uint32_t networkResult = htonl(result);
        send(socket, &networkResult, sizeof(uint32_t), 0); // Отправляем результат обратно клиенту
    }

    return 0;
}
