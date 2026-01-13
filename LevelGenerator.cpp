#include "LevelGenerator.h"
#include "GameLevel.h"
#include "LevelLoader.h"
#include "Logger.h"
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Режимы игры
const int MODE_MANUAL = 0;
const int MODE_ZEN_ENDLESS = 1;
const int MODE_STATIC_LEVEL = 2;

LevelGenerator::LevelGenerator() : lastX(0), lastY(220), currentVelocity(40.0), currentAngle(0.0) {}

void LevelGenerator::loadLevel(LevelLoader* loader, int mode, int levelId) {
    if (!loader || !loader->gameLevel) return;
    
    GameLevel* level = loader->gameLevel;
    level->init(); 
    
    lastX = 0; 
    lastY = 220; 
    currentVelocity = 40.0;
    currentAngle = 0.0;

    if (mode == MODE_ZEN_ENDLESS) {
        // Для дзен-режима ставим финиш очень далеко
        level->setStartFinish(0, 220, 2000000000, 220); 
        lastX = -200; lastY = 220;
        level->addPointSimple(-200, 220);
        addFlat(level, 200);
        addFlat(level, 500); 
        
        // Начальная генерация нескольких кусков
        for (int i = 0; i < 3; i++) generateZenChunk(level);
        loader->prepareLevelData(level);
    } 
    else if (mode == MODE_STATIC_LEVEL) {
        if (levelId == 0) loader->loadHardcodedLevel();
        else if (levelId == 1) {
            generateStaticHills(level);
            level->setStartFinish(0, 220, lastX, 220); 
            loader->prepareLevelData(level);
        }
    }
}

void LevelGenerator::updateZen(LevelLoader* loader, int cameraX_Logic) {
    if (!loader || !loader->gameLevel) return;
    // Если до края сгенерированного участка меньше 4000 единиц - генерируем еще
    if (lastX - cameraX_Logic < 4000) { 
        int oldPointsCount = loader->gameLevel->pointsCount;
        generateZenChunk(loader->gameLevel);
        // Обновляем физику (нормали) только для новых точек
        loader->appendLevelData(oldPointsCount); 
    }
}

// === ПРИМИТИВЫ ГЕНЕРАЦИИ ===

void LevelGenerator::addFlat(GameLevel* level, int len) {
    currentAngle = 0.0;
    lastX += len;
    level->addPointSimple(lastX, lastY);
}

void LevelGenerator::addSlope(GameLevel* level, int len, int dy) {
    // Ограничение крутизны (0.8 ~ 40 градусов)
    int maxDy = (int)(len * 0.8);
    if (dy > maxDy) dy = maxDy;
    if (dy < -maxDy) dy = -maxDy;

    int steps = len / 20; 
    if (steps < 2) steps = 2;
    int startY = lastY; 
    int startX = lastX;
    
    currentAngle = std::atan2((double)dy, (double)len);
    // Оценка изменения скорости (энергия)
    double v2 = currentVelocity * currentVelocity + 2.0 * GRAVITY * dy;
    if (v2 < 100.0) v2 = 100.0; 
    if (v2 > 15000.0) v2 = 15000.0;
    currentVelocity = std::sqrt(v2);

    for(int i = 1; i <= steps; ++i) {
        double t = (double)i / steps;
        lastX = startX + (int)(len * t);
        lastY = startY + (int)(dy * t);
        level->addPointSimple(lastX, lastY);
    }
}

void LevelGenerator::addCosineInterpolation(GameLevel* level, int len, int dy) {
    int maxDy = (int)(len * 0.8);
    if (dy > maxDy) dy = maxDy;
    if (dy < -maxDy) dy = -maxDy;

    int steps = len / 20; 
    if (steps < 2) steps = 2;
    int startY = lastY; 
    int startX = lastX;
    
    double v2 = currentVelocity * currentVelocity + 2.0 * GRAVITY * dy;
    if (v2 < 100.0) v2 = 100.0; 
    if (v2 > 15000.0) v2 = 15000.0;
    currentVelocity = std::sqrt(v2);
    currentAngle = 0.0; 

    for(int i = 1; i <= steps; ++i) {
        double t = (double)i / steps;
        // Косинусная интерполяция для гладкости
        double ft = (1.0 - std::cos(t * M_PI)) * 0.5;
        lastX = startX + (int)((double)len * t);
        lastY = startY + (int)(dy * ft);
        level->addPointSimple(lastX, lastY);
    }
}

