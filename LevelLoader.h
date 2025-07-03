#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include "GamePhysics.h"
#include "GameCanvas.h"
#include "GameLevel.h"
#include "TimerOrMotoPartOrMenuElem.h"

class LevelLoader {
private:
    std::vector<std::vector<int>> field_121;
    int field_123[3];
    int field_124[3];
    int field_132 = 0;
    static int field_133;
    static int field_134;
    static int field_135;
    static int field_136;

public:
    static bool isEnabledPerspective;
    static bool isEnabledShadows;
    GameLevel* gameLevel = nullptr;
    int field_131;
    int field_137;
    int field_138;

    LevelLoader();
    ~LevelLoader();
    
    void loadHardcodedLevel();
    
    int getFinishFlagX();
    int getStartFlagX();
    int getStartPosX();
    int getStartPosY();
    int getProgressAt(int x);
    void prepareLevelData(GameLevel* gameLevel);
    void setMinMaxX(int minX, int maxX);
    void renderTrackNearestLine(GameCanvas* canvas);
    void renderLevel3D(GameCanvas* gameCanvas, int xF16, int yF16);
    void updateVisibleRange(int x1, int x2, int y);
    int detectCollision(TimerOrMotoPartOrMenuElem* part, int wheelType);
};