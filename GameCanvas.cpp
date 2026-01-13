#include "GameCanvas.h"
#include "MathF16.h"
#include "GamePhysics.h"
#include "iostream"
#include <memory>
#include <vector>
#include <cmath> // для ceil

GameCanvas::GameCanvas(Micro* micro) : micro(micro) {
    width  = 0;
    height = 0;
    dx = 0;
    dy = 0;
    
    // Инициализация векторного шрифта (координаты штрихов)
    vectorFont['0'] = { {{0,0},{3,0},{3,5},{0,5},{0,0}} }; // O
    vectorFont['1'] = { {{1,0},{1,5}} }; // |
    vectorFont['2'] = { {{0,0},{3,0},{3,2},{0,2},{0,3},{3,3},{3,5},{0,5}} }; 
    vectorFont['3'] = { {{0,0},{3,0},{3,5},{0,5}}, {{0,2},{3,2}} }; 
    vectorFont['4'] = { {{0,0},{0,2},{3,2}}, {{3,0},{3,5}} }; 
    vectorFont['5'] = { {{3,0},{0,0},{0,2},{3,2},{3,5},{0,5}} }; 
    vectorFont['6'] = { {{3,0},{0,0},{0,5},{3,5},{3,2},{0,2}} }; 
    vectorFont['7'] = { {{0,0},{3,0},{3,5}} }; 
    vectorFont['8'] = { {{0,0},{3,0},{3,5},{0,5},{0,0}}, {{0,2},{3,2}} }; 
    vectorFont['9'] = { {{0,5},{3,5},{3,0},{0,0},{0,2},{3,2}} }; 
    vectorFont[':'] = { {{1,1},{2,1},{2,2},{1,2},{1,1}}, {{1,3},{2,3},{2,4},{1,4},{1,3}} }; 
    
    // Буквы для HUD (Manual / AI)
    vectorFont['A'] = { {{0,5},{0,2},{1,0},{2,0},{3,2},{3,5}}, {{0,2},{3,2}} }; 
    vectorFont['I'] = { {{0,0},{3,0}}, {{1,0},{1,5}}, {{0,5},{3,5}} }; 
    vectorFont['M'] = { {{0,5},{0,0},{1,2},{2,0},{3,5}} }; // M
    vectorFont['N'] = { {{0,5},{0,0},{3,5},{3,0}} }; // N
    vectorFont['U'] = { {{0,0},{0,5},{3,5},{3,0}} }; // U
    vectorFont['L'] = { {{0,0},{0,5},{3,5}} }; // L
}

GameCanvas::~GameCanvas() = default;

void GameCanvas::init(GamePhysics* gp) {
    this->gamePhysics = gp;
    // Установка минимального размера экрана для камеры
    gamePhysics->setMinimalScreenWH(width < height ? width : height);
}

// --- Отрисовка ---

void GameCanvas::paint(Graphics* g) {
    drawGame(g);
}

void GameCanvas::drawGame(Graphics* g) {
    if (!Micro::isReady || micro->isRunning) { // isRunning используется как флаг паузы/выхода?
        return;
    }
    graphics = g;

    // Обновляем размеры если изменились
    if (height != getHeight()) {
        updateSizeAndRepaint();
    }

    // Подготовка физики к рендеру (интерполяция)
    gamePhysics->setMotoComponents();
    
    static constexpr float CAM_ZOOM = 2.0f;
    int logicalW = int(width  / CAM_ZOOM);
    int logicalH = int(height / CAM_ZOOM);

    // Установка камеры
    setViewPosition(
        -gamePhysics->getCamPosX() + cameraOffsetX + logicalW/2,
        gamePhysics->getCamPosY() + cameraOffsetY + logicalH/2
    );
    
    // Рендер игрового мира
    gamePhysics->renderGame(this);
    
    // --- HUD (Интерфейс) ---
    // Отрисовка режима управления (AI / Manual)
    std::string modeText = gamePhysics->isGenerateInputAI ? "AI" : "MAN";
    drawVectorString(modeText, 10, 10, 2);

    graphics = nullptr;
}

