#pragma once

#include <cstdint>
#include <vector>

class GameCanvas;

class GameLevel {
private:
    int minX = 0;
    int maxX = 0;
    int field_264 = 0;
    int field_265 = 0;
    int field_266 = 0;
    int field_277 = 0;

public:
    int startPosX;
    int startPosY;
    int finishPosX;
    int startFlagPoint = 0;
    int finishFlagPoint = 0;
    int finishPosY;
    int pointsCount;
    int field_274;
    std::vector<std::vector<int>> pointPositions;

    GameLevel();
    ~GameLevel();
    void init();
    void setStartFinish(int startX, int startY, int finishX, int finishY);
    int getStartPosX();
    int getStartPosY();
    int getFinishPosX();
    int getFinishPosY();
    int getPointX(int pointNo);
    int getPointY(int pointNo);
    int computeProgress(int x);
    int getTrackHeightAt(int x_pos);
    void setMinMaxX(int minX, int maxX);
    void setSegmentRange(int start, int end);
    void setSegmentRangeExact(int start, int end, int extra);
    void renderShadow(GameCanvas* gameCanvas, int var2, int var3);
    void renderLevel3D(GameCanvas* gameCanvas, int xF16, int yF16);
    void renderTrackNearestGreenLine(GameCanvas* canvas);
    void addPointSimple(int var1, int var2);
    void addPoint(int x, int y);
};