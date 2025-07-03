#include "Micro.h"

#include "GameCanvas.h"
#include "GamePhysics.h"
#include "LevelLoader.h"
#include "Time.h" // Предполагается наличие этой утилиты
#include "CanvasImpl.h" // Предполагается наличие этой утилиты
#include "RecordStore.h" // Предполагается наличие этой утилиты
#include <iostream>
#include <algorithm>

bool Micro::field_249 = false;

Micro::Micro() {}

Micro::~Micro() {
    delete gameCanvas;
    delete levelLoader;
    delete gamePhysics;
}

void Micro::destroyApp(bool var1) {
    (void)var1;
    field_249 = false;
    field_242 = true;
}

void Micro::startApp(int argc, char** argv) {
    (void)argc;
    // Инициализация RecordStore может быть нужна для SDL_GetPrefPath
    // если у вас такая зависимость, если нет - можно удалить.
    RecordStore::setRecordStoreDir(argv[0]);
    field_249 = true;
    run();
}

// Micro.cpp

void Micro::init() {
    // 1. Создаем основные компоненты
    gameCanvas = new GameCanvas(this);
    levelLoader = new LevelLoader();

    // 2. Загружаем данные уровня. Это действие должно быть до создания физики,
    // так как физика зависит от данных уровня.
    levelLoader->loadHardcodedLevel();

    // 3. Теперь, когда данные загружены, создаем физический движок
    gamePhysics = new GamePhysics(levelLoader);

    // 4. Передаем физику в холст для отрисовки
    gameCanvas->init(gamePhysics);

    // 5. Настраиваем параметры физики и графики.
    // Теперь эти вызовы только меняют настройки, не вызывая лишних сбросов.
    gamePhysics->setRenderFlags(0); // линии вместо спрайтов
    LevelLoader::isEnabledPerspective = false;
    LevelLoader::isEnabledShadows = false;
    gamePhysics->setEnableLookAhead(false);
    
    gamePhysics->setMode(1);
    gamePhysics->setMotoLeague(3);

    // 6. Устанавливаем размер холста и готовим его к первому показу
    gameCanvas->updateSizeAndRepaint();

    isInited = true;
}

void Micro::restart(bool var1) {
    gamePhysics->resetSmth(true);
    gameTimeMs = 0;
    if (var1) {
        // Так как текста нет, просто делаем задержку
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

    while (field_249) {
        
        try {


            for (int i = numPhysicsLoops; i > 0; --i) {
                if (field_248) {
                    gameTimeMs += 20L;
                }

                int gameStatus = gamePhysics->updatePhysics();

                if (gameStatus == 3 || gameStatus == 5) {
                    Time::sleep(1500LL);
                    restart(true);      
                    break;
                }
                else if (gameStatus == 1 || gameStatus == 2) {
                    Time::sleep(1500LL);
                    restart(true);
                    break;
                }
                else if (gameStatus == 4) {
                    gameTimeMs = 0L;
                }
                field_248 = gameStatus != 4;
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
    destroyApp(true);
}