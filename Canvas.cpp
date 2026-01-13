#include "Canvas.h"

#include <stdexcept>
#include <algorithm>

#include "CanvasImpl.h"
#include "Graphics.h"
#include "Command.h"
#include "CommandListener.h"

Canvas::Canvas()
    : impl(std::make_unique<CanvasImpl>(this)),
      graphics(std::make_unique<Graphics>(impl->getRenderer())),
      commandListener(nullptr) {}

Canvas::~Canvas() = default;

int Canvas::getWidth() {
    return impl->getWidth();
}

int Canvas::getHeight() {
    return impl->getHeight();
}

void Canvas::setWindowTitle(const std::string& title) {
    impl->setWindowTitle(title);
}

bool Canvas::isShown() {
    return true;
}

CanvasImpl* Canvas::getCanvasImpl() {
    return impl.get();
}

void Canvas::repaint() {
    // Подготовка кадра: вызываем пользовательский метод paint
    paint(graphics.get());
}

void Canvas::handleEventsAndPresent() {
    // 1. Обработка событий SDL (клавиатура, мышь, окно)
    impl->processEvents();
    // 2. Отображение кадра (SDL_RenderPresent)
    impl->repaint(); 
}

void Canvas::serviceRepaints() {
    // Заглушка
}

int Canvas::getGameAction(int keyCode) {
    switch (keyCode) {
    case Keys::UP:
    case Keys::DOWN:
    case Keys::LEFT:
    case Keys::RIGHT:
    case Keys::FIRE:
        return keyCode;
    default:
        throw std::runtime_error("getGameAction(" + std::to_string(keyCode) + ") не реализован!");
    }
}

void Canvas::removeCommand(Command* command) {
    currentCommands.erase(command);
}

void Canvas::addCommand(Command* command) {
    currentCommands.insert(command);
}

void Canvas::setCommandListener(CommandListener* listener) {
    commandListener = listener;
}

void Canvas::publicKeyPressed(int keyCode) {
    keyPressed(keyCode);
}

void Canvas::publicKeyReleased(int keyCode) {
    keyReleased(keyCode);
}

void Canvas::pressedEsc() {
    // Обработка кнопки "Назад" (ESC) для меню и команд
    for (const auto& command : currentCommands) {
        if (command->type == Command::Type::BACK || currentCommands.size() == 1) {
            commandListener->commandAction(command, this);
            return;
        }
    }
}