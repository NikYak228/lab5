#include "RecordStore.h"
#include <iostream>
#include <SDL2/SDL.h>
// устанавливает директорию для хранения RecordStore
void RecordStore::setRecordStoreDir(const char* progName) {
    // SDL_GetPrefPath предоставляет кроссплатформенный путь к папке для пользовательских данных
    char* prefPath = SDL_GetPrefPath("my_org", "motorcycle_sim");
    if (prefPath) {
        recordStoreDir = std::filesystem::path(prefPath);
        SDL_free(prefPath);
    } else {
        recordStoreDir = std::filesystem::current_path();
    }
}

// Оставим остальные методы пустыми, они не нужны для симуляции
RecordStore* RecordStore::openRecordStore(std::string name, bool createIfNecessary) { return nullptr; }
void RecordStore::closeRecordStore() {}
void RecordStore::deleteRecordStore(std::string name) {}
std::vector<std::string> RecordStore::listRecordStores() { return {}; }
RecordEnumeration* RecordStore::enumerateRecords(RecordFilter* filter, RecordComparator* comparator, bool keepUpdated) { return nullptr; }
int RecordStore::addRecord(std::vector<int8_t> arr, int offset, int numBytes) { return 0; }
void RecordStore::setRecord(int recordId, std::vector<int8_t> arr, int offset, int numBytes) {}