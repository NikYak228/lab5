#pragma once

#include <memory>
#include <vector>
#include <unordered_set>

#include "Displayable.h"
#include "Command.h"

class CanvasImpl;
class Graphics;

/**
 * Canvas
 * 
 * Абстрактный базовый класс для игрового экрана.
 * Предоставляет интерфейс для рисования и обработки событий.
 * Использует Pimpl идиому (CanvasImpl) для сокрытия деталей SDL2.
 */
class Canvas : public Displayable {
private:
    std::unique_ptr<CanvasImpl> impl;
    std::unique_ptr<Graphics> graphics;
    CommandListener* commandListener = nullptr;

    std::unordered_set<Command*, Command::HashFunction, Command::EqualFunction> currentCommands;

public:
    // Коды игровых клавиш
    enum Keys {
        UP = 1,
        DOWN = 6,
        LEFT = 2,
        RIGHT = 5,
        FIRE = 8
    };

    Canvas();
    ~Canvas() override;
    
    // --- Интерфейс Displayable ---
    bool isShown() override;
    int getWidth() override;
    int getHeight() override;
    void removeCommand(Command* command) override;
    void addCommand(Command* command) override;
    void setCommandListener(CommandListener* listener) override;

    // --- Управление окном ---
    void setWindowTitle(const std::string& title);
    CanvasImpl* getCanvasImpl(); // Доступ к реализации (для низкоуровневых операций)
    
    // --- Цикл отрисовки ---
    void repaint();                 // Запрос перерисовки
    void handleEventsAndPresent();  // Обработка очереди событий и swap buffers
    void serviceRepaints();         // Служебный метод (legacy)
    
    // --- Ввод ---
    int getGameAction(int keyCode); // Маппинг клавиш
    void publicKeyPressed(int keyCode);
    void publicKeyReleased(int keyCode);
    void pressedEsc();
    
    // --- Виртуальные методы (реализуются в GameCanvas) ---
    virtual void paint(Graphics* g) = 0;
    virtual void keyPressed(int keyCode) = 0;
    virtual void keyReleased(int keyCode) = 0;
};