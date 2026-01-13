#include "CanvasImpl.h"
#include "Canvas.h"
#include "Keys.h" 
#include "Micro.h"
#include "Logger.h"

CanvasImpl::CanvasImpl(Canvas* canvas) {
    this->canvas = canvas;
    
    // Инициализация видео подсистемы SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("Не удалось инициализировать SDL! Ошибка: " + std::string(SDL_GetError()));
    }
    
    // Создание окна
    window = SDL_CreateWindow("Motorcycle Simulator", 
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                              width, height, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        throw std::runtime_error("Не удалось создать окно! Ошибка: " + std::string(SDL_GetError()));
    }
    
    // Создание рендерера (GPU ускорение)
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    // Масштабирование (2.0x для пиксель-арт вида)
    SDL_RenderSetScale(renderer, 2.0f, 2.0f);
    
    if (renderer == nullptr) {
        throw std::runtime_error("Не удалось создать рендерер! Ошибка: " + std::string(SDL_GetError()));
    }
}

CanvasImpl::~CanvasImpl() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int CanvasImpl::getWidth() { return width; }
int CanvasImpl::getHeight() { return height; }

void CanvasImpl::setWindowTitle(const std::string& title) {
    SDL_SetWindowTitle(window, title.c_str());
}

SDL_Renderer* CanvasImpl::getRenderer() { return renderer; }

void CanvasImpl::repaint() {
    SDL_RenderPresent(renderer);
}

int CanvasImpl::convertKeyCharToKeyCode(SDL_Keycode keyCode) {
    switch (keyCode) {
        case SDLK_UP: return Canvas::Keys::UP;
        case SDLK_DOWN: return Canvas::Keys::DOWN;
        case SDLK_LEFT: return Canvas::Keys::LEFT;
        case SDLK_RIGHT: return Canvas::Keys::RIGHT;
        case SDLK_RETURN: 
        case SDLK_SPACE:  
            return Canvas::Keys::FIRE;
        
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
        
        // Спец. клавиши для управления AI (User Request Task preparation)
        case SDLK_m: return 'M'; // Manual
        case SDLK_a: return 'A'; // AI
        
        default: return 0; 
    }
}

void CanvasImpl::processEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            LOG_INFO("SYS", "Получен сигнал SDL_QUIT. Завершение...");
            Micro::isReady = false;
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
