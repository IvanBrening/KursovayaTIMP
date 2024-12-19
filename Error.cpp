#include "Error.h"
#include <iostream>

/**
 * @file Error.cpp
 * @brief Содержит реализацию класса Error.
 * @author Бренинг Иван
 * @date 13.12.24
 * @version 1.0
 */

/**
 * @brief Выводит сообщение об ошибке в поток ошибок.
 *
 * @param message Сообщение об ошибке.
 * @param isCritical Флаг, указывающий, является ли ошибка критической.
 *                   Если `true`, то сообщение будет выведено с префиксом "Critical Error: ", иначе с префиксом "Error: ".
 */
void Error::logError(const std::string& message, bool isCritical) {
    std::cerr << (isCritical ? "Critical Error: " : "Error: ") << message << std::endl;
}
