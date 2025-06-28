#pragma once

#include <string>
#include <functional>
// хеш-функция для комбинирования хешей различных типов
template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
// класс, представляющий команду в пользовательском интерфейсе
class Command {
public:
    enum Type {
        SCREEN = 1,
        BACK = 2,
        CANCEL = 3,
        OK = 4,
        HELP = 5,
        STOP = 6,
        EXIT = 7,
        ITEM = 8
    };
    // сравнение команд по значению
    struct EqualFunction {
        bool operator()(Command* lhs, Command* rhs) const
        {
            return lhs->name == rhs->name && lhs->type == rhs->type && lhs->priority == rhs->priority;
        }
    };
    // хеш-функция
    struct HashFunction {
        size_t operator()(Command* command) const
        {
            std::size_t seed = 0;
            hash_combine(seed, command->name);
            hash_combine(seed, command->type);
            hash_combine(seed, command->priority);
            return seed;
        }
    };
    // конструктор, деструктор
    Command(std::string commandName, Type commandType, int commandPriority);
    ~Command();
    // открытые константные члены для доступа к данным команды
    const std::string name;
    const int type;
    const int priority;
};