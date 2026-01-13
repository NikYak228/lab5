#include "LevelLoader.h"
#include <algorithm>
#include <climits>
#include <cmath>

// --- Статические переменные ---
int LevelLoader::visibleStart = 0;
int LevelLoader::visibleEnd = 0;
int LevelLoader::visibleStartX = 0;
int LevelLoader::visibleEndX = 0;

bool LevelLoader::isEnabledPerspective = true;
bool LevelLoader::isEnabledShadows = true;

// --- Конструктор / Деструктор ---

LevelLoader::LevelLoader() {
  // Предварительный расчет квадратов радиусов колес для оптимизации коллизий
  for (int i = 0; i < 3; ++i) {
    int radius = GamePhysics::const175_1_half[i];
    // Внешний радиус (для контакта)
    wheelRadiusSq[i] = (int)((int64_t)((radius + 19660) >> 1) *
                             (int64_t)((radius + 19660) >> 1) >> 16);
    // Внутренний радиус (для жесткого удара)
    wheelInnerRadiusSq[i] = (int)((int64_t)((radius - 19660) >> 1) *
                                  (int64_t)((radius - 19660) >> 1) >> 16);
  }
}

LevelLoader::~LevelLoader() { delete gameLevel; }

// --- Загрузка уровня ---

void LevelLoader::loadHardcodedLevel() {
  if (gameLevel == nullptr) {
    gameLevel = new GameLevel();
  }
  gameLevel->init();

  const int groundY = 220;

  // --- НАСТРОЙКИ ПЕТЛИ ---
  const int loopCenterX = 2200;
  const int loopRadius = 300; 
  const int loopCenterY = groundY + loopRadius;
  const int loopSteps = 72; 

  // Установка старта и финиша
  gameLevel->setStartFinish(0, groundY, loopCenterX + loopRadius + 1500, groundY);

  // 1. Разгонная прямая
  gameLevel->addPointSimple(-400, groundY);
  gameLevel->addPointSimple(0, groundY);
  gameLevel->addPointSimple(800, groundY);
  gameLevel->addPointSimple(1200, groundY);
  gameLevel->addPointSimple(1600, groundY);

  // Плавный подход к петле
  gameLevel->addPointSimple(loopCenterX - 300, groundY);

  // 2. Вход в петлю (нижняя точка)
  gameLevel->addPointSimple(loopCenterX, groundY);

  // 3. Генерация петли (против часовой стрелки)
  for (int step = 1; step <= loopSteps; ++step) {
    double angleRad = -M_PI_2 + static_cast<double>(step) / loopSteps * 2.0 * M_PI;
    int x = loopCenterX + static_cast<int>(loopRadius * cos(angleRad));
    int y = loopCenterY + static_cast<int>(loopRadius * sin(angleRad));
    gameLevel->addPointSimple(x, y);
  }

  // 4. Выход из петли
  gameLevel->addPointSimple(loopCenterX + loopRadius + 1500, groundY); 

  // Подготовка данных уровня (нормали)
  prepareLevelData(gameLevel);
}

// --- Getters ---

int LevelLoader::getFinishFlagX() {
  return gameLevel->pointPositions[gameLevel->finishFlagPoint][0] << 1;
}

int LevelLoader::getStartFlagX() {
  return gameLevel->pointPositions[gameLevel->startFlagPoint][0] << 1;
}

int LevelLoader::getStartPosX() { return gameLevel->startPosX << 1; }

int LevelLoader::getStartPosY() { return gameLevel->startPosY << 1; }

int LevelLoader::getProgressAt(int posXF16) {
  return gameLevel->computeProgress(posXF16 >> 1);
}

// --- Подготовка данных (нормали) ---

void LevelLoader::prepareLevelData(GameLevel *gameLevel) {
  maxTrackX = INT_MIN;
  this->gameLevel = gameLevel;
  int pointCount = gameLevel->pointsCount;
  
  if (segmentNormals.empty() || segmentCapacity < pointCount) {
    segmentCapacity = pointCount < 100 ? 100 : pointCount;
    segmentNormals.assign(segmentCapacity, std::vector<int>(2));
  }

  visibleStart = 0;
  visibleEnd = 0;
  visibleStartX = gameLevel->pointPositions[visibleStart][0];
  visibleEndX = gameLevel->pointPositions[visibleEnd][0];

  for (int i = 0; i < pointCount - 1; ++i) {
    int dx = gameLevel->pointPositions[i + 1][0] - gameLevel->pointPositions[i][0];
    int dy = gameLevel->pointPositions[i + 1][1] - gameLevel->pointPositions[i][1];

    if (i != 0 && i != pointCount - 1) {
      maxTrackX = std::max(maxTrackX, gameLevel->pointPositions[i][0]);
    }

    // Расчет нормали к сегменту
    int normalX = -dy;
    int lengthF16 = GamePhysics::calcVectorLengthF16(normalX, dx);

    if (lengthF16 == 0) {
      segmentNormals[i][0] = 0;
      segmentNormals[i][1] = 0;
      continue;
    }

    // Нормализация вектора нормали
    segmentNormals[i][0] = (int)(((int64_t)normalX << 32) / (int64_t)lengthF16 >> 16);
    segmentNormals[i][1] = (int)(((int64_t)dx << 32) / (int64_t)lengthF16 >> 16);
  }

  gameLevel->startFlagPoint = 0;
  gameLevel->finishFlagPoint = pointCount - 1;

  visibleStart = 0;
  visibleEnd = 0;
  loopApexPassed = false;
  visibleStartX = 0;
  visibleEndX = 0;
}

