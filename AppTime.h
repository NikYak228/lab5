#pragma once

#include <stdint.h>

namespace Time {
    // возвращает текущее время в миллисекундах с момента эпохи
    int64_t currentTimeMillis();
    // приостанавливает выполнение потока на указанное количество миллисекунд
    void sleep(int64_t ms);
};