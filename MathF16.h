#pragma once
#include <cstdint>
#include <cmath>

/**
 * MathF16
 * 
 * Библиотека для математических операций с фиксированной точкой (Fixed-point 16.16).
 * F16 формат: старшие 16 бит - целая часть, младшие 16 бит - дробная.
 * 1.0 представлено как 65536.
 */
class MathF16 {
public:
    // Константы PI в формате F16
    static const int PiHalfF16 = 102944; // PI/2
    static const int PiF16 = 205887;     // PI

    // Тригонометрия
    static int sinF16(int angle);
    static int cosF16(int angle);
    
    // Арктангенс двух аргументов. 
    // Внимание: порядок аргументов (dy, dx) для соответствия atan2(y, x).
    // Но в коде игры часто вызывается как (x, y) для получения угла от оси Y.
    static int atan2F16(int dy, int dx);

private:
    // Внутренние операции (не используются в текущей реализации, так как есть float-эмуляция)
    static int multiplyF16(int a, int b);
    static int divideF16(int a, int b);
    static int atanF16(int angle);
};
