#include "GameCanvas.h"
#include "MathF16.h"
#include "GamePhysics.h"
#include "iostream"
#include <memory>
#include <vector>
GameCanvas::GameCanvas(Micro* micro) : micro(micro) {
    width  = 0;
    height = 0;
    dx = 0;
    dy = 0;
    vectorFont['0'] = { {{0,0},{3,0},{3,5},{0,5},{0,0}} }; // O
    vectorFont['1'] = { {{1,0},{1,5}} }; // |
    vectorFont['2'] = { {{0,0},{3,0},{3,2},{0,2},{0,3},{3,3},{3,5},{0,5}} }; // S-образная
    vectorFont['3'] = { {{0,0},{3,0},{3,5},{0,5}}, {{0,2},{3,2}} }; // Две дуги
    vectorFont['4'] = { {{0,0},{0,2},{3,2}}, {{3,0},{3,5}} }; // L и |
    vectorFont['5'] = { {{3,0},{0,0},{0,2},{3,2},{3,5},{0,5}} }; // Обратная S
    vectorFont['6'] = { {{3,0},{0,0},{0,5},{3,5},{3,2},{0,2}} }; // G
    vectorFont['7'] = { {{0,0},{3,0},{3,5}} }; // 7
    vectorFont['8'] = { {{0,0},{3,0},{3,5},{0,5},{0,0}}, {{0,2},{3,2}} }; // B
    vectorFont['9'] = { {{0,5},{3,5},{3,0},{0,0},{0,2},{3,2}} }; // перевернутая G
    vectorFont[':'] = { {{1,1},{2,1},{2,2},{1,2},{1,1}}, {{1,3},{2,3},{2,4},{1,4},{1,3}} }; // двоеточие
}
// GameCanvas.cpp

void GameCanvas::drawVectorChar(char c, int start_x, int start_y, int scale) {
    // Проверяем, есть ли такой символ в нашем шрифте. Если нет - выходим.
    // Это ключевая проверка, которая предотвратит падение.
    if (vectorFont.find(c) == vectorFont.end()) {
        return; 
    }

    // Получаем "штрихи" для символа
    const auto& strokes = vectorFont.at(c);

    // Рисуем каждый штрих
    for (const auto& stroke : strokes) {
        // Проверяем, что в штрихе есть хотя бы 2 точки для рисования линии
        if (stroke.size() < 2) {
            continue;
        }

        for (size_t i = 0; i < stroke.size() - 1; ++i) {
            const auto& p1 = stroke[i];
            const auto& p2 = stroke[i + 1];

            int x1 = start_x + p1.x * scale;
            int y1 = start_y + p1.y * scale;
            int x2 = start_x + p2.x * scale;
            int y2 = start_y + p2.y * scale;

            graphics->drawLine(x1, y1, x2, y2);
        }
    }
}

void GameCanvas::drawVectorString(const std::string& text, int start_x, int start_y, int scale) {
    int current_x = start_x;
    // Ширина символа (3 юнита) + отступ (1 юнит) * масштаб
    int char_width = (3 + 1) * scale; 

    // Устанавливаем цвет один раз для всей строки
    setColor(255, 255, 255); // Белый цвет для таймера

    for (const char c : text) {
        drawVectorChar(c, current_x, start_y, scale);
        current_x += char_width; // Сдвигаем позицию для следующего символа
    }
}
void GameCanvas::requestRepaint(int var1) {
    field_184 = var1;
    repaint();
    serviceRepaints();
}

void GameCanvas::updateSizeAndRepaint() {
    std::cout << "[LOG] updateSizeAndRepaint() — before getWidth/getHeight" << std::endl;
    width = getWidth();
    height = getHeight();
    std::cout << "[LOG] updateSizeAndRepaint() — new size: " 
              << width << "×" << height << std::endl;
    repaint();
}


