#include "Calculator.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <climits>
#include <iostream>

/**
 * @file Calculator.cpp
 * @brief Содержит реализацию класса Calculator.
 * @author Бренинг Иван
 * @date 13.12.24
 * @version 1.0
 */

/**
 * @brief Вычисляет сумму квадратов элементов вектора.
 *
 * Функция принимает вектор 16-битных беззнаковых целых чисел (`uint16_t`) и вычисляет сумму квадратов его элементов.
 * В функции реализована проверка на переполнение:
 * - Если в процессе вычисления произойдет переполнение 32-битного беззнакового целого (`uint32_t`), функция вернет 65535 (максимальное значение `uint16_t`).
 *
 * @param vec Входной вектор, для которого необходимо рассчитать сумму квадратов.
 * @return Сумма квадратов элементов вектора. Возвращает 65535 в случае переполнения.
 *
 * @note Функция использует статическое приведение типов для предотвращения переполнения при промежуточных вычислениях.
 * @note Функция использует цикл `for` на основе диапазона (range-based for loop) для итерации по элементам вектора.
 */
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

/**
 * @brief Обрабатывает данные векторов, полученные от клиента через сокет.
 *
 * Функция принимает дескриптор сокета, через который происходит взаимодействие с клиентом.
 * Сначала от клиента ожидается получение количества векторов (numberOfVectors) в сетевом порядке байт (big-endian).
 * Затем для каждого вектора:
 * 1. Получает размер вектора (также в сетевом порядке байт).
 * 2. Получает данные вектора. Элементы вектора (`uint16_t`) также ожидаются в сетевом порядке байт.
 * 3. Вычисляет сумму квадратов элементов вектора с помощью функции `calculateSumOfSquares`.
 * 4. Отправляет результат обратно клиенту в сетевом порядке байт.
 *
 * @param socket Сокет, через который происходит обмен данными с клиентом.
 * @return 0 в случае успешной обработки, -1 в случае ошибки при получении данных или если `recv` вернул 0 (соединение закрыто клиентом).
 *
 * @note Функция использует функции `ntohl` и `ntohs` для преобразования порядка байт из сетевого в порядок байт хоста и `htonl` для обратного преобразования.
 * @note Функция не обрабатывает ошибки отправки данных (`send`).
 * @note Функция не закрывает сокет после завершения работы.
 */
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
