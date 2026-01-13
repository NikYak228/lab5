#pragma once
#include <memory>
#include <stdexcept>
#include <cmath>
#include <string>
#include <SDL2/SDL.h>

constexpr auto PI_CONV = 3.1415926 / 180.0;

/**
 * Graphics
 * 
 * Обертка над SDL_Renderer для рисования примитивов (линии, круги, прямоугольники).
 * Предоставляет простой API, похожий на Java AWT Graphics или J2ME Graphics.
 */
class Graphics {
private:
    SDL_Renderer* renderer;
    SDL_Color currentColor;
    void _putpixel(int x, int y);

public:
    Graphics(SDL_Renderer* renderer);
    
    // Установка цвета (RGB)
    void setColor(int r, int g, int b);
    
    // Установка области клиппинга (отсечения)
    void setClip(int x, int y, int w, int h);
    
    // Рисование примитивов
    void fillRect(int x, int y, int w, int h);
    void drawArc(int x, int y, int w, int h, int startAngle, int arcAngle);
    void drawLine(int x1, int y1, int x2, int y2);
    void fillCircle(int x, int y, int radius);
};
