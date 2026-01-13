#pragma once

#include <cstdint>
#include <vector>

class GameCanvas;

class GameLevel {
private:
  int minX = 0;
  int maxX = 0;
  int segmentStart = 0;
  int segmentEnd = 0;
  int segmentExtra = 0;

public:
  int startPosX;
  int startPosY;
  int finishPosX;
  int startFlagPoint = 0;
  int finishFlagPoint = 0;
  int finishPosY;
  int pointsCount;
  int currentSegment;
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
  int getTrackHeightAt(int x_pos, int current_y = 0);
  void setMinMaxX(int minX, int maxX);
  void setSegmentRange(int start, int end);
  void setSegmentRangeExact(int start, int end, int extra);
  void renderTrackNearestGreenLine(GameCanvas *canvas);
  void addPointSimple(int var1, int var2);
  void addPoint(int x, int y);
  void shiftPoints(int shiftX);
};
