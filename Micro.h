#pragma once
#include <cstdint>

class GameCanvas;
class GamePhysics;
class LevelLoader;

class Micro {
private:
    void destroyApp(bool var1);

public:
    GameCanvas* gameCanvas = nullptr;
    LevelLoader* levelLoader = nullptr;
    GamePhysics* gamePhysics = nullptr;

    bool field_242 = false;
    int numPhysicsLoops = 4;
    int64_t gameTimeMs = 0;
    bool isInited = false;
    bool field_248 = false;
    static bool field_249;

    Micro();
    ~Micro();

    void startApp(int argc, char** argv);
    void init();
    void restart(bool var1);
    void run();
};