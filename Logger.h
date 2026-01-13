#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <cstdarg>

/**
 * Logger
 * 
 * Простая система логгирования в файл и консоль.
 * Поддерживает уровни важности и категории.
 */
class Logger {
public:
    enum Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR_LVL
    };

    static void init(const std::string& filename);
    static void close();
    static void log(Level level, const char* category, const char* format, ...);

private:
    static std::ofstream logFile;
};

// Макросы для удобства использования
#define LOG_INFO(cat, fmt, ...) Logger::log(Logger::INFO, cat, fmt, ##__VA_ARGS__)
#define LOG_WARN(cat, fmt, ...) Logger::log(Logger::WARNING, cat, fmt, ##__VA_ARGS__)
#define LOG_ERR(cat, fmt, ...)  Logger::log(Logger::ERROR_LVL, cat, fmt, ##__VA_ARGS__)

// Специфичные макросы для подсистем
#define LOG_GEN(fmt, ...) LOG_INFO("GEN", fmt, ##__VA_ARGS__)
#define LOG_PHYS(fmt, ...) LOG_INFO("PHYS", fmt, ##__VA_ARGS__)
#define LOG_AI(fmt, ...) LOG_INFO("AI", fmt, ##__VA_ARGS__)

#endif