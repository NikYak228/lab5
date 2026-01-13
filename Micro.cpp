#include "Micro.h"

#include "GameCanvas.h"
#include "GamePhysics.h"
#include "LevelLoader.h"
#include "AppTime.h"
#include "CanvasImpl.h"
#include "Logger.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <ctime>
#include <cstdlib>

bool Micro::isReady = false;

Micro::Micro() = default;

Micro::~Micro() = default;

void Micro::destroyApp(bool saveState) {
    (void)saveState;
    isReady = false;
    isRunning = true; // Видимо, флаг того, что приложение завершилось? (Legacy logic)
}

void Micro::startApp(int argc, char** argv) {
    (void)argc;
    (void)argv;
    // Инициализация генератора случайных чисел
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    isReady = true;
    run();
}

void Micro::init() {
    // 1. Создаем основные компоненты
    gameCanvas = std::make_unique<GameCanvas>(this);
    levelLoader = std::make_unique<LevelLoader>();

    // 2. Загружаем данные уровня
    levelLoader->loadHardcodedLevel();

    // 3. Создаем физический движок
    gamePhysics = std::make_unique<GamePhysics>(levelLoader.get());

    // ⭐ ВКЛЮЧЕНИЕ ДЗЕН-РЕЖИМА (бесконечная генерация + AI управление)
    // LevelMode::ZEN_ENDLESS = 1
    gamePhysics->setLevelMode(LevelMode::ZEN_ENDLESS, 0);

    // 4. Передаем физику в холст для отрисовки
    gameCanvas->init(gamePhysics.get());

    // 5. Настройка параметров физики и графики
    gamePhysics->setRenderFlags(0); 
    LevelLoader::isEnabledPerspective = false;
    LevelLoader::isEnabledShadows = false;
    gamePhysics->setEnableLookAhead(false);
    
    // Режим физики (1 = стандартный)
    gamePhysics->setMode(1);
    
    // Выбор лиги (настроек байка). 
    // League 3 (Demonstration Mode) - умеренная скорость и стабильность
    gamePhysics->setMotoLeague(3);

    // 6. Устанавливаем размер холста и готовим его к первому показу
    gameCanvas->updateSizeAndRepaint();

    isInited = true;
}

void Micro::restart(bool fullReset) {
    gamePhysics->resetSmth(true);
    gameTimeMs = 0;
    if (fullReset) {
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

    // --- ГЛАВНЫЙ ЦИКЛ ---
    while (isReady) {
        try {
            // 1. Физический цикл (несколько подшагов для стабильности)
            for (int i = numPhysicsLoops; i > 0; --i) {
                if (shouldStop) {
                    gameTimeMs += 20L;
                }

                // Обновление физики
                int gameStatus = gamePhysics->updatePhysics();

                // Обработка статусов игры
                if (gameStatus == 3 || gameStatus == 5) {
                    LOG_INFO("SYS", "Статус игры %d (Авария/Ошибка). Выход...", gameStatus);
                    Time::sleep(500LL);
                    isReady = false; // Завершаем приложение (для AI теста)
                    break;
                }
                else if (gameStatus == 1 || gameStatus == 2) {
                    LOG_INFO("SYS", "Статус игры %d (Победа/След. уровень). Рестарт...", gameStatus);
                    Time::sleep(1500LL);
                    restart(true);
                    break;
                }
                else if (gameStatus == 4) {
                    LOG_INFO("SYS", "Статус игры 4 (Конец трассы). Рестарт...");
                    Time::sleep(1000LL);
                    restart(true); 
                    break; 
                }
                
                shouldStop = false; 
            }

            // 2. Сохранение состояния (снапшот для интерполяции)
            gamePhysics->snapshotMotoState();

            // 3. Ограничение FPS (30ms = ~33 FPS)
            int64_t currentTime = Time::currentTimeMillis();
            if (currentTime - lastFrameTime < 30L) {
                Time::sleep(std::max(30LL - (currentTime - lastFrameTime), 1LL));
            }
            lastFrameTime = currentTime;
            
            // 4. Отрисовка кадра
            gameCanvas->repaint();
            gameCanvas->handleEventsAndPresent();

        } catch (const std::exception& e) {
            std::cerr << "Ошибка в игровом цикле: " << e.what() << std::endl;
            continue;
        }
    }
    
    LOG_INFO("SYS", "Цикл Micro::run завершен. isReady=%d", isReady);
    destroyApp(true);
}