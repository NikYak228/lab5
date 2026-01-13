#pragma once
#include <cstdint>
#include <memory>

class GameCanvas;
class GamePhysics;
class LevelLoader;

/**
 * Micro
 * 
 * Главный класс приложения. 
 * Управляет жизненным циклом (Init, Run, Destroy) и основным игровым циклом.
 */
class Micro {
private:
    void destroyApp(bool saveState); // Завершение работы

public:
    std::unique_ptr<GameCanvas> gameCanvas;
    std::unique_ptr<LevelLoader> levelLoader;
    std::unique_ptr<GamePhysics> gamePhysics;

    bool isRunning = false;       // Флаг работы (паузы?) - странное название в оригинале
    int numPhysicsLoops = 4;      // Количество шагов физики за кадр (sub-stepping)
    int64_t gameTimeMs = 0;
    bool isInited = false;
    bool shouldStop = false;
    static bool isReady;          // Глобальный флаг готовности/работы

    Micro();
    ~Micro();

    void startApp(int argc, char** argv);
    void init();
    void restart(bool fullReset);
    void run(); // Главный цикл
};