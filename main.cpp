#include <memory>
#include <string>
#include <stdexcept>
#include <iostream>
#include "Micro.h"
#include "Logger.h"

// Точка входа в программу
int main(int argc, char** argv)
{
    // Инициализация системы логгирования в файл game_log.txt
    Logger::init("game_log.txt");
    LOG_INFO("SYS", "Программа запущена (main).");
    
    try {
        std::cout << "main: Создание экземпляра Micro..." << std::endl; 
        std::unique_ptr<Micro> micro = std::make_unique<Micro>();
        
        std::cout << "main: Micro создан. Запуск приложения..." << std::endl;
        micro->startApp(argc, argv);
        
        std::cout << "main: Выполнение startApp завершено." << std::endl; 
    } catch (const std::exception& e) {
        // Ловим любые стандартные исключения, возникшие в процессе работы
        LOG_ERR("SYS", "Исключение: %s", e.what());
        std::cerr << "Критическая ошибка: " << e.what() << std::endl;
        Logger::close();
        return EXIT_FAILURE;
    }

    LOG_INFO("SYS", "Программа завершена корректно.");
    Logger::close();
    return EXIT_SUCCESS;
}
