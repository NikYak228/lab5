#pragma once

#include <cstdint>
#include <string>
#include <vector>


#include "GameCanvas.h"
#include "GameLevel.h"
#include "GamePhysics.h"
#include "TimerOrMotoPartOrMenuElem.h"


class LevelLoader {
private:
  std::vector<std::vector<int>> segmentNormals;
  int wheelRadiusSq[3];
  int wheelInnerRadiusSq[3];
  int segmentCapacity = 0;
  static int visibleStart;
  static int visibleEnd;

public:
  static int visibleStartX;
  static int visibleEndX;

private:
public:
  static bool isEnabledPerspective;
  static bool isEnabledShadows;
  GameLevel *gameLevel = nullptr;
  int maxTrackX;
  bool loopApexPassed = false;
  int collisionNormalX;
  int collisionNormalY;
  int maxPassedSegmentX = 0; // Максимальная X координата пройденного сегмента

  LevelLoader();
  ~LevelLoader();

  void loadHardcodedLevel();

  int getFinishFlagX();
  int getStartFlagX();
  int getStartPosX();
  int getStartPosY();
  int getProgressAt(int x);
  void prepareLevelData(GameLevel *gameLevel);
  
  // Метод для частичного обновления физики (нормалей) при генерации новых участков
  // startPointIndex - индекс точки, с которой начали добавление
  void appendLevelData(int startPointIndex);
  
  void setMinMaxX(int minX, int maxX);
  void renderTrackNearestLine(GameCanvas *canvas);
  void updateVisibleRange(int x1, int x2, int y);
  int detectCollision(MotoComponent *part, int wheelType);
  void shiftLevel(int shiftX);
};
