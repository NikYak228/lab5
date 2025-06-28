#pragma once
#include <cstdint>
#include <cmath>
// класс для математических операций с числами в формате F16 (Fixed-point 16.16)
class MathF16 {
public:
    // константы для представления PI в формате F16
    static const int PiHalfF16 = 102944;
    static const int PiF16 = 205887;
    // синус косинус арктангенс
    static int sinF16(int angle);
    static int cosF16(int angle);
    static int atan2F16(int dx, int dy);
private:
    static int multiplyF16(int a, int b);
    static int divideF16(int a, int b);
    static int atanF16(int angle);
};