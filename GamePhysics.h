#pragma once

#include <vector>
#include <memory>
#include "TimerOrMotoPartOrMenuElem.h"

class LevelLoader;
class BikePart;
class GameCanvas;
class AIController;
class LevelGenerator;

// Режимы работы генератора уровней и AI
enum class LevelMode {
    MANUAL = 0,         // Ручное управление, обычная трасса
    ZEN_ENDLESS = 1,    // Бесконечная процедурная генерация + AI
    STATIC_LEVEL = 2    // Прохождение предзаданных уровней с AI
};

/**
 * GamePhysics
 * 
 * Основной класс физического движка. 
 * Отвечает за симуляцию мотоцикла, обработку коллизий, 
 * управление состоянием (положение, скорость, вращение) и отрисовку игрового мира.
 */
class GamePhysics {
    // Дружественные классы для доступа к внутренним компонентам байка
    friend class AIController;
    friend class LevelGenerator;

private:
    // --- Внутренние индексы состояния физики (двойная буферизация) ---
    int primaryStateIndex = 0;
    int secondaryStateIndex = 1;
    
    // --- Компоненты симуляции ---
    int activePartIndex = -1; // Индекс части байка, которая сейчас обрабатывается
    std::vector<std::unique_ptr<MotoComponent>> springComponents; // Пружины и связи
    // Вектор компонентов для камеры и других целей (копия состояния?)
    std::vector<std::unique_ptr<MotoComponent>> motoComponents = std::vector<std::unique_ptr<MotoComponent>>(6);

    // --- Текущее состояние физики ---
    int engineTorqueF16 = 0;      // Текущий крутящий момент двигателя (F16)
    int collisionNormalX = 0;     // Нормаль последней коллизии X
    int collisionNormalY = 0;     // Нормаль последней коллизии Y
    bool isCrashed = false;       // Флаг аварии
    bool isGroundColliding = false; // Флаг касания земли
    int tiltAngleF16 = 32768;     // Угол наклона водителя (для анимации)
    int wheelBalance = 0;         // Баланс веса на колесах
    
    // --- Ссылки на внешние системы ---
    LevelLoader* levelLoader;

    // --- Камера ---
    bool isEnableLookAhead;       // Включено ли опережение камеры
    int camShiftX;                // Смещение камеры X
    int camShiftY;                // Смещение камеры Y
    int cameraShiftLimit;         // Лимит смещения
    bool cameraToggle = false;    // Флаг переключения камеры (legacy?)
    std::vector<std::vector<int>> cameraParams; // Параметры камеры

    // --- Управление вводом (Флаги) ---
    bool isInputAcceleration;
    bool isInputBreak;
    bool isInputBack;
    bool isInputForward;
    bool isInputUp;
    bool isInputDown;
    bool isInputLeft;
    bool isInputRight;
    bool inAir;                   // Находится ли байк в воздухе
    
    // --- Данные для отрисовки анимации (скелет водителя) ---
    // Хардкодные массивы координат для интерполяции поз водителя
    const std::vector<std::vector<int>> hardcodedArr1 = { { 183500, -52428 }, { 262144, -163840 }, { 406323, -65536 }, { 445644, -39321 }, { 235929, 39321 }, { 16384, -144179 }, { 13107, -78643 }, { 288358, 81920 } };
    const std::vector<std::vector<int>> hardcodedArr2 = { { 190054, -111411 }, { 308019, -235929 }, { 334233, -114688 }, { 393216, -58982 }, { 262144, 98304 }, { 65536, -124518 }, { 13107, -78643 }, { 288358, 81920 } };
    const std::vector<std::vector<int>> hardcodedArr3 = { { 157286, 13107 }, { 294912, -13107 }, { 367001, 91750 }, { 406323, 190054 }, { 347340, 72089 }, { 39321, -98304 }, { 13107, -52428 }, { 294912, 81920 } };
    const std::vector<std::vector<int>> hardcodedArr4 = { { 183500, -39321 }, { 262144, -131072 }, { 393216, -65536 }, { 458752, -39321 }, { 294912, 6553 }, { 16384, -144179 }, { 13107, -78643 }, { 288358, 85196 } };
    const std::vector<std::vector<int>> hardcodedArr5 = { { 190054, -91750 }, { 255590, -235929 }, { 334233, -114688 }, { 393216, -42598 }, { 301465, 6553 }, { 65536, -78643 }, { 13107, -78643 }, { 288358, 85196 } };
    const std::vector<std::vector<int>> hardcodedArr6 = { { 157286, 13107 }, { 294912, -13107 }, { 367001, 104857 }, { 406323, 176947 }, { 347340, 72089 }, { 39321, -98304 }, { 13107, -52428 }, { 288358, 85196 } };
    
