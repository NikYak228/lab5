#pragma once

#include <vector>
#include <iostream>

#include "RecordEnumeration.h"
#include "FileStream.h"
// реализация интерфейса RecordEnumeration
class RecordEnumerationImpl : public RecordEnumeration {
private:
    // Текущая позиция для итерации по записям
    int currentPos = 0;
public:
    RecordEnumerationImpl(std::vector<std::vector<int8_t>> data);
    RecordEnumerationImpl();
    ~RecordEnumerationImpl();

    // Реализация методов RecordEnumeration:
    int getNumberOfRecords() override;
    std::vector<int8_t> getNextRecord() override;
    void reset() override;
    int getNextRecordId() override;
    void destroy() override;

    // Методы для добавления и установки записей
    int addRecord(std::vector<int8_t> recordBytes);
    void setRecord(int index, std::vector<int8_t> recordBytes);

    // Методы для сериализации и десериализации данных
    void serialize(FileStream* outStream);
    void deserialize(FileStream* inStream);

    // Хранилище записей
    std::vector<std::vector<int8_t>> recordsData;
};