void GameCanvas::drawSkyGradient() {
    const int num_bands = 16; 
    const float band_height = (float)height / num_bands;

    for (int i = 0; i < num_bands; ++i) {
        int y_pos = (int)(i * band_height);
        float ratio = (float)i / (float)num_bands;
        Uint8 r = (Uint8)(50 + ratio * 85);
        Uint8 g = (Uint8)(100 + ratio * 106);
        Uint8 b = (Uint8)(150 + ratio * 100);

        graphics->setColor(r, g, b);
        graphics->fillRect(0, y_pos, width, (int)ceil(band_height));
    }
}

// --- Векторный шрифт ---

void GameCanvas::drawVectorChar(char c, int start_x, int start_y, int scale) {
    if (vectorFont.find(c) == vectorFont.end()) return; 

    const auto& strokes = vectorFont.at(c);
    
    // Смещение HUD, чтобы он не двигался вместе с камерой (фиксированный на экране)
    // Но drawLine использует addDx/addDy! 
    // Нам нужно рисовать HUD в экранных координатах.
    // drawLine прибавляет dx/dy.
    // Чтобы рисовать в экранных, нужно вычесть dx/dy.
    
    int hudOffsetX = -dx;
    int hudOffsetY = -dy; 
    // dy инвертирован в addDy: return -y + dy.
    // y_screen = -y_world + dy.
    // y_world = dy - y_screen.
    // Если мы хотим y_screen = 10.
    // y_world = dy - 10.
    
    // start_y - экранная координата Y (сверху вниз).
    // addDy ожидает Y "снизу вверх" (мировой).
    // Это сложно. Проще добавить метод drawLineScreen.
    // Но пока используем хак с отменой трансформации.
    
    // Или просто временно сбросим dx/dy?
    int savedDx = dx;
    int savedDy = dy;
    dx = 0; dy = 0; // Сброс камеры для HUD
    
    // Но постойте, addDy инвертирует Y.
    // addDy(y) = -y + 0 = -y.
    // SDL рисует Y вниз.
    // Если я хочу нарисовать в (10, 10).
    // drawLine(10, -10) -> addDy(-10) -> -(-10) = 10.
    // То есть для экранных координат нужно передавать отрицательный Y?
    
    int screen_y = -start_y;

    for (const auto& stroke : strokes) {
        if (stroke.size() < 2) continue;

        for (size_t i = 0; i < stroke.size() - 1; ++i) {
            const auto& p1 = stroke[i];
            const auto& p2 = stroke[i + 1];

            int x1 = start_x + p1.x * scale;
            int y1 = screen_y - p1.y * scale; // Y растет вниз в шрифте, но вверх в addDy?
            // Шрифт: (0,0) top-left. (0,5) bottom-left.
            // addDy(-y): y=0 -> 0. y=5 -> -5.
            // SDL Y: 0 -> 0. 5 -> 5.
            // Если screen_y = -10.
            // Point(0,0) -> y1 = -10. addDy(-10) = 10. Correct.
            // Point(0,5) -> y1 = -10 - 5 = -15. addDy(-15) = 15. Correct.
            
            int x2 = start_x + p2.x * scale;
            int y2 = screen_y - p2.y * scale;

            graphics->drawLine(x1, y1, x2, y2);
        }
    }
    
    dx = savedDx;
    dy = savedDy;
}

void GameCanvas::drawVectorString(const std::string& text, int start_x, int start_y, int scale) {
    int current_x = start_x;
    int char_width = (3 + 1) * scale; 
    setColor(255, 255, 255); 

    for (const char c : text) {
        drawVectorChar(c, current_x, start_y, scale);
        current_x += char_width; 
    }
}

// --- Обработка ввода ---

