#include "Timer.h"
#include "Time.h"
// конструктор таймера
Timer::Timer(int id, int64_t timeoutMs) {
    this->id = id;
    this->timeoutMs = timeoutMs;
    this->startTimeMs = Time::currentTimeMillis();
}
// проверяет, истекло ли время таймера
bool Timer::ready() {
    return Time::currentTimeMillis() - startTimeMs >= timeoutMs;
}
// возвращает идентификатор таймера
int Timer::getId() {
    return id;
}