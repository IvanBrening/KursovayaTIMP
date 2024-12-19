/**
 * @file main.cpp
 * @brief Главный файл приложения сервера.
 * @author Бренинг Иван
 * @date 13.12.24
 * @version 1.0
 *
 * Файл содержит точку входа в приложение.
 * Инициирует запуск сервера с помощью вызова Interface::runServer.
 */

#include "Interface.h"

/**
 * @brief Точка входа в приложение.
 *
 * @param argc Количество аргументов командной строки.
 * @param argv Аргументы командной строки.
 * @return 0 в случае успешного завершения, ненулевое значение в случае ошибки.
 */
int main(int argc, char* argv[]) {
    return Interface::runServer(argc, argv);
}
