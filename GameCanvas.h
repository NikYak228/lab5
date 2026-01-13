#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>

#include "Graphics.h"
#include "Canvas.h"
#include "Micro.h"

class GamePhysics;
struct Point { int x; int y; };

/**
 * GameCanvas
 * 
 * Основной класс отрисовки игры.
 * Наследуется от Canvas и реализует игровую логику ввода и отрисовки (HUD, мир).
 */
class GameCanvas : public Canvas {
private:
    // --- Ресурсы ---
    std::map<char, std::vector<std::vector<Point>>> vectorFont; // Векторный шрифт
    Graphics* graphics = nullptr;
    
    // --- Связи ---
    GamePhysics* gamePhysics = nullptr;
    Micro* micro = nullptr;
    
    // --- Камера ---
    int dx; // Смещение Viewport X
    int dy; // Смещение Viewport Y
    int cameraOffsetX = 0;
    int cameraOffsetY = 0;
    
    // --- Состояние ввода ---
    // Таблицы направлений для клавиш и действий
    int actionDirTable[7][2] = { { 0, 0 }, { 1, 0 }, { 0, -1 }, { 0, 0 }, { 0, 0 }, { 0, 1 }, { -1, 0 } };
    int numKeyDirTable[3][10][2] = { 
        { { 0, 0 }, { 1, -1 }, { 1, 0 }, { 1, 1 }, { 0, -1 }, { -1, 0 }, { 0, 1 }, { -1, -1 }, { -1, 0 }, { -1, 1 } }, 
        { { 0, 0 }, { 1, 0 }, { 0, 0 }, { 0, 0 }, { -1, 0 }, { 0, -1 }, { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 0 } }, 
        { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 }, { -1, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } } 
    };
    int numKeyMode = 2; // Режим цифровой клавиатуры
    std::vector<bool> activeActions = std::vector<bool>(7);
    std::vector<bool> activeKeys = std::vector<bool>(10);

    // --- Приватные методы ---
    void clearInputStates();
    void handleUpdatedInput();

public:
    GameCanvas(Micro* micro);
    ~GameCanvas();

    // --- Инициализация ---
    void init(GamePhysics* gamePhysics);
    
    // --- Отрисовка ---
    void paint(Graphics* g) override;
    void drawGame(Graphics* g);
    void updateSizeAndRepaint();
    void requestRepaint(int mode);
    
    // Примитивы отрисовки (высокоуровневые)
    void drawSkyGradient();
    void drawWheel(int x, int y, int radius);
    void drawBrakeDisc(int x, int y, int radius, int angle);
    void drawLine(int x, int y, int x2, int y2);
    void drawLineF16(int x, int y, int x2, int y2);
    void drawCircle(int x, int y, int size);
    void setColor(int red, int green, int blue);
    void clearScreenWithWhite(); // Не используется?
    
    // Векторный текст
    void drawVectorChar(char c, int start_x, int start_y, int scale);
    void drawVectorString(const std::string& text, int start_x, int start_y, int scale);

    // --- Камера ---
    void setViewPosition(int dx, int dy);
    int addDx(int x);
    int addDy(int y);

    // --- Ввод ---
    void processKeyPressed(int keyCode);
    void processKeyReleased(int keyCode);
    void keyPressed(int var1) override;
    void keyReleased(int var1) override;
    void resetInput();
    
    // --- Состояние ---
    int width;
    int height;
    std::string timeToDisplay; // Для HUD
};