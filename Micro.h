#pragma once
#include <cstdint>
#include <memory>

class GameCanvas;
class GamePhysics;
class LevelLoader;

class Micro {
private:
    void destroyApp(bool var1);

public:
    std::unique_ptr<GameCanvas> gameCanvas;
    std::unique_ptr<LevelLoader> levelLoader;
    std::unique_ptr<GamePhysics> gamePhysics;

    bool isRunning = false;
    int numPhysicsLoops = 4;
    int64_t gameTimeMs = 0;
    bool isInited = false;
    bool shouldStop = false;
    static bool isReady;

    Micro();
    ~Micro();

    void startApp(int argc, char** argv);
    void init();
    void restart(bool var1);
    void run();
};
