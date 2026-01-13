#ifndef LEVELGENERATOR_H
#define LEVELGENERATOR_H

class GameLevel;
class LevelLoader;
class GamePhysics;

class LevelGenerator {
public:
    LevelGenerator();
    
    // Загрузка уровня (дзен или статический)
    void loadLevel(LevelLoader* loader, int mode, int levelId);
    
    // Обновление дзен-режима (вызывать каждый кадр)
    void updateZen(LevelLoader* loader, int cameraX_LogicUnit);
    
    void shiftGeneration(int shiftX);

private:
    int lastX;
    int lastY;
    
    // Physics-aware generation
    double currentVelocity;
    double currentAngle;
    const double GRAVITY = 25.0;

    // Примитивы генерации
    void generateZenChunk(GameLevel* level);
    void generateStaticHills(GameLevel* level);
    void addFlat(GameLevel* level, int len);
    void addSlope(GameLevel* level, int len, int dy);
    void addCosineInterpolation(GameLevel* level, int len, int dy);
    
    // Special
    void addBallisticJump(GameLevel* level, double vx, double vy);
    void addCliff(GameLevel* level, int height);
    void addWhoops(GameLevel* level);
};

#endif