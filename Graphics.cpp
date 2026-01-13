#include "Graphics.h"
#include <iostream>

Graphics::Graphics(SDL_Renderer* renderer) {
    this->renderer = renderer;
    this->currentColor = { 0, 0, 0, 255 };
}

void Graphics::setColor(int r, int g, int b) {
    currentColor.r = r;
    currentColor.g = g;
    currentColor.b = b;
    currentColor.a = 255;
    SDL_SetRenderDrawColor(renderer, (Uint8)r, (Uint8)g, (Uint8)b, 255);
}

void Graphics::setClip(int x, int y, int w, int h) {
    if (w <= 0 || h <= 0) {               
        SDL_RenderSetClipRect(renderer, nullptr); 
        return;
    }
    SDL_Rect clipRect{ x, y, w, h };
    SDL_RenderSetClipRect(renderer, &clipRect);
}

void Graphics::fillCircle(int x, int y, int radius) {
    // Простой алгоритм сканирования для залитого круга
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w; 
            int dy = radius - h; 
            if ((dx*dx + dy*dy) <= (radius * radius)) {
                _putpixel(x + dx, y + dy);
            }
        }
    }
}

void Graphics::fillRect(int x, int y, int w, int h) {
    SDL_Rect rect { x, y, w, h };
    SDL_RenderFillRect(renderer, &rect);
}

void Graphics::drawArc(int x, int y, int width, int height, int startAngle, int arcAngle) {
    int xradius = width / 2;
    int yradius = height / 2;
    x += xradius; // Сдвиг к центру
    y += yradius;
    
    if (xradius == 0 && yradius == 0) return;

    // Рисуем дугу сегментами линий
    for (int angle = startAngle; angle < startAngle + arcAngle; angle++) {
        drawLine(
            x + int(xradius * cos(angle * PI_CONV)),
            y - int(yradius * sin(angle * PI_CONV)),
            x + int(xradius * cos((angle + 1) * PI_CONV)),
            y - int(yradius * sin((angle + 1) * PI_CONV))
        );
    }
}

void Graphics::_putpixel(int x, int y) {
    SDL_RenderDrawPoint(renderer, x, y);
}

void Graphics::drawLine(int x1, int y1, int x2, int y2) {
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}
