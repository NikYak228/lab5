#pragma once

#include <stdint.h>
// класс, представляющий простой таймер обратного отсчета
class Timer {
public:
    // конструктор таймера
    Timer(int id, int64_t timeoutMs);
    // проверяет, истекло ли время таймера
    bool ready();
    // возвращает идентификатор таймера
    int getId();
private:
    int id;                 // идентификатор таймера
    int64_t startTimeMs;    // время запуска таймера в миллисекундах
    int64_t timeoutMs;      // продолжительность таймера в миллисекундах
};