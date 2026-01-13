#pragma once

#include <memory>
#include <iostream>
#include <string>

#include <SDL2/SDL.h>

class Canvas;

/**
 * CanvasImpl
 * 
 * Реализация платформо-зависимой части Canvas (SDL2).
 * Отвечает за создание окна, рендерера и цикл обработки событий.
 */
class CanvasImpl {
private:
    Canvas* canvas;

    SDL_Window* window;
    SDL_Renderer* renderer;

    const int width = 640;
    const int height = 480;

    // Конвертация SDL Keycode -> Game Keycode
    static int convertKeyCharToKeyCode(SDL_Keycode keyCode);

public:
    CanvasImpl(Canvas* canvas);
    ~CanvasImpl();

    void repaint(); // Вызов SDL_RenderPresent
    int getWidth();
    int getHeight();

    SDL_Renderer* getRenderer();
    void processEvents(); // Главный цикл событий SDL
    void setWindowTitle(const std::string& title);
};
