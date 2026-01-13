#include "Micro.h"

#include "GameCanvas.h"
#include "GamePhysics.h"
#include "LevelLoader.h"
#include "AppTime.h" // Предполагается наличие этой утилиты
#include "CanvasImpl.h" // Предполагается наличие этой утилиты
#include "Logger.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <ctime>
#include <cstdlib>

bool Micro::isReady = false;

Micro::Micro() = default;

Micro::~Micro() = default;

void Micro::destroyApp(bool var1) {
    (void)var1;
    isReady = false;
    isRunning = true;
}

void Micro::startApp(int argc, char** argv) {
    (void)argc;
    (void)argv;
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    isReady = true;
    run();
}

void Micro::init() {
    // 1. Создаем основные компоненты
    gameCanvas = std::make_unique<GameCanvas>(this);
    levelLoader = std::make_unique<LevelLoader>();

    // 2. Загружаем данные уровня (будет перезаписано в дзен-режиме)
    levelLoader->loadHardcodedLevel();

    // 3. создаем физический движок
    gamePhysics = std::make_unique<GamePhysics>(levelLoader.get());

    // ⭐ ВКЛЮЧЕНИЕ ДЗЕН-РЕЖИМА (бесконечная генерация + AI управление)
    gamePhysics->setLevelMode(LevelMode::ZEN_ENDLESS, 0);

    // 4. Передаем физику в холст для отрисовки
    gameCanvas->init(gamePhysics.get());

    // 5. параметры физики и графики.
    gamePhysics->setRenderFlags(0); 
    LevelLoader::isEnabledPerspective = false;
    LevelLoader::isEnabledShadows = false;
    gamePhysics->setEnableLookAhead(false);
    
    gamePhysics->setMode(1);
    // Use League 3 (Demonstration Mode) for moderate speed and stability
    gamePhysics->setMotoLeague(3);

    // 6. Устанавливаем размер холста и готовим его к первому показу
    gameCanvas->updateSizeAndRepaint();

    isInited = true;
}

void Micro::restart(bool var1) {
    gamePhysics->resetSmth(true);
    gameTimeMs = 0;
    if (var1) {
         Time::sleep(1500LL);
    }
    gameCanvas->resetInput();
}

void Micro::run() {
    if (!isInited) {
        init();
    }
    restart(true);

    int64_t lastFrameTime = 0L;

    while (isReady) {
        
        try {


            for (int i = numPhysicsLoops; i > 0; --i) {
                if (shouldStop) {
                    gameTimeMs += 20L;
                }

                int gameStatus = gamePhysics->updatePhysics();

                if (gameStatus == 3 || gameStatus == 5) {
                    LOG_INFO("SYS", "Game Status %d (Crash/Fail). Quitting as requested...", gameStatus);
                    Time::sleep(500LL);
                    isReady = false; // Quit the app
                    break;
                }
                else if (gameStatus == 1 || gameStatus == 2) {
                    LOG_INFO("SYS", "Game Status %d (Win/Next). Restarting...", gameStatus);
                    Time::sleep(1500LL);
                    restart(true);
                    break;
                }
                else if (gameStatus == 4) {
                    LOG_INFO("SYS", "Game Status 4 (Track End). Restarting...");
                    // Level finished or track end reached
                    Time::sleep(1000LL);
                    restart(true); // Restart the level
                    break; 
                }
                // shouldStop logic removed/simplified as we want endless play
                shouldStop = false; 
            }

            gamePhysics->snapshotMotoState();

            int64_t currentTime = Time::currentTimeMillis();
            if (currentTime - lastFrameTime < 30L) {
                Time::sleep(std::max(30LL - (currentTime - lastFrameTime), 1LL));
            }
            lastFrameTime = currentTime;
            
            // Рисуем кадр
            gameCanvas->repaint();
            gameCanvas->handleEventsAndPresent();

        } catch (const std::exception& e) {
            std::cerr << "Error in game loop: " << e.what() << std::endl;
            continue;
        }
    }
    LOG_INFO("SYS", "Micro::run loop exited. isReady=%d", isReady);
    destroyApp(true);
}
