#include "AppTime.h"
#include <chrono>
#include <thread>

int64_t Time::currentTimeMillis() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

void Time::sleep(int64_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