void LevelLoader::appendLevelData(int startPointIndex) {
  if (!gameLevel) return;
  int pointCount = gameLevel->pointsCount;

  if (static_cast<int>(segmentNormals.size()) < pointCount) {
    segmentNormals.resize(pointCount + 500, std::vector<int>(2));
    segmentCapacity = pointCount + 500;
  }

  int start = (startPointIndex > 0) ? startPointIndex - 1 : 0;

  for (int i = start; i < pointCount - 1; ++i) {
    int dx = gameLevel->pointPositions[i + 1][0] - gameLevel->pointPositions[i][0];
    int dy = gameLevel->pointPositions[i + 1][1] - gameLevel->pointPositions[i][1];
    
    int normalX = -dy; 
    int lenF16 = GamePhysics::calcVectorLengthF16(normalX, dx);

    if (lenF16 == 0) {
      segmentNormals[i][0] = 0;
      segmentNormals[i][1] = 0;
    } else {
      segmentNormals[i][0] = (int)(((int64_t)normalX << 32) / (int64_t)lenF16 >> 16);
      segmentNormals[i][1] = (int)(((int64_t)dx << 32) / (int64_t)lenF16 >> 16);
    }
    
    if (i != 0 && i != pointCount - 1) {
      maxTrackX = std::max(maxTrackX, gameLevel->pointPositions[i][0]);
    }
  }

  visibleEnd = pointCount - 1; 
  gameLevel->finishFlagPoint = pointCount - 1;
}

void LevelLoader::setMinMaxX(int minX, int maxX) {
  gameLevel->setMinMaxX(minX, maxX);
}

void LevelLoader::renderTrackNearestLine(GameCanvas *canvas) {
  gameLevel->renderTrackNearestGreenLine(canvas);
}

void LevelLoader::updateVisibleRange(int minXF16, int maxXF16, int cameraYF16) {
  // Устанавливаем точный диапазон сегментов для проверки коллизий
  gameLevel->setSegmentRangeExact((minXF16 + 98304) >> 1,
                                  (maxXF16 - 98304) >> 1, cameraYF16 >> 1);

  // Для петли проверяем все точки (оптимизация отключена для корректности)
  visibleStart = 0;
  visibleEnd = gameLevel->pointsCount - 1;

  visibleStartX = minXF16 >> 1;
  visibleEndX = maxXF16 >> 1;
}

// --- Детекция коллизий ---

