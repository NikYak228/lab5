#include "AppTime.h"
#include <chrono>
#include <thread>

int64_t Time::currentTimeMillis() {
    using namespace std::chrono;
    // Используем steady_clock для монотонного времени (не зависит от перевода часов)
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

void Time::sleep(int64_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}