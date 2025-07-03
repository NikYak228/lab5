#pragma once

#include <string>
#include <memory>
#include <vector>

#include "Graphics.h"
#include "Canvas.h"
#include <map>
#include "Micro.h"

class GamePhysics;
struct Point { int x; int y; };
class GameCanvas : public Canvas {
private:
    void clearInputStates();
    void handleUpdatedInput();
    std::map<char, std::vector<std::vector<Point>>> vectorFont;
    Graphics* graphics = nullptr;
    int dx;
    int dy;
    GamePhysics* gamePhysics = nullptr;
    int field_178 = 0;
    int field_179 = 0;
    Micro* micro = nullptr;
    
    int field_184 = 1;
    
    int field_230[7][2] = { { 0, 0 }, { 1, 0 }, { 0, -1 }, { 0, 0 }, { 0, 0 }, { 0, 1 }, { -1, 0 } };
    int field_231[3][10][2] = { { { 0, 0 }, { 1, -1 }, { 1, 0 }, { 1, 1 }, { 0, -1 }, { -1, 0 }, { 0, 1 }, { -1, -1 }, { -1, 0 }, { -1, 1 } }, { { 0, 0 }, { 1, 0 }, { 0, 0 }, { 0, 0 }, { -1, 0 }, { 0, -1 }, { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 0 } }, { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 }, { -1, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } } };
    int field_232 = 2;
    std::vector<bool> activeActions = std::vector<bool>(7);
    std::vector<bool> activeKeys = std::vector<bool>(10);
public:
    ~GameCanvas();
    void drawCloud(int x, int y, int size);
    void drawSky();   
    void drawSkyGradient();
    GameCanvas(Micro* micro);
    std::string timeToDisplay;
    void drawVectorChar(char c, int start_x, int start_y, int scale);
    void drawVectorString(const std::string& text, int start_x, int start_y, int scale);
    void requestRepaint(int var1);
    void drawWheel(int x, int y, int radius);
    void updateSizeAndRepaint();
    void resetInput();
    void setViewPosition(int dx, int dy);
    int addDx(int x);
    int addDy(int y);
    void drawLine(int x, int y, int x2, int y2);
    void drawLineF16(int x, int y, int x2, int y2);
    void drawBrakeDisc(int x, int y, int radius, int angle); // Рисует дугу, используется для тормозных дисков
    void drawCircle(int x, int y, int size);
    void clearScreenWithWhite();
    void setColor(int red, int green, int blue);
    void drawGame(Graphics* g);
    void paint(Graphics* g) override;
    void init(GamePhysics* gamePhysics);
    void processKeyPressed(int keyCode);
    void processKeyReleased(int keyCode);
    void keyPressed(int var1) override;
    void keyReleased(int var1) override;
    int width;
    int height;

};