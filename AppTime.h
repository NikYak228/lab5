#pragma once
#include <cstdint>

/**
 * AppTime
 * 
 * Утилиты для работы со временем.
 * Обертка над системными функциями для кросс-платформенности.
 */
class Time {
public:
    // Получить текущее время в миллисекундах (с момента запуска системы или эпохи)
    static int64_t currentTimeMillis();
    
    // Приостановить выполнение потока на ms миллисекунд
    static void sleep(int64_t ms);
};
