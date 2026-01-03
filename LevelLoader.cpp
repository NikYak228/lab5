#include "LevelLoader.h"
#include <climits>
#include <algorithm>
#include <cmath>

// Статические переменные
int LevelLoader::visibleStart = 0;
int LevelLoader::visibleEnd = 0;
int LevelLoader::visibleStartX = 0;
int LevelLoader::visibleEndX = 0;

bool LevelLoader::isEnabledPerspective = true;
bool LevelLoader::isEnabledShadows = true;

LevelLoader::LevelLoader() {
    // Предварительный расчет квадратов радиусов колес для оптимизации
    for (int i = 0; i < 3; ++i) {
        wheelRadiusSq[i] = (int)((int64_t)((GamePhysics::const175_1_half[i] + 19660) >> 1) * (int64_t)((GamePhysics::const175_1_half[i] + 19660) >> 1) >> 16);
        wheelInnerRadiusSq[i] = (int)((int64_t)((GamePhysics::const175_1_half[i] - 19660) >> 1) * (int64_t)((GamePhysics::const175_1_half[i] - 19660) >> 1) >> 16);
    }
}

LevelLoader::~LevelLoader() {
    delete gameLevel;
}

void LevelLoader::loadHardcodedLevel() {
    if (gameLevel == nullptr) {
        gameLevel = new GameLevel();
    }
    gameLevel->init();
    
    const int groundY = 220;

    // --- НАСТРОЙКИ ПЕТЛИ ---
    const int loopCenterX = 2200;
    const int loopRadius  = 350;                  // Радиус петли
    const int loopCenterY = groundY + loopRadius; // Центр петли: земля внизу, Y растет вверх
    const int loopSteps   = 72;                   // Количество сегментов

    // Старт/финиш
    gameLevel->setStartFinish(0, groundY, loopCenterX + loopRadius + 1500, groundY);

    // 1. Разгонная прямая
    gameLevel->addPointSimple(-400, groundY);
    gameLevel->addPointSimple(   0, groundY);
    gameLevel->addPointSimple( 800, groundY);
    
    // Плавный подход к петле
    gameLevel->addPointSimple(loopCenterX - 300, groundY);

    // 2. Вход в петлю (нижняя точка)
    gameLevel->addPointSimple(loopCenterX, groundY);

    // 3. Генерация самой петли
    // Начинаем снизу (-PI/2) и идем против часовой стрелки, чтобы плавно выйти наверх
    for (int step = 1; step <= loopSteps; ++step) {
        double angleRad = -M_PI_2 + static_cast<double>(step) / loopSteps * 2.0 * M_PI;
        
        int x = loopCenterX + static_cast<int>(loopRadius * cos(angleRad));
        int y = loopCenterY + static_cast<int>(loopRadius * sin(angleRad));
        gameLevel->addPointSimple(x, y);
    }

    // 4. Выход из петли
    gameLevel->addPointSimple(loopCenterX + loopRadius + 1500, groundY); // Финишная прямая

    // Подготовка данных уровня (нормали и т.д.)
    prepareLevelData(gameLevel);
}

int LevelLoader::getFinishFlagX() {
    return gameLevel->pointPositions[gameLevel->finishFlagPoint][0] << 1;
}

int LevelLoader::getStartFlagX() {
    return gameLevel->pointPositions[gameLevel->startFlagPoint][0] << 1;
}

int LevelLoader::getStartPosX() {
    return gameLevel->startPosX << 1;
}

int LevelLoader::getStartPosY() {
    return gameLevel->startPosY << 1;
}

int LevelLoader::getProgressAt(int posXF16) {
    return gameLevel->computeProgress(posXF16 >> 1);
}

void LevelLoader::prepareLevelData(GameLevel* gameLevel) {
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
        // Расчет вектора сегмента (без замыкания последней точки на первую)
        int dx = gameLevel->pointPositions[i + 1][0] - gameLevel->pointPositions[i][0];
        int dy = gameLevel->pointPositions[i + 1][1] - gameLevel->pointPositions[i][1];
        
        if (i != 0 && i != pointCount - 1) {
            maxTrackX = std::max(maxTrackX, gameLevel->pointPositions[i][0]);
        }

        // Расчет нормали
        int normalX = -dy;
        int lengthF16 = GamePhysics::calcVectorLengthF16(normalX, dx);
        
        if (lengthF16 == 0) {
            segmentNormals[i][0] = 0;
            segmentNormals[i][1] = 0;
            continue;
        }
        
        // Нормализация вектора
        segmentNormals[i][0] = (int)(((int64_t)normalX << 32) / (int64_t)lengthF16 >> 16);
        segmentNormals[i][1] = (int)(((int64_t)dx << 32) / (int64_t)lengthF16 >> 16);
    }
    
    // Явно фиксируем точки старта/финиша
    gameLevel->startFlagPoint = 0;
    gameLevel->finishFlagPoint = pointCount - 1;

    // Сброс видимого диапазона
    visibleStart = 0;
    visibleEnd = 0;
    visibleStartX = 0;
    visibleEndX = 0;
}

void LevelLoader::setMinMaxX(int minX, int maxX) {
    gameLevel->setMinMaxX(minX, maxX);
}

void LevelLoader::renderTrackNearestLine(GameCanvas* canvas) {
    // Трек рисуется как толстая линия (см. GameLevel::renderTrackNearestGreenLine)
    gameLevel->renderTrackNearestGreenLine(canvas);
}