void GameCanvas::processKeyPressed(int keyCode) {
    // Спец. клавиши для управления режимом (User Request)
    if (keyCode == 'M') {
        gamePhysics->disableGenerateInputAI();
        return;
    }
    if (keyCode == 'A') {
        gamePhysics->enableGenerateInputAI();
        return;
    }

    int action = getGameAction(keyCode);
    int numKey;
    if ((numKey = keyCode - 48) >= 0 && numKey < 10) {
        activeKeys[numKey] = true;
    } else if (action >= 0 && action < 7) {
        activeActions[action] = true;
    }
    handleUpdatedInput();
}

void GameCanvas::processKeyReleased(int keyCode) {
    int action = getGameAction(keyCode);
    int numKey;
    if ((numKey = keyCode - 48) >= 0 && numKey < 10) {
        activeKeys[numKey] = false;
    } else if (action >= 0 && action < 7) {
        activeActions[action] = false;
    }
    handleUpdatedInput();
}

void GameCanvas::clearInputStates() {
    for (int i = 0; i < 10; ++i) activeKeys[i] = false;
    for (int i = 0; i < 7; ++i) activeActions[i] = false;
}

void GameCanvas::handleUpdatedInput() {
    int dirX = 0, dirY = 0;
    for (int i = 0; i < 10; ++i) {
        if (activeKeys[i]) {
            dirX += numKeyDirTable[numKeyMode][i][0];
            dirY += numKeyDirTable[numKeyMode][i][1];
        }
    }
    for (int i = 0; i < 7; ++i) {
        if (activeActions[i]) {
            dirX += actionDirTable[i][0];
            dirY += actionDirTable[i][1];
        }
    }
    gamePhysics->applyUserInput(dirX, dirY);
}

void GameCanvas::resetInput() {
    clearInputStates();
}

void GameCanvas::updateSizeAndRepaint() {
    width = getWidth();
    height = getHeight();
    repaint();
}

void GameCanvas::requestRepaint(int mode) {
    // Legacy
    repaint();
}

void GameCanvas::setViewPosition(int dx, int dy) {
    this->dx = dx;
    this->dy = dy;
    static constexpr float CAM_ZOOM = 2.0f;
    int logicalW = int(width / CAM_ZOOM);
    gamePhysics->setRenderMinMaxX(-dx, -dx + logicalW);
}

int GameCanvas::addDx(int x) { return x + dx; }
int GameCanvas::addDy(int y) { return -y + dy; }

void GameCanvas::drawLine(int x, int y, int x2, int y2) {
    graphics->drawLine(addDx(x), addDy(y), addDx(x2), addDy(y2));
}

void GameCanvas::drawLineF16(int x, int y, int x2, int y2) {
    graphics->drawLine(addDx(x << 2 >> 16), addDy(y << 2 >> 16), addDx(x2 << 2 >> 16), addDy(y2 << 2 >> 16));
}

void GameCanvas::drawBrakeDisc(int x, int y, int radius, int angle) {
    ++radius;
    int localX = addDx(x - radius);
    int localY = addDy(y + radius);
    int diameter = radius << 1;
    if ((angle = -((int)(((int64_t)((int)((int64_t)angle * 11796480L >> 16)) << 32) / 205887L >> 16))) < 0) {
        angle += 360;
    }
    graphics->drawArc(localX, localY, diameter, diameter, (angle >> 16) + 170, 90);
}

void GameCanvas::drawCircle(int x, int y, int size) {
    int radius = size / 2;
    int localX = addDx(x - radius);
    int localY = addDy(y + radius);
    graphics->drawArc(localX, localY, size, size, 0, 360);
}

void GameCanvas::clearScreenWithWhite() {
    graphics->setColor(255, 255, 255);
    graphics->fillRect(0, 0, width, height);
}

void GameCanvas::setColor(int red, int green, int blue) {
    graphics->setColor(red, green, blue);
}

void GameCanvas::keyPressed(int keyCode) {
    processKeyPressed(keyCode);
}

void GameCanvas::keyReleased(int keyCode) {
    processKeyReleased(keyCode);
}

void GameCanvas::drawWheel(int x, int y, int radius) {
    setColor(0, 0, 0); 
    drawCircle(x, y, radius * 2);
    setColor(150, 150, 150); 
    drawCircle(x, y, radius);
}