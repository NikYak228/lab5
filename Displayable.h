#pragma once

class CommandListener;
class Command;
// интерфейс для объектов, которые могут быть отображены и взаимодействовать с командами
class Displayable {
public:
    virtual ~Displayable() = default;
    virtual bool isShown() = 0;
    virtual int getWidth() = 0;
    virtual int getHeight() = 0;
    virtual void setCommandListener(CommandListener* listener) = 0;
    virtual void addCommand(Command* command) = 0;
    virtual void removeCommand(Command* command) = 0;
};
