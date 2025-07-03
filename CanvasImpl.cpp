#include "CanvasImpl.h"
#include "Canvas.h"
#include "Keys.h" // Создадим этот файл ниже
#include "Micro.h"

CanvasImpl::CanvasImpl(Canvas* canvas) {
    this->canvas = canvas;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("SDL could not initialize! SDL_Error: " + std::string(SDL_GetError()));
    }
    window = SDL_CreateWindow("Motorcycle Simulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        throw std::runtime_error("Window could not be created! SDL_Error: " + std::string(SDL_GetError()));
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetScale(renderer, 2.0f, 2.0f);
    if (renderer == nullptr) {
        throw std::runtime_error("Renderer could not be created! SDL Error: " + std::string(SDL_GetError()));
    }
}

CanvasImpl::~CanvasImpl() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int CanvasImpl::getWidth() {
    return width;
}

int CanvasImpl::getHeight() {
    return height;
}

void CanvasImpl::setWindowTitle(const std::string& title) {
    SDL_SetWindowTitle(window, title.c_str());
}

SDL_Renderer* CanvasImpl::getRenderer() {
    return renderer;
}

void CanvasImpl::repaint() {
    SDL_RenderPresent(renderer);
}


// Конвертирует коды клавиш SDL в коды, которые использует игра
int CanvasImpl::convertKeyCharToKeyCode(SDL_Keycode keyCode) {
    switch (keyCode) {
        case SDLK_UP: return Keys::UP;
        case SDLK_DOWN: return Keys::DOWN;
        case SDLK_LEFT: return Keys::LEFT;
        case SDLK_RIGHT: return Keys::RIGHT;
        case SDLK_RETURN: // Enter
        case SDLK_SPACE:  // Space
            return Keys::FIRE;
        // Цифровые клавиши
        case SDLK_0: return '0';
        case SDLK_1: return '1';
        case SDLK_2: return '2';
        case SDLK_3: return '3';
        case SDLK_4: return '4';
        case SDLK_5: return '5';
        case SDLK_6: return '6';
        case SDLK_7: return '7';
        case SDLK_8: return '8';
        case SDLK_9: return '9';
        default: return 0; // Неизвестная клавиша
    }
}

void CanvasImpl::processEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            // Чтобы можно было закрыть окно крестиком
            Micro::field_249 = false;
        } else if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                 canvas->pressedEsc();
            } else {
                int gameKeyCode = convertKeyCharToKeyCode(e.key.keysym.sym);
                if (gameKeyCode != 0) {
                    canvas->publicKeyPressed(gameKeyCode);
                }
            }
        } else if (e.type == SDL_KEYUP) {
            int gameKeyCode = convertKeyCharToKeyCode(e.key.keysym.sym);
            if (gameKeyCode != 0) {
                canvas->publicKeyReleased(gameKeyCode);
            }
        }
    }
}