    // --- Приватные методы физики ---
    void setupBike(int startX, int startY); // Настройка начального положения байка
    void setInputFromAI();                  // Получение ввода от AI
    void updateBikePhysics();               // Обновление физики самого байка (двигатель, подвеска)
    int runPhysicsLoop(int maxStep);        // Основной цикл шагов физики (интеграция)
    void applyForces(int componentIndex);   // Применение сил (гравитация, пружины)
    
    // Хелперы для applyForces
    void applyGravity(int componentIndex, bool inLoopZone, int bikeVel);
    void applyInternalConstraints(int componentIndex);
    void applyEngineTorque(int componentIndex);
    void applyAerodynamics(int componentIndex, bool inLoopZone);
    bool checkLoopZone(int componentIndex, int& bikeVel);
    
    // Применение пружинной связи (Constraints)
    void applySpringConstraint(BikePart* partA, MotoComponent* spring, BikePart* partB, int componentIndex, int stiffnessF16);
    
    // Интерполяция и смешивание состояний
    void blendComponentState(int dst, int src, int scale);
    void combineComponentState(int dst, int src, int add);
    void updateComponents(int delta);
    
    // Корректировка базы колес (защита от схлопывания)
    void enforceMinimumWheelBase(int stateIndex);
    
    // Обработка столкновений с уровнем
    int updateLevelCollision(int componentIndex);
    void updateWheelPhysics(int componentIndex);
    
    // Хелперы отрисовки
    void renderMotoFork(GameCanvas* canvas);
    void renderRiderSkeleton(GameCanvas* gameCanvas, int dirXF16, int dirYF16, int rotXF16, int rotYF16);
    void renderBikeWireframe(GameCanvas* gameCanvas, int dirXF16, int dirYF16, int rotXF16, int rotYF16);

public:
    // --- Публичные параметры мотоцикла (Настройки лиги) ---
    inline static int wheelBase;
    inline static int gravityF16;
    inline static int suspensionFront;
    inline static int suspensionBack;
    inline static int forkLength;
    inline static int massFactor;
    inline static int dampingF16;
    inline static int frictionCoeff;
    
    // Параметры, переименованные для ясности (бывшие motoParamX)
    inline static int torqueFront;       // motoParam1
    inline static int torqueRear;        // motoParam2
    inline static int maxAngularVelocity; // motoParam3
    inline static int maxTorque;         // motoParam4
    inline static int accelStep;         // motoParam5
    inline static int brakeDamping;      // motoParam6
    inline static int brakeForce;        // motoParam7
    inline static int airControlForce;   // motoParam8
    inline static int airControlLimit;   // motoParam9
    inline static int springStiffness;   // motoParam10

    // Константы геометрии колес
    inline static std::vector<int> const175_1_half = { 114688, 65536, 32768 };

    // --- Состояние игры ---
    int initialWheelSeparationF16 = 0;
    std::vector<std::unique_ptr<BikePart>> motorcycleParts;
    bool atStart = false;
    int gameMode;
    bool useSpriteGraphics;
    bool isRenderMotoWithSprites;
    inline static int curentMotoLeague = 0;
    bool trackStarted;
    bool isGenerateInputAI = false;
    int restartCounter; // перенесен из private для доступа
    
    // --- AI система ---
    LevelMode currentLevelMode;
    std::unique_ptr<AIController> aiController;
    std::unique_ptr<LevelGenerator> levelGenerator;
    int aiRestartTimer;
    
    // --- Конструктор / Деструктор ---
    GamePhysics(LevelLoader* levelLoader);
    ~GamePhysics();

    // --- Управление настройками ---
    int getRenderMode();
    void setRenderFlags(int flags);
    void setMode(int mode);
    void setMotoLeague(int league);
    void resetSmth(bool fullReset);
    void setLevelMode(LevelMode mode, int levelId = 0);
    
    // --- Управление камерой и миром ---
    void shiftBikeVertical(bool up);
    void setRenderMinMaxX(int minX, int maxX);
    void setEnableLookAhead(bool value);
    void setMinimalScreenWH(int minWH);
    int getCamPosX();
    int getCamPosY();
    void checkAndShiftWorld();
    
    // --- Ввод ---
    void processPointerReleased();
    void applyUserInput(int xDir, int yDir);
    void enableGenerateInputAI();
    void disableGenerateInputAI();
    
    // --- Основной цикл обновления ---
    int updatePhysics();
    
    // --- Запросы состояния ---
    bool isTrackStarted();
    bool isTrackFinished();
    int getGroundHeight();
    
    // --- Хелперы ---
    static int calcVectorLengthF16(int xF16, int yF16);
    void snapshotMotoState();
    void setMotoComponents();
    
    // --- Отрисовка ---
    void renderGame(GameCanvas* gameCanvas);
    void enforceGroundCollision();
};
