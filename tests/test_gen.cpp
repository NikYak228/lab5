#include "LevelGenerator.h"
#include "LevelLoader.h"
#include "GameLevel.h"
#include "Logger.h"
#include <iostream>
#include <vector>
#include <cmath>

/**
 * Тест генератора уровней.
 * Проверяет корректность процедурной генерации в дзен-режиме.
 */
int main() {
    Logger::init("test_gen.log");
    
    LevelLoader loader;
    LevelGenerator generator;
    GameLevel* level = new GameLevel();
    
    // Связываем компоненты
    loader.gameLevel = level;
    
    // 1. Тест бесконечной генерации
    std::cout << "Тестирование генерации в Дзен-режиме..." << std::endl;
    
    // MODE_ZEN_ENDLESS = 1
    generator.loadLevel(&loader, 1, 0); 
    
    std::cout << "Начальное количество точек: " << level->pointsCount << std::endl;
    
    // Симуляция движения вперед
    int cameraX = 0;
    for (int step = 0; step < 100; ++step) {
        cameraX += 1000; // Двигаем камеру
        generator.updateZen(&loader, cameraX); // Генератор должен добавлять точки
    }
    
    int points = level->pointsCount;
    std::cout << "Сгенерировано " << points << " точек после симуляции." << std::endl;
    
    bool wallFound = false;
    
    // Проверка геометрии на наличие разрывов или вертикальных стен
    for (int i = 0; i < points - 1; ++i) {
        int x1 = level->getPointX(i);
        int y1 = level->getPointY(i);
        int x2 = level->getPointX(i+1);
        int y2 = level->getPointY(i+1);
        
        int dx = x2 - x1;
        int dy = y2 - y1;
        
        // Проверка движения назад или строго вертикально
        if (dx <= 0) {
            std::cout << "ОБНАРУЖЕНА СТЕНА (dx<=0) в индексе " << i 
                      << ": (" << x1 << "," << y1 << ") -> (" << x2 << "," << y2 << ")" << std::endl;
            wallFound = true;
        } else {
             double slope = (double)dy / dx;
             // Слишком крутой склон может быть проблемой для AI
             if (std::abs(slope) > 5.0) { 
                 std::cout << "КРУТОЙ СКЛОН в индексе " << i 
                           << ": (" << x1 << "," << y1 << ") -> (" << x2 << "," << y2 << ") Slope: " << slope << std::endl;
             }
        }
    }
    
    if (!wallFound) {
        std::cout << "Вертикальных или обратных стен не обнаружено. Тест пройден." << std::endl;
    }
    
    Logger::close();
    return 0;
}