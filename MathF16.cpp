#include "MathF16.h"
#include <cmath> 

const double CUSTOM_PI = 3.14159265358979323846;

// Вспомогательная функция: Конвертация угла F16 в радианы
static double to_radians(int angleF16) {
    return (double)angleF16 / MathF16::PiF16 * CUSTOM_PI;
}

// Вспомогательная функция: Конвертация радиан в угол F16
static int to_f16_angle(double rad_angle) {
    return (int)(rad_angle / CUSTOM_PI * MathF16::PiF16);
}

// Синус (F16 -> F16)
int MathF16::sinF16(int angle) {
    // В текущей реализации используем std::sin (float) для простоты
    return (int)(sin(to_radians(angle)) * 65536.0);
}

// Косинус (F16 -> F16)
int MathF16::cosF16(int angle) {
    return (int)(cos(to_radians(angle)) * 65536.0);
}

// Арктангенс (Координаты -> Угол F16)
int MathF16::atan2F16(int dy, int dx) {
    double angle_rad = atan2((double)dy, (double)dx);
    return to_f16_angle(angle_rad);
}
