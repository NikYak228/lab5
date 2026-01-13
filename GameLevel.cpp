#include "GameLevel.h"
#include "GameCanvas.h"
#include "GamePhysics.h"
#include <climits>

GameLevel::GameLevel() { init(); }
GameLevel::~GameLevel() {}

void GameLevel::init() {
  startPosX = 0;
  startPosY = 0;
  finishPosX = 13107200;
  pointsCount = 0;
  currentSegment = 0;
  pointPositions.clear();
}

void GameLevel::setStartFinish(int startX, int startY, int finishX,
                               int finishY) {
  // Конвертация координат: x << 16 >> 3 (x * 8192)
  startPosX = (int)((int64_t)startX << 16 >> 3);
  startPosY = (int)((int64_t)startY << 16 >> 3);
  finishPosX = (int)((int64_t)finishX << 16 >> 3);
  finishPosY = (int)((int64_t)finishY << 16 >> 3);
}

void GameLevel::renderTrackNearestGreenLine(GameCanvas *gameCanvas) {
  // Рисуем линию трассы
  gameCanvas->setColor(40, 200, 40); // Зеленый

  for (int pointNo = 0; pointNo < pointsCount - 1; ++pointNo) {
    // Обратная конвертация для отрисовки
    int x1 = (int)((int64_t)pointPositions[pointNo][0] << 3 >> 16);
    int y1 = (int)((int64_t)pointPositions[pointNo][1] << 3 >> 16);
    int x2 = (int)((int64_t)pointPositions[pointNo + 1][0] << 3 >> 16);
    int y2 = (int)((int64_t)pointPositions[pointNo + 1][1] << 3 >> 16);

    // Рисуем толстую линию (3 пикселя)
    gameCanvas->drawLine(x1, y1, x2, y2);
    gameCanvas->drawLine(x1, y1 - 1, x2, y2 - 1);
    gameCanvas->drawLine(x1, y1 + 1, x2, y2 + 1);
  }
}

void GameLevel::addPoint(int x, int y) {
  if (pointPositions.empty() ||
      static_cast<int>(pointPositions.size()) <= pointsCount) {
    pointPositions.resize(pointsCount + 30, std::vector<int>(2));
  }
  pointPositions[pointsCount][0] = x;
  pointPositions[pointsCount][1] = y;
  ++pointsCount;
}

void GameLevel::addPointSimple(int x, int y) {
  addPoint((int)((int64_t)x << 16 >> 3), (int)((int64_t)y << 16 >> 3));
}

int GameLevel::getStartPosX() { return (int)((int64_t)startPosX << 3 >> 16); }
int GameLevel::getStartPosY() { return (int)((int64_t)startPosY << 3 >> 16); }
int GameLevel::getFinishPosX() { return (int)((int64_t)finishPosX << 3 >> 16); }
int GameLevel::getFinishPosY() { return (int)((int64_t)finishPosY << 3 >> 16); }

int GameLevel::getPointX(int pointNo) {
  return (int)((int64_t)pointPositions[pointNo][0] << 3 >> 16);
}
int GameLevel::getPointY(int pointNo) {
  return (int)((int64_t)pointPositions[pointNo][1] << 3 >> 16);
}

int GameLevel::computeProgress(int posX) {
  int deltaStart = posX - pointPositions[startFlagPoint][0];
  int deltaFinish = pointPositions[finishFlagPoint][0] - pointPositions[startFlagPoint][0];
  int absDeltaFinish = deltaFinish < 0 ? -deltaFinish : deltaFinish;
  
  if (absDeltaFinish >= 3 && deltaStart <= deltaFinish) {
      return (int)(((int64_t)deltaStart << 32) / (int64_t)deltaFinish >> 16);
  }
  return 65536; // 100% (1.0 в F16)
}

void GameLevel::setMinMaxX(int minX, int maxX) {
  this->minX = (int)((int64_t)minX << 16 >> 3);
  this->maxX = (int)((int64_t)maxX << 16 >> 3);
}

void GameLevel::setSegmentRange(int start, int end) {
  segmentStart = start >> 1;
  segmentEnd = end >> 1;
}

void GameLevel::setSegmentRangeExact(int start, int end, int extra) {
  segmentStart = start;
  segmentEnd = end;
  segmentExtra = extra;
}

int GameLevel::getTrackHeightAt(int x_pos, int current_y_hint) {
  if (pointsCount < 2)
    return 0;

  int bestY = 0;
  int minDiffY = INT_MAX;
  bool found = false;

  // Ищем все сегменты, пересекающие x_pos, и выбираем ближайший по Y к hint
  for (int i = 0; i < pointsCount - 1; ++i) {
    int x1 = pointPositions[i][0];
    int y1 = pointPositions[i][1];
    int x2 = pointPositions[i + 1][0];
    int y2 = pointPositions[i + 1][1];

    // Попадание в диапазон по X
    bool inRange = (x1 <= x_pos && x_pos <= x2) || (x2 <= x_pos && x_pos <= x1);

    if (inRange) {
      // Интерполяция высоты
      int interpolated_y;
      if (x2 == x1) {
        interpolated_y = y1; // Вертикальная стена
      } else {
        double t = (double)(x_pos - x1) / (double)(x2 - x1);
        interpolated_y = (int)(y1 + t * (y2 - y1));
      }

      // Если подсказки нет (0), берем первый попавшийся (совместимость)
      if (current_y_hint == 0) {
        return interpolated_y;
      }

      int diff = interpolated_y - current_y_hint;
      if (diff < 0) diff = -diff;

      if (diff < minDiffY) {
        minDiffY = diff;
        bestY = interpolated_y;
        found = true;
      }
    }
  }

  if (found) {
    return bestY;
  }

  // Если не нашли пересечения (например, вылетели за пределы), ищем ближайшую точку
  int nearest = 0;
  int bestDx = INT_MAX;
  for (int i = 0; i < pointsCount; ++i) {
    int dx = pointPositions[i][0] - x_pos;
    dx = dx < 0 ? -dx : dx;
    if (dx < bestDx) {
      bestDx = dx;
      nearest = i;
    }
  }
  return pointPositions[nearest][1];
}

void GameLevel::shiftPoints(int shiftX) {
    // Сдвигаем все точки
    for(int i = 0; i < pointsCount; ++i) {
        pointPositions[i][0] -= shiftX;
    }
    // Сдвигаем маркеры
    startPosX -= shiftX;
    finishPosX -= shiftX;
    minX -= shiftX;
    maxX -= shiftX;
}