#include "Logger.h"
#include <iostream>
#include <ctime>
#include <vector>
#include <cstdio>

std::ofstream Logger::logFile;

void Logger::init(const std::string& filename) {
    if (logFile.is_open()) {
        logFile.close();
    }
    // Открываем файл с очисткой
    logFile.open(filename, std::ios::out | std::ios::trunc);
    if (!logFile.is_open()) {
        std::cerr << "Не удалось открыть лог-файл: " << filename << std::endl;
    } else {
        log(INFO, "SYS", "Logger инициализирован. Файл: %s", filename.c_str());
    }
}

void Logger::close() {
    if (logFile.is_open()) {
        log(INFO, "SYS", "Logger завершает работу...");
        logFile.close();
    }
}

void Logger::log(Level level, const char* category, const char* format, ...) {
    // Получаем текущее время
    std::time_t now = std::time(nullptr);
    char timeStr[20];
    std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", std::localtime(&now));

    // Форматируем сообщение
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Уровень лога в строку
    const char* levelStr = "INFO";
    if (level == DEBUG) levelStr = "DBG ";
    if (level == WARNING) levelStr = "WARN";
    if (level == ERROR_LVL) levelStr = "ERR ";

    // Формируем финальную строку
    char finalBuffer[1200];
    snprintf(finalBuffer, sizeof(finalBuffer), "[%s] [%s] [%s] %s", timeStr, levelStr, category, buffer);

    // Вывод в консоль
    std::cout << finalBuffer << std::endl;

    // Вывод в файл
    if (logFile.is_open()) {
        logFile << finalBuffer << std::endl;
        logFile.flush(); // Важно, чтобы данные не потерялись при крэше
    }
}