void GameCanvas::method_129() {
    method_164();
}
static constexpr float CAM_ZOOM = 2.0f;
void GameCanvas::setViewPosition(int dx, int dy) {
    this->dx = dx;
    this->dy = dy;
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

void GameCanvas::method_142(int var1, int var2, int var3, int var4) {
    ++var3;
    int var5 = addDx(var1 - var3);
    int var6 = addDy(var2 + var3);
    int var7 = var3 << 1;
    if ((var4 = -((int)(((int64_t)((int)((int64_t)var4 * 11796480L >> 16)) << 32) / 205887L >> 16))) < 0) {
        var4 += 360;
    }
    graphics->drawArc(var5, var6, var7, var7, (var4 >> 16) + 170, 90);
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

void GameCanvas::paint(Graphics* g) {
    processTimers();
    drawGame(g);
    std::cout << timeToDisplay << std::endl;
    // drawVectorString(timeToDisplay, 20, 20, 4); 
}
void GameCanvas::drawGame(Graphics* g) {
    std::cout << "[LOG] GameCanvas::drawGame() — begin" << std::endl;
    if (!Micro::field_249 || micro->field_242) {
        std::cout << "[LOG] drawGame — skipping (field_249=" 
                  << Micro::field_249 << ", field_242=" << micro->field_242 << ")" << std::endl;
        return;
    }
    graphics = g;

    // Проверяем размеры
    std::cout << "[LOG] drawGame — width=" << width << ", height=" << height
              << ", getWidth()=" << getWidth() << ", getHeight()=" << getHeight() << std::endl;

    if (height != getHeight()) {
        std::cout << "[LOG] drawGame — size changed, calling updateSizeAndRepaint()" << std::endl;
        updateSizeAndRepaint();
    }

    std::cout << "[LOG] drawGame — calling gamePhysics->setMotoComponents()" << std::endl;
    gamePhysics->setMotoComponents();

    std::cout << "[LOG] drawGame — rendering physics" << std::endl;
    int logicalW = int(width  / CAM_ZOOM);
    int logicalH = int(height / CAM_ZOOM);

    setViewPosition(
        -gamePhysics->getCamPosX() + field_178 + logicalW/2,
        gamePhysics->getCamPosY() + field_179 + logicalH/2
    );
    gamePhysics->renderGame(this);

    graphics = nullptr;
    std::cout << "[LOG] GameCanvas::drawGame() — end" << std::endl;
}


void GameCanvas::method_164() {
    for (int i = 0; i < 10; ++i) activeKeys[i] = false;
    for (int i = 0; i < 7; ++i) activeActions[i] = false;
}

void GameCanvas::handleUpdatedInput() {
    int var1 = 0, var2 = 0;
    for (int i = 0; i < 10; ++i) {
        if (activeKeys[i]) {
            var1 += field_231[field_232][i][0];
            var2 += field_231[field_232][i][1];
        }
    }
    for (int i = 0; i < 7; ++i) {
        if (activeActions[i]) {
            var1 += field_230[i][0];
            var2 += field_230[i][1];
        }
    }
    gamePhysics->method_30(var1, var2);
}

void GameCanvas::processTimers() {
    for (auto i = timers.begin(); i != timers.end();) {
        if (i->ready()) {
            timerTriggered = true; // Вместо показа текста просто взводим флаг
            i = timers.erase(i);
        } else {
            i++;
        }
    }
}

void GameCanvas::processKeyPressed(int keyCode) {
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

void GameCanvas::init(GamePhysics* gp) {
    this->gamePhysics = gp;
    gamePhysics->setMinimalScreenWH(width < height ? width : height);
}

void GameCanvas::scheduleGameTimerTask(const std::string& message, int delayMs) {
    (void)message; // Сообщение игнорируется
    timerTriggered = false;
    ++timerId;
    timers.push_back(Timer(timerId, delayMs));
}

void GameCanvas::keyPressed(int var1) {
    processKeyPressed(var1);
}

void GameCanvas::keyReleased(int var1) {
    processKeyReleased(var1);
}

void GameCanvas::drawWheel(int x, int y, int radius) {
    // Рисуем внешнюю часть колеса (покрышку)
    setColor(0, 0, 0); // Черный цвет
    drawCircle(x, y, radius * 2);

    // Рисуем внутреннюю часть (диск)
    setColor(150, 150, 150); // Серый цвет
    drawCircle(x, y, radius);
}
// GameCanvas.cpp (в конец файла)

// GameCanvas.cpp

void GameCanvas::drawSkyGradient() {
    const int num_bands = 16; // Количество "полос" в нашем градиенте. Больше = плавнее, но медленнее. 32 - хороший баланс.
    const float band_height = (float)height / num_bands;

    for (int i = 0; i < num_bands; ++i) {
        // Вычисляем позицию текущей полосы
        int y_pos = (int)(i * band_height);

        // Рассчитываем цвет для текущей полосы
        float ratio = (float)i / (float)num_bands;
        Uint8 r = (Uint8)(50 + ratio * 85);
        Uint8 g = (Uint8)(100 + ratio * 106);
        Uint8 b = (Uint8)(150 + ratio * 100);

        graphics->setColor(r, g, b);
        // Рисуем одну широкую полосу вместо множества тонких
        graphics->fillRect(0, y_pos, width, (int)ceil(band_height));
    }
}
// GameCanvas.cpp (в конец файла)

GameCanvas::~GameCanvas() {
    // Очищаем память от запеченной текстуры при закрытии игры
    if (bakedBackground) {
        SDL_DestroyTexture(bakedBackground);
    }
}

void GameCanvas::bakeBackground() {

}

void GameCanvas::drawBakedBackground() {

}