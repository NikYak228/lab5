#pragma once

#include <vector>
#include <stdint.h>
// интерфейс для перечисления записей
class RecordEnumeration {
public:
    // Получить количество записей
    virtual int getNumberOfRecords() = 0;
    // Получить следующую запись
    virtual std::vector<int8_t> getNextRecord() = 0;
    // Сбросить перечисление в начало
    virtual void reset() = 0;
    // Получить ID следующей записи (если применимо)
    virtual int getNextRecordId() = 0;
    // Уничтожить перечисление и связанные ресурсы
    virtual void destroy() = 0;
};
