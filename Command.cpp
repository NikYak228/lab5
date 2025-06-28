#include "Command.h"
// реализация конструктора и деструктора
Command::Command(std::string commandName, Type commandType, int commandPriority)
    : name(commandName)
    , type(commandType)
    , priority(commandPriority)
{}

Command::~Command() = default;