// === СПЕЦИАЛЬНЫЕ УЧАСТКИ ===

void LevelGenerator::addBallisticJump(GameLevel* level, double vx, double vy) {
    double t = 15.0 + (rand() % 20); // Время полета
    double dx = vx * t;
    double dy = vy * t + 0.5 * GRAVITY * t * t;
    
    int landingX = lastX + (int)dx;
    int landingY = lastY + (int)dy;
    int jumpDist = landingX - lastX; 

    // Расчет глубины ямы под прыжком
    int maxPossibleDepth = (int)(jumpDist / 2.5); 
    int desiredDepth = 300; 
    int pitDepth = std::min(desiredDepth, maxPossibleDepth);
    if (pitDepth < 50) pitDepth = 0;

    // Генерируем "вход" в яму
    int entryLen = std::max((int)(pitDepth * 2.0), 100); 
    addSlope(level, entryLen, pitDepth);
    
    // Генерируем "выход" из ямы к точке приземления
    int exitLen = std::max((int)(pitDepth * 2.0), 100);
    int dyExit = landingY - lastY; 
    addSlope(level, exitLen, dyExit); 
    
    LOG_GEN("JUMP: Скорость=%.1f Дистанция=%d Глубина=%d", currentVelocity, jumpDist, pitDepth);
    
    // Угол приземления
    double vy_land = vy + GRAVITY * t;
    double vx_land = vx;
    double landingAngle = std::atan2(vy_land, vx_land);
    if (landingAngle > 0.8) landingAngle = 0.8;
    
    currentAngle = landingAngle;
    double v2 = currentVelocity * currentVelocity + 2.0 * GRAVITY * dy;
    if (v2 < 100.0) v2 = 100.0;
    currentVelocity = std::sqrt(v2);
    
    double runoutLen = 200.0;
    addSlope(level, (int)runoutLen, (int)(runoutLen * std::tan(landingAngle))); 
    addFlat(level, 100);
}

void LevelGenerator::addCliff(GameLevel* level, int height) {
    // Обрыв (резкий спуск)
    int len = (int)(height / 0.8) + 10;
    addSlope(level, len, height);
    addFlat(level, 100); // Площадка после обрыва
}

void LevelGenerator::addWhoops(GameLevel* level) {
    // Серия кочек ("волны")
    int count = 5 + rand() % 5;
    for(int i=0; i<count; i++) {
        int h = 40 + rand() % 30;
        int w = 80 + rand() % 40;
        addCosineInterpolation(level, w, -h); 
        addCosineInterpolation(level, w, h);  
    }
}

void LevelGenerator::generateZenChunk(GameLevel* level) {
    // Контроль высоты (чтобы не уйти за границы мира)
    bool tooHigh = (lastY < 0); 
    bool tooLow = (lastY > 1500);  

    int r = rand() % 100;
    
    if (tooHigh) addCosineInterpolation(level, 600, 300); // Спускаемся
    else if (tooLow) addCosineInterpolation(level, 600, -300); // Поднимаемся
    else {
        if (currentVelocity > 50.0 && r < 20) {
            // Трамплин
            addSlope(level, 200, -150); 
            double vx = currentVelocity * std::cos(currentAngle);
            double vy = currentVelocity * std::sin(currentAngle);
            addBallisticJump(level, vx, vy);
        }
        else if (r < 30) {
            // Кочки
            addWhoops(level);
        }
        else if (r < 40) {
            // Обрыв
            addCliff(level, 200 + rand() % 200);
        }
        else if (r < 70) {
            // Большой холм
            int h = 150 + rand() % 150;
            int len = 400 + rand() % 200;
            if (len < h * 2.5) len = (int)(h * 2.5);
            addCosineInterpolation(level, len, -h); 
            addFlat(level, 50); 
            addCosineInterpolation(level, len, h);  
        } else {
            // Низина (впадина)
            int h = 100 + rand() % 100;
            int len = 400 + rand() % 200;
            addCosineInterpolation(level, len, h); 
            addFlat(level, 50); 
            addCosineInterpolation(level, len, -h); 
        }
    }
}

void LevelGenerator::generateStaticHills(GameLevel* level) {
    addFlat(level, 500);
    for(int i=0; i<10; i++) {
        addCosineInterpolation(level, 400, 150);
        addCosineInterpolation(level, 400, -150);
    }
    addFlat(level, 1000);
}

void LevelGenerator::shiftGeneration(int shiftX) {
    lastX -= shiftX;
}