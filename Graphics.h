#pragma once
#include <memory>
#include <stdexcept>
#include <cmath>
#include <string>
#include <SDL2/SDL.h>

constexpr auto PI_CONV = 3.1415926 / 180.0;

class Graphics {
private:
    SDL_Renderer* renderer;
    SDL_Color currentColor;
    void _putpixel(int x, int y);

public:
    Graphics(SDL_Renderer* renderer);
    void setColor(int r, int g, int b);
    void setClip(int x, int y, int w, int h);
    void fillRect(int x, int y, int w, int h);
    void drawArc(int x, int y, int w, int h, int startAngle, int arcAngle);
    void drawLine(int x1, int y1, int x2, int y2);
    void fillCircle(int x, int y, int radius);
};