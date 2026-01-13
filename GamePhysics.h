#pragma once

#include <vector>
#include <memory>
#include "TimerOrMotoPartOrMenuElem.h"

class LevelLoader;
class BikePart;
class GameCanvas;
class AIController;
class LevelGenerator;

// Режимы игры для AI
enum class LevelMode {
    MANUAL = 0,         // Обычная игра
    ZEN_ENDLESS = 1,    // Бесконечный AI
    STATIC_LEVEL = 2    // Прохождение уровней AI
};

class GamePhysics {
    // Разрешаем контроллерам доступ к внутренностям (motoComponents)
    friend class AIController;
    friend class LevelGenerator;
private:
    int primaryStateIndex = 0;
    int secondaryStateIndex = 1;
    int activePartIndex= -1;
    std::vector<std::unique_ptr<MotoComponent>> springComponents;

    int engineTorqueF16 = 0;
    LevelLoader* levelLoader;
    int collisionNormalX = 0;
    int collisionNormalY = 0;
    bool isCrashed = false;
    bool isGroundColliding = false;
    int tiltAngleF16 = 32768;
    int wheelBalance = 0;
    bool cameraToggle = false;
    std::vector<std::unique_ptr<MotoComponent>> motoComponents = std::vector<std::unique_ptr<MotoComponent>>(6);
    int restartCounter;
    bool isInputAcceleration;
    bool isInputBreak;
    bool isInputBack;
    bool isInputForward;
    bool isInputUp;
    bool isInputDown;
    bool isInputLeft;
    bool isInputRight;
    bool inAir;
    bool isEnableLookAhead;
    int camShiftX;
    int camShiftY;
    int cameraShiftLimit;
    const std::vector<std::vector<int>> hardcodedArr1 = { { 183500, -52428 }, { 262144, -163840 }, { 406323, -65536 }, { 445644, -39321 }, { 235929, 39321 }, { 16384, -144179 }, { 13107, -78643 }, { 288358, 81920 } };
    const std::vector<std::vector<int>> hardcodedArr2 = { { 190054, -111411 }, { 308019, -235929 }, { 334233, -114688 }, { 393216, -58982 }, { 262144, 98304 }, { 65536, -124518 }, { 13107, -78643 }, { 288358, 81920 } };
    const std::vector<std::vector<int>> hardcodedArr3 = { { 157286, 13107 }, { 294912, -13107 }, { 367001, 91750 }, { 406323, 190054 }, { 347340, 72089 }, { 39321, -98304 }, { 13107, -52428 }, { 294912, 81920 } };
    const std::vector<std::vector<int>> hardcodedArr4 = { { 183500, -39321 }, { 262144, -131072 }, { 393216, -65536 }, { 458752, -39321 }, { 294912, 6553 }, { 16384, -144179 }, { 13107, -78643 }, { 288358, 85196 } };
    const std::vector<std::vector<int>> hardcodedArr5 = { { 190054, -91750 }, { 255590, -235929 }, { 334233, -114688 }, { 393216, -42598 }, { 301465, 6553 }, { 65536, -78643 }, { 13107, -78643 }, { 288358, 85196 } };
    const std::vector<std::vector<int>> hardcodedArr6 = { { 157286, 13107 }, { 294912, -13107 }, { 367001, 104857 }, { 406323, 176947 }, { 347340, 72089 }, { 39321, -98304 }, { 13107, -52428 }, { 288358, 85196 } };
    std::vector<std::vector<int>> cameraParams;

    void setupBike(int startX, int startY);
    void setInputFromAI();
    void updateBikePhysics();
    int runPhysicsLoop(int maxStep);
    void applyForces(int var1);
    void applySpringConstraint(BikePart* partA, MotoComponent* spring, BikePart* partB, int componentIndex, int stiffnessF16);
    void blendComponentState(int dst, int src, int scale);
    void combineComponentState(int dst, int src, int add);
    void updateComponents(int delta);
    void enforceMinimumWheelBase(int stateIndex);
    int updateLevelCollision(int componentIndex);
    void updateWheelPhysics(int componentIndex);
    void renderMotoFork(GameCanvas* canvas);
    void renderRiderSkeleton(GameCanvas* gameCanvas, int dirXF16, int dirYF16, int rotXF16, int rotYF16);
    void renderBikeWireframe(GameCanvas* gameCanvas, int dirXF16, int dirYF16, int rotXF16, int rotYF16);

public:
    inline static int wheelBase;
    inline static int gravityF16;
    inline static int suspensionFront;
    inline static int suspensionBack;
    inline static int forkLength;
    inline static int motoParam1;
    inline static int motoParam2;
    inline static int massFactor;
    inline static int motoParam10;
    inline static int dampingF16;
    inline static std::vector<int> const175_1_half = { 114688, 65536, 32768 };
    inline static int motoParam3;
    inline static int frictionCoeff;
    inline static int motoParam4;
    inline static int motoParam5;
    inline static int motoParam6;
    inline static int motoParam7;
    inline static int motoParam8;
    inline static int motoParam9;
    int initialWheelSeparationF16 = 0;
    std::vector<std::unique_ptr<BikePart>> motorcycleParts;
    bool atStart = false;
    int gameMode;
    bool useSpriteGraphics;
    bool isRenderMotoWithSprites;
    inline static int curentMotoLeague = 0;
    bool trackStarted;
    bool isGenerateInputAI = false;
    
    // AI система
    LevelMode currentLevelMode;
    std::unique_ptr<AIController> aiController;
    std::unique_ptr<LevelGenerator> levelGenerator;
    int aiRestartTimer;
    
    ~GamePhysics();
    GamePhysics(LevelLoader* levelLoader);
    int getRenderMode();
    void setRenderFlags(int flags);
    void setMode(int mode);
    void setMotoLeague(int league);
    void resetSmth(bool unused);
    void shiftBikeVertical(bool up);
    void setRenderMinMaxX(int minX, int maxX);
    void processPointerReleased();
    void applyUserInput(int xDir, int yDir);
    void enableGenerateInputAI();
    void disableGenerateInputAI();
    
    // Основной метод переключения режимов
    // mode: ZEN_ENDLESS или STATIC_LEVEL
    // levelId: 0 = Петля, 1 = Холмы (для статики)
    void setLevelMode(LevelMode mode, int levelId = 0);
    
    int updatePhysics();
    bool isTrackStarted();
    bool isTrackFinished();
    static int calcVectorLengthF16(int xF16, int yF16);
    void setEnableLookAhead(bool value);
    void setMinimalScreenWH(int minWH);
    int getCamPosX();
    int getCamPosY();
    int getGroundHeight();
    void snapshotMotoState();
    void renderGame(GameCanvas* gameCanvas);
    void enforceGroundCollision();
    void setMotoComponents();
    void checkAndShiftWorld();
};
