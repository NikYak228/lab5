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
    
    int method_91();
    int method_92();
    int method_93();
    int method_94();
    int method_95(int var1);
    void method_96(GameLevel* gameLevel);
    void setMinMaxX(int minX, int maxX);
    void renderTrackNearestLine(GameCanvas* canvas);
    void renderLevel3D(GameCanvas* gameCanvas, int xF16, int yF16);
    void method_100(int var1, int var2, int var3);
    int method_101(TimerOrMotoPartOrMenuElem* var1, int var2);
};