void LevelLoader::updateVisibleRange(int minXF16, int maxXF16, int cameraYF16) {
    // Передаем точные координаты камере
    gameLevel->setSegmentRangeExact((minXF16 + 98304) >> 1, (maxXF16 - 98304) >> 1, cameraYF16 >> 1);
    
    // ВАЖНО: Для корректной работы петли мы должны проверять ВСЕ сегменты уровня.
    // Стандартная оптимизация (отсечение по X) сломает физику на вертикальных участках и потолке.
    visibleStart = 0;
    visibleEnd = gameLevel->pointsCount - 1;
    
    visibleStartX = minXF16 >> 1;
    visibleEndX = maxXF16 >> 1;
}

int LevelLoader::detectCollision(MotoComponent* wheel, int wheelIndex) {
    int collisionsCount = 0;
    int8_t collisionType = 2; // 2 = нет коллизии
    int wheelX = wheel->xF16 >> 1;
    int wheelY = wheel->yF16 >> 1;
    
    if (isEnabledPerspective) {
        wheelY -= 65536;
    }

    int normalSumX = 0, normalSumY = 0;

    for (int segmentIdx = visibleStart; segmentIdx < visibleEnd; ++segmentIdx) {
        int x1 = gameLevel->pointPositions[segmentIdx][0];
        int y1 = gameLevel->pointPositions[segmentIdx][1];
        int x2 = gameLevel->pointPositions[segmentIdx + 1][0];
        int y2 = gameLevel->pointPositions[segmentIdx + 1][1];

        // --- ИСПРАВЛЕНИЕ ФИЗИКИ ПЕТЛИ ---
        // Используем std::min и std::max для определения границ сегмента.
        // Это позволяет корректно обрабатывать линии, идущие справа налево (потолок петли).
        int segMinX = std::min(x1, x2);
        int segMaxX = std::max(x1, x2);
        int segMinY = std::min(y1, y2);
        int segMaxY = std::max(y1, y2);
        
        int r = wheelRadiusSq[wheelIndex]; // Здесь это используется как радиус bounding box-а

        // Проверка AABB (Axis-Aligned Bounding Box)
        if (wheelX + r >= segMinX && wheelX - r <= segMaxX && 
            wheelY + r >= segMinY && wheelY - r <= segMaxY) 
        {
            int segDX = x1 - x2;
            int segDY = y1 - y2;
            
            // Квадрат длины сегмента
            int segLenSq = (int)((int64_t)segDX * (int64_t)segDX >> 16) + (int)((int64_t)segDY * (int64_t)segDY >> 16);
            
            // Проекция центра колеса на прямую сегмента
            int proj = (int)((int64_t)(wheelX - x1) * (int64_t)(-segDX) >> 16) + (int)((int64_t)(wheelY - y1) * (int64_t)(-segDY) >> 16);
            
            int tF16;
            if ((segLenSq < 0 ? -segLenSq : segLenSq) >= 3) {
                tF16 = (int)(((int64_t)proj << 32) / (int64_t)segLenSq >> 16);
            } else {
                tF16 = (proj > 0 ? 1 : -1) * (segLenSq > 0 ? 1 : -1) * INT_MAX;
            }

            // Ограничиваем tF16 диапазоном [0, 65536] (отрезок от x1 до x2)
            if (tF16 < 0) tF16 = 0;
            if (tF16 > 65536) tF16 = 65536;

            // Ближайшая точка на сегменте
            int closestX = x1 + (int)((int64_t)tF16 * (int64_t)(-segDX) >> 16);
            int closestY = y1 + (int)((int64_t)tF16 * (int64_t)(-segDY) >> 16);
            
            int diffX = wheelX - closestX;
            int diffY = wheelY - closestY;
            
            int8_t contactState;
            int64_t distSq = ((int64_t)diffX * (int64_t)diffX >> 16) + ((int64_t)diffY * (int64_t)diffY >> 16);
            
            // Проверяем расстояние до сегмента
            if (distSq < (int64_t)wheelRadiusSq[wheelIndex]) {
                if (distSq >= (int64_t)wheelInnerRadiusSq[wheelIndex]) {
                    contactState = 1; // Касание (мягкое)
                } else {
                    contactState = 0; // Глубокое проникновение (жесткая коллизия)
                }
            } else {
                contactState = 2; // Нет контакта
            }

            // Расчет скалярного произведения скорости и нормали (движемся ли мы К стене)
            int64_t dotProd = (int64_t)segmentNormals[segmentIdx][0] * (int64_t)wheel->velX + 
                              (int64_t)segmentNormals[segmentIdx][1] * (int64_t)wheel->velY;

            // Обработка жесткого удара
            if (contactState == 0 && (dotProd >> 16) < 0) {
                collisionNormalX = segmentNormals[segmentIdx][0];
                collisionNormalY = segmentNormals[segmentIdx][1];
                return 0;
            }

            // Обработка мягкого касания
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
    }

    // Если было касание с несколькими поверхностями (углы), усредняем нормаль
    if (collisionType == 1) {
        if ((int)((int64_t)normalSumX * (int64_t)wheel->velX >> 16) + (int)((int64_t)normalSumY * (int64_t)wheel->velY >> 16) >= 0) {
            return 2;
        }

        collisionNormalX = normalSumX;
        collisionNormalY = normalSumY;
    }

    return collisionType;
}
