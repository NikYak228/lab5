#pragma once

#include <memory>
#include <vector>
#include <unordered_set>

#include "Displayable.h"
#include "Command.h"

class CanvasImpl;
class Graphics;

class Canvas : public Displayable {
private:
    std::unique_ptr<CanvasImpl> impl;
    std::unique_ptr<Graphics> graphics;
    CommandListener* commandListener = nullptr;

    std::unordered_set<Command*, Command::HashFunction, Command::EqualFunction> currentCommands;

public:
    enum Keys {
        UP = 1,
        DOWN = 6,
        LEFT = 2,
        RIGHT = 5,
        FIRE = 8
    };

    Canvas();
    ~Canvas() override;
    bool isShown() override;
    int getWidth() override;
    int getHeight() override;
    void setWindowTitle(const std::string& title);
    CanvasImpl* getCanvasImpl();
    void repaint();
    void handleEventsAndPresent();
    void serviceRepaints();
    int getGameAction(int keyCode);
    void removeCommand(Command* command) override;
    void addCommand(Command* command) override;
    void setCommandListener(CommandListener* listener) override;
    void publicKeyPressed(int keyCode);
    void publicKeyReleased(int keyCode);
    void pressedEsc();
    virtual void paint(Graphics* g) = 0;
    virtual void keyPressed(int keyCode) = 0;
    virtual void keyReleased(int keyCode) = 0;
};
