#pragma once

#include <fstream>
#include <algorithm>
#include <filesystem>
#include <vector>
// класс для работы с файловыми потоками, наследуется от std::fstream
class FileStream : public std::fstream {
public:
    FileStream()
        : std::fstream()
    {
    }

    FileStream(const std::filesystem::path& file, std::ios::openmode mode)
        : std::fstream(file, mode)
    {
    }

    ~FileStream()
    {
        std::fstream::close();
    }
    // метод для чтения переменной с опциональным изменением порядка байтов
    template <class T>
    void readVariable(T* variable, bool swapEndianness = false, std::size_t size = 0)
    {
        char* bytePtr = reinterpret_cast<char*>(variable);
        if (!size) {
            size = sizeof(T);
        }
        read_impl(bytePtr, size);
        if (swapEndianness) {
            std::reverse(bytePtr, bytePtr + size);
        }
    }
    // метод для записи переменной
    template <class T>
    void writeVariable(T* variable, std::size_t size = 0)
    {
        char* bytePtr = reinterpret_cast<char*>(variable);
        if (!size) {
            size = sizeof(T);
        }
        write_impl(bytePtr, size);
    }
    // проверяет, открыт ли файловый поток
    virtual bool isOpen()
    {
        return std::fstream::is_open();
    }
    // устанавливает позицию чтения/записи в потоке
    virtual void setPos(std::streampos pos)
    {
        std::fstream::seekg(pos);
    }

private:
    // функция для фактического чтения байтов
    virtual void read_impl(char* s, std::streamsize n)
    {
        std::fstream::read(s, n);
    }
    // функция для фактической записи байтов
    virtual void write_impl(char* s, std::streamsize n)
    {
        std::fstream::write(s, n);
    }
};
