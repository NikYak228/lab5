#pragma once

class GameLevel;
class LevelLoader;
class GamePhysics;

/**
 * LevelGenerator
 * 
 * Генератор трасс. Поддерживает как предзаданные участки, 
 * так и бесконечную процедурную генерацию для дзен-режима.
 */
class LevelGenerator {
public:
    LevelGenerator();
    
    // Загрузка уровня (дзен или статический)
    void loadLevel(LevelLoader* loader, int mode, int levelId);
    
    // Обновление дзен-режима (вызывать каждый кадр)
    void updateZen(LevelLoader* loader, int cameraX_LogicUnit);
    
    // Сдвиг координат генерации (для бесконечного мира)
    void shiftGeneration(int shiftX);

private:
    int lastX; // Координата X конца последнего сегмента
    int lastY; // Координата Y конца последнего сегмента
    
    // Параметры для физически-корректной генерации (прыжки и т.д.)
    double currentVelocity;
    double currentAngle;
    const double GRAVITY = 25.0;

    // Примитивы генерации
    void generateZenChunk(GameLevel* level);      // Процедурный кусок
    void generateStaticHills(GameLevel* level);   // Фиксированные холмы
    void addFlat(GameLevel* level, int len);      // Прямой участок
    void addSlope(GameLevel* level, int len, int dy); // Склон (линейный)
    void addCosineInterpolation(GameLevel* level, int len, int dy); // Плавный холм
    
    // Специальные препятствия
    void addBallisticJump(GameLevel* level, double vx, double vy); // Прыжок по баллистической траектории
    void addCliff(GameLevel* level, int height); // Обрыв
    void addWhoops(GameLevel* level);            // Серия мелких кочек
};