int LevelLoader::detectCollision(MotoComponent *wheel, int wheelIndex) {
  int collisionsCount = 0;
  int8_t collisionType = 2; // 2 = Нет коллизии
  int wheelX = wheel->xF16 >> 1;
  int wheelY = wheel->yF16 >> 1;

  if (isEnabledPerspective) {
    wheelY -= 65536;
  }

  // --- Логика Петли ---
  const int loopCenterX = 2200;
  const int loopRadius = 300;
  int apexThresholdY = (220 + loopRadius + 100) << 1;

  // Определение прохождения верхней точки петли (Apex)
  if (wheelX > ((loopCenterX - 100) << 1) &&
      wheelX < ((loopCenterX + 100) << 1)) {
    if (wheelY > apexThresholdY) {
      loopApexPassed = true;
    }
  }

  // Сброс флага Apex, если уехали далеко
  if (wheelX < ((loopCenterX - loopRadius - 200) << 1) ||
      wheelX > ((loopCenterX + loopRadius + 500) << 1)) {
    loopApexPassed = false;
  }

  int normalSumX = 0, normalSumY = 0;
  int maxSegmentIdx = std::min(visibleEnd, gameLevel->pointsCount - 1);

  // Перебор видимых сегментов трассы
  for (int segmentIdx = visibleStart; segmentIdx < maxSegmentIdx; ++segmentIdx) {
    if (segmentIdx < 0 || segmentIdx >= gameLevel->pointsCount - 1) continue;
    if (segmentIdx >= static_cast<int>(segmentNormals.size())) continue;

    int x1 = gameLevel->pointPositions[segmentIdx][0];
    int y1 = gameLevel->pointPositions[segmentIdx][1];
    int x2 = gameLevel->pointPositions[segmentIdx + 1][0];
    int y2 = gameLevel->pointPositions[segmentIdx + 1][1];

    // --- Игнорирование "стены" при выходе из петли ---
    if (loopApexPassed) {
      int x1_F16 = x1 << 1;
      int startZoneMin = (loopCenterX - 50) << 1;
      int startZoneMax = (loopCenterX + 100) << 1;

      bool isRising = (y2 > y1 + 50);
      bool isLoopStartZone = (x1_F16 >= startZoneMin) && (x1_F16 <= startZoneMax);
      bool isNearGround = (wheelY < ((220 + 100) << 1));

      if (isLoopStartZone && isRising && isNearGround) {
        continue; // Игнорируем этот сегмент, чтобы не врезаться на выходе
      }
    }

    // --- Проверка AABB (Axis-Aligned Bounding Box) ---
    int segMaxX = std::max(x1, x2);
    int segMinX = std::min(x1, x2);
    int segMinY = std::min(y1, y2);
    int segMaxY = std::max(y1, y2);

    int r = wheelRadiusSq[wheelIndex]; 

    // Если колесо не в прямоугольнике сегмента - пропускаем
    if (!(wheelX + r >= segMinX && wheelX - r <= segMaxX &&
          wheelY + r >= segMinY && wheelY - r <= segMaxY)) {
        continue;
    }

    // --- Точная проверка коллизии (проекция точки на отрезок) ---
    int segDX = x1 - x2;
    int segDY = y1 - y2;
    
    // Квадрат длины сегмента
    int segLenSq = (int)((int64_t)segDX * (int64_t)segDX >> 16) +
                   (int)((int64_t)segDY * (int64_t)segDY >> 16);

    // Проекция
    int proj = (int)((int64_t)(wheelX - x1) * (int64_t)(-segDX) >> 16) +
               (int)((int64_t)(wheelY - y1) * (int64_t)(-segDY) >> 16);

    int tF16;
    if (segLenSq == 0 || (segLenSq < 0 ? -segLenSq : segLenSq) < 3) {
      continue; // Сегмент вырожден
    }
    tF16 = (int)(((int64_t)proj << 32) / (int64_t)segLenSq >> 16);

    // Clamp tF16 to [0, 65536] (сегмент ограничен точками)
    if (tF16 < 0) tF16 = 0;
    if (tF16 > 65536) tF16 = 65536;

    // Ближайшая точка на сегменте
    int closestX = x1 + (int)((int64_t)tF16 * (int64_t)(-segDX) >> 16);
    int closestY = y1 + (int)((int64_t)tF16 * (int64_t)(-segDY) >> 16);

    int diffX = wheelX - closestX;
    int diffY = wheelY - closestY;

    // Расстояние до сегмента
    int64_t distSq = ((int64_t)diffX * (int64_t)diffX >> 16) +
                     ((int64_t)diffY * (int64_t)diffY >> 16);

    int8_t contactState;
    if (distSq < (int64_t)wheelRadiusSq[wheelIndex]) {
      if (distSq >= (int64_t)wheelInnerRadiusSq[wheelIndex]) {
        contactState = 1; // Мягкое касание
      } else {
        contactState = 0; // Жесткая коллизия (проникновение)
      }
    } else {
      contactState = 2; // Нет контакта
    }

    if (contactState == 2) continue;

    // Проверка направления скорости (скалярное произведение с нормалью)
    int64_t dotProd = (int64_t)segmentNormals[segmentIdx][0] * (int64_t)wheel->velX +
                      (int64_t)segmentNormals[segmentIdx][1] * (int64_t)wheel->velY;

    // Жесткий удар
    if (contactState == 0 && (dotProd >> 16) < 0) {
      collisionNormalX = segmentNormals[segmentIdx][0];
      collisionNormalY = segmentNormals[segmentIdx][1];
      return 0; // Возвращаем тип 0 (Collision)
    }

    // Мягкое касание
    if (contactState == 1 && (dotProd >> 16) < 0) {
      ++collisionsCount;
      collisionType = 1;
      if (collisionsCount == 1) {
        normalSumX = segmentNormals[segmentIdx][0];
        normalSumY = segmentNormals[segmentIdx][1];
      } else {
        normalSumX += segmentNormals[segmentIdx][0];
        normalSumY += segmentNormals[segmentIdx][1];
      }
    }
  }

  // Усреднение нормали при множественных касаниях
  if (collisionType == 1) {
    if ((int)((int64_t)normalSumX * (int64_t)wheel->velX >> 16) +
            (int)((int64_t)normalSumY * (int64_t)wheel->velY >> 16) >= 0) {
      return 2; // Скользящий контакт без удара
    }

    collisionNormalX = normalSumX;
    collisionNormalY = normalSumY;
  }

  return collisionType;
}

void LevelLoader::shiftLevel(int shiftX) {
    if (gameLevel) {
        gameLevel->shiftPoints(shiftX);
    }
    visibleStartX -= (shiftX >> 1);
    visibleEndX -= (shiftX >> 1);
}