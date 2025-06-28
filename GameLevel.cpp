#include "GameLevel.h"
#include "GameCanvas.h"
#include "GamePhysics.h"

GameLevel::GameLevel() {
    init();
}
GameLevel::~GameLevel() {}

void GameLevel::init() {
    startPosX = 0;
    startPosY = 0;
    finishPosX = 13107200;
    pointsCount = 0;
    field_274 = 0;
    pointPositions.clear();
}

void GameLevel::method_174(int var1, int var2, int var3, int var4) {
    startPosX = var1 << 16 >> 3;
    startPosY = var2 << 16 >> 3;
    finishPosX = var3 << 16 >> 3;
    finishPosY = var4 << 16 >> 3;
}

void GameLevel::renderTrackNearestGreenLine(GameCanvas* gameCanvas) {
    const int gradientSteps = 80;  // сколько «слоёв» градиента
    // цвет в верхней точке (самая трасса)
    const int r1 =  40, g1 = 200, b1 =  40;
    // цвет в нижней точке градиента
    const int r2 =   0, g2 = 100, b2 =   0;

    int pointNo = 0;
    // ищем первый видимый сегмент
    while (pointNo < pointsCount-1 && pointPositions[pointNo][0] <= minX) {
        ++pointNo;
    }
    if (pointNo>0) --pointNo;

    for (; pointNo < pointsCount-1; ++pointNo) {
        int x1 = pointPositions[pointNo    ][0] << 3 >> 16;
        int y1 = pointPositions[pointNo    ][1] << 3 >> 16;
        int x2 = pointPositions[pointNo + 1][0] << 3 >> 16;
        int y2 = pointPositions[pointNo + 1][1] << 3 >> 16;

        // если сегмент за экраном — выходим
        if (x1 > maxX && x2 > maxX) break;

        // рисуем градиент снизу вверх
        for (int step = 0; step < gradientSteps; ++step) {
            float t = float(step) / float(gradientSteps - 1);
            int r = int(r1 * (1 - t) + r2 * t);
            int g = int(g1 * (1 - t) + g2 * t);
            int b = int(b1 * (1 - t) + b2 * t);

            gameCanvas->setColor(r, g, b);
            gameCanvas->drawLine(
                x1, y1 - step,
                x2, y2 - step
            );
        }
    }
}


void GameLevel::addPoint(int x, int y) {
    if (pointPositions.empty() || static_cast<int>(pointPositions.size()) <= pointsCount) {
        pointPositions.resize(pointsCount + 30, std::vector<int>(2));
    }
    if (pointsCount == 0 || pointPositions[pointsCount - 1][0] < x) {
        pointPositions[pointsCount][0] = x;
        pointPositions[pointsCount][1] = y;
        ++pointsCount;
    }
}

void GameLevel::addPointSimple(int var1, int var2) {
    addPoint(var1 << 16 >> 3, var2 << 16 >> 3);
}


// Остальные методы можно скопировать из вашего оригинального GameLevel.cpp
// так как они в основном занимаются математикой, а не отрисовкой.
// Для простоты я включу их реализации ниже.

int GameLevel::getStartPosX() { return startPosX << 3 >> 16; }
int GameLevel::getStartPosY() { return startPosY << 3 >> 16; }
int GameLevel::getFinishPosX() { return finishPosX << 3 >> 16; }
int GameLevel::getFinishPosY() { return finishPosY << 3 >> 16; }
int GameLevel::getPointX(int pointNo) { return pointPositions[pointNo][0] << 3 >> 16; }
int GameLevel::getPointY(int pointNo) { return pointPositions[pointNo][1] << 3 >> 16; }

int GameLevel::method_181(int var1) {
    int var2 = var1 - pointPositions[startFlagPoint][0];
    int var3;
    return ((var3 = pointPositions[finishFlagPoint][0] - pointPositions[startFlagPoint][0]) < 0 ? -var3 : var3) >= 3 && var2 <= var3 ? (int)(((int64_t)var2 << 32) / (int64_t)var3 >> 16) : 65536;
}

void GameLevel::setMinMaxX(int minX, int maxX) {
    this->minX = minX << 16 >> 3;
    this->maxX = maxX << 16 >> 3;
}

void GameLevel::method_183(int var1, int var2) {
    field_264 = var1 >> 1;
    field_265 = var2 >> 1;
}

void GameLevel::method_184(int var1, int var2, int var3) {
    field_264 = var1;
    field_265 = var2;
    field_266 = var3;
}

// Функции отрисовки теней и 3D можно сделать пустыми, так как мы их отключили
void GameLevel::renderShadow(GameCanvas* gameCanvas, int var2, int var3) {}
void GameLevel::renderLevel3D(GameCanvas* gameCanvas, int xF16, int yF16) {}
// GameLevel.cpp (в конец файла)

int GameLevel::getTrackHeightAt(int x_pos) {
    if (pointsCount < 2) return 0; // Нечего проверять, если нет трассы

    // Находим два поинта трассы, между которыми находится x_pos
    int p1_idx = -1;
    for (int i = 0; i < pointsCount - 1; ++i) {
        if (pointPositions[i][0] <= x_pos && pointPositions[i + 1][0] >= x_pos) {
            p1_idx = i;
            break;
        }
    }

    if (p1_idx == -1) {
        // Если мы за пределами трассы, возвращаем высоту ближайшей точки
        if (x_pos < pointPositions[0][0]) return pointPositions[0][1];
        return pointPositions[pointsCount - 1][1];
    }

    // Выполняем линейную интерполяцию, чтобы найти точную высоту
    int p2_idx = p1_idx + 1;
    int x1 = pointPositions[p1_idx][0];
    int y1 = pointPositions[p1_idx][1];
    int x2 = pointPositions[p2_idx][0];
    int y2 = pointPositions[p2_idx][1];

    if (x2 == x1) return y1; // Избегаем деления на ноль

    double t = (double)(x_pos - x1) / (double)(x2 - x1);
    int interpolated_y = (int)(y1 + t * (y2 - y1));

    return interpolated_